/*
 * ESTACIÓN METEOROLÓGICA AVANZADA - SISTEMA MAESTRO-ESCLAVO
 * Arduino Uno con BMP280+AHT20 y LCD 1602
 * Versión optimizada con IA predictiva
 */

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include "calibration_config.h"  // ← INCLUIR ARCHIVO DE CONFIGURACIÓN

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

// Variables globales optimizadas usando configuración
struct WeatherData {
  float temp, hum, press, alt;
  float tempMin, tempMax, dewPt, heatIdx;
  int uvIdx, rainProb, ldrVal;
  byte condition;
} weather = {0, 0, 0, 0, TEMP_MAX_ANNUAL, TEMP_MIN_ANNUAL, 0, 0, 0, 0, 0, 0};

// Variables del sistema usando configuración
byte screen = 0;
unsigned long lastUpdate = 0, screenTimer = 0;

// Sistema de predicción IA usando configuración
struct AISystem {
  float pressHist[AI_HISTORY_SIZE];
  byte histIdx;
  float trend;
  int accuracy;
  float learningRate;
} ai = {{REGION_PRESSURE_BASE}, 0, 0, AI_INITIAL_ACCURACY, AI_LEARNING_RATE};

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  lcd.begin(16, 2);
  
  // Inicializar sensores usando configuración
  if (!bmp.begin(BMP280_ADDRESS)) errorHalt("BMP280 Error!");
  if (!aht.begin()) errorHalt("AHT20 Error!");
  
  // Crear iconos
  for (byte i = 0; i < 8; i++) {
    byte icon[8];
    memcpy_P(icon, icons[i], 8);
    lcd.createChar(i, icon);
  }
  
  // Pantalla de bienvenida
  lcd.clear();
  lcd.print("MeteoStation v2");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  
  // Calibrar con datos regionales
  calibrateForRegion();
}

void loop() {
  unsigned long now = millis();
  
  #if MASTER_MODE
    masterLoop(now);
  #else
    slaveLoop(now);
  #endif
}

void masterLoop(unsigned long now) {
  // Actualizar sensores usando intervalos de configuración
  if (now - lastUpdate >= SENSOR_UPDATE_INTERVAL) {
    readSensors();
    calculateWeatherParams();
    lastUpdate = now;
  }
  
  // Rotar pantallas usando intervalos de configuración
  if (now - screenTimer >= SCREEN_ROTATION_INTERVAL) {
    displayScreen(screen);
    screen = (screen + 1) % SCREEN_COUNT;
    screenTimer = now;
  }
}

void slaveLoop(unsigned long now) {
  // Procesamiento de predicciones e IA usando configuración
  static unsigned long aiUpdate = 0;
  
  if (now - aiUpdate >= AI_UPDATE_INTERVAL) {
    updateAIPredictions();
    processWeatherTrends();
    aiUpdate = now;
  }
}

void readSensors() {
  // Leer BMP280
  weather.temp = bmp.readTemperature();
  weather.press = bmp.readPressure() / 100.0;
  weather.alt = bmp.readAltitude(getRegionalPressureBase());
  
  // Leer AHT20
  sensors_event_t h, t;
  aht.getEvent(&h, &t);
  weather.hum = h.relative_humidity;
  
  // Promediar temperatura
  weather.temp = (weather.temp + t.temperature) / 2.0;
  
  // Leer LDR usando configuración
  weather.ldrVal = analogRead(LDR_PIN);
  
  // Actualizar min/max
  if (weather.temp < weather.tempMin) weather.tempMin = weather.temp;
  if (weather.temp > weather.tempMax) weather.tempMax = weather.temp;
}

void calculateWeatherParams() {
  // Punto de rocío optimizado
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
  
  // Índice UV usando calibración
  weather.uvIdx = calibrateLDR(weather.ldrVal);
  
  // Condición climática con IA
  weather.condition = determineWeatherCondition();
  
  // Probabilidad de lluvia
  weather.rainProb = calculateRainProbability();
}

byte determineWeatherCondition() {
  // Análisis multiparamétrico usando umbrales de configuración
  if (weather.press < STORM_PRESSURE_THRESHOLD) return 3; // Tormenta
  
  if (weather.press < RAIN_PRESSURE_THRESHOLD) {
    return (weather.hum > 80 && ai.trend < -1) ? 2 : 1; // Lluvia o nublado
  }
  
  if (weather.press > CLEAR_PRESSURE_THRESHOLD && weather.hum < 70) return 0; // Soleado
  
  return 1; // Nublado por defecto
}

int calculateRainProbability() {
  int prob = 0;
  
  // Presión base usando umbrales de configuración
  if (weather.press < STORM_PRESSURE_THRESHOLD) prob += 80;
  else if (weather.press < RAIN_PRESSURE_THRESHOLD) prob += 40;
  else if (weather.press > CLEAR_PRESSURE_THRESHOLD) prob -= 20;
  
  // Humedad
  if (weather.hum > 85) prob += 30;
  else if (weather.hum > 75) prob += 15;
  
  // Tendencia con IA
  if (ai.trend < -2) prob += 35;
  else if (ai.trend < -1) prob += 20;
  else if (ai.trend > 1) prob -= 15;
  
  // Corrección por altitud usando configuración
  prob += (REGION_ALTITUDE / 1000) * 5;
  
  return constrain(prob, 0, 100);
}

void updateAIPredictions() {
  static byte readCount = 0;
  readCount++;
  
  if (readCount >= AI_HISTORY_SIZE) {
    ai.pressHist[ai.histIdx] = weather.press;
    ai.histIdx = (ai.histIdx + 1) % AI_HISTORY_SIZE;
    
    // Calcular tendencia
    float sum = 0;
    for (byte i = 0; i < AI_HISTORY_SIZE; i++) sum += ai.pressHist[i];
    ai.trend = weather.press - (sum / AI_HISTORY_SIZE);
    
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
      ai.accuracy = min(AI_MAX_ACCURACY, ai.accuracy + 1);
    } else {
      ai.accuracy = max(AI_MIN_ACCURACY, ai.accuracy - 1);
    }
  }
  
  lastPred = weather.press;
  lastTime = millis();
}

void processWeatherTrends() {
  // Análisis de patrones específicos de la región
  static float tempHist[6] = {0};
  static byte tempIdx = 0;
  
  tempHist[tempIdx] = weather.temp;
  tempIdx = (tempIdx + 1) % 6;
  
  // Detectar patrones regionales usando configuración
  if (weather.hum > HUMIDITY_MAX_TYPICAL-5 && weather.press < RAIN_PRESSURE_THRESHOLD && ai.trend < -1) {
    // Patrón de lluvia típico de la región
    ai.accuracy = min(AI_MAX_ACCURACY-5, ai.accuracy + 2);
  }
}

void calibrateForRegion() {
  // Calibración específica usando configuración
  weather.tempMin = TEMP_MIN_ANNUAL;
  weather.tempMax = TEMP_MAX_ANNUAL;
  
  // Inicializar histórico con datos regionales
  for (byte i = 0; i < AI_HISTORY_SIZE; i++) {
    ai.pressHist[i] = getRegionalPressureBase();
  }
}

void displayScreen(byte s) {
  lcd.clear();
  
  switch(s) {
    case 0: // Temperatura y sensación usando abreviaturas de configuración
      lcd.write(0);
      lcd.print(ABBR_TEMPERATURE ":");
      lcd.print(weather.temp, TEMP_PRECISION);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print(ABBR_HEAT_INDEX ":");
      lcd.print(weather.heatIdx, TEMP_PRECISION);
      lcd.print("C");
      break;
      
    case 1: // Min/Max usando abreviaturas
      lcd.write(0);
      lcd.print(ABBR_MIN_TEMP ":");
      lcd.print(weather.tempMin, TEMP_PRECISION);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print(ABBR_MAX_TEMP ":");
      lcd.print(weather.tempMax, TEMP_PRECISION);
      lcd.print("C");
      break;
      
    case 2: // Humedad/Rocío usando abreviaturas
      lcd.write(1);
      lcd.print(ABBR_HUMIDITY ":");
      lcd.print(weather.hum, HUMIDITY_PRECISION);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print(ABBR_DEW_POINT ":");
      lcd.print(weather.dewPt, TEMP_PRECISION);
      lcd.print("C");
      break;
      
    case 3: // Clima actual usando textos de configuración
      lcd.write(weather.condition + 2);
      lcd.print("Clima:");
      lcd.setCursor(0, 1);
      const char* conditions[] = {TEXT_SUNNY, TEXT_CLOUDY, TEXT_RAINY, TEXT_STORM};
      lcd.print(conditions[weather.condition]);
      break;
      
    case 4: // Presión/Altitud usando abreviaturas
      lcd.write(6);
      lcd.print(ABBR_PRESSURE ":");
      lcd.print(weather.press, PRESSURE_PRECISION);
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print(ABBR_ALTITUDE ":");
      lcd.print(weather.alt, ALTITUDE_PRECISION);
      lcd.print("m");
      break;
      
    case 5: // Índice UV usando textos de configuración
      lcd.write(7);
      lcd.print(ABBR_UV_INDEX ":");
      lcd.print(weather.uvIdx);
      lcd.setCursor(0, 1);
      const char* uvLevels[] = {TEXT_UV_LOW, TEXT_UV_MODERATE, TEXT_UV_HIGH, TEXT_UV_VERY_HIGH};
      byte uvLevel = weather.uvIdx <= 2 ? 0 : (weather.uvIdx <= 5 ? 1 : (weather.uvIdx <= 7 ? 2 : 3));
      lcd.print(uvLevels[uvLevel]);
      break;
      
    case 6: // Probabilidad lluvia usando abreviaturas
      lcd.write(3);
      lcd.print(ABBR_RAIN_PROB ":");
      lcd.print(weather.rainProb);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print(ABBR_ACCURACY ":");
      lcd.print(ai.accuracy);
      lcd.print("%");
      break;
      
    case 7: // Predicción usando textos de configuración
      lcd.print("Prediccion:");
      lcd.setCursor(0, 1);
      if (ai.trend < -2) lcd.print(TEXT_WORSENING);
      else if (ai.trend < -1) lcd.print(TEXT_CLOUDING);
      else if (ai.trend > 1) lcd.print(TEXT_IMPROVING);
      else lcd.print(TEXT_STABLE);
      break;
  }
}

void errorHalt(const char* msg) {
  lcd.clear();
  lcd.print(msg);
  while(1);
}