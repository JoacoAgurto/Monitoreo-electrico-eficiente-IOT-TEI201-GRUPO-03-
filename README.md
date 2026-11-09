### Información del Equipo
- **Integrantes:**
  - Vasco Urrutia - Encargado de hardware y software
  - Joaquín Agurto - Encargado de Documentación y Recopilado de Datos
  - Ignacio Mora - Encargado de modelado 3D
  
- **ODS Seleccionado:**
ODS 7: Energía Asequible y No Contaminante
- **Problema a resolver:**
Ayudar a hogares no considerados dentro del subsidio eléctrico en Chile, regulando sus gastos a través de un monitoreo eléctrico que evite situaciones de sobreconsumo que agraven su situación económica, considerando las altas tarifas eléctricas de la actualidad.

### Descripción del Proyecto
Este proyecto provee un sistema de monitoreo eléctrico eficiente que lee la cantidad de energía utilizada por un dispositivo tecnológico de interés. A base Ardiuino, el sistema tiene la capacidad de transferir los datos obtenidos a una página web, donde se logra visualizar el consumo en tiempo real y una predicción de gasto por kWh. Con esta información, los usuarios podrán leer y entender fácilmente la cantidad de energía que están consumiendo, y por consiguiente, el dinero que tendrían que pagar a fin de mes. Esto les servirá para tomar decisiones informadas y regular su consumo.

### Estado del Proyecto
- **Versión actual:** v3.0
- **Última actualización:** [23/11/2026]
- **Estado:** Prototipo final y mejorado
---

## Resultados Clave

| Medida | Resultado | Origen |
|---|---|---|
| Lectura de corriente constante y sin interrupciones |  |  |
| Error de detección de ruido |  |  |
| Capacidad de predecir gasto en kWH | 0.62 % de error medio absoluto | Real, simulación |
| Gráficos de consumo frente a tiempo legibles | 0.12 mm de error medio absoluto | Real, simulación |
| Pruebas con 5 usuarios | Ver `testing/` | |

---

##  Estructura del Repositorio
```
├── hardware/          # Circuitos, esquemas y BOM
├── software/          # Código Arduino y librerías
├── diseno_3d/        # Modelos Fusion 360 y renders
├── testing/          # Validación con usuarios
├── documentacion/    # Reportes y presentación
└── iteraciones/      # Historial de versiones
```

---

##  Inicio Rápido

### Requisitos Previos
- Arduino IDE 2.x
- Fusion 360 para abrir los archivos `.f3d`
- 

### Instalación
1. Clonar el repositorio
2. Instalar librerías (ver `software/librerias/`)
3. Cargar código en ESP32/Arduino
4. [Pasos adicionales]

---

##  Checklist de Entrega

### Hardware ✓
- [ ] Esquema del circuito (Fritzing/Wokwi)
- [ ] BOM completo
- [ ] Fotos de alta resolución

### Software ✓
- [ ] Código comentado
- [ ] Librerías documentadas
- [ ] Manual de instalación

### Diseño 3D ✓
- [ ] Archivos Fusion 360 (.f3d) con TODOS los componentes
- [ ] Renders de alta calidad (1+ ángulos)
- [ ] Planos técnicos

### Testing ✓
- [ ] Reporte de testing con usuarios (mín. 5)
- [ ] Evidencias fotográficas/videos
- [ ] Datos cuantitativos

### Documentación ✓
- [ ] Reporte final (máx. 15 páginas)
- [ ] Presentación (10 min)

---


---

##  Licencia
Este proyecto es desarrollado como parte del curso TEI201 - Facultad de Ingenieria y Ciencias - Universidad Adolfo Ibañez
