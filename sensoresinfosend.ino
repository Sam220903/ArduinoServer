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
const char* ssid = "";           // Reemplaza con el SSID de tu red
const char* password = "";   // Reemplaza con la contraseña de tu red
const char* server = "";   // IP pública o nombre de dominio de tu servidor

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

  // Enviar datos al servidor
  if (client.connect(server, 80)) {  
    Serial.println("Conectado al servidor");

    // Crear datos en formato JSON
    String jsonData = "{\"temperature_sensor\": " + String(t, 2) + 
                      ", \"humidity_sensor\": " + String(h, 2) +
                      ", \"sm_sensor\": " + String(soil_read) + 
                      ", \"light_sensor\": " + String(light_percentage) + 
                      ", \"ph_sensor\": " + String(pH, 2) + "}";

    Serial.println(jsonData);

    // Enviar solicitud HTTP POST
    client.println("POST /datos HTTP/1.1");
    client.println("Host: 127.0.0.1");
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

  delay(5000);  // Enviar datos cada 5 segundos
}
