# Iteración 2 - Primera Mejora Significativa

## Información General

**Fecha:** [23/11/2026]  
**Versión:** v2.0  
**Estado:** Prototipo funcional mejorado  
**Avance asociado:** Avance #2

---

## Descripción

Segunda versión del prototipo, incorporando feedback de la iteración 1 y del Avance #1. Incluye mejoras significativas en funcionalidad y primeros pasos en diseño de encapsulado.

**Objetivos de esta iteración:**
- Corregir problemas identificados en v1
- Agregar funcionalidades adicionales
- Iniciar diseño 3D del encapsulado
- Realizar primeras pruebas con usuarios

---

## Cambios Respecto a v1

### Hardware
**Agregado:**
- 4 resistencias de 100 KΩ
- Alargador casero de cables separados, permitiendo conexión con cualquier objeto eléctrico de cualquier tipo de cable.
- Diodo LED rojo

**Mejora principal**
- Las resistencias permiten que el sistema no tenga problemas con la detección de ruido y corriente real.
- El alargador casero soluciona la dependencia del uso de cables separados para la lectura verídica del lector de corriente.
- El LED permite al usuario saber si el dispositivo está encendido o no.

### Software

**Bugs corregidos:**
- [Bug 1 de v1]
- Railway presentó problemas para crear una cuenta, por lo cual se prefirio moverse a otra plataforma como Supabase
- No obstante, es importante considerar que este cambio nos genera un par de restricciones en el manejo de la página web y la comunicación con el ESP32. Como en Railway las funciones eran de pago y las de Supabase eran gratuitas, nuestro software está limitado de tal manera que la información se encuentra en el computador y además en un dispositivo móvil. No hemos logrado unir ambos planillas de información en una sola.

**Mejora principal**
- Conexión a nube Supabase
- Creación de página web que contiene varios elementos de información, como la energía total consumida en un periodo determinado, el costo estimado en $, el promedio diario de kWh, como también la creación de gráficos de la evolución del consumo y comparación mensual.

### Diseño Mecánico
- Primer diseño de encapsulado en Fusion 360
- Prototipo impreso en 3D
- Integración de componentes en carcasa

---

## Resultados de Testing

### Funcionalidad Lograda
✅ [Lectura de Corriente]  
✅ [Traspaso de lectura a Kwh y pesos chilenos]  
✅ [Detección de ruido: Solucionado]  
- (+/-) [Función 4 - Legibilidad de la página web]
