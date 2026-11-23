# Código Fuente

## Descripción del proyecto

Código fuente del sistema de monitoreo de corriente eléctrica basado en ESP32. Incluye la adquisición y procesamiento de mediciones, servidor web local, almacenamiento de registros y comunicación con Supabase.

## Archivos principales

### `main.ino`

Archivo principal del proyecto, encargado de coordinar el funcionamiento del sistema.

**Funciones principales:**
- Inicialización del sistema.
- Adquisición y procesamiento de mediciones.
- Cálculo de corriente RMS.
- Gestión de conexiones Wi-Fi.
- Administración del servidor web.
- Almacenamiento de datos en microSD.
- Envío de mediciones a Supabase.

### `pagina_web.h`

Contiene el código de la interfaz web alojada en el ESP32.

**Funciones principales:**
- Visualización de mediciones en tiempo real.
- Presentación de los cuatro conectores.
- Gráficos independientes por conector.
- Opciones para mostrar u ocultar gráficos.
- Consulta y descarga de registros CSV.
- Configuración de parámetros del sistema.

## Comunicaciones

El ESP32 contempla dos modalidades de conexión:

- **Punto de acceso Wi-Fi:** permite acceder directamente al servidor web del ESP32.
- **Red Wi-Fi externa:** permite conectar el dispositivo a Internet y transmitir mediciones a Supabase.

La comunicación con Supabase se realiza mediante solicitudes HTTP.

## Almacenamiento de datos

El sistema permite guardar mediciones localmente en archivos CSV mediante una tarjeta microSD.

También transmite registros a Supabase, donde se almacenan para su posterior consulta desde el dashboard web.

## Base de datos

La comunicación con Supabase utiliza la tabla `public.mediciones`.

Los registros contienen:
- Identificación del dispositivo.
- Fecha y hora de la medición.
- Valores de corriente de los cuatro conectores.
- Fecha y hora de recepción.

## Dashboard web

El dashboard permite consultar y analizar los datos almacenados en Supabase.

**Funciones principales:**
- Consulta de dispositivos registrados.
- Selección de fechas y períodos.
- Visualización de mediciones históricas.
- Comparación entre conectores.
- Análisis de tendencias de consumo.
- Estimación de energía y costos eléctricos.

## Tecnologías utilizadas

- Arduino IDE y C++
- HTML
- CSS
- JavaScript
- Wi-Fi y HTTP
- Supabase (Base de datos)
- GitHub (Repositorio)
- Netlify (Pagina web)

## Declaración de uso de IA

Para la parte del código se declara ver utilizado ChatGPT para hacerlo, se le pidieron instrucciones para ir generando el código a base de lo que se necesitaba y orientándolo en todo el proceso.
