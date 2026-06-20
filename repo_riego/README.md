# Sistema de Riego IoT (ESP32)

Sistema de riego automático basado en **ESP32** que monitorea la humedad del suelo y el nivel de luz para decidir cuándo activar una bomba de agua, con pantalla **LCD I2C 20x4** para feedback en tiempo real y protección automática contra funcionamiento en seco (tanque vacío).

## Tabla de contenido

- [Características](#características)
- [Hardware requerido](#hardware-requerido)
- [Conexiones (pinout)](#conexiones-pinout)
- [Librerías necesarias](#librerías-necesarias)
- [Instalación y carga](#instalación-y-carga)
- [Cómo funciona](#cómo-funciona)
- [Calibración](#calibración)
- [Estado del proyecto](#estado-del-proyecto)
- [Licencia](#licencia)

## Características

- **Encendido/apagado** del sistema mediante un botón físico.
- **Lectura de humedad del suelo** (sensor analógico) y **nivel de luz** (sensor digital) en cada ciclo.
- **Decisión con ventana de confirmación**: antes de regar, evalúa la humedad durante 5 segundos para evitar activaciones por lecturas falsas o ruido del sensor.
- **Ciclo de riego controlado**: activa la bomba durante 10 segundos y vuelve a medir la humedad para decidir si continuar o detener el riego.
- **Protección de tanque vacío**: si tras regar la tierra sigue seca, el sistema asume que el tanque está vacío, bloquea el riego y exige confirmación manual (botón) para reactivarse.
- **Feedback visual en LCD** con animaciones de estado: "Analizando suelo...", "Regando...", "Tierra húmeda", "No hay luz", "Tanque vacío".
- **LED indicador** de sistema apagado.

## Hardware requerido

- 1x ESP32 (Dev Module o similar)
- 1x Pantalla LCD 20x4 con módulo I2C (dirección por defecto `0x27`)
- 1x Sensor de humedad de suelo (salida analógica)
- 1x Sensor/módulo de luz (LDR con salida digital)
- 1x Módulo relé (1 canal) para controlar la bomba de agua
- 1x Mini bomba de agua sumergible (alimentada vía el relé)
- 1x Pulsador (botón)
- 1x LED + resistencia (indicador de estado)
- Cables, protoboard y fuente de alimentación adecuada para la bomba

## Conexiones (pinout)

| Componente              | Pin ESP32 | Tipo               |
|--------------------------|:---------:|---------------------|
| Sensor de humedad (señal) | GPIO 34   | Entrada analógica   |
| Sensor de luz (salida)    | GPIO 27   | Entrada digital     |
| Relé (bomba)              | GPIO 26   | Salida digital      |
| Botón (pulsador)          | GPIO 14   | Entrada (`INPUT_PULLUP`) |
| LED indicador             | GPIO 13   | Salida digital      |
| LCD I2C — SDA             | GPIO 21*  | I2C                 |
| LCD I2C — SCL             | GPIO 22*  | I2C                 |

> \* Los pines I2C (SDA/SCL) son los predeterminados del ESP32 y no se declaran explícitamente en el código (`Wire.h` los usa por defecto). Verifica que correspondan a tu placa específica.

> ⚠️ GPIO 34 en el ESP32 es **solo entrada** (no tiene resistencias internas de pull-up/pull-down ni puede usarse como salida), lo cual es correcto para el sensor analógico.

## Librerías necesarias

Instala estas librerías desde el **Gestor de Librerías** del Arduino IDE (`Sketch > Incluir Librería > Administrar Bibliotecas`):

- `LiquidCrystal_I2C` (para el control del LCD por I2C)
- `Wire` (incluida por defecto con el core de Arduino/ESP32)

## Instalación y carga

1. **Instala el soporte de placas ESP32** en el Arduino IDE (`Archivo > Preferencias`, agrega la URL del gestor de placas de Espressif, o instala el paquete "esp32" desde el Gestor de Placas).
2. **Instala la librería** `LiquidCrystal_I2C` mencionada arriba.
3. **Abre** `SistemaRiegoIoT/SistemaRiegoIoT.ino` en el Arduino IDE.
4. **Selecciona la placa** correspondiente a tu módulo ESP32 (`Herramientas > Placa > ESP32 Arduino > ...`) y el puerto serie correcto.
5. **Verifica la dirección I2C** de tu LCD (por defecto `0x27`; algunos módulos usan `0x3F`). Si tu pantalla no enciende correctamente, ejecuta un *I2C scanner* para confirmar la dirección.
6. **Conecta el hardware** según la tabla de pinout.
7. **Carga el sketch** a la placa.

## Cómo funciona

1. Al energizar el ESP32, la pantalla muestra un mensaje de inicio y el sistema queda **apagado** (LED encendido, bomba apagada).
2. Al **pulsar el botón**, el sistema se activa y comienza a leer humedad y luz en cada ciclo.
3. Si la **tierra está seca** (`humedad > 2500`) y **hay luz detectada**:
   - El sistema entra en estado de **evaluación** durante 5 segundos (animación "Analizando suelo...") para confirmar la lectura.
   - Si tras los 5 segundos sigue seca, **activa la bomba** (relé) durante 10 segundos (animación "Regando...").
   - Pasados los 10 segundos, vuelve a medir:
     - Si **sigue seca** → asume que el **tanque está vacío**, apaga la bomba y bloquea el sistema hasta que el usuario presione el botón para confirmar que rellenó el tanque.
     - Si **ya está húmeda** → apaga la bomba y muestra "Tierra húmeda".
4. Si la tierra está seca pero **no hay luz**, el sistema no riega (se asume que es de noche o el sensor de luz no está en condiciones fiables) y muestra "No hay luz".
5. Si la tierra **ya está húmeda**, el sistema no riega y lo indica en pantalla.
6. Pulsar el botón en cualquier momento alterna el sistema entre encendido/apagado (excepto cuando está bloqueado por tanque vacío, donde primero debe confirmarse el rellenado).

## Calibración

El umbral `humedadSeca = 2500` (lectura ADC de 0–4095 en ESP32) **debe calibrarse** según tu sensor específico y tipo de suelo:

1. Sube un sketch simple que solo imprima `analogRead(34)` por el monitor serie.
2. Toma lecturas con el sensor en aire seco, en agua y en tierra húmeda/seca real.
3. Ajusta el valor de `humedadSeca` en el código según esos resultados (recuerda que, según el tipo de sensor, valores más altos pueden indicar suelo más seco o más húmedo).

## Estado del proyecto

- ✅ Lógica de riego automático con doble verificación (evaluación + revisión post-riego).
- ✅ Protección contra funcionamiento en seco (bloqueo por tanque vacío).
- ✅ Interfaz LCD con animaciones de estado.
- 🚧 **Conectividad IoT pendiente**: la versión actual opera de forma completamente local (sin WiFi/MQTT/nube). El nombre del proyecto anticipa funcionalidad IoT que aún no está implementada en este sketch.

### Posibles mejoras futuras

- Conectividad WiFi para monitoreo y control remoto (por ejemplo, vía MQTT o un dashboard web/app).
- Registro histórico de riegos y lecturas (almacenamiento local o en la nube).
- Notificaciones (tanque vacío, riego completado) vía Telegram/correo/push.
- Múltiples zonas de riego con sensores independientes.
- Programación de horarios de riego configurables remotamente.

## Licencia

Este proyecto no especifica una licencia. Si deseas que el código sea reutilizable por terceros, considera añadir una licencia (por ejemplo, [MIT](https://choosealicense.com/licenses/mit/)).
