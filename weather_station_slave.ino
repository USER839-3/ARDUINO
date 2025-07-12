// ARDUINO ESCLAVO - LECTURA DE SENSORES
// Optimizado para menos de 2048 bytes
// Pines: BMP280+AHT20 (I2C), LDR (A0)

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

// Configuración de pines
#define LDR_PIN A0
#define SLAVE_ADDRESS 0x08

// Objetos de sensores
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

// Estructura de datos para enviar al maestro
struct SensorData {
  float temp;      // Temperatura actual
  float humidity;  // Humedad relativa
  float pressure;  // Presión atmosférica
  float altitude;  // Altitud
  int ldr;         // Valor LDR (luz)
  float temp_min;  // Temperatura mínima del día
  float temp_max;  // Temperatura máxima del día
  unsigned long timestamp; // Timestamp
};

SensorData data;
float temp_min = 999, temp_max = -999;

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  
  // Inicializar sensores
  if (!bmp.begin(0x76)) {
    bmp.begin(0x77); // Intentar dirección alternativa
  }
  
  if (!aht.begin()) {
    // Error en AHT20
  }
  
  // Configurar BMP280 para máxima precisión
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  readSensors();
  updateMinMax();
  delay(1000); // Actualizar cada segundo
}

void readSensors() {
  // Leer AHT20 (temperatura y humedad)
  sensors_event_t humidity, temp;
  if (aht.getEvent(&humidity, &temp)) {
    data.temp = temp.temperature;
    data.humidity = humidity.relative_humidity;
  }
  
  // Leer BMP280 (presión y altitud)
  data.pressure = bmp.readPressure() / 100.0; // Convertir a hPa
  data.altitude = bmp.readAltitude(1013.25); // Altitud con presión estándar
  
  // Leer LDR
  data.ldr = analogRead(LDR_PIN);
  
  data.timestamp = millis();
}

void updateMinMax() {
  if (data.temp < temp_min) temp_min = data.temp;
  if (data.temp > temp_max) temp_max = data.temp;
  
  data.temp_min = temp_min;
  data.temp_max = temp_max;
}

void requestEvent() {
  // Enviar datos al maestro cuando lo solicite
  Wire.write((uint8_t*)&data, sizeof(data));
}