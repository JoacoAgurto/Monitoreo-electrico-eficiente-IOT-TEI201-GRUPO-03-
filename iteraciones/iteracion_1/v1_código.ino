
/*
  =========================================================
              MONITOR DE CORRIENTE ESP32
  =========================================================

  COMPONENTES:
  - ESP32
  - 2 módulos ADS1115
  - 4 pinzas SCT-013 de 100 A / 1 V
  - 1 módulo microSD
  - Servidor web Wi-Fi sin router

  DISTRIBUCIÓN DE LOS CONECTORES:
  Conector 1 -> ADS2 A2-A3
  Conector 2 -> ADS2 A0-A1
  Conector 3 -> ADS1 A2-A3
  Conector 4 -> ADS1 A0-A1

  FUNCIONES:
  - Medir corriente RMS en cuatro canales.
  - Mostrar cuatro gráficos independientes.
  - Ocultar o mostrar cada gráfico.
  - Guardar mediciones en un archivo CSV.
  - Descargar y reiniciar el historial CSV.
  - Modificar el SSID y la contraseña desde la web.
  - Guardar la configuración Wi-Fi en la microSD.
  - Usar Wi-Fi predeterminado si falla la microSD.

  IMPORTANTE:
  La detección física de una pinza conectada todavía
  no está implementada. El programa únicamente detecta
  si los módulos ADS1115 están disponibles.
*/

// Comunicación I2C entre ESP32 y ADS1115.
#include <Wire.h>

// Comunicación SPI con la microSD.
#include <SPI.h>

// Lectura y escritura de archivos en la microSD.
#include <SD.h>

// Creación de la red Wi-Fi del ESP32.
#include <WiFi.h>

// Servidor HTTP para mostrar la página web.
#include <WebServer.h>

// Control de los convertidores ADS1115.
#include <Adafruit_ADS1X15.h>

// Funciones matemáticas, como la raíz cuadrada.
#include <math.h>


// Credenciales predeterminadas de la red Wi-Fi.
// Se utilizan si no hay configuración válida en la SD.
const char* DEFAULT_SSID = "Monitor-ESP32";
const char* DEFAULT_PASS = "corriente123";

// Archivos almacenados en la microSD.
const char* CSV_PATH = "/corriente_conectores.csv";
const char* WIFI_PATH = "/wifi.cfg";

// Pin CS del módulo microSD.
const int SD_CS = 5;

// Intervalo objetivo entre ciclos de medición, en ms.
const unsigned long SAMPLE_INTERVAL = 1000;

// Cantidad de muestras tomadas por cada conector.
const int SAMPLES = 128;

// Conversión nominal de la pinza SCT-013 de 100 A / 1 V.
// Por cada voltio RMS, corresponden nominalmente 100 A.
const float AMPS_PER_VOLT = 100.0f;

// Objetos que representan los dos ADS1115.
Adafruit_ADS1115 ads1, ads2;

// Servidor web HTTP en el puerto 80.
WebServer server(80);

// Indican si cada dispositivo se inicializó correctamente.
bool ads1OK = false;
bool ads2OK = false;
bool sdOK = false;

// Permite suspender temporalmente el registro CSV.
bool loggingPaused = false;

// Credenciales Wi-Fi que se utilizarán realmente.
String wifiSSID = DEFAULT_SSID;
String wifiPass = DEFAULT_PASS;

// Corriente RMS de cada conector, en amperios.
float amps[4] = {0, 0, 0, 0};

// Indica si el ADS1115 correspondiente está disponible.
// NO indica si hay una pinza enchufada.
bool available[4] = {false, false, false, false};

// Control de tiempo y registros.
unsigned long lastSample = 0;
unsigned long records = 0;

// Momento programado para reiniciar el ESP32.
unsigned long restartAt = 0;

// Interfaz web

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Monitor de corriente</title>

<style>
/* Apariencia general de la página */
:root {
  color-scheme: dark;
  font-family: system-ui, Arial, sans-serif;
}
* { box-sizing: border-box; }
body {
  margin: 0;
  background: #0c1423;
  color: #eef4ff;
}

header {
  padding: 24px;
  background: #13233b;
  border-bottom: 1px solid #34465e;
}
header h1 { margin: 0 0 6px; }
header p { margin: 0; color: #acbdd0; }

main {
  max-width: 1100px;
  margin: auto;
  padding: 16px;
}

.status {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
  margin-bottom: 16px;
}
.pill, .card {
  background: #17263e;
  border: 1px solid #34465e;
  border-radius: 14px;
}
.pill { padding: 9px 12px; }

.grid {
  display: grid;
  grid-template-columns: repeat(2,minmax(0,1fr));
  gap: 14px;
}
.card { padding: 18px; min-width: 0; }
.top {
  display: flex;
  justify-content: space-between;
  gap: 8px;
  align-items: center;
}
h2 { font-size: 20px; margin: 0 0 12px; }

.value {
  font-size: 38px;
  font-weight: 700;
  margin: 12px 0;
}
.unit, .muted {
  color: #acbdd0;
  font-size: 14px;
}

.chart { margin-top: 15px; }
.chart[hidden] { display: none; }
canvas {
  width: 100%;
  height: 180px;
  background: #0e1b2e;
  border-radius: 9px;
  border: 1px solid #34465e;
}

section.extra { margin-top: 15px; }
input[type=text], input[type=password] {
  width: 100%;
  padding: 11px;
  border-radius: 8px;
  background: #0c1728;
  border: 1px solid #536782;
  color: white;
  font-size: 16px;
}
label.field { display: block; margin: 12px 0; }
label.field span {
  display: block;
  margin-bottom: 6px;
}
button {
  background: #27618e;
  color: white;
  border: 1px solid #568ab1;
  border-radius: 9px;
  padding: 11px 15px;
  font-size: 15px;
  cursor: pointer;
}
button.danger {
  background: #8b3535;
  border-color: #c77878;
}
a { color: #9dd5ff; }
.actions {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
  align-items: center;
}
.message {
  min-height: 1.4em;
  color: #ffd7a0;
}

@media(max-width:720px) {
  .grid { grid-template-columns: 1fr; }
  .value { font-size: 34px; }
}
</style>
</head>

<body>
<header>
  <h1>Monitor de corriente</h1>
  <p>Cuatro conectores independientes</p>
</header>

<main>

  <div class="status">
    <div class="pill">
      ESP32: <b id="connection">Conectando</b>
    </div>
    <div class="pill">
      microSD: <b id="sd">Consultando</b>
    </div>
    <div class="pill">
      Registros: <b id="records">0</b>
    </div>
  </div>

  <div id="cards" class="grid"></div>

  <section class="card extra">
    <h2>Historial CSV</h2>
    <p class="muted">
      Se registran los cuatro conectores
      aunque ocultes sus gráficos.
    </p>

    <div class="actions">
      <a href="/download">Descargar CSV</a>
      <button class="danger" id="reset">
        Reiniciar archivo CSV
      </button>
    </div>

    <p id="resetMsg" class="message"></p>
  </section>

  <section class="card extra">
    <h2>Configuración Wi-Fi</h2>
    <p class="muted">
      La red es creada por el ESP32, sin router.
      Los cambios se guardan en la microSD.
    </p>

    <form id="wifiForm">
      <label class="field">
        <span>Nombre de la red (SSID)</span>
        <input id="ssid" type="text"
          minlength="1" maxlength="32" required>
      </label>

      <label class="field">
        <span>Contraseña (mínimo 8 caracteres)</span>
        <input id="pass" type="password"
          minlength="8" maxlength="63" required>
      </label>

      <button type="submit">
        Guardar Wi-Fi y reiniciar red
      </button>
    </form>

    <p id="wifiMsg" class="message"></p>
  </section>
</main>

<script>

const names = [
  'Conector 1',
  'Conector 2',
  'Conector 3',
  'Conector 4'
];

const colors = [
  '#6bb7ff',
  '#56d7ac',
  '#ffbc70',
  '#c8a2ff'
];

const hist = [[],[],[],[]];

const visible = [true,true,true,true];

const maxPoints = 90;

const el = id => document.getElementById(id);

names.forEach((name,i) => {

  try {
    visible[i] =
      localStorage.getItem('visible'+i) !== 'false';
  } catch(e) {}

  const card = document.createElement('section');
  card.className = 'card';

  card.innerHTML = `
    <div class="top">
      <h2>${name}</h2>
      <label class="muted">
        <input type="checkbox" id="toggle${i}"
          ${visible[i] ? 'checked' : ''}>
        Gráfico
      </label>
    </div>

    <div class="value">
      <span id="amp${i}">--</span>
      <span class="unit">A RMS</span>
    </div>

    <div id="state${i}" class="muted">
      Esperando lectura
    </div>

    <div class="chart" id="chartbox${i}"
      ${visible[i] ? '' : 'hidden'}>
      <canvas id="canvas${i}"
        width="680" height="220"></canvas>
    </div>`;

  el('cards').appendChild(card);

  el('toggle'+i).addEventListener('change',e => {

    visible[i] = e.target.checked;
    el('chartbox'+i).hidden = !visible[i];

    try {
      localStorage.setItem(
        'visible'+i,
        String(visible[i])
      );
    } catch(err) {}

    if(visible[i]) draw(i);
  });
});

function draw(i) {

  if(!visible[i]) return;

  const c = el('canvas'+i);
  const ctx = c.getContext('2d');

  const w = c.width;
  const h = c.height;

  const L = 50, R = 12, T = 15, B = 30;

  ctx.clearRect(0,0,w,h);

  const valid = hist[i].filter(Number.isFinite);
  const maximum = Math.max(.1,...valid)*1.15;

  ctx.font = '13px Arial';
  ctx.fillStyle = '#acbdd0';
  ctx.lineWidth = 1;

  for(let k=0;k<=4;k++) {

    const y = T+(h-T-B)*k/4;

    ctx.strokeStyle = '#354861';
    ctx.beginPath();
    ctx.moveTo(L,y);
    ctx.lineTo(w-R,y);
    ctx.stroke();

    ctx.fillText(
      (maximum*(1-k/4)).toFixed(maximum<1?2:1),
      5,y+4
    );
  }

  ctx.strokeStyle = colors[i];
  ctx.lineWidth = 2.5;
  ctx.beginPath();

  let started = false;

  hist[i].forEach((v,k) => {

    if(!Number.isFinite(v)) {
      started = false;
      return;
    }

    const x = L+(w-L-R)*k/(maxPoints-1);

    const y = h-B-(h-T-B)*
      Math.min(v,maximum)/maximum;

    if(!started) {
      ctx.moveTo(x,y);
      started = true;
    } else {
      ctx.lineTo(x,y);
    }
  });

  ctx.stroke();
}

let polling = false;

async function update() {

  if(polling) return;
  polling = true;

  try {

    const r = await fetch('/api/data',{
      cache:'no-store'
    });

    if(!r.ok) throw Error('HTTP '+r.status);

    const d = await r.json();

    el('connection').textContent = 'Conectado';
    el('sd').textContent =
      d.sd ? 'Disponible' : 'No disponible';
    el('records').textContent = d.records;

    d.channels.forEach((ch,i) => {

      el('amp'+i).textContent =
        ch.available ?
        Number(ch.amps).toFixed(3) : '--';

      el('state'+i).textContent =
        ch.available ?
        'Lectura actualizada' :
        'ADS1115 no disponible';

      hist[i].push(
        ch.available ? Number(ch.amps) : null
      );

      if(hist[i].length>maxPoints) {
        hist[i].shift();
      }

      draw(i);
    });

  } catch(e) {

    el('connection').textContent = 'Sin conexión';

  } finally {
    polling = false;
  }
}

async function loadWifi() {
  try {

    const r = await fetch('/api/wifi');
    const d = await r.json();

    el('ssid').value = d.ssid;
    el('pass').value = d.password;

  } catch(e) {}
}

el('reset').onclick = async () => {

  if(!confirm(
    '¿Borrar definitivamente todo el historial CSV '+
    'y comenzar desde cero?'
  )) return;

  el('resetMsg').textContent =
    'Reiniciando historial...';

  try {

    const r = await fetch('/api/resetcsv',{
      method:'POST'
    });

    el('resetMsg').textContent = await r.text();

    update();

  } catch(e) {
    el('resetMsg').textContent =
      'Error de comunicación';
  }
};

el('wifiForm').onsubmit = async e => {

  e.preventDefault();

  const ssid = el('ssid').value;
  const password = el('pass').value;

  if(!confirm(
    'La red Wi-Fi cambiará y tendrás que '+
    'reconectarte. ¿Continuar?'
  )) return;

  el('wifiMsg').textContent = 'Guardando...';

  try {

    const body = new URLSearchParams({
      ssid,password
    });

    const r = await fetch('/api/wifi',{
      method:'POST',
      headers:{
        'Content-Type':
          'application/x-www-form-urlencoded'
      },
      body
    });

    el('wifiMsg').textContent = await r.text();

  } catch(err) {
    el('wifiMsg').textContent =
      'Error de comunicación';
  }
};

loadWifi();
update();

setInterval(update,1000);

</script>
</body>
</html>
)HTML";


/*
  Esta función recibe:

  ads:
    ADS1115 que realizará la medición.

  pair23:
    true  -> medir A2-A3.
    false -> medir A0-A1.

  PROCEDIMIENTO:

  1. Tomar 128 muestras de voltaje diferencial.
  2. Calcular la media de esas muestras.
  3. Restar matemáticamente la componente continua.
  4. Calcular el valor RMS de la componente variable.
  5. Convertir voltios RMS a amperios RMS.

  La fórmula utilizada es:

  Vrms = sqrt( promedio(V²) - promedio(V)² )

  Irms = Vrms * 100

  NOTA:
  La relación 100 A/V es nominal. No sustituye una
  calibración con una corriente conocida.
*/

float measure(
  Adafruit_ADS1115 &ads,
  bool pair23
) {

  // Suma de los voltajes.
  double sum = 0;

  // Suma de los voltajes elevados al cuadrado.
  double sum2 = 0;

  for(int i=0;i<SAMPLES;i++) {

    // Selecciona el par diferencial correspondiente.
    int16_t raw = pair23 ?
      ads.readADC_Differential_2_3() :
      ads.readADC_Differential_0_1();

    // Convierte la lectura digital a voltios.
    double v = ads.computeVolts(raw);

    // Acumula los valores necesarios para el RMS.
    sum += v;
    sum2 += v*v;
  }

  // Componente media o continua.
  double mean = sum/SAMPLES;

  // Varianza de la señal.
  double variance = sum2/SAMPLES-mean*mean;

  // Evita raíces de valores negativos por redondeo.
  return sqrt(fmax(0.0,variance))*
    AMPS_PER_VOLT;
}

// Medición de los cuatro conectores

void sampleChannels() {

  available[0] = ads2OK;
  available[1] = ads2OK;
  available[2] = ads1OK;
  available[3] = ads1OK;

  // ADS2 contiene los conectores 1 y 2.
  if(ads2OK) {

    // Conector 1: ADS2 A2-A3.
    amps[0] = measure(ads2,true);

    // Conector 2: ADS2 A0-A1.
    amps[1] = measure(ads2,false);
  }

  // ADS1 contiene los conectores 3 y 4.
  if(ads1OK) {

    // Conector 3: ADS1 A2-A3.
    amps[2] = measure(ads1,true);

    // Conector 4: ADS1 A0-A1.
    amps[3] = measure(ads1,false);
  }
}

// Creación del archivo CSV para la microSD
bool createCSV() {

  // No intentar escribir si no hay microSD.
  if(!sdOK) return false;

  // Si el archivo ya existe, conservar su contenido.
  if(SD.exists(CSV_PATH)) return true;

  // Crear el archivo.
  File f = SD.open(CSV_PATH,FILE_WRITE);

  if(!f) return false;

  // Escribir los encabezados de las columnas.
  size_t n = f.println(
    "tiempo_ms,Conector_1_A,Conector_2_A,"
    "Conector_3_A,Conector_4_A"
  );

  // Cerrar el archivo para completar la escritura.
  f.close();

  return n>0;
}

// Registro de las mediciones de la corriente
void appendCSV() {

  // No registrar si la SD falla o el registro está pausado.
  if(!sdOK || loggingPaused) return;

  // Abrir el CSV para agregar una nueva fila.
  File f = SD.open(CSV_PATH,FILE_APPEND);

  if(!f) return;

  // Primera columna: tiempo desde el arranque, en ms.
  f.print(millis());

  // Guardar los cuatro valores de corriente.
  for(int i=0;i<4;i++) {

    f.print(',');

    // Dejar vacío si el ADS no está disponible.
    if(available[i]) {
      f.print(amps[i],3);
    }
  }

  // Terminar la fila.
  size_t n = f.println();

  // Cerrar el archivo.
  f.close();

  // Incrementar el contador si se escribió la fila.
  if(n) records++;
}

// Cargar la configuracion de la red de la microSD
void loadWiFiConfig() {

  // Primero establecer los valores predeterminados.
  wifiSSID = DEFAULT_SSID;
  wifiPass = DEFAULT_PASS;

  // Si no existe la SD o el archivo, conservarlos.
  if(!sdOK || !SD.exists(WIFI_PATH)) return;

  File f = SD.open(WIFI_PATH,FILE_READ);
  if(!f) return;

  // Primera línea: SSID.
  String s = f.readStringUntil('\n');

  // Segunda línea: contraseña.
  String p = f.readStringUntil('\n');

  f.close();

  // Eliminar espacios al inicio y al final.
  s.trim();
  p.trim();

  // Validar las longitudes permitidas.
  if(
    s.length()>=1 &&
    s.length()<=32 &&
    p.length()>=8 &&
    p.length()<=63
  ) {

    // Utilizar los valores guardados.
    wifiSSID = s;
    wifiPass = p;

    Serial.println(
      "Wi-Fi cargado desde microSD"
    );

  } else {

    // Si el archivo contiene datos inválidos,
    // mantener los valores predeterminados.
    Serial.println(
      "Configuracion invalida: usando predeterminados"
    );
  }
}

// Guardar la configuracion de la red
bool saveWiFiConfig(
  const String &s,
  const String &p
) {

  if(!sdOK) return false;

  // Archivo temporal utilizado durante la escritura.
  const char* temp = "/wifi.tmp";

  // Eliminar un temporal anterior si existe.
  SD.remove(temp);

  File f = SD.open(temp,FILE_WRITE);
  if(!f) return false;

  // Escribir SSID y contraseña en líneas separadas.
  bool ok = f.println(s)>0;
  ok = (f.println(p)>0) && ok;

  f.close();

  if(!ok) {
    SD.remove(temp);
    return false;
  }

  // Eliminar la configuración anterior.
  if(
    SD.exists(WIFI_PATH) &&
    !SD.remove(WIFI_PATH)
  ) {
    return false;
  }

  // Renombrar el archivo temporal como wifi.cfg.
  return SD.rename(temp,WIFI_PATH);
}

// Generar los datos para la pagina web

String jsonData() {

  String out;
  out.reserve(350);

  out = "{\"sd\":";
  out += sdOK ? "true" : "false";

  out += ",\"records\":";
  out += String(records);

  out += ",\"channels\":[";

  for(int i=0;i<4;i++) {

    if(i) out += ",";

    out += "{\"available\":";
    out += available[i] ? "true" : "false";

    out += ",\"amps\":";
    out += String(amps[i],3);

    out += "}";
  }

  out += "]}";

  return out;
}

// Reiniciar el archivo CSV de la web
void handleResetCSV() {

  // No permitir el borrado sin microSD.
  if(!sdOK) {
    server.send(
      503,
      "text/plain; charset=utf-8",
      "microSD no disponible"
    );
    return;
  }

  // Pausar temporalmente el registro.
  loggingPaused = true;

  // Eliminar el CSV si existe.
  bool removed =
    !SD.exists(CSV_PATH) ||
    SD.remove(CSV_PATH);

  // Crear nuevamente el CSV con sus encabezados.
  bool created = removed && createCSV();

  // Reiniciar el contador de registros.
  if(created) records = 0;

  // Reanudar el registro.
  loggingPaused = false;

  // Informar al navegador del resultado.
  server.send(
    created ? 200 : 500,
    "text/plain; charset=utf-8",
    created ?
      "CSV reiniciado. El registro continua." :
      "No se pudo reiniciar el CSV."
  );
}

// Guardar los cambios de la red por la web
void handleWifiSave() {

  // Sin microSD no se pueden guardar cambios
  // de forma persistente.
  if(!sdOK) {
    server.send(
      503,
      "text/plain; charset=utf-8",
      "microSD no disponible. No se pueden guardar cambios."
    );
    return;
  }

  // Leer los datos enviados por el formulario.
  String s = server.arg("ssid");
  String p = server.arg("password");

  // Comprobar que los datos sean válidos.
  if(
    s.length()<1 ||
    s.length()>32 ||
    p.length()<8 ||
    p.length()>63 ||
    s.indexOf('\n')>=0 ||
    s.indexOf('\r')>=0 ||
    p.indexOf('\n')>=0 ||
    p.indexOf('\r')>=0
  ) {
    server.send(
      400,
      "text/plain; charset=utf-8",
      "SSID o contraseña no validos."
    );
    return;
  }

  // Guardar los nuevos datos en la microSD.
  if(!saveWiFiConfig(s,p)) {
    server.send(
      500,
      "text/plain; charset=utf-8",
      "Error al guardar configuracion en microSD."
    );
    return;
  }

  // Actualizar las variables del programa.
  wifiSSID = s;
  wifiPass = p;

  // Avisar al navegador antes de reiniciar.
  server.send(
    200,
    "text/plain; charset=utf-8",
    "Configuracion guardada. Reconectate a la nueva red."
  );

  // Programar el reinicio después de 2,5 segundos.
  restartAt = millis()+2500;
}

// Funciones principales del ESP32 (setup y loop)

void setup() {

  // Iniciar comunicación con el Monitor Serie.
  Serial.begin(115200);
  delay(900);

  Serial.println();
  Serial.println("=== MONITOR ESP32 ===");

  // Iniciar I2C:
  // SDA = GPIO21
  // SCL = GPIO22
  Wire.begin(21,22);

  // Buscar los ADS1115 por su dirección I2C.
  ads1OK = ads1.begin(0x48);
  ads2OK = ads2.begin(0x49);

  if(ads1OK) {

    // Esto NO permite superar los límites
    // absolutos de tensión de cada entrada.
    ads1.setGain(GAIN_ONE);

    // Velocidad de conversión con 860 muestras
    ads1.setDataRate(RATE_ADS1115_860SPS);
  }

  if(ads2OK) {
    ads2.setGain(GAIN_ONE);
    ads2.setDataRate(RATE_ADS1115_860SPS);
  }

  // Informar qué ADC fueron detectados.
  Serial.printf(
    "ADS1: %s | ADS2: %s\n",
    ads1OK ? "OK" : "ERROR",
    ads2OK ? "OK" : "ERROR"
  );

  // Iniciar bus SPI para la microSD:
  // SCK   GPIO18
  // MISO  GPIO19
  // MOSI  GPIO23
  // CS    GPIO5
  SPI.begin(18,19,23,SD_CS);

  pinMode(SD_CS,OUTPUT);
  digitalWrite(SD_CS,HIGH);

  // Intentar iniciar la microSD
  sdOK = SD.begin(
    SD_CS,
    SPI,
    1000000
  ) && SD.cardType()!=CARD_NONE;

  Serial.println(
    sdOK ?
    "microSD detectada" :
    "microSD no disponible"
  );

  // Crear el CSV si todavía no existe.
  if(sdOK) {
    if(!createCSV()) {
      Serial.println(
        "ERROR: no se pudo crear CSV"
      );
    }
  }

  // Cargar las credenciales guardadas.
  // Si falla, utilizar las predeterminadas.
  loadWiFiConfig();

  // Configurar el ESP32 como punto de acceso Wi-Fi.
  // No necesita router.
  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    wifiSSID.c_str(),
    wifiPass.c_str()
  );

  Serial.printf(
    "Wi-Fi: %s (%s)\n",
    wifiSSID.c_str(),
    started ? "OK" : "ERROR"
  );

  // Información de la Red wifi
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // Página principal del monitor.
  server.on("/",HTTP_GET,[]() {
    server.send_P(
      200,
      "text/html; charset=utf-8",
      PAGE
    );
  });

  // API que entrega las corrientes en formato JSON.
  server.on("/api/data",HTTP_GET,[]() {

    server.sendHeader(
      "Cache-Control",
      "no-store"
    );

    server.send(
      200,
      "application/json",
      jsonData()
    );
  });

  // API para reiniciar el archivo CSV.
  server.on(
    "/api/resetcsv",
    HTTP_POST,
    handleResetCSV
  );

  // API para consultar las credenciales actuales.
  server.on("/api/wifi",HTTP_GET,[]() {

    String s = wifiSSID;
    String p = wifiPass;

    // Escapar caracteres especiales para JSON.
    s.replace("\\","\\\\");
    s.replace("\"","\\\"");

    p.replace("\\","\\\\");
    p.replace("\"","\\\"");

    server.send(
      200,
      "application/json",
      "{\"ssid\":\""+s+
      "\",\"password\":\""+p+"\"}"
    );
  });

  // API para guardar nuevas credenciales.
  server.on(
    "/api/wifi",
    HTTP_POST,
    handleWifiSave
  );

  // API para descargar el historial CSV.
  server.on("/download",HTTP_GET,[]() {

    // Comprobar que el archivo esté disponible.
    if(
      !sdOK ||
      !SD.exists(CSV_PATH)
    ) {
      server.send(
        503,
        "text/plain",
        "CSV no disponible"
      );
      return;
    }

    // Abrir el archivo para lectura.
    File f = SD.open(
      CSV_PATH,
      FILE_READ
    );

    if(!f) {
      server.send(
        500,
        "text/plain",
        "No se pudo abrir CSV"
      );
      return;
    }

    // Indicar al navegador que descargue el archivo.
    server.sendHeader(
      "Content-Disposition",
      "attachment; filename=\"corriente_conectores.csv\""
    );

    // Enviar el contenido directamente desde la SD.
    server.streamFile(f,"text/csv");

    f.close();
  });

  // Iniciar el servidor HTTP.
  server.begin();

  // Realizar las primeras mediciones.
  sampleChannels();

  // Guardar el instante inicial.
  lastSample = millis();

  Serial.println("Servidor iniciado");
}

// Loop de la ESP32
void loop() {

  // Procesar las solicitudes HTTP pendientes.
  server.handleClient();

  // Comprobar si hay un reinicio Wi-Fi programado.
  if(
    restartAt &&
    (long)(millis()-restartAt)>=0
  ) {

    Serial.println(
      "Reiniciando ESP32 para aplicar Wi-Fi..."
    );

    delay(100);

    // Reiniciar el microcontrolador.
    ESP.restart();
  }

  // Comprobar si transcurrió el intervalo de medición.
  if(
    millis()-lastSample >=
    SAMPLE_INTERVAL
  ) {

    // Actualizar la referencia temporal.
    lastSample = millis();

    // Medir los cuatro conectores.
    sampleChannels();

    // Guardar las lecturas en el CSV.
    appendCSV();

    // Mostrar las corrientes por el Monitor Serie.
    for(int i=0;i<4;i++) {

      Serial.printf(
        "Conector %d: ",
        i+1
      );

      if(available[i]) {

        Serial.printf(
          "%.3f A RMS\n",
          amps[i]
        );

      } else {

        Serial.println(
          "ADS no disponible"
        );
      }
    }
  }
}
