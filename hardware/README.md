# Hardware - Documentación

## Contenido
Esta carpeta contiene toda la documentación técnica del hardware del proyecto, incluyendo la justificación de cada componente y el montaje.

## Carpetas

- `esquemas/` tiene el esquema general del sistema arduino y el detalle de las conexiones.
- `bom/` contiene la lista de materiales con cantidades y costo total en pesos chilenos.
- `fotos/` reúne las fotos del prototipo armado, incluyendo su instalación en el encapsulado 3D.

## Componentes principales y por qué se eligieron

| Componente | Función | Justificación técnica |
|---|---|---|
|ESP32| Procesa los datos de corriente obtenidos por el sistema y los transmite a la plataforma de monitoreo mediante Wi-Fi. | Actúa como el controlador principal del sistema. Su capacidad de procesamiento y conectividad inalámbrica permite calcular variables eléctricas y enviar la información al usuario en tiempo real, facilitando el seguimiento del consumo energético. |
| Transformador de corriente | Medición de corriente alterna no invasiva. | Permite medir el consumo eléctrico sin intervenir directamente el cableado de la vivienda, aumentando la seguridad de la instalación y facilitando la implementación en hogares. |
| Tira de Conector JST | Permite conexión y desconexión de componentes en el protoboard. | Facilita el montaje, mantenimiento y reemplazo de componentes, proporcionando conexiones seguras y reduciendo errores de cableado durante el desarrollo y operación del dispositivo. |
| Conector TRRS | Actúa como la conexión para el sensor de corriente. | Proporciona una interfaz física confiable para conectar el transformador de corriente al circuito de medición, asegurando una transmisión estable de la señal eléctrica capturada por el sensor. |
|Resistencias| Completa el circuito eléctrico, absorbiendo corriente eléctrica diminuta| Las resistencias le permite al sistema poder diferenciar entre corriente real y ruido. Cuando no hay resistencia aplicada y el sistema es desconectado, sigue midiendo una corriente que fluctúa entre valores pequeños y muy grandes. La resistencia soluciona esto al absorber la corriente remanente, así el sistema inmediatamente marca 0 cuando se desconecta.|
| Módulo Ads1115 | Lee la información del sensor y la convierte en datos digitales para la ESP32. | Su convertidor analógico-digital de 16 bits ofrece mayor resolución y precisión que el ADC interno de la ESP32, permitiendo mediciones de corriente más exactas y, por consiguiente, estimaciones de consumo y costo más confiables para el usuario. |

## Instrucciones de ensamble

### Montaje mecánico

1. Colocar protoboard dentro del encapsulado, alineando las salidas de conectores/módulos con los respectivos puertos
2. Cerrar encapsulado con caja y atornillar desde la parte inferior hasta la parte superior
3. Añadir batería portátil y amarrarla, utilizando los espacios triangulares de la caja como soporte

### Montaje eléctrico

1. Instalar cada componente siguiendo el esquema (en carpeta esquemas)
2. Para inicializar, conectar batería con puerto de alimentación a la ESP32
3. El lector de corriente va conectado a cualquiera de los 4 puertos de los conectores TRRS
