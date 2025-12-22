const API_URL = "http://192.168.137.116:8000/api/produccion";

const params = new URLSearchParams(window.location.search);
const maquinaId = params.get("id");

let grafico = null;

function getISOWeek(date) {
  const d = new Date(Date.UTC(date.getFullYear(), date.getMonth(), date.getDate()));
  const dayNum = d.getUTCDay() || 7;
  d.setUTCDate(d.getUTCDate() + 4 - dayNum);
  const yearStart = new Date(Date.UTC(d.getUTCFullYear(), 0, 1));
  const weekNum = Math.ceil((((d - yearStart) / 86400000) + 1) / 7);
  return `${d.getUTCFullYear()}-W${weekNum}`;
}

function agruparProduccionPorSemana(historial) {
  const semanas = {};

  historial.forEach(r => {
    const fecha = new Date(r.timestamp);
    const semana = getISOWeek(fecha);

    if (!semanas[semana]) {
      semanas[semana] = 0;
    }

    semanas[semana] += r.unidades_producidas;
  });

  return semanas;
}

async function cargarMaquina() {
  const res = await fetch(API_URL);
  const registros = await res.json();

  // Filtrar SOLO esta máquina
  const historial = registros.filter(r => r.id == maquinaId);

  if (historial.length === 0) return;

  // Registro más reciente
  const actual = historial.reduce((a, b) =>
    new Date(a.timestamp) > new Date(b.timestamp) ? a : b
  );

  // Título
  document.getElementById("titulo-maquina").textContent =
    actual.nombre_maquina;

  // KPIs
  document.getElementById("kpi-unidades").textContent =
    actual.unidades_producidas;

  document.getElementById("kpi-turno").textContent =
    actual.turno || "-";

  document.getElementById("kpi-falla").textContent =
    actual.ultima_falla || "Sin falla";

  // Gráfico
crearGraficoSemanal(historial);
mostrarOEE(historial);



  // Historial (últimos 10)
  const body = document.getElementById("historial-body");
  body.innerHTML = "";

  historial
    .sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp))
    .slice(0, 10)
    .forEach(r => {
      const tr = document.createElement("tr");
      tr.innerHTML = `
        <td>${r.timestamp}</td>
        <td>${r.unidades_producidas}</td>
        <td>${r.ultima_falla || "Sin falla"}</td>
        <td>${r.turno}</td>
      `;
      body.appendChild(tr);
    });
}

function crearGraficoSemanal(historial) {
  const ctx = document.getElementById("grafico-produccion");

  if (grafico) grafico.destroy();

  const datosSemana = agruparProduccionPorSemana(historial);

  const labels = Object.keys(datosSemana).sort();
  const valores = labels.map(l => datosSemana[l]);

  grafico = new Chart(ctx, {
    type: "bar", // también puede ser "line"
    data: {
      labels: labels,
      datasets: [{
        label: "Producción semanal",
        data: valores,
        backgroundColor: "#3498db",
        borderColor: "#2980b9"
      }]
    },
    options: {
      responsive: true,
      plugins: {
        legend: { display: true }
      },
      scales: {
        y: { beginAtZero: true }
      }
    }
  });

  function calcularOEE(historial) {

  // === PRODUCCIÓN SEMANAL ===
  const produccionSemanal = historial.reduce(
    (acc, r) => acc + r.unidades_producidas, 0
  );

  // === TIEMPO PRODUCIDO ===
  const tiempoProducido = historial.reduce(
    (acc, r) => acc + (r.tiempo_producido || 0), 0
  );

  // === TIEMPO TOTAL SEMANAL (7 días) ===
  const tiempoTotal = 7 * 24 * 60 * 60; // segundos

  const disponibilidad = tiempoProducido / tiempoTotal;

  // === RENDIMIENTO ===
  const maxProduccion = Math.max(
    ...historial.map(r => r.unidades_producidas)
  );

  const rendimiento = maxProduccion > 0
    ? produccionSemanal / (maxProduccion * historial.length)
    : 0;

  const calidad = 1.0;

  const oee = disponibilidad * rendimiento * calidad;

  return {
    disponibilidad,
    rendimiento,
    calidad,
    oee
  };
}

function mostrarOEE(historial) {
  const oee = calcularOEE(historial);

  document.getElementById("oee-disponibilidad").textContent =
    (oee.disponibilidad * 100).toFixed(1) + "%";

  document.getElementById("oee-rendimiento").textContent =
    (oee.rendimiento * 100).toFixed(1) + "%";

  document.getElementById("oee-calidad").textContent = "100%";

  document.getElementById("oee-total").textContent =
    (oee.oee * 100).toFixed(1) + "%";
}


}




document.addEventListener("DOMContentLoaded", cargarMaquina);
