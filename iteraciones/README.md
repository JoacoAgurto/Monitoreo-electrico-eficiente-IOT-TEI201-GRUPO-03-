# Historial de Iteraciones

## Propósito

Esta carpeta documenta la evolución del prototipo a través de las diferentes versiones desarrolladas durante el curso.

---

## Evolución del Diseño

### Versión 1 (Iteración 1) - Concepto Inicial
**Fecha:** [19/10/2026]  
**Estado:** Prototipo alpha - Prueba de concepto

**Características:**
- El sistema es a base de conexiones de arduino / sensores montados en un protoboard, capaces de leer corriente eléctrica y mandar esos datos a una página web.
- El sistema es muy dependiente por el hecho de que los cables de conexión al sistema sean separados y no juntos. Eso significa que no puede medir la corriente de cualquier objeto, debido a que la mayoría de los cables son juntos.
- Para mejor usabilidad, es mejor implementar una nube para así ver el sistema de manera remota.

### Versión 2 (Iteración 2) - Primera Mejora
**Fecha:** [23/11/2026]  
**Estado:** Prototipo funcional mejorado

**Mejoras implementadas:**
- Se agregaron 4 resistencias de 100 KOHM, solucionando el problema del sistema en su incapacidad de diferenciar ruido con corriente real.
- El código de la ESP32 ahora es capaz de convertir la corriente en KWh y $ en tiempo real según una tarifa especificada por el usuario. También permite guardar los datos en CSV, como también comparar consumo por días o meses, posibilitando decisiones más informadas.
- Se utilizó un alargador casero de cables separados. Así, el lector de corriente es capaz de medir el alargador y la corriente proveniente de cualquier objeto con cualquier tipo de cable.
- Se agregó un LED para indicar cuando el dispositivo está encendido.
- El uso de Supabase permite visualizar los datos en una página web de manera fácil y legible

### Versión 3 (Iteración 3) - Versión Final
**Fecha:** [XX/XX/2025]  
**Estado:** Prototipo de alta resolución

**Mejoras finales:**
- [Mejora 1]
- [Mejora 2]

**Resultado:**
- Sistema robusto y probado
- Listo para piloto en terreno

---

## Justificación de Cambios

### Hardware
**De v1 a v2:**
- Adición de 4 resistencias de 100 KOHM (soluciona detección de ruido)
- Alargador de cables separados (garantiza lectura universal de objetos de cualquier tipo de cable)
- Diodo LED rojo (permite al usuario conocer si el dispositivo está conectado y encendido para su funcionamiento)

**De v2 a v3:**
- Cambio: [Descripción]
- Razón: [Justificación técnica]
- Impacto: [Mejora cuantitativa]

### Software
**De v1 a v2:**
- Conexión a nube Supabase (Railway presentaba fallas significativas)
  
**De v2 a v3:**
- [Cambios principales]

### Diseño 3D
**De v1 a v2:**
- [Cambios en encapsulado]

**De v2 a v3:**
- [Cambios finales]

---

## Incorporación de Feedback

### Feedback del Avance #1 → Cambios en v2
1. [Comentario profesor/ayudante]:
   - Utilizar un servidor global para garantizar un acceso al sistema de manera remota, facilitando su uso para el usuario en vez de estar en cercanía con el sistema montado en el protoboard. La importancia de esto para el usuario es que es capaz de saber la cantidad de energía que está consumiendo en su hogar cuando NO está en ella.
   - Acción tomada: Usar Supabase

### Feedback del Avance #2 → Cambios en v3
1. [Comentario profesor/ayudante]:
   - ¿Por qué los datos se ven en el computador y además en el celular? ¿No sería más simple que toda la arquitectura del software y los datos importantes de la página web se vean solamente en un dispositivo?
   - Acción tomada: Averiguar si Supabase permite sintetizar y poner la información en un sólo dispositivo para el funcionamiento correcto del dispositivo, dentro de sus límites de uso gratuito. Railway era capaz de hacer eso, pero sus errores relacionados a inicio de sesión no permitió más sus opciones adicionales de pago nos complicó.

### Feedback de Usuarios → Cambios en v3
1. [Comentario común de usuarios]
   - Acción tomada: [Descripción]

---

## Lecciones aprendidas

1. El modelaje 3D requiere mucha revisión antes de mandar a imprimir. Pequeños desvíos de diseño de tamaño minúsculo afectan completamente a la conexión con el prototipo
2. Es importante que la información de la nube Supabase sea accesible en un sólo dispositivo en vez de dos. Facilita mucho más su uso para el futuro usuario.
