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
- El sistema es muy dependiente por el hecho de que los cables de conexión al sistema sean separados y no juntos.
- Para mejor usabilidad, es mejor implementar una nube para así ver el sistema de manera remota.

### Versión 2 (Iteración 2) - Primera Mejora
**Fecha:** [23/11/2026]  
**Estado:** Prototipo funcional mejorado

**Mejoras implementadas:**
- Se agregaron 4 resistencias de 100 KOHM, solucionando el problema del sistema en su incapacidad de diferenciar ruido con corriente real.

**Problemas pendientes:**
- [Problema 1]
- [Problema 2]

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
- Adición de 4 resistencias de 100 KOHM

**De v2 a v3:**
- Cambio: [Descripción]
- Razón: [Justificación técnica]
- Impacto: [Mejora cuantitativa]

### Software
**De v1 a v2:**
- Conexión a nube Supabase
  
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
1. [Comentario profesor/ayudante]
   - Acción tomada: [Descripción]

### Feedback de Usuarios → Cambios en v3
1. [Comentario común de usuarios]
   - Acción tomada: [Descripción]

---

## Lecciones aprendidas

1. 
2. 
3. 
