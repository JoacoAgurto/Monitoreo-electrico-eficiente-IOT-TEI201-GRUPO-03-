/*
  ==========================================================
               MONITOR DE CORRIENTE ESP32 V7
  ==========================================================

  CAMBIO V7:
  - Eliminada la comprobacion de IP que bloqueaba el panel.
  - Sin usuario ni contrasena en el navegador.
  - Acceso directo desde http://192.168.4.1

  HARDWARE:
    ESP32 Dev Module
    ADS1: 0x48
    ADS2: 0x49
    SCT-013: 100 A / 1 V
    MicroSD SPI

  MAPEO:
    Conector 1 = ADS2 A2-A3
    Conector 2 = ADS2 A0-A1
    Conector 3 = ADS1 A2-A3
    Conector 4 = ADS1 A0-A1

  IMPORTANTE:
  - El ADS1115 necesita entradas acondicionadas para
    permanecer entre GND y VDD en todo momento.
  - No existe deteccion fisica de pinzas conectadas.
  - setInsecure() es solamente para pruebas de HTTPS.
  ==========================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_ADS1X15.h>
#include <time.h>
#include <math.h>

#include "pagina_web.h"

// ==========================================================
// PINES
// ==========================================================

#define I2C_SDA 21
#define I2C_SCL 22

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

// ==========================================================
// WIFI DEL ESP32
// ==========================================================

const char* AP_SSID = "Monitor-ESP32";
const char* AP_PASSWORD = "corriente123";

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

// ==========================================================
// OBJETOS
// ==========================================================

WebServer server(80);
Preferences preferences;

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;

// ==========================================================
// ESTADO DEL HARDWARE
// ==========================================================

bool ads1OK = false;
bool ads2OK = false;
bool sdOK = false;

unsigned long ultimaRevisionSD = 0;
const unsigned long INTERVALO_REVISION_SD = 5000;

bool canalDisponible[4] = {
  false, false, false, false
};

// ==========================================================
// CONFIGURACION PERSISTENTE
// ==========================================================

String wifiSSID = "";
String wifiPassword = "";

String apiURL = "";
String apiToken = "";

String deviceID = "esp32-01";

uint32_t uploadIntervalSeconds = 10;
uint32_t queueOffset = 0;

// Se mantienen los nombres anteriores para conservar
// el historial de la tarjeta microSD.

const char* CSV_FILE = "/corriente_v5.csv";
const char* QUEUE_FILE = "/pendientes_v5.csv";

// ==========================================================
// MEDICIONES
// ==========================================================

float corriente[4] = {
  NAN, NAN, NAN, NAN
};

const float AMPERIOS_POR_VOLTIO = 100.0f;
const int NUM_MUESTRAS = 128;

// ==========================================================
// TEMPORIZADORES
// ==========================================================

unsigned long ultimaMedicion = 0;
unsigned long ultimoEnvio = 0;
unsigned long ultimoIntentoWiFi = 0;
unsigned long ultimaRevisionHora = 0;

const unsigned long INTERVALO_MEDICION = 1000;
const unsigned long INTERVALO_RECONEXION = 15000;
const unsigned long INTERVALO_REVISION_HORA = 60000;

// ==========================================================
// ESTADISTICAS
// ==========================================================

uint32_t registrosLocales = 0;
uint32_t registrosEnviados = 0;

String ultimoEstadoAPI = "Sin configurar";
String ultimoErrorAPI = "";

int ultimoHTTP = 0;

uint64_t ultimoBloqueRemoto = UINT64_MAX;

// ==========================================================
// HORA DE CHILE
// ==========================================================

const char* CHILE_TZ =
  "CLT4CLST,M9.1.6/24,M4.1.6/24";

// ==========================================================
// FUNCIONES AUXILIARES
// ==========================================================

bool wifiConectado() {
  return WiFi.status() == WL_CONNECTED;
}

bool horaValida() {
  return time(nullptr) > 1735689600;
}

String fechaUTC() {

  if (!horaValida()) {
    return "";
  }

  time_t ahora = time(nullptr);

  struct tm utc;
  gmtime_r(&ahora, &utc);

  char buffer[32];

  strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%dT%H:%M:%SZ",
    &utc
  );

  return String(buffer);
}

String fechaChile() {

  if (!horaValida()) {
    return "SIN_HORA";
  }

  time_t ahora = time(nullptr);

  struct tm local;
  localtime_r(&ahora, &local);

  char buffer[40];

  strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d %H:%M:%S %z",
    &local
  );

  return String(buffer);
}

String escaparJSON(const String& texto) {

  String resultado;
  resultado.reserve(texto.length() + 12);

  for (size_t i = 0; i < texto.length(); i++) {

    char c = texto[i];

    switch (c) {

      case '"':
        resultado += "\\\"";
        break;

      case '\\':
        resultado += "\\\\";
        break;

      case '\n':
        resultado += "\\n";
        break;

      case '\r':
        resultado += "\\r";
        break;

      case '\t':
        resultado += "\\t";
        break;

      default:
        if ((uint8_t)c >= 32) {
          resultado += c;
        }
        break;
    }
  }

  return resultado;
}

String numeroJSON(float valor) {

  if (!isfinite(valor)) {
    return "null";
  }

  return String(valor, 3);
}

// ==========================================================
// CONFIGURACION
// ==========================================================

void cargarConfiguracion() {

  preferences.begin("monitor", false);

  wifiSSID = preferences.getString(
    "wifi_ssid", ""
  );

  wifiPassword = preferences.getString(
    "wifi_pass", ""
  );

  apiURL = preferences.getString(
    "api_url", ""
  );

  apiToken = preferences.getString(
    "api_token", ""
  );

  deviceID = preferences.getString(
    "device_id", "esp32-01"
  );

  uploadIntervalSeconds = preferences.getUInt(
    "upload_sec", 10
  );

  queueOffset = preferences.getUInt(
    "queue_pos", 0
  );

  if (uploadIntervalSeconds < 5) {
    uploadIntervalSeconds = 5;
  }

  if (uploadIntervalSeconds > 3600) {
    uploadIntervalSeconds = 3600;
  }

  Serial.println("[CONFIG] Cargada");
}

void guardarConfiguracion() {

  preferences.putString(
    "wifi_ssid", wifiSSID
  );

  preferences.putString(
    "wifi_pass", wifiPassword
  );

  preferences.putString(
    "api_url", apiURL
  );

  preferences.putString(
    "api_token", apiToken
  );

  preferences.putString(
    "device_id", deviceID
  );

  preferences.putUInt(
    "upload_sec",
    uploadIntervalSeconds
  );
}

// ==========================================================
// WIFI
// ==========================================================

void iniciarWiFi() {

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  WiFi.softAPConfig(
    AP_IP,
    AP_GATEWAY,
    AP_SUBNET
  );

  bool apOK = WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  if (apOK) {

    Serial.println("[WIFI] AP iniciado");

    Serial.print("[WIFI] SSID: ");
    Serial.println(AP_SSID);

    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.softAPIP());

  } else {

    Serial.println("[WIFI] Error iniciando AP");
  }

  if (wifiSSID.length() > 0) {

    Serial.print("[WIFI] Conectando a ");
    Serial.println(wifiSSID);

    WiFi.begin(
      wifiSSID.c_str(),
      wifiPassword.c_str()
    );
  }
}

void mantenerWiFi() {

  if (wifiSSID.length() == 0) {
    return;
  }

  if (wifiConectado()) {
    return;
  }

  unsigned long ahora = millis();

  if (
    ahora - ultimoIntentoWiFi <
    INTERVALO_RECONEXION
  ) {
    return;
  }

  ultimoIntentoWiFi = ahora;

  Serial.println("[WIFI] Reconectando...");

  WiFi.disconnect();

  WiFi.begin(
    wifiSSID.c_str(),
    wifiPassword.c_str()
  );
}

// ==========================================================
// NTP
// ==========================================================

void iniciarNTP() {

  setenv("TZ", CHILE_TZ, 1);
  tzset();

  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.google.com",
    "time.cloudflare.com"
  );

  Serial.println("[NTP] Iniciado");
}

void mantenerHora() {

  if (!wifiConectado()) {
    return;
  }

  unsigned long ahora = millis();

  if (
    ahora - ultimaRevisionHora <
    INTERVALO_REVISION_HORA
  ) {
    return;
  }

  ultimaRevisionHora = ahora;

  Serial.print("[NTP] ");
  Serial.println(fechaChile());
}

// ==========================================================
// MICROSD
// ==========================================================

void crearArchivoSiNoExiste(
  const char* ruta,
  const char* cabecera
) {

  if (!sdOK) {
    return;
  }

  if (SD.exists(ruta)) {
    return;
  }

  File archivo = SD.open(
    ruta,
    FILE_WRITE
  );

  if (!archivo) {
    return;
  }

  archivo.println(cabecera);
  archivo.close();
}

bool montarSD() {

  if (!SD.begin(SD_CS, SPI, 4000000)) {
    sdOK = false;
    Serial.println("[SD] No disponible");
    return false;
  }

  if (SD.cardType() == CARD_NONE) {
    sdOK = false;
    Serial.println("[SD] Tarjeta no detectada");
    return false;
  }

  sdOK = true;

  crearArchivoSiNoExiste(
    CSV_FILE,
    "fecha_chile,fecha_utc,epoch_utc,tiempo_ms,"
    "Conector_1_A,Conector_2_A,"
    "Conector_3_A,Conector_4_A"
  );

  crearArchivoSiNoExiste(
    QUEUE_FILE,
    "fecha_utc,epoch_utc,"
    "Conector_1_A,Conector_2_A,"
    "Conector_3_A,Conector_4_A"
  );

  // Verifica que el archivo de historial realmente se pueda abrir.
  File prueba = SD.open(CSV_FILE, FILE_READ);

  if (!prueba) {
    sdOK = false;
    Serial.println("[SD] Error verificando CSV");
    return false;
  }

  prueba.close();

  Serial.println("[SD] Detectada y operativa");
  return true;
}

void iniciarSD() {

  SPI.begin(
    SD_SCK,
    SD_MISO,
    SD_MOSI,
    SD_CS
  );

  montarSD();

  ultimaRevisionSD = millis();
}

void revisarSD() {

  unsigned long ahora = millis();

  if (
    ahora - ultimaRevisionSD <
    INTERVALO_REVISION_SD
  ) {
    return;
  }

  ultimaRevisionSD = ahora;

  if (sdOK) {

    bool presente =
      (SD.cardType() != CARD_NONE);

    File prueba;

    if (presente) {

      prueba = SD.open(
        CSV_FILE,
        FILE_READ
      );

      presente = (bool)prueba;

      if (prueba) {
        prueba.close();
      }
    }

    if (!presente) {

      sdOK = false;

      Serial.println(
        "[SD] Desconectada o inaccesible"
      );
    }
  }

  if (!sdOK) {

    Serial.println(
      "[SD] Intentando recuperar tarjeta..."
    );

    montarSD();
  }
}

// ==========================================================
// ADS1115
// ==========================================================

void iniciarADC() {

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  ads1OK = ads1.begin(0x48);
  ads2OK = ads2.begin(0x49);

  if (ads1OK) {

    ads1.setGain(GAIN_ONE);

    ads1.setDataRate(
      RATE_ADS1115_860SPS
    );

    Serial.println("[ADC] ADS1 detectado");

  } else {

    Serial.println("[ADC] ADS1 no detectado");
  }

  if (ads2OK) {

    ads2.setGain(GAIN_ONE);

    ads2.setDataRate(
      RATE_ADS1115_860SPS
    );

    Serial.println("[ADC] ADS2 detectado");

  } else {

    Serial.println("[ADC] ADS2 no detectado");
  }

  canalDisponible[0] = ads2OK;
  canalDisponible[1] = ads2OK;

  canalDisponible[2] = ads1OK;
  canalDisponible[3] = ads1OK;
}

float medirCorriente(
  Adafruit_ADS1115& ads,
  uint8_t canal
) {

  double suma = 0;
  double sumaCuadrados = 0;

  for (int i = 0; i < NUM_MUESTRAS; i++) {

    int16_t lectura;

    if (canal == 0) {

      lectura =
        ads.readADC_Differential_0_1();

    } else {

      lectura =
        ads.readADC_Differential_2_3();
    }

    float voltaje =
      ads.computeVolts(lectura);

    suma += voltaje;

    sumaCuadrados +=
      (double)voltaje * voltaje;
  }

  double promedio =
    suma / NUM_MUESTRAS;

  double varianza =
    sumaCuadrados / NUM_MUESTRAS -
    promedio * promedio;

  if (varianza < 0) {
    varianza = 0;
  }

  float voltajeRMS =
    sqrt(varianza);

  return voltajeRMS *
    AMPERIOS_POR_VOLTIO;
}

void actualizarCorrientes() {

  // Medicion base y calibracion individual de cada conector.
  const float FACTOR_CALIBRACION[4] = {
    0.8602f, 0.8636f, 0.8600f, 0.8558f
  };

  // Conector 1 = ADS2 A2-A3
  corriente[0] = ads2OK
    ? medirCorriente(ads2, 1) * FACTOR_CALIBRACION[0]
    : NAN;

  // Conector 2 = ADS2 A0-A1
  corriente[1] = ads2OK
    ? medirCorriente(ads2, 0) * FACTOR_CALIBRACION[1]
    : NAN;

  // Conector 3 = ADS1 A2-A3
  corriente[2] = ads1OK
    ? medirCorriente(ads1, 1) * FACTOR_CALIBRACION[2]
    : NAN;

  // Conector 4 = ADS1 A0-A1
  corriente[3] = ads1OK
    ? medirCorriente(ads1, 0) * FACTOR_CALIBRACION[3]
    : NAN;

  // Elimina lecturas residuales inferiores a 0,03 A.
  for (int i = 0; i < 4; i++) {

    if (
      isfinite(corriente[i]) &&
      corriente[i] < 0.030f
    ) {
      corriente[i] = 0.0f;
    }
  }
}

// ==========================================================
// HISTORIAL CSV
// ==========================================================

void registrarMedicionLocal() {

  if (!sdOK) {
    return;
  }

  File archivo = SD.open(
    CSV_FILE,
    FILE_APPEND
  );

  if (!archivo) {
    return;
  }

  archivo.print(fechaChile());
  archivo.print(",");

  archivo.print(fechaUTC());
  archivo.print(",");

  if (horaValida()) {

    archivo.print(
      (uint32_t)time(nullptr)
    );
  }

  archivo.print(",");
  archivo.print(millis());

  for (int i = 0; i < 4; i++) {

    archivo.print(",");

    if (isfinite(corriente[i])) {

      archivo.print(
        corriente[i],
        3
      );
    }
  }

  archivo.println();
  archivo.close();

  registrosLocales++;
}

// ==========================================================
// COLA REMOTA
// ==========================================================

void agregarMedicionPendiente() {

  if (!sdOK || !horaValida()) {
    return;
  }

  File archivo = SD.open(
    QUEUE_FILE,
    FILE_APPEND
  );

  if (!archivo) {
    return;
  }

  archivo.print(fechaUTC());
  archivo.print(",");

  archivo.print(
    (uint32_t)time(nullptr)
  );

  for (int i = 0; i < 4; i++) {

    archivo.print(",");

    if (isfinite(corriente[i])) {

      archivo.print(
        corriente[i],
        3
      );
    }
  }

  archivo.println();
  archivo.close();
}

bool filaAJSON(
  String fila,
  String& json
) {

  fila.trim();

  String campos[6];
  int inicio = 0;

  for (int i = 0; i < 5; i++) {

    int separador =
      fila.indexOf(',', inicio);

    if (separador < 0) {
      return false;
    }

    campos[i] = fila.substring(
      inicio,
      separador
    );

    inicio = separador + 1;
  }

  campos[5] = fila.substring(inicio);

  if (
    campos[0].length() == 0 ||
    campos[1].length() == 0
  ) {
    return false;
  }

  json = "{";

  json += "\"dispositivo\":\"";
  json += escaparJSON(deviceID);
  json += "\",";

  json += "\"fecha_hora\":\"";
  json += escaparJSON(campos[0]);
  json += "\",";

  json += "\"epoch_utc\":";
  json += campos[1];
  json += ",";

  for (int i = 0; i < 4; i++) {

    json += "\"conector_";
    json += String(i + 1);
    json += "\":";

    String valor = campos[i + 2];
    valor.trim();

    if (valor.length() > 0) {
      json += valor;
    } else {
      json += "null";
    }

    if (i < 3) {
      json += ",";
    }
  }

  json += "}";

  return true;
}

// ==========================================================
// API HTTPS
// ==========================================================

bool apiConfigurada() {

  return (
    apiURL.startsWith("https://") &&
    apiToken.length() > 0 &&
    deviceID.length() > 0
  );
}

bool enviarJSON(const String& json) {

  if (!wifiConectado()) {

    ultimoEstadoAPI = "Sin Wi-Fi";
    return false;
  }

  if (!apiConfigurada()) {

    ultimoEstadoAPI = "Sin configurar";
    return false;
  }

  WiFiClientSecure cliente;

  // ATENCION:
  // Desactiva la verificacion del certificado.
  // Utilizar solamente durante las pruebas.

  cliente.setInsecure();
  cliente.setTimeout(8);

  HTTPClient http;

  if (!http.begin(cliente, apiURL)) {

    ultimoEstadoAPI = "Error HTTPS";

    ultimoErrorAPI =
      "No se pudo iniciar HTTP";

    return false;
  }

  http.setTimeout(8000);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  http.addHeader(
    "Authorization",
    "Bearer " + apiToken
  );

  http.addHeader(
    "X-Device-ID",
    deviceID
  );

  int codigo = http.POST(json);

  ultimoHTTP = codigo;

  bool correcto =
    codigo >= 200 && codigo < 300;

  if (correcto) {

    ultimoEstadoAPI = "Conectado";
    ultimoErrorAPI = "";

    registrosEnviados++;

    Serial.print("[API] HTTP ");
    Serial.println(codigo);

  } else {

    ultimoEstadoAPI = "Error";

    String respuesta =
      http.getString();

    ultimoErrorAPI =
      "HTTP " + String(codigo) +
      ": " + respuesta.substring(0, 120);

    Serial.print("[API] ");
    Serial.println(ultimoErrorAPI);
  }

  http.end();

  return correcto;
}

void procesarCola() {

  if (!sdOK) {
    return;
  }

  if (!wifiConectado()) {
    return;
  }

  if (!apiConfigurada()) {
    return;
  }

  File archivo = SD.open(
    QUEUE_FILE,
    FILE_READ
  );

  if (!archivo) {
    return;
  }

  uint32_t tamano = archivo.size();

  if (queueOffset > tamano) {

    queueOffset = 0;

    preferences.putUInt(
      "queue_pos",
      queueOffset
    );
  }

  if (!archivo.seek(queueOffset)) {

    archivo.close();
    return;
  }

  const int MAX_ENVIO_LOTE = 12;
  int enviados = 0;

  while (
    archivo.available() &&
    enviados < MAX_ENVIO_LOTE
  ) {

    uint32_t inicioFila =
      archivo.position();

    String fila =
      archivo.readStringUntil('\n');

    uint32_t finFila =
      archivo.position();

    fila.trim();

    if (fila.length() == 0) {

      queueOffset = finFila;

      preferences.putUInt(
        "queue_pos",
        queueOffset
      );

      continue;
    }

    if (fila.startsWith("fecha_utc,")) {

      queueOffset = finFila;

      preferences.putUInt(
        "queue_pos",
        queueOffset
      );

      continue;
    }

    String json;

    if (!filaAJSON(fila, json)) {

      Serial.println(
        "[API] Fila invalida"
      );

      queueOffset = inicioFila;
      break;
    }

    if (!enviarJSON(json)) {

      queueOffset = inicioFila;
      break;
    }

    queueOffset = finFila;

    preferences.putUInt(
      "queue_pos",
      queueOffset
    );

    enviados++;

    delay(1);
  }

  archivo.close();
}

// ==========================================================
// ESTADO JSON
// ==========================================================

String generarEstadoJSON() {

  String json = "{";

  json += "\"fecha_chile\":\"";
  json += escaparJSON(fechaChile());
  json += "\",";

  json += "\"fecha_utc\":\"";
  json += escaparJSON(fechaUTC());
  json += "\",";

  json += "\"hora_valida\":";
  json += horaValida() ? "true" : "false";
  json += ",";

  json += "\"wifi_conectado\":";
  json += wifiConectado() ? "true" : "false";
  json += ",";

  json += "\"wifi_ssid\":\"";

  if (wifiConectado()) {
    json += escaparJSON(WiFi.SSID());
  }

  json += "\",";

  json += "\"ip_wifi\":\"";

  if (wifiConectado()) {
    json += WiFi.localIP().toString();
  }

  json += "\",";

  json += "\"sd_disponible\":";
  json += sdOK ? "true" : "false";
  json += ",";

  json += "\"api_estado\":\"";
  json += escaparJSON(ultimoEstadoAPI);
  json += "\",";

  json += "\"api_error\":\"";
  json += escaparJSON(ultimoErrorAPI);
  json += "\",";

  json += "\"api_http\":";
  json += String(ultimoHTTP);
  json += ",";

  json += "\"registros_locales\":";
  json += String(registrosLocales);
  json += ",";

  json += "\"registros_enviados\":";
  json += String(registrosEnviados);
  json += ",";

  json += "\"corrientes\":[";

  for (int i = 0; i < 4; i++) {

    json += numeroJSON(corriente[i]);

    if (i < 3) {
      json += ",";
    }
  }

  json += "],";

  json += "\"adc_disponibles\":[";

  for (int i = 0; i < 4; i++) {

    json += canalDisponible[i]
      ? "true"
      : "false";

    if (i < 3) {
      json += ",";
    }
  }

  json += "]}";

  return json;
}

// ==========================================================
// SERVIDOR WEB V7
// ==========================================================

void configurarServidor() {

  /*
    CAMBIO PRINCIPAL DE V7:

    Se elimino por completo permitirAccesoWeb().

    No se comprueba la IP del cliente ni la interfaz
    de llegada.

    Tampoco se solicita usuario o contrasena HTTP.
  */

  // PAGINA PRINCIPAL

  server.on("/", HTTP_GET, []() {

    server.send_P(
      200,
      "text/html; charset=utf-8",
      PAGINA_HTML
    );
  });

  // ESTADO EN TIEMPO REAL

  server.on("/api/status", HTTP_GET, []() {

    server.send(
      200,
      "application/json",
      generarEstadoJSON()
    );
  });

  // CONFIGURACION ACTUAL

  server.on("/api/config", HTTP_GET, []() {

    String json = "{";

    json += "\"wifi_ssid\":\"";
    json += escaparJSON(wifiSSID);
    json += "\",";

    json += "\"api_url\":\"";
    json += escaparJSON(apiURL);
    json += "\",";

    json += "\"device_id\":\"";
    json += escaparJSON(deviceID);
    json += "\",";

    json += "\"upload_interval\":";
    json += String(uploadIntervalSeconds);

    json += "}";

    server.send(
      200,
      "application/json",
      json
    );
  });

  // GUARDAR WIFI DEL ROUTER

  server.on(
    "/api/config/wifi",
    HTTP_POST,
    []() {

      String nuevoSSID =
        server.arg("ssid");

      String nuevaPassword =
        server.arg("password");

      nuevoSSID.trim();

      if (
        nuevoSSID.length() == 0 ||
        nuevoSSID.length() > 32
      ) {

        server.send(
          400,
          "text/plain",
          "SSID invalido"
        );

        return;
      }

      if (nuevaPassword.length() > 63) {

        server.send(
          400,
          "text/plain",
          "Contrasena demasiado larga"
        );

        return;
      }

      bool cambioRed =
        nuevoSSID != wifiSSID;

      wifiSSID = nuevoSSID;

      if (nuevaPassword.length() > 0) {

        wifiPassword = nuevaPassword;

      } else if (cambioRed) {

        wifiPassword = "";
      }

      guardarConfiguracion();

      server.send(
        200,
        "text/plain",
        "Wi-Fi guardado. Reiniciando ESP32..."
      );

      delay(500);
      ESP.restart();
    }
  );

  // GUARDAR API REMOTA

  server.on(
    "/api/config/remote",
    HTTP_POST,
    []() {

      String nuevaURL =
        server.arg("url");

      String nuevoID =
        server.arg("device_id");

      String nuevoToken =
        server.arg("token");

      String intervalo =
        server.arg("interval");

      nuevaURL.trim();
      nuevoID.trim();

      if (
        nuevaURL.length() > 0 &&
        !nuevaURL.startsWith("https://")
      ) {

        server.send(
          400,
          "text/plain",
          "La API debe utilizar HTTPS"
        );

        return;
      }

      if (nuevaURL.length() > 500) {

        server.send(
          400,
          "text/plain",
          "URL demasiado larga"
        );

        return;
      }

      if (
        nuevoID.length() == 0 ||
        nuevoID.length() > 64
      ) {

        server.send(
          400,
          "text/plain",
          "ID de dispositivo invalido"
        );

        return;
      }

      if (nuevoToken.length() > 300) {

        server.send(
          400,
          "text/plain",
          "Token demasiado largo"
        );

        return;
      }

      uint32_t nuevoIntervalo =
        intervalo.toInt();

      if (
        nuevoIntervalo < 5 ||
        nuevoIntervalo > 3600
      ) {

        server.send(
          400,
          "text/plain",
          "Intervalo permitido: 5 a 3600 segundos"
        );

        return;
      }

      bool cambioDestino =
        nuevaURL != apiURL ||
        nuevoID != deviceID;

      apiURL = nuevaURL;
      deviceID = nuevoID;

      if (nuevoToken.length() > 0) {
        apiToken = nuevoToken;
      }

      uploadIntervalSeconds =
        nuevoIntervalo;

      guardarConfiguracion();

      if (cambioDestino) {

        queueOffset = 0;

        preferences.putUInt(
          "queue_pos",
          queueOffset
        );
      }

      ultimoEstadoAPI = "Configurado";

      server.send(
        200,
        "text/plain",
        "Configuracion API guardada"
      );
    }
  );

  // DESCARGAR CSV

  server.on("/download", HTTP_GET, []() {

    if (
      !sdOK ||
      SD.cardType() == CARD_NONE ||
      !SD.exists(CSV_FILE)
    ) {

      sdOK = false;

      server.send(
        503,
        "text/plain",
        "MicroSD no disponible; descarga CSV bloqueada"
      );

      return;
    }

    File archivo = SD.open(
      CSV_FILE,
      FILE_READ
    );

    if (!archivo) {

      sdOK = false;

      server.send(
        503,
        "text/plain",
        "MicroSD desconectada o CSV inaccesible"
      );

      return;
    }

    server.sendHeader(
      "Content-Disposition",
      "attachment; filename=mediciones.csv"
    );

    server.streamFile(
      archivo,
      "text/csv"
    );

    archivo.close();
  });

  // REINICIAR HISTORIAL CSV

  server.on(
    "/api/resetcsv",
    HTTP_POST,
    []() {

      if (!sdOK) {

        server.send(
          500,
          "text/plain",
          "MicroSD no disponible"
        );

        return;
      }

      SD.remove(CSV_FILE);

      crearArchivoSiNoExiste(
        CSV_FILE,
        "fecha_chile,fecha_utc,epoch_utc,tiempo_ms,"
        "Conector_1_A,Conector_2_A,"
        "Conector_3_A,Conector_4_A"
      );

      registrosLocales = 0;

      server.send(
        200,
        "text/plain",
        "Historial CSV reiniciado"
      );
    }
  );

  // RUTAS DESCONOCIDAS

  server.onNotFound([]() {

    server.send(
      404,
      "text/plain",
      "Ruta no encontrada"
    );
  });

  server.begin();

  Serial.println("[WEB] Servidor iniciado");
  Serial.println("[WEB] Sin autenticacion");
  Serial.println("[WEB] Sin filtro de IP");
}

// ==========================================================
// SETUP
// ==========================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" MONITOR DE CORRIENTE ESP32 V7");
  Serial.println("================================");

  cargarConfiguracion();

  iniciarADC();

  iniciarSD();

  iniciarWiFi();

  iniciarNTP();

  configurarServidor();

  ultimaMedicion = millis();
  ultimoEnvio = millis();
  ultimoIntentoWiFi = millis();
  ultimaRevisionHora = millis();

  Serial.println();
  Serial.println("[SISTEMA] Listo");
  Serial.println("[WEB] http://192.168.4.1");
}

// ==========================================================
// LOOP
// ==========================================================

void loop() {

  server.handleClient();

  mantenerWiFi();
  mantenerHora();
  revisarSD();

  unsigned long ahora = millis();

  // MEDICIONES LOCALES

  if (
    ahora - ultimaMedicion >=
    INTERVALO_MEDICION
  ) {

    ultimaMedicion = ahora;

    actualizarCorrientes();

    registrarMedicionLocal();

    Serial.println();
    Serial.println("-------- MEDICIONES --------");

    Serial.print("Fecha: ");
    Serial.println(fechaChile());

    for (int i = 0; i < 4; i++) {

      Serial.print("Conector ");
      Serial.print(i + 1);
      Serial.print(": ");

      if (isfinite(corriente[i])) {

        Serial.print(corriente[i], 3);
        Serial.println(" A");

      } else {

        Serial.println("ADC no disponible");
      }
    }

    Serial.println("----------------------------");

    if (horaValida()) {

      uint64_t bloque =
        (uint64_t)time(nullptr) /
        uploadIntervalSeconds;

      if (bloque != ultimoBloqueRemoto) {

        ultimoBloqueRemoto = bloque;

        agregarMedicionPendiente();
      }
    }
  }

  // ENVIO REMOTO

  if (
    ahora - ultimoEnvio >=
    uploadIntervalSeconds * 1000UL
  ) {

    ultimoEnvio = ahora;

    procesarCola();
  }

  delay(1);
}
