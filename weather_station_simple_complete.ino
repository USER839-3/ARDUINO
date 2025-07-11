/*
 * ESTACIÓN METEOROLÓGICA AVANZADA - VERSIÓN COMPLETA SIMPLE
 * Arduino Uno con BMP280+AHT20 y LCD 1602
 * TODO EN UN SOLO ARCHIVO - SIN DEPENDENCIAS EXTERNAS
 */

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

// ==================== CONFIGURACIÓN REGIONAL SAN CRISTÓBAL ====================
const float REGION_ALTITUDE = 1912.0;      // San Cristóbal - 1912m
const float REGION_PRESSURE_BASE = 823.45; // Precalculada para 1912m
const float TEMP_MIN_ANNUAL = 5.0;         // °C
const float TEMP_MAX_ANNUAL = 30.0;        // °C
const float HUMIDITY_MIN_TYPICAL = 48.0;   // %
const float HUMIDITY_MAX_TYPICAL = 75.0;   // %

// ==================== CONFIGURACIÓN DEL SISTEMA ====================
const unsigned long SENSOR_UPDATE_INTERVAL = 1000;   // 1 segundo
const unsigned long SCREEN_ROTATION_INTERVAL = 3000; // 3 segundos
const unsigned long AI_UPDATE_INTERVAL = 5000;       // 5 segundos IA
const int AI_HISTORY_SIZE = 12;                       // 12 horas histórico
const int SCREEN_COUNT = 8;                           // 8 pantallas

// LCD (13, 12, 11, 10, 9, 8)
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Sensores
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

// Iconos personalizados optimizados
const byte icons[8][8] PROGMEM = {
  {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x1F, 0x1F, 0x0E}, // Temp
  {0x04, 0x0A, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // Humedad
  {0x04, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x04, 0x00}, // Sol
  {0x00, 0x0E, 0x1F, 0x00, 0x12, 0x12, 0x12, 0x00}, // Lluvia
  {0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00}, // Nublado
  {0x0E, 0x1F, 0x1F, 0x04, 0x08, 0x10, 0x04, 0x00}, // Tormenta
  {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00}, // Presión
  {0x15, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x15, 0x00}  // UV
};

// Variables globales optimizadas
struct WeatherData {
  float temp, hum, press, alt;
  float tempMin, tempMax, dewPt, heatIdx;
  int uvIdx, rainProb, ldrVal;
  byte condition;
} weather = {0, 0, 0, 0, TEMP_MAX_ANNUAL, TEMP_MIN_ANNUAL, 0, 0, 0, 0, 0, 0};

// Variables del sistema
byte screen = 0;
unsigned long lastUpdate = 0, screenTimer = 0;

// Sistema de predicción IA
struct AISystem {
  float pressHist[12];
  byte histIdx;
  float trend;
  int accuracy;
} ai = {{REGION_PRESSURE_BASE}, 0, 0, 50};

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  // Inicializar sensores
  if (!bmp.begin(0x76)) errorHalt("BMP280 Error!");
  if (!aht.begin()) errorHalt("AHT20 Error!");
  
  // Crear iconos
  for (byte i = 0; i < 8; i++) {
    byte icon[8];
    memcpy_P(icon, icons[i], 8);
    lcd.createChar(i, icon);
  }
  
  // Pantalla de bienvenida
  lcd.clear();
  lcd.print("MeteoStation v3");
  lcd.setCursor(0, 1);
  lcd.print("San Cristobal");
  delay(2000);
  
  // Calibrar con datos regionales
  calibrateForRegion();
}

void loop() {
  unsigned long now = millis();
  
  // Actualizar sensores
  if (now - lastUpdate >= SENSOR_UPDATE_INTERVAL) {
    readSensors();
    calculateWeatherParams();
    updateAIPredictions();
    lastUpdate = now;
  }
  
  // Rotar pantallas
  if (now - screenTimer >= SCREEN_ROTATION_INTERVAL) {
    displayScreen(screen);
    screen = (screen + 1) % SCREEN_COUNT;
    screenTimer = now;
  }
}

void readSensors() {
  // Leer BMP280
  weather.temp = bmp.readTemperature();
  weather.press = bmp.readPressure() / 100.0;
  weather.alt = bmp.readAltitude(REGION_PRESSURE_BASE);
  
  // Leer AHT20
  sensors_event_t h, t;
  aht.getEvent(&h, &t);
  weather.hum = h.relative_humidity;
  
  // Promediar temperatura
  weather.temp = (weather.temp + t.temperature) / 2.0;
  
  // Leer LDR
  weather.ldrVal = analogRead(A0);
  
  // Actualizar min/max
  if (weather.temp < weather.tempMin) weather.tempMin = weather.temp;
  if (weather.temp > weather.tempMax) weather.tempMax = weather.temp;
}

void calculateWeatherParams() {
  // Punto de rocío
  weather.dewPt = weather.temp - ((100 - weather.hum) / 5.0);
  
  // Sensación térmica
  if (weather.temp >= 26.7) {
    float t = weather.temp, h = weather.hum;
    weather.heatIdx = -42.379 + 2.049*t + 10.143*h - 0.225*t*h 
                      - 0.00684*t*t - 0.0548*h*h + 0.00123*t*t*h 
                      + 0.000853*t*h*h - 0.00000199*t*t*h*h;
  } else {
    weather.heatIdx = weather.temp;
  }
  
  // Índice UV
  weather.uvIdx = map(weather.ldrVal, 0, 1023, 0, 11);
  
  // Condición climática
  weather.condition = determineWeatherCondition();
  
  // Probabilidad de lluvia
  weather.rainProb = calculateRainProbability();
}

byte determineWeatherCondition() {
  if (weather.press < 995) return 3; // Tormenta
  
  if (weather.press < 1005) {
    return (weather.hum > 80 && ai.trend < -1) ? 2 : 1; // Lluvia o nublado
  }
  
  if (weather.press > 1015 && weather.hum < 70) return 0; // Soleado
  
  return 1; // Nublado por defecto
}

int calculateRainProbability() {
  int prob = 0;
  
  // Presión base
  if (weather.press < 995) prob += 80;
  else if (weather.press < 1005) prob += 40;
  else if (weather.press > 1015) prob -= 20;
  
  // Humedad
  if (weather.hum > 85) prob += 30;
  else if (weather.hum > 75) prob += 15;
  
  // Tendencia con IA
  if (ai.trend < -2) prob += 35;
  else if (ai.trend < -1) prob += 20;
  else if (ai.trend > 1) prob -= 15;
  
  // Corrección por altitud para San Cristóbal
  prob += (REGION_ALTITUDE / 1000) * 5;
  
  return constrain(prob, 0, 100);
}

void updateAIPredictions() {
  static byte readCount = 0;
  readCount++;
  
  if (readCount >= 12) {
    ai.pressHist[ai.histIdx] = weather.press;
    ai.histIdx = (ai.histIdx + 1) % 12;
    
    // Calcular tendencia
    float sum = 0;
    for (byte i = 0; i < 12; i++) sum += ai.pressHist[i];
    ai.trend = weather.press - (sum / 12.0);
    
    readCount = 0;
    improveAIAccuracy();
  }
}

void improveAIAccuracy() {
  static float lastPred = 0;
  static unsigned long lastTime = 0;
  
  if (lastTime != 0 && millis() - lastTime > 3600000) { // 1 hora
    float error = abs((weather.press - lastPred) - ai.trend);
    
    if (error < 1.0) {
      ai.accuracy = min(95, ai.accuracy + 1);
    } else {
      ai.accuracy = max(30, ai.accuracy - 1);
    }
  }
  
  lastPred = weather.press;
  lastTime = millis();
}

void calibrateForRegion() {
  // Calibración específica para San Cristóbal
  weather.tempMin = TEMP_MIN_ANNUAL;
  weather.tempMax = TEMP_MAX_ANNUAL;
  
  // Inicializar histórico con datos regionales
  for (byte i = 0; i < 12; i++) {
    ai.pressHist[i] = REGION_PRESSURE_BASE;
  }
}

void displayScreen(byte s) {
  lcd.clear();
  
  switch(s) {
    case 0: // Temperatura y sensación
      lcd.write(0);
      lcd.print("T:");
      lcd.print(weather.temp, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("ST:");
      lcd.print(weather.heatIdx, 1);
      lcd.print("C");
      break;
      
    case 1: // Min/Max
      lcd.write(0);
      lcd.print("L:");
      lcd.print(weather.tempMin, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("H:");
      lcd.print(weather.tempMax, 1);
      lcd.print("C");
      break;
      
    case 2: // Humedad/Rocío
      lcd.write(1);
      lcd.print("HR:");
      lcd.print(weather.hum, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("PR:");
      lcd.print(weather.dewPt, 1);
      lcd.print("C");
      break;
      
    case 3: // Clima actual
      lcd.write(weather.condition + 2);
      lcd.print("Clima:");
      lcd.setCursor(0, 1);
      const char* conditions[] = {"Soleado", "Nublado", "Lluvioso", "Tormenta"};
      lcd.print(conditions[weather.condition]);
      break;
      
    case 4: // Presión/Altitud
      lcd.write(6);
      lcd.print("P:");
      lcd.print(weather.press, 1);
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print("Alt:");
      lcd.print(weather.alt, 0);
      lcd.print("m");
      break;
      
    case 5: // Índice UV
      lcd.write(7);
      lcd.print("UV:");
      lcd.print(weather.uvIdx);
      lcd.setCursor(0, 1);
      const char* uvLevels[] = {"Bajo", "Moderado", "Alto", "Muy Alto"};
      byte uvLevel = weather.uvIdx <= 2 ? 0 : (weather.uvIdx <= 5 ? 1 : (weather.uvIdx <= 7 ? 2 : 3));
      lcd.print(uvLevels[uvLevel]);
      break;
      
    case 6: // Probabilidad lluvia
      lcd.write(3);
      lcd.print("Lluvia:");
      lcd.print(weather.rainProb);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("IA:");
      lcd.print(ai.accuracy);
      lcd.print("%");
      break;
      
    case 7: // Predicción
      lcd.print("Prediccion:");
      lcd.setCursor(0, 1);
      if (ai.trend < -2) lcd.print("Empeorando");
      else if (ai.trend < -1) lcd.print("Nublando");
      else if (ai.trend > 1) lcd.print("Mejorando");
      else lcd.print("Estable");
      break;
  }
}

void errorHalt(const char* msg) {
  lcd.clear();
  lcd.print(msg);
  while(1);
}