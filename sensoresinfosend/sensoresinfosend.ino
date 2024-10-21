#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN  2         // Pin donde está conectado el sensor DHT11
#define DHTTYPE DHT11     // Tipo de sensor DHT11
#define HMT_PIN  A2        // Pin del sensor de humedad de tierra   
#define wet  210
#define dry    510
#define LDR_PIN A1        // Pin de la fotoresistencia
#define PH_PIN  A3        // Pin del sensor de pH

DHT dht(DHTPIN, DHT11);
const char* ssid = "INFINITUM5DCB";           // Reemplaza con el SSID de tu red
const char* password = "9rBYnAsey9";   // Reemplaza con la contraseña de tu red
const char* server = "ec2-3-14-69-183.us-east-2.compute.amazonaws.com";   // IP pública o nombre de dominio de tu servidor

int status = WL_IDLE_STATUS;
WiFiClient client;  // Crear cliente WiFi

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Iniciando el sensor DHT11...");
  while (!Serial) {
    ; // Esperar a que el puerto serie esté listo
  }

  // Conectarse a la red WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, password);

  // Esperar hasta conectarse a la red WiFi
  while (status != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    status = WiFi.status();
  }

  Serial.println("\nConectado a la red WiFi");
}

void loop() {
  // Leer humedad y temperatura
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Leer humedad del suelo
  int soil_moisture = analogRead(HMT_PIN);
  int soil_read = map(soil_moisture, wet, dry , 100, 0);

  // Leer valor de la fotoresistencia (luz ambiente)
  int light_value = analogRead(LDR_PIN);
  int light_percentage = map(light_value, 0, 1023, 0, 100);  // Convertir a porcentaje

  // Leer valor del sensor de pH
  int ph_value = analogRead(PH_PIN);  
  // Mapear la lectura del ADC (0-1023) a un rango de pH (0-14)
  float pH = map(ph_value, 0, 1023, 0, 14);

  // Comprobar si alguna lectura ha fallado
  if (isnan(h) || isnan(t)) {
    Serial.println("Error al leer del sensor DHT11.");
  } else {
    Serial.println("Sensor de humedad y temperatura");
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  Serial.println("Porcentaje de humedad en la tierra:");
  Serial.print(soil_read);
  Serial.println("%");

  Serial.println("Porcentaje de luz ambiente:");
  Serial.print(light_percentage);
  Serial.println("%");

  Serial.println("Valor de pH:");
  Serial.print(pH);
  Serial.println(" pH");

  // Enviar datos al servidor para cada sensor
  enviarDatos(1, soil_read);       // Sensor de Humedad de Suelo (ID: 1)
  enviarDatos(2, h);               // Sensor de Humedad del Aire (ID: 2)
  enviarDatos(3, light_percentage);// Sensor de Luz (ID: 3)
  enviarDatos(4, pH);              // Sensor de pH (ID: 4)
  enviarDatos(5, t);               // Sensor de Temperatura (ID: 5)

  delay(600000);  // Enviar datos cada minuto (600000 ms = 10 min)
}

void enviarDatos(int sensorId, float valor) {
  if (client.connect(server, 8080)) {  // Conectar al servidor en el puerto 8080
    Serial.println("Conectado al servidor");

    // Crear datos en formato JSON para cada sensor
    String jsonData = "{\"sensor\": {\"id\": " + String(sensorId) + "}, \"stats\": " + String(valor, 2) + "}";

    Serial.println(jsonData);

    // Enviar solicitud HTTP POST
    client.println("POST /api/data/add HTTP/1.1");  // Nueva ruta de la API
    client.println("Host: " + String(server));      // Cambia a la dirección del servidor
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();
    client.println(jsonData);

    // Esperar la respuesta del servidor
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();  // Cerrar la conexión
    Serial.println("Datos enviados con éxito");
  } else {
    Serial.println("Error al conectarse al servidor");
  }
}

