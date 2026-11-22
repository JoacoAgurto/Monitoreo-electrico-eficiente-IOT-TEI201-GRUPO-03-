#ifndef PAGINA_WEB_H
#define PAGINA_WEB_H

#include <Arduino.h>

/*
  ==========================================================
                 PAGINA WEB ESP32 V7
  ==========================================================

  - Sin inicio de sesion.
  - Cuatro graficos independientes.
  - Configuracion WiFi.
  - Configuracion API.
  - Historial CSV.
  ==========================================================
*/

const char PAGINA_HTML[] PROGMEM = R"ESP32WEB(
<!DOCTYPE html>
<html lang="es">

<head>

<meta charset="UTF-8">

<meta
  name="viewport"
  content="width=device-width, initial-scale=1"
>

<title>Monitor de Corriente ESP32</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  background: #0f172a;
  color: #e2e8f0;
  font-family: Arial, sans-serif;
}

header {
  background: #020617;
  padding: 22px;
  text-align: center;
  border-bottom: 1px solid #334155;
}

header h1 {
  margin: 0 0 8px;
  font-size: 26px;
}

main {
  max-width: 1150px;
  margin: auto;
  padding: 18px;
}

.card {
  background: #1e293b;
  border: 1px solid #334155;
  border-radius: 12px;
  padding: 18px;
  margin-bottom: 18px;
}

h2 {
  margin-top: 0;
  color: #93c5fd;
}

h3 {
  margin-top: 0;
}

label {
  display: block;
  margin-top: 12px;
  margin-bottom: 6px;
}

input {
  width: 100%;
  padding: 11px;
  border: 1px solid #475569;
  border-radius: 7px;
  background: #0f172a;
  color: white;
}

button,
.btn {
  background: #2563eb;
  color: white;
  border: 0;
  border-radius: 7px;
  padding: 11px 15px;
  margin-top: 14px;
  cursor: pointer;
  display: inline-block;
  text-decoration: none;
}

button:hover,
.btn:hover {
  background: #1d4ed8;
}

.danger {
  background: #b91c1c;
}

.danger:hover {
  background: #991b1b;
}

.grid {
  display: grid;
  grid-template-columns:
    repeat(auto-fit, minmax(240px, 1fr));
  gap: 15px;
}

.value {
  font-size: 32px;
  font-weight: bold;
  color: #4ade80;
}

.small {
  color: #94a3b8;
  font-size: 13px;
}

canvas {
  width: 100%;
  height: 160px;
  background: #0f172a;
  border-radius: 7px;
}

.status {
  white-space: pre-line;
  line-height: 1.8;
}

.check {
  display: flex;
  align-items: center;
  gap: 8px;
}

.check input {
  width: auto;
}

#mensaje {
  position: fixed;
  bottom: 16px;
  right: 16px;
  max-width: 350px;
  background: #334155;
  color: white;
  padding: 14px;
  border-radius: 8px;
  display: none;
  z-index: 100;
}

</style>

</head>

<body>

<header>

<h1>Monitor de Corriente ESP32</h1>

<div id="fecha">
Esperando fecha y hora...
</div>

</header>

<main>

<section class="card">

<h2>Estado del sistema</h2>

<div id="estado" class="status">
Cargando...
</div>

</section>

<section class="card">

<h2>Mediciones en tiempo real</h2>

<div class="grid" id="mediciones"></div>

</section>

<section class="card">

<h2>Configuración Wi-Fi</h2>

<form id="formWifi">

<label>
Nombre de la red (SSID)
</label>

<input
  id="wifiSSID"
  name="ssid"
  placeholder="Nombre de tu router"
>

<label>
Contraseña Wi-Fi
</label>

<input
  id="wifiPassword"
  name="password"
  type="password"
  placeholder="Dejar vacío para conservar"
>

<button type="submit">
Guardar Wi-Fi
</button>

</form>

<p class="small">
La conexión al router permite sincronizar NTP
y enviar mediciones a la API.
</p>

</section>

<section class="card">

<h2>Servidor remoto / API</h2>

<form id="formAPI">

<label>
URL completa de la API HTTPS
</label>

<input
  id="apiURL"
  name="url"
  type="url"
  placeholder="https://servidor.com/api/mediciones"
>

<label>
Identificador del dispositivo
</label>

<input
  id="deviceID"
  name="device_id"
  placeholder="esp32-01"
>

<label>
Token de autenticación
</label>

<input
  id="apiToken"
  name="token"
  type="password"
  placeholder="Dejar vacío para conservar"
>

<label>
Intervalo de envío (segundos)
</label>

<input
  id="uploadInterval"
  name="interval"
  type="number"
  min="5"
  max="3600"
  value="10"
>

<button type="submit">
Guardar configuración API
</button>

</form>

<p class="small">
El token guardado no se muestra.
No utilices claves administrativas de Supabase.
</p>

</section>

<section class="card">

<h2>Historial microSD</h2>

<a class="btn" href="/download">
Descargar CSV
</a>

<button
  class="danger"
  onclick="reiniciarCSV()"
>
Reiniciar historial CSV
</button>

<p class="small">
Reiniciar el historial no elimina la cola
de mediciones pendientes para la API.
</p>

</section>

</main>

<div id="mensaje"></div>

<script>

// ==========================================================
// CONFIGURACION DE GRAFICOS
// ==========================================================

const MAX_PUNTOS = 90;

const historiales = [
  [], [], [], []
];

const colores = [
  "#38bdf8",
  "#4ade80",
  "#fbbf24",
  "#f472b6"
];

const visibles = [
  true, true, true, true
];

// ==========================================================
// MENSAJES
// ==========================================================

function mostrarMensaje(texto) {

  const elemento =
    document.getElementById("mensaje");

  elemento.textContent = texto;
  elemento.style.display = "block";

  setTimeout(() => {
    elemento.style.display = "none";
  }, 4000);
}

// ==========================================================
// CREAR TARJETAS
// ==========================================================

function construirMediciones() {

  const contenedor =
    document.getElementById("mediciones");

  contenedor.innerHTML = "";

  for (let i = 0; i < 4; i++) {

    const tarjeta =
      document.createElement("div");

    tarjeta.className = "card";

    tarjeta.innerHTML = `
      <h3>Conector ${i + 1}</h3>

      <div
        class="value"
        id="valor${i}"
      >
        -- A
      </div>

      <label class="check">
        <input
          type="checkbox"
          id="visible${i}"
          checked
        >
        Mostrar gráfico
      </label>

      <canvas
        id="grafico${i}"
        width="400"
        height="160"
      ></canvas>
    `;

    contenedor.appendChild(tarjeta);

    const guardado =
      localStorage.getItem("visible" + i);

    if (guardado === "false") {
      visibles[i] = false;
    }

    const check =
      document.getElementById("visible" + i);

    check.checked = visibles[i];

    check.addEventListener("change", () => {

      visibles[i] = check.checked;

      localStorage.setItem(
        "visible" + i,
        String(visibles[i])
      );

      dibujarGrafico(i);
    });
  }
}

// ==========================================================
// DIBUJAR GRAFICOS
// ==========================================================

function dibujarGrafico(i) {

  const canvas =
    document.getElementById("grafico" + i);

  if (!canvas) {
    return;
  }

  canvas.style.display =
    visibles[i] ? "block" : "none";

  if (!visibles[i]) {
    return;
  }

  const ctx = canvas.getContext("2d");

  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);

  const datos = historiales[i];

  if (datos.length < 2) {
    return;
  }

  const validos =
    datos.filter(v => Number.isFinite(v));

  if (validos.length === 0) {
    return;
  }

  let maximo = Math.max(...validos, 1);

  maximo *= 1.15;

  // Cuadricula

  ctx.strokeStyle = "#334155";
  ctx.lineWidth = 1;

  for (let j = 1; j <= 4; j++) {

    const y = j * h / 5;

    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(w, y);
    ctx.stroke();
  }

  // Linea de corriente

  ctx.strokeStyle = colores[i];
  ctx.lineWidth = 2;

  ctx.beginPath();

  let empezado = false;

  for (let j = 0; j < datos.length; j++) {

    const valor = datos[j];

    if (!Number.isFinite(valor)) {

      empezado = false;
      continue;
    }

    const x =
      j * w / (MAX_PUNTOS - 1);

    const y =
      h - (valor / maximo) * (h - 12);

    if (!empezado) {

      ctx.moveTo(x, y);
      empezado = true;

    } else {

      ctx.lineTo(x, y);
    }
  }

  ctx.stroke();
}

// ==========================================================
// ACTUALIZAR ESTADO
// ==========================================================

async function actualizarEstado() {

  try {

    const respuesta =
      await fetch("/api/status");

    if (!respuesta.ok) {

      throw new Error(
        "HTTP " + respuesta.status
      );
    }

    const datos = await respuesta.json();

    document.getElementById("fecha").textContent =
      datos.fecha_chile;

    let texto = "";

    texto +=
      "Wi-Fi: " +
      (
        datos.wifi_conectado
          ? "Conectado"
          : "Desconectado"
      ) +
      "\n";

    texto +=
      "Red: " +
      (datos.wifi_ssid || "-") +
      "\n";

    texto +=
      "IP del router: " +
      (datos.ip_wifi || "-") +
      "\n";

    texto +=
      "Hora NTP: " +
      (
        datos.hora_valida
          ? "Disponible"
          : "No sincronizada"
      ) +
      "\n";

    texto +=
      "MicroSD: " +
      (
        datos.sd_disponible
          ? "Disponible"
          : "No disponible"
      ) +
      "\n";

    texto +=
      "API remota: " +
      datos.api_estado +
      "\n";

    texto +=
      "Registros locales desde inicio: " +
      datos.registros_locales +
      "\n";

    texto +=
      "Registros enviados desde inicio: " +
      datos.registros_enviados;

    if (datos.api_error) {

      texto +=
        "\nÚltimo error API: " +
        datos.api_error;
    }

    document.getElementById("estado").textContent =
      texto;

    // Actualizar conectores

    for (let i = 0; i < 4; i++) {

      const valor = datos.corrientes[i];

      const disponible =
        datos.adc_disponibles[i];

      const elemento =
        document.getElementById("valor" + i);

      if (
        !disponible ||
        valor === null
      ) {

        elemento.textContent =
          "ADC no disponible";

        historiales[i].push(null);

      } else {

        elemento.textContent =
          valor.toFixed(3) + " A";

        historiales[i].push(valor);
      }

      if (
        historiales[i].length >
        MAX_PUNTOS
      ) {

        historiales[i].shift();
      }

      dibujarGrafico(i);
    }

  } catch (error) {

    console.error(error);

    document.getElementById("estado").textContent =
      "No se pudo consultar el ESP32.";
  }
}

// ==========================================================
// CARGAR CONFIGURACION
// ==========================================================

async function cargarConfiguracion() {

  try {

    const respuesta =
      await fetch("/api/config");

    if (!respuesta.ok) {
      return;
    }

    const datos = await respuesta.json();

    document.getElementById("wifiSSID").value =
      datos.wifi_ssid || "";

    document.getElementById("apiURL").value =
      datos.api_url || "";

    document.getElementById("deviceID").value =
      datos.device_id || "";

    document.getElementById("uploadInterval").value =
      datos.upload_interval || 10;

  } catch (error) {

    console.error(error);
  }
}

// ==========================================================
// GUARDAR WIFI
// ==========================================================

document.getElementById("formWifi")
  .addEventListener("submit", async e => {

    e.preventDefault();

    const datos = new URLSearchParams();

    datos.set(
      "ssid",
      document.getElementById("wifiSSID").value
    );

    datos.set(
      "password",
      document.getElementById("wifiPassword").value
    );

    try {

      const respuesta = await fetch(
        "/api/config/wifi",
        {
          method: "POST",
          body: datos
        }
      );

      mostrarMensaje(
        await respuesta.text()
      );

    } catch (error) {

      mostrarMensaje(
        "Error guardando Wi-Fi"
      );
    }
  });

// ==========================================================
// GUARDAR API
// ==========================================================

document.getElementById("formAPI")
  .addEventListener("submit", async e => {

    e.preventDefault();

    const datos = new URLSearchParams();

    datos.set(
      "url",
      document.getElementById("apiURL").value
    );

    datos.set(
      "device_id",
      document.getElementById("deviceID").value
    );

    datos.set(
      "token",
      document.getElementById("apiToken").value
    );

    datos.set(
      "interval",
      document.getElementById("uploadInterval").value
    );

    try {

      const respuesta = await fetch(
        "/api/config/remote",
        {
          method: "POST",
          body: datos
        }
      );

      mostrarMensaje(
        await respuesta.text()
      );

    } catch (error) {

      mostrarMensaje(
        "Error guardando API"
      );
    }
  });

// ==========================================================
// REINICIAR CSV
// ==========================================================

async function reiniciarCSV() {

  try {

    const respuesta = await fetch(
      "/api/resetcsv",
      {
        method: "POST"
      }
    );

    mostrarMensaje(
      await respuesta.text()
    );

  } catch (error) {

    mostrarMensaje(
      "Error reiniciando CSV"
    );
  }
}

// ==========================================================
// INICIO
// ==========================================================

construirMediciones();

cargarConfiguracion();

actualizarEstado();

setInterval(
  actualizarEstado,
  1000
);

</script>

</body>
</html>
)ESP32WEB";

#endif
