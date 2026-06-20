#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =========================
// LCD
// =========================
LiquidCrystal_I2C lcd(0x27, 20, 4);

// =========================
// Pines
// =========================
const int sensorHumedad = 34;
const int sensorLuz = 27;
const int rele = 26;
const int boton = 14;
const int led = 13;

// =========================
// Variables
// =========================
bool sistemaActivo = false;
bool bombaActiva = false;
bool evaluando = false;
bool tanqueVacio = false;

unsigned long tiempoRiego = 0;
unsigned long tiempoDecision = 0;

int humedad = 0;
int luz = 0;

// =========================
// Configuración
// =========================
const int humedadSeca = 2500;

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);

  // =========================
  // Pines
  // =========================
  pinMode(rele, OUTPUT);

  pinMode(sensorLuz, INPUT);

  pinMode(boton, INPUT_PULLUP);

  pinMode(led, OUTPUT);

  // =========================
  // Estado inicial
  // =========================

  // Relé apagado
  digitalWrite(rele, LOW);

  // LED encendido
  // Sistema apagado
  digitalWrite(led, HIGH);

  // =========================
  // LCD
  // =========================
  lcd.init();
  lcd.backlight();

  // =========================
  // Pantalla inicio
  // =========================
  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("Iniciando sistema");

  lcd.setCursor(0, 2);
  lcd.print("de riego v1.0");

  delay(3000);

  lcd.clear();

  // Evita falsas pulsaciones
  delay(500);
}

// =========================
// LOOP
// =========================
void loop() {

  manejarBoton();

  // =========================
  // BLOQUEO POR TANQUE VACIO
  // =========================
  if (tanqueVacio) {

    digitalWrite(rele, LOW);

    bombaActiva = false;

    delay(200);

    return;
  }

  // =========================
  // SISTEMA APAGADO
  // =========================
  if (!sistemaActivo) {

    // LED encendido
    digitalWrite(led, HIGH);

    // Bomba apagada
    digitalWrite(rele, LOW);

    bombaActiva = false;
    evaluando = false;

    lcd.setCursor(0, 0);
    lcd.print(" SISTEMA DE RIEGO ");

    lcd.setCursor(0, 1);
    lcd.print("      IoT v1.0    ");

    lcd.setCursor(0, 2);
    lcd.print(" Presione boton   ");

    lcd.setCursor(0, 3);
    lcd.print("   para activar   ");

    delay(200);

    return;
  }

  // =========================
  // SISTEMA ENCENDIDO
  // =========================

  // LED apagado
  digitalWrite(led, LOW);

  // =========================
  // Lectura sensores
  // =========================
  humedad = analogRead(sensorHumedad);

  luz = digitalRead(sensorLuz);

  // =========================
  // Mostrar humedad
  // =========================
  lcd.setCursor(0, 0);
  lcd.print("Humedad: ");
  lcd.print(humedad);
  lcd.print("     ");

  // =========================
  // Mostrar luz
  // =========================
  lcd.setCursor(0, 1);

  if (luz == LOW) {
    lcd.print("Luz: Alta        ");
  } else {
    lcd.print("Luz: Baja        ");
  }

  // =========================
  // TIERRA SECA
  // =========================
  if (humedad > humedadSeca) {

    // =========================
    // NO HAY LUZ
    // =========================
    if (luz == HIGH) {

      // Apagar bomba
      digitalWrite(rele, LOW);

      bombaActiva = false;
      evaluando = false;

      lcd.setCursor(0, 2);
      lcd.print("No hay luz       ");

      lcd.setCursor(0, 3);
      lcd.print("                    ");
    }

    // =========================
    // HAY LUZ
    // =========================
    else {

      // =========================
      // Iniciar evaluación
      // =========================
      if (!evaluando && !bombaActiva) {

        evaluando = true;

        tiempoDecision = millis();
      }

      // =========================
      // ANALIZANDO
      // =========================
      if (evaluando) {

        mostrarEvaluacion();

        // Esperar 5 segundos
        if (millis() - tiempoDecision >= 5000) {

          evaluando = false;

          // Encender bomba
          digitalWrite(rele, HIGH);

          tiempoRiego = millis();

          bombaActiva = true;
        }
      }

      // =========================
      // BOMBA ACTIVA
      // =========================
      if (bombaActiva) {

        mostrarAnimacion();

        // =========================
        // Revisar despues de 10s
        // =========================
        if (millis() - tiempoRiego >= 10000) {

          int nuevaHumedad = analogRead(sensorHumedad);

          // Sigue seca
          if (nuevaHumedad > humedadSeca) {

            // Apagar bomba
            digitalWrite(rele, LOW);

            bombaActiva = false;

            // Activar bloqueo
            tanqueVacio = true;

            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("TANQUE VACIO");

            lcd.setCursor(0, 1);
            lcd.print("Rellene tanque");

            lcd.setCursor(0, 2);
            lcd.print("Pulse boton");

            lcd.setCursor(0, 3);
            lcd.print("para reactivar");
          }

          // Tierra húmeda nuevamente
          else {

            digitalWrite(rele, LOW);

            bombaActiva = false;

            lcd.setCursor(0, 2);
            lcd.print("Tierra humeda    ");

            lcd.setCursor(0, 3);
            lcd.print("                    ");
          }
        }
      }
    }
  }

  // =========================
  // TIERRA HUMEDA
  // =========================
  else {

    // Bomba apagada
    digitalWrite(rele, LOW);

    bombaActiva = false;
    evaluando = false;

    lcd.setCursor(0, 2);
    lcd.print("Tierra humeda    ");

    lcd.setCursor(0, 3);
    lcd.print("                    ");
  }

  delay(300);
}

// =========================
// BOTON ON/OFF
// =========================
void manejarBoton() {

  static bool ultimoEstado = HIGH;

  bool estadoActual = digitalRead(boton);

  // Detectar pulsación
  if (estadoActual == LOW && ultimoEstado == HIGH) {

    // =========================
    // REACTIVAR TANQUE VACIO
    // =========================
    if (tanqueVacio) {

      tanqueVacio = false;

      lcd.clear();

      delay(300);

      ultimoEstado = estadoActual;

      return;
    }

    // =========================
    // ON/OFF NORMAL
    // =========================
    sistemaActivo = !sistemaActivo;

    lcd.clear();

    delay(250);
  }

  ultimoEstado = estadoActual;
}

// =========================
// ANIMACION RIEGO
// =========================
void mostrarAnimacion() {

  static int barra = 0;

  lcd.setCursor(0, 2);
  lcd.print("Regando...       ");

  lcd.setCursor(0, 3);

  for (int i = 0; i < barra; i++) {
    lcd.print("=");
  }

  for (int i = barra; i < 20; i++) {
    lcd.print(" ");
  }

  barra++;

  if (barra > 20) {
    barra = 0;
  }
}

// =========================
// ANIMACION EVALUACION
// =========================
void mostrarEvaluacion() {

  static int puntos = 0;

  lcd.setCursor(0, 2);
  lcd.print("Analizando suelo ");

  lcd.setCursor(0, 3);

  for (int i = 0; i < puntos; i++) {
    lcd.print(".");
  }

  for (int i = puntos; i < 20; i++) {
    lcd.print(" ");
  }

  puntos++;

  if (puntos > 10) {
    puntos = 0;
  }
}