#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL2knfn3jQK" // colocar el ID del blynk generado, en esta linea
#define BLYNK_TEMPLATE_NAME "Control de motores"
#define BLYNK_AUTH_TOKEN "jmd3LltfJRCb2isspmTS1XoJVaZE_15K" // token generado por la pagina web del blynk colocar en esta linea
// INSTALAR TODAS ESTAS LIBRERIRAS
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Stepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
char ssid[] = "COLOCAR SU RED WIFI";
char pass[] = "COLOCAR SU CONTRASEÑA";

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= HARDWARE =================
#define IN1 15
#define IN2 2
#define IN3 4
#define IN4 16

const int STEPS_PER_REV = 2048;
int pasos = 500;
int pasosMovimiento = 100;

Stepper motor(STEPS_PER_REV, IN1, IN3, IN2, IN4);

Servo motor1;
Servo motor2;
Servo motor3;

// ================= MODO =================
enum Modo {
  MODO_MANUAL,
  MODO_AUTOMATICO
};

Modo modoActual = MODO_MANUAL;
bool ejecutarAutomatico = false;

// ================= FUNCION TEXTO CENTRADO =================
void mostrarTextoCentrado(const char* texto, uint8_t size) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h);

  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(x, y);
  display.println(texto);
  display.display();
}

// ================= BLYNK =================
BLYNK_WRITE(V0) { if (modoActual == MODO_MANUAL) motor1.write(param.asInt()); }
BLYNK_WRITE(V1) { if (modoActual == MODO_MANUAL) motor2.write(param.asInt()); }
BLYNK_WRITE(V2) { if (modoActual == MODO_MANUAL) motor3.write(param.asInt()); }

BLYNK_WRITE(V3) {
  if (modoActual == MODO_MANUAL && param.asInt()) {
    motor.step(pasosMovimiento);
    Blynk.virtualWrite(V3, 0);
  }
}

BLYNK_WRITE(V4) {
  if (modoActual == MODO_MANUAL && param.asInt()) {
    motor.step(-pasosMovimiento);
    Blynk.virtualWrite(V4, 0);
  }
}

// Selector de modo
BLYNK_WRITE(V5) {
  if (param.asInt()) {
    modoActual = MODO_AUTOMATICO;
    ejecutarAutomatico = true;
    mostrarTextoCentrado("AUTO", 3);
  } else {
    modoActual = MODO_MANUAL;
    mostrarTextoCentrado("MANUAL", 3);
  }
}

// ================= SECUENCIA AUTOMÁTICA =================
void modoAutomatico() {
  if (!ejecutarAutomatico) return;
  ejecutarAutomatico = false;

  mostrarTextoCentrado("RUN", 3);
//calibrar angulos para movimiento del brazo robotico
  motor1.write(45);
  motor2.write(180);
  motor3.write(0);
  delay(1000);

  motor1.write(160);
  motor2.write(50);
  mostrarTextoCentrado("DOWN", 3);
  delay(1000);

  motor3.write(100);
  mostrarTextoCentrado("GRAB", 3);
  delay(1500);

  motor1.write(45);
  motor2.write(180);
  mostrarTextoCentrado("UP", 3);
  delay(1000);

  motor.step(pasos);
  mostrarTextoCentrado("MOVE 90", 3);
  delay(1000);

  motor1.write(160);
  motor2.write(50);
  mostrarTextoCentrado("DOWN", 3);
  delay(1000);

  motor3.write(0);
  mostrarTextoCentrado("DROP", 3);
  delay(1500);

  motor1.write(45);
  motor2.write(180);
  delay(1000);

  mostrarTextoCentrado("DONE", 3);

  modoActual = MODO_MANUAL;
  Blynk.virtualWrite(V5, 0);

  mostrarTextoCentrado("MANUAL", 3);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  motor1.attach(18);
  motor2.attach(19);
  motor3.attach(23);
  motor.setSpeed(10);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  mostrarTextoCentrado("MANUAL", 3);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// ================= LOOP =================
void loop() {
  Blynk.run();

  if (modoActual == MODO_AUTOMATICO) {
    modoAutomatico();
  }
}

