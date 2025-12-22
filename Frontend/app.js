const API_URL = "http://192.168.137.116:8000/api/produccion";
const WS_URL  = "ws://192.168.137.116:8000/ws";

let socket = null;
let datosGlobales = [];

/******************************
 * CARGA DATOS
 ******************************/
async function cargarDatos() {
  const res = await fetch(API_URL);
  datosGlobales = await res.json();
  renderizarTarjetas();
}

/******************************
 * ESTADO INDUSTRIAL
 ******************************/
function obtenerEstado(maquina) {
  if (maquina.ultima_falla && maquina.ultima_falla !== "Sin falla") {
    return "estado-fault";
  }
  if (maquina.unidades_producidas === 0) {
    return "estado-stop";
  }
  return "estado-run";
}

/******************************
 * TARJETAS
 ******************************/
function renderizarTarjetas() {
  const contenedor = document.getElementById("contenedor-maquinas");
  contenedor.innerHTML = "";

  datosGlobales.forEach(maquina => {
    const card = document.createElement("div");
    card.className = `machine-card ${obtenerEstado(maquina)}`;

    card.onclick = () => {
      // 👉 Navegamos a la página detalle
      window.location.href =
        `maquina.html?id=${maquina.id}`;
    };

    card.innerHTML = `
      <h3>${maquina.nombre_maquina}</h3>
      <div>Unidades: <b>${maquina.unidades_producidas}</b></div>
      <div>Estado: ${maquina.ultima_falla || "RUN"}</div>
      <div class="small">${maquina.turno}</div>
    `;

    contenedor.appendChild(card);
  });
}

/******************************
 * WEBSOCKET
 ******************************/
function conectarWebSocket() {
  socket = new WebSocket(WS_URL);

  socket.onmessage = () => cargarDatos();
  socket.onopen = () => console.log("WebSocket conectado");
  socket.onclose = () => setTimeout(conectarWebSocket, 3000);
}

/******************************
 * INICIO
 ******************************/
document.addEventListener("DOMContentLoaded", () => {
  cargarDatos();
  conectarWebSocket();
});

