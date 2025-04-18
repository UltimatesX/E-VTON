#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "samohik net";
const char* password = "iloveChila6969";

// Web server on port 80
WebServer server(80);

// Sensor and display setup
Adafruit_BMP280 bmp;  // I2C BMP280

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // OLED

// Ultrasonic and pump pin setup
#define TRIG_PIN 5
#define ECHO_PIN 4
#define PUMP_PIN 18

// Function to handle HTTP request and return sensor data as JSON
void handleRoot() {
  long duration;
  float distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.0343) / 2.0;

  float pressure = bmp.readPressure();  // in Pa

  String json = "{\"distance_cm\": " + String(distance, 2) + ", \"pressure\": " + String(pressure / 100.0, 2) + "}"; // pressure in hPa
  server.send(200, "application/json", json);

}

void setup() {
  Serial.begin(115200);

  // Start BMP280 sensor
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find BMP280 sensor!");
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // Ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Pump setup
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  // OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // WiFi setup
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Register HTTP endpoint
  server.on("/", []() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.0343) / 2.0;

  float pressure = bmp.readPressure() / 100.0;  // Convert to hPa

  String json = "{\"distance_cm\": " + String(distance, 2) + ", \"pressure\": " + String(pressure, 2) + "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
});

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Distance measurement
  long duration;
  float distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.0343) / 2.0;

  // Pressure reading
  float pressure = bmp.readPressure(); // in Pa

  // Pump logic
  if (distance < 8) {
    digitalWrite(PUMP_PIN, HIGH);  // Mid-power PWM
  } else {
    digitalWrite(PUMP_PIN, LOW);
  }

  // OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Distance:");
  display.print(distance, 2);
  display.println(" cm");

  display.setCursor(0, 20);
  if (distance > 8 && distance < 10) {
    display.println("Glaucoma Detected");
  } else {
    display.println("Glaucoma Not Detected");
  }

  display.setCursor(0, 40);
  display.print("Pressure:");
  display.print(pressure / 100.0, 2); // hPa
  display.println(" hPa");
  display.display();

  // Handle HTTP requests
  server.handleClient();

  delay(500);
}
