/*********************************
 * CONFIGURACIÓN
 *********************************/
const API_URL = "http://192.168.137.116:8000/api/produccion";
const WS_URL  = "ws://192.168.137.116:8000/ws";

let socket = null;
let datosGlobales = [];

/*********************************
 * AGRUPAR POR MÁQUINA
 * (1 tarjeta por nombre_maquina,
 *  usando el registro más reciente)
 *********************************/
function agruparPorMaquina(registros) {
  const mapa = {};

  registros.forEach(r => {
    const nombre = r.nombre_maquina;

    if (!mapa[nombre]) {
      mapa[nombre] = r;
    } else {
      const fechaActual = new Date(mapa[nombre].timestamp);
      const fechaNueva  = new Date(r.timestamp);

      if (fechaNueva > fechaActual) {
        mapa[nombre] = r;
      }
    }
  });

  return Object.values(mapa);
}

/*********************************
 * CARGA DATOS DESDE API
 *********************************/
async function cargarDatos() {
  try {
    const res = await fetch(API_URL);
    const json = await res.json();

    // Soporta API que devuelve [] o { data: [] }
    const registros = Array.isArray(json) ? json : json.data;

    // 🔥 1 tarjeta por máquina
    datosGlobales = agruparPorMaquina(registros);

    console.log("Máquinas únicas:", datosGlobales);
    renderizarTarjetas();

  } catch (e) {
    console.error("Error cargando datos:", e);
  }
}

/*********************************
 * ESTADO INDUSTRIAL
 *********************************/
function obtenerEstado(maquina) {
  const unidades = maquina.unidades_producidas ?? 0;

  if (maquina.ultima_falla && maquina.ultima_falla !== "Sin falla") {
    return "estado-fault";   // 🔴 Rojo
  }

  if (unidades === 0) {
    return "estado-stop";    // 🔵 Azul
  }

  return "estado-run";       // 🟢 Verde
}

/*********************************
 * RENDERIZAR TARJETAS
 *********************************/
function renderizarTarjetas() {
  const contenedor = document.getElementById("contenedor-maquinas");
  contenedor.innerHTML = "";

  if (!datosGlobales || datosGlobales.length === 0) {
    contenedor.innerHTML = "<p>No hay máquinas disponibles</p>";
    return;
  }

  datosGlobales.forEach(maquina => {
    const card = document.createElement("div");
    card.className = `machine-card ${obtenerEstado(maquina)}`;

    card.onclick = () => {
      window.location.href = `maquina.html?id=${maquina.id}`;
    };

    card.innerHTML = `
      <h3>${maquina.nombre_maquina}</h3>
      <div>Unidades: <b>${maquina.unidades_producidas}</b></div>
      <div>Última falla: ${maquina.ultima_falla || "Sin falla"}</div>
      <div class="small">Turno: ${maquina.turno || "-"}</div>
      <div class="small">${maquina.timestamp}</div>
    `;

    contenedor.appendChild(card);
  });
}

/*********************************
 * WEBSOCKET (TIEMPO REAL)
 *********************************/
function conectarWebSocket() {
  socket = new WebSocket(WS_URL);

  socket.onopen = () => {
    console.log("WebSocket conectado");
  };

  socket.onmessage = () => {
    cargarDatos();
  };

  socket.onerror = (err) => {
    console.error("Error WebSocket:", err);
  };

  socket.onclose = () => {
    console.warn("WebSocket cerrado, reconectando...");
    setTimeout(conectarWebSocket, 3000);
  };
}

/*********************************
 * INICIO
 *********************************/
document.addEventListener("DOMContentLoaded", () => {
  cargarDatos();
  conectarWebSocket();
});
