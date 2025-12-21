const API_URL = "http://192.168.137.116:8000/api/produccion";
const WS_URL  = "ws://192.168.137.116:8000/ws";

let socket = null;

/* =========================
   Cargar datos (HTTP)
   ========================= */
async function cargarDatos() {
    try {
        const res = await fetch(API_URL);
        const data = await res.json();

        const tbody = document.querySelector("#tabla tbody");
        tbody.innerHTML = "";

        data.forEach(row => {
            const tr = document.createElement("tr");
            tr.innerHTML = `
                <td>${row.id}</td>
                <td>${row.nombre_maquina}</td>
                <td>${row.unidades_producidas}</td>
                <td>${row.ultima_falla}</td>
                <td>${row.tiempo_producido}</td>
                <td>${row.turno}</td>
                <td>${row.timestamp}</td>
            `;
            tbody.appendChild(tr);
        });

    } catch (e) {
        console.error("Error cargando datos:", e);
    }
}

/* =========================
   WebSocket (Tiempo real)
   ========================= */
function conectarWebSocket() {
    socket = new WebSocket(WS_URL);

    socket.onopen = () => {
        console.log("WebSocket conectado");
    };

    socket.onmessage = (event) => {
        console.log("Evento WS:", event.data);

        // Cada vez que llega un evento, recargamos datos
        cargarDatos();
    };

    socket.onerror = (err) => {
        console.error("Error WebSocket", err);
    };

    socket.onclose = () => {
        console.warn("WebSocket cerrado, reconectando en 3s...");
        setTimeout(conectarWebSocket, 3000);
    };
}

/* =========================
   Inicio
   ========================= */
cargarDatos();          // Primera carga
conectarWebSocket();    // Tiempo real
