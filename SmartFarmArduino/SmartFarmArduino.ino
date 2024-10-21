#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN  2         // Pin donde está conectado el sensor DHT11
#define DHTTYPE DHT11     // Tipo de sensor DHT11
#define HMT_PIN  A2       // Pin del sensor de humedad de tierra   
#define wet  210
#define dry  510
#define LDR_PIN A1        // Pin de la fotoresistencia
#define PH_PIN  A3        // Pin del sensor de pH

DHT dht(DHTPIN, DHT11);
const char* ssid = "INFINITUM5DCB";           // Reemplaza con el SSID de tu red
const char* password = "9rBYnAsey9";       // Reemplaza con la contraseña de tu red
const char* server = "ec2-3-14-69-183.us-east-2.compute.amazonaws.com";         // IP pública o nombre de dominio de tu servidor

int status = WL_IDLE_STATUS;
WiFiClient client;
unsigned long previousMillis = 0;   // Para controlar el tiempo de envío de datos
const long interval = 600000;       // Intervalo de 10 minutos (600000 ms)

// Variables para valores umbrales
const float lowTempThreshold = 10.0;    // Umbral bajo de temperatura
const float highTempThreshold = 35.0;   // Umbral alto de temperatura
const int lowHumidityThreshold = 30;    // Umbral bajo de humedad de suelo
const int highLightThreshold = 80;      // Umbral alto de luz

// Variables para controlar el tiempo entre alertas por sensor
unsigned long previousAlertMillis[5] = {0, 0, 0, 0, 0};  // Para 5 sensores
const long alertInterval = 60000;  // 1 minuto (60000 ms) entre alertas por sensor

// Variable para asegurar que los datos se envíen la primera vez
bool firstRun = true;

void verificarAlertas(int sensorId, float valor, String descripcion, String accion, float lowThreshold, float highThreshold = -1) {
  bool enviarAlerta = false;
  
  if (highThreshold == -1) {
    // Umbral bajo únicamente
    if (valor < lowThreshold) enviarAlerta = true;
  } else {
    // Umbrales bajo y alto
    if (valor < lowThreshold || valor > highThreshold) enviarAlerta = true;
  }

  unsigned long currentMillis = millis();
  if (enviarAlerta && (currentMillis - previousAlertMillis[sensorId - 1] >= alertInterval)) {
    previousAlertMillis[sensorId - 1] = currentMillis;  // Actualizar el tiempo de la última alerta
    enviarAlertaJson(sensorId, descripcion, accion);
  }
}

void enviarAlertaJson(int sensorId, String descripcion, String accion) {
  if (client.connect(server, 8080)) {
    Serial.println("Enviando alerta...");
    
    // Construir JSON de la alerta
    String alertJson = "{\"sensor\": {\"id\": " + String(sensorId) + "}, \"user\": {\"id\": 5}, \"type\": \"Alert\", \"description\": \"" + descripcion + "\", \"action\": \"" + accion + "\"}";
    
    client.println("POST /api/alerts/add HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(alertJson.length());
    client.println();
    client.println(alertJson);
    
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
    Serial.println("Alerta enviada con éxito");
  } else {
    Serial.println("Error al conectarse al servidor para enviar alerta");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Iniciando el sensor DHT11...");
  
  while (!Serial) { }
  
  // Conectarse a la red WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, password);
  
  while (status != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    status = WiFi.status();
  }

  Serial.println("\nConectado a la red WiFi");
}

void loop() {
  // Leer los sensores
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soil_moisture = analogRead(HMT_PIN);
  int soil_read = map(soil_moisture, wet, dry , 100, 0);
  int light_value = analogRead(LDR_PIN);
  int light_percentage = map(light_value, 0, 1023, 0, 100);
  int ph_value = analogRead(PH_PIN);  
  float pH = map(ph_value, 0, 1023, 0, 14);

  // Verificar alertas de sensores inusuales
  verificarAlertas(1, soil_read, "Low soil humidity", "Increase soil moisture", lowHumidityThreshold);
  verificarAlertas(3, light_percentage, "High light levels", "Reduce light exposure", highLightThreshold);
  verificarAlertas(5, t, "Low temperature", "Regulate environment temperature", lowTempThreshold, highTempThreshold);

  unsigned long currentMillis = millis();

  // Enviar datos al inicio (firstRun) y luego cada 10 minutos
  if (firstRun || currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Enviar datos al servidor
    enviarDatos(1, soil_read);       // Sensor de Humedad de Suelo (ID: 1)
    enviarDatos(2, h);               // Sensor de Humedad del Aire (ID: 2)
    enviarDatos(3, light_percentage);// Sensor de Luz (ID: 3)
    enviarDatos(4, pH);              // Sensor de pH (ID: 4)
    enviarDatos(5, t);               // Sensor de Temperatura (ID: 5)
    
    firstRun = false;  // Desactivar la bandera después del primer envío
  }
}

void enviarDatos(int sensorId, float valor) {
  if (client.connect(server, 8080)) {
    Serial.println("Conectado al servidor");
    
    // Crear datos en formato JSON
    String jsonData = "{\"sensor\": {\"id\": " + String(sensorId) + "}, \"stats\": " + String(valor, 2) + "}";
    
    client.println("POST /api/data/add HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();
    client.println(jsonData);
    
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
    Serial.println("Datos enviados con éxito");
  } else {
    Serial.println("Error al conectarse al servidor");
  }
}

