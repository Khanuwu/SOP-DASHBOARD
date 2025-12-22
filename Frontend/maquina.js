const API_URL = "http://192.168.137.116:8000/api/produccion";

const params = new URLSearchParams(window.location.search);
const maquinaId = params.get("id");

let grafico = null;

async function cargarMaquina() {
  const res = await fetch(API_URL);
  const data = await res.json();

  const maquina = data.find(m => m.id == maquinaId);
  if (!maquina) return;

  document.getElementById("titulo-maquina").textContent =
    maquina.nombre_maquina;

  document.getElementById("info-maquina").innerHTML = `
    <p>Unidades: <b>${maquina.unidades_producidas}</b></p>
    <p>Última falla: ${maquina.ultima_falla}</p>
    <p>Turno: ${maquina.turno}</p>
  `;

  crearGrafico(maquina);
}

function crearGrafico(maquina) {
  const ctx = document.getElementById("grafico-produccion");

  if (grafico) grafico.destroy();

  grafico = new Chart(ctx, {
    type: "bar",
    data: {
      labels: ["Producción"],
      datasets: [{
        data: [maquina.unidades_producidas],
        backgroundColor: "#3498db"
      }]
    },
    options: {
      plugins: { legend: { display: false } }
    }
  });
}

document.addEventListener("DOMContentLoaded", cargarMaquina);
