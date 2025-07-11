// ARDUINO MAESTRO - ESTACIÓN METEOROLÓGICA
// Sensores: BMP280+AHT20 (I2C), LDR (A0)
// Display: LCD 1602 sin módulo I2C
// Comunicación: I2C con Arduino Esclavo

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

// Configuración LCD 1602
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Sensores
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

// Direcciones I2C
#define SLAVE_ADDRESS 0x08
#define MASTER_ADDRESS 0x04

// Variables de sensores
float temp = 0, humidity = 0, pressure = 0, altitude = 0;
float tempMin = 50, tempMax = -50, dewPoint = 0;
float feelsLike = 0, uvIndex = 0, rainProb = 0;
int lightLevel = 0;
unsigned long lastReading = 0;
unsigned long lastScreen = 0;
unsigned long lastSend = 0;
int currentScreen = 0;
const int screenDelay = 3000;

// Datos históricos (últimas 24 horas)
float tempHistory[24];
float pressureHistory[24];
float humidityHistory[24];
int historyIndex = 0;
bool historyFull = false;

// Caracteres personalizados para iconos
byte tempIcon[8] = {0x04,0x0E,0x0E,0x0E,0x1F,0x1F,0x0E,0x00};
byte humIcon[8] = {0x04,0x0E,0x0E,0x1F,0x1F,0x1F,0x0E,0x00};
byte pressIcon[8] = {0x1F,0x11,0x11,0x1F,0x1F,0x1F,0x1F,0x00};
byte sunIcon[8] = {0x15,0x0E,0x1F,0x1F,0x1F,0x0E,0x15,0x00};
byte cloudIcon[8] = {0x00,0x0E,0x1F,0x1F,0x1F,0x0E,0x00,0x00};
byte rainIcon[8] = {0x0E,0x1F,0x1F,0x0E,0x04,0x04,0x04,0x00};
byte upArrow[8] = {0x04,0x0E,0x1F,0x04,0x04,0x04,0x00,0x00};
byte downArrow[8] = {0x04,0x04,0x04,0x1F,0x0E,0x04,0x00,0x00};

void setup() {
  Serial.begin(9600);
  Wire.begin(MASTER_ADDRESS);
  
  // Inicializar LCD
  lcd.begin(16, 2);
  lcd.createChar(0, tempIcon);
  lcd.createChar(1, humIcon);
  lcd.createChar(2, pressIcon);
  lcd.createChar(3, sunIcon);
  lcd.createChar(4, cloudIcon);
  lcd.createChar(5, rainIcon);
  lcd.createChar(6, upArrow);
  lcd.createChar(7, downArrow);
  
  // Pantalla de inicio
  lcd.setCursor(0, 0);
  lcd.print("ESTACION METEO");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  
  // Inicializar sensores
  if (!bmp.begin()) {
    lcd.clear();
    lcd.print("Error BMP280!");
    while(1);
  }
  
  if (!aht.begin()) {
    lcd.clear();
    lcd.print("Error AHT20!");
    while(1);
  }
  
  // Configurar BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  
  // Inicializar historial
  for(int i = 0; i < 24; i++) {
    tempHistory[i] = 20.0;
    pressureHistory[i] = 1013.25;
    humidityHistory[i] = 60.0;
  }
  
  lcd.clear();
  lcd.print("Listo!");
  delay(1000);
}

void loop() {
  // Leer sensores cada 30 segundos
  if (millis() - lastReading > 30000) {
    readSensors();
    calculateMetrics();
    updateHistory();
    lastReading = millis();
  }
  
  // Enviar datos al esclavo cada 5 minutos
  if (millis() - lastSend > 300000) {
    sendDataToSlave();
    lastSend = millis();
  }
  
  // Cambiar pantalla cada 3 segundos
  if (millis() - lastScreen > screenDelay) {
    displayScreen();
    currentScreen = (currentScreen + 1) % 8;
    lastScreen = millis();
  }
  
  delay(100);
}

void readSensors() {
  // Leer temperatura y humedad del AHT20
  sensors_event_t h, t;
  aht.getEvent(&h, &t);
  temp = t.temperature;
  humidity = h.relative_humidity;
  
  // Leer presión y altitud del BMP280
  pressure = bmp.readPressure() / 100.0F; // hPa
  altitude = bmp.readAltitude(1013.25); // metros
  
  // Leer sensor de luz
  lightLevel = analogRead(A0);
  
  // Actualizar min/max
  if (temp < tempMin) tempMin = temp;
  if (temp > tempMax) tempMax = temp;
}

void calculateMetrics() {
  // Calcular punto de rocío
  dewPoint = temp - ((100 - humidity) / 5.0);
  
  // Calcular sensación térmica
  if (temp > 26.7) {
    // Índice de calor
    feelsLike = temp + 0.348 * (humidity * 0.01 * 6.105 * exp(17.27 * temp / (237.7 + temp))) - 0.70;
  } else {
    feelsLike = temp;
  }
  
  // Calcular índice UV aproximado (basado en luz y hora)
  int hour = (millis() / 3600000) % 24;
  if (hour >= 6 && hour <= 18) {
    uvIndex = map(lightLevel, 0, 1023, 0, 11);
    if (uvIndex > 11) uvIndex = 11;
  } else {
    uvIndex = 0;
  }
  
  // Calcular probabilidad de lluvia básica
  float pressureTrend = getPressureTrend();
  if (pressureTrend < -2 && humidity > 80) {
    rainProb = 85;
  } else if (pressureTrend < -1 && humidity > 70) {
    rainProb = 65;
  } else if (humidity > 85) {
    rainProb = 45;
  } else if (humidity > 75) {
    rainProb = 25;
  } else {
    rainProb = 5;
  }
}

float getPressureTrend() {
  if (!historyFull && historyIndex < 3) return 0;
  
  int idx = historyIndex >= 3 ? historyIndex - 3 : 24 - (3 - historyIndex);
  return pressure - pressureHistory[idx];
}

void updateHistory() {
  tempHistory[historyIndex] = temp;
  pressureHistory[historyIndex] = pressure;
  humidityHistory[historyIndex] = humidity;
  
  historyIndex = (historyIndex + 1) % 24;
  if (historyIndex == 0) historyFull = true;
}

void displayScreen() {
  lcd.clear();
  
  switch(currentScreen) {
    case 0: // Temperatura y sensación térmica
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.print("T:");
      lcd.print(temp, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("ST:");
      lcd.print(feelsLike, 1);
      lcd.print("C");
      break;
      
    case 1: // Temperatura min/max
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.print("L:");
      lcd.print(tempMin, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("H:");
      lcd.print(tempMax, 1);
      lcd.print("C");
      break;
      
    case 2: // Humedad y punto de rocío
      lcd.setCursor(0, 0);
      lcd.write(byte(1));
      lcd.print("HR:");
      lcd.print(humidity, 0);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("PR:");
      lcd.print(dewPoint, 1);
      lcd.print("C");
      break;
      
    case 3: // Clima actual
      lcd.setCursor(0, 0);
      lcd.print("CLIMA:");
      lcd.setCursor(0, 1);
      if (pressure > 1020) {
        lcd.write(byte(3));
        lcd.print("Soleado");
      } else if (pressure > 1005) {
        lcd.write(byte(4));
        lcd.print("Nublado");
      } else {
        lcd.write(byte(5));
        lcd.print("Lluvioso");
      }
      break;
      
    case 4: // Presión y altitud
      lcd.setCursor(0, 0);
      lcd.write(byte(2));
      lcd.print("P:");
      lcd.print(pressure, 0);
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print("Alt:");
      lcd.print(altitude, 0);
      lcd.print("m");
      break;
      
    case 5: // Índice UV
      lcd.setCursor(0, 0);
      lcd.write(byte(3));
      lcd.print("Indice UV:");
      lcd.setCursor(0, 1);
      lcd.print("UV:");
      lcd.print(uvIndex, 0);
      if (uvIndex <= 2) lcd.print(" Bajo");
      else if (uvIndex <= 5) lcd.print(" Mod");
      else if (uvIndex <= 7) lcd.print(" Alto");
      else lcd.print(" Ext");
      break;
      
    case 6: // Probabilidad de lluvia
      lcd.setCursor(0, 0);
      lcd.write(byte(5));
      lcd.print("Prob. Lluvia:");
      lcd.setCursor(0, 1);
      lcd.print(rainProb, 0);
      lcd.print("%");
      break;
      
    case 7: // Tendencia de presión
      lcd.setCursor(0, 0);
      lcd.print("Tendencia:");
      lcd.setCursor(0, 1);
      float trend = getPressureTrend();
      if (trend > 1) {
        lcd.write(byte(6));
        lcd.print("Mejorando");
      } else if (trend < -1) {
        lcd.write(byte(7));
        lcd.print("Empeorando");
      } else {
        lcd.print("Estable");
      }
      break;
  }
}

void sendDataToSlave() {
  Wire.beginTransmission(SLAVE_ADDRESS);
  
  // Enviar datos actuales
  Wire.write((byte*)&temp, sizeof(temp));
  Wire.write((byte*)&humidity, sizeof(humidity));
  Wire.write((byte*)&pressure, sizeof(pressure));
  Wire.write((byte*)&lightLevel, sizeof(lightLevel));
  
  // Enviar historial (últimas 6 horas)
  for(int i = 0; i < 6; i++) {
    int idx = (historyIndex - i + 24) % 24;
    Wire.write((byte*)&tempHistory[idx], sizeof(float));
    Wire.write((byte*)&pressureHistory[idx], sizeof(float));
    Wire.write((byte*)&humidityHistory[idx], sizeof(float));
  }
  
  Wire.endTransmission();
  
  Serial.println("Datos enviados al esclavo");
}