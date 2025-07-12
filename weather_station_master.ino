// ARDUINO MAESTRO - CÁLCULOS, IA Y LCD
// Optimizado para menos de 2048 bytes
// Pines LCD: 13,12,11,10,9,8

#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

// Configuración LCD
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Configuración I2C
#define SLAVE_ADDRESS 0x08

// Estructura de datos del esclavo
struct SensorData {
  float temp, humidity, pressure, altitude;
  int ldr;
  float temp_min, temp_max;
  unsigned long timestamp;
};

// Variables globales
SensorData sensorData;
unsigned long lastUpdate = 0;
int currentScreen = 0;
const int SCREEN_COUNT = 8;
unsigned long screenTimer = 0;
const unsigned long SCREEN_DURATION = 3000; // 3 segundos por pantalla

// Datos históricos para IA (últimas 24 horas)
#define HISTORY_SIZE 24
float tempHistory[HISTORY_SIZE];
float humidityHistory[HISTORY_SIZE];
float pressureHistory[HISTORY_SIZE];
int historyIndex = 0;
bool historyFull = false;

// Configuración regional (altitud 2219m)
const float REGIONAL_ALTITUDE = 2219.0;
const float REGIONAL_PRESSURE = 1013.25;

void setup() {
  Wire.begin();
  lcd.begin(16, 2);
  
  // Pantalla de inicio
  lcd.print("Estacion Meteo");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  
  // Inicializar historial
  for (int i = 0; i < HISTORY_SIZE; i++) {
    tempHistory[i] = 20.0;
    humidityHistory[i] = 75.0;
    pressureHistory[i] = 1013.25;
  }
}

void loop() {
  // Solicitar datos al esclavo cada segundo
  if (millis() - lastUpdate >= 1000) {
    requestSensorData();
    updateHistory();
    lastUpdate = millis();
  }
  
  // Rotar pantallas cada 3 segundos
  if (millis() - screenTimer >= SCREEN_DURATION) {
    currentScreen = (currentScreen + 1) % SCREEN_COUNT;
    screenTimer = millis();
  }
  
  // Mostrar pantalla actual
  displayCurrentScreen();
  
  delay(100);
}

void requestSensorData() {
  Wire.requestFrom(SLAVE_ADDRESS, sizeof(SensorData));
  if (Wire.available() >= sizeof(SensorData)) {
    Wire.readBytes((uint8_t*)&sensorData, sizeof(SensorData));
  }
}

void updateHistory() {
  tempHistory[historyIndex] = sensorData.temp;
  humidityHistory[historyIndex] = sensorData.humidity;
  pressureHistory[historyIndex] = sensorData.pressure;
  
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  if (historyIndex == 0) historyFull = true;
}

void displayCurrentScreen() {
  lcd.clear();
  
  switch (currentScreen) {
    case 0: displayTemperature(); break;
    case 1: displayMinMax(); break;
    case 2: displayHumidity(); break;
    case 3: displayWeather(); break;
    case 4: displayPressure(); break;
    case 5: displayUV(); break;
    case 6: displayRainProb(); break;
    case 7: displayForecast(); break;
  }
}

void displayTemperature() {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(sensorData.temp, 1);
  lcd.print("C");
  
  // Sensación térmica
  float feelsLike = calculateFeelsLike();
  lcd.setCursor(0, 1);
  lcd.print("Sens: ");
  lcd.print(feelsLike, 1);
  lcd.print("C");
}

void displayMinMax() {
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(sensorData.temp_min, 1);
  lcd.print(" H:");
  lcd.print(sensorData.temp_max, 1);
  
  lcd.setCursor(0, 1);
  lcd.print("Actual: ");
  lcd.print(sensorData.temp, 1);
  lcd.print("C");
}

void displayHumidity() {
  lcd.setCursor(0, 0);
  lcd.print("HR: ");
  lcd.print(sensorData.humidity, 1);
  lcd.print("%");
  
  // Punto de rocío
  float dewPoint = calculateDewPoint();
  lcd.setCursor(0, 1);
  lcd.print("Rocio: ");
  lcd.print(dewPoint, 1);
  lcd.print("C");
}

void displayWeather() {
  lcd.setCursor(0, 0);
  lcd.print("Clima Actual");
  
  lcd.setCursor(0, 1);
  String weather = getCurrentWeather();
  lcd.print(weather);
}

void displayPressure() {
  lcd.setCursor(0, 0);
  lcd.print("Pres: ");
  lcd.print(sensorData.pressure, 1);
  lcd.print("hPa");
  
  lcd.setCursor(0, 1);
  lcd.print("Alt: ");
  lcd.print(sensorData.altitude, 0);
  lcd.print("m");
}

void displayUV() {
  lcd.setCursor(0, 0);
  lcd.print("Indice UV");
  
  lcd.setCursor(0, 1);
  String uvLevel = getUVLevel();
  lcd.print(uvLevel);
}

void displayRainProb() {
  lcd.setCursor(0, 0);
  lcd.print("Prob Lluvia");
  
  float prob = calculateRainProbability();
  lcd.setCursor(0, 1);
  lcd.print(prob, 0);
  lcd.print("%");
}

void displayForecast() {
  lcd.setCursor(0, 0);
  lcd.print("Prediccion");
  
  lcd.setCursor(0, 1);
  String forecast = getWeatherForecast();
  lcd.print(forecast);
}

// FUNCIONES DE CÁLCULO Y IA

float calculateFeelsLike() {
  // Fórmula de sensación térmica simplificada
  float temp = sensorData.temp;
  float humidity = sensorData.humidity;
  
  if (temp >= 27.0) {
    // Sensación térmica para calor
    return temp + 0.348 * humidity - 0.7 * temp + 0.7;
  } else if (temp <= 10.0) {
    // Sensación térmica para frío
    return 13.12 + 0.6215 * temp - 11.37 * pow(humidity/100, 0.16) + 0.3965 * temp * pow(humidity/100, 0.16);
  }
  return temp;
}

float calculateDewPoint() {
  // Fórmula de Magnus para punto de rocío
  float a = 17.62;
  float b = 243.12;
  float gamma = (a * sensorData.temp) / (b + sensorData.temp) + log(sensorData.humidity / 100.0);
  return (b * gamma) / (a - gamma);
}

String getCurrentWeather() {
  float pressure = sensorData.pressure;
  float humidity = sensorData.humidity;
  int ldr = sensorData.ldr;
  
  // Lógica de clima basada en presión, humedad y luz
  if (pressure < 1000) {
    if (humidity > 85) return "Tormenta";
    return "Lluvia";
  } else if (pressure < 1010) {
    if (humidity > 80) return "Nublado";
    return "Parcial";
  } else if (pressure > 1020) {
    if (ldr > 500) return "Soleado";
    return "Despejado";
  } else {
    if (humidity > 75) return "Nublado";
    return "Variable";
  }
}

String getUVLevel() {
  int ldr = sensorData.ldr;
  
  if (ldr < 100) return "Noche";
  else if (ldr < 300) return "Amanecer";
  else if (ldr < 600) return "Dia";
  else if (ldr < 800) return "Tarde";
  else return "Atardecer";
}

float calculateRainProbability() {
  // IA simple basada en datos históricos y actuales
  float prob = 0;
  
  // Factor de presión (baja presión = más lluvia)
  float pressureFactor = (1013.25 - sensorData.pressure) * 2;
  if (pressureFactor > 0) prob += pressureFactor;
  
  // Factor de humedad
  prob += (sensorData.humidity - 60) * 0.5;
  
  // Factor de tendencia de presión
  if (historyFull) {
    float pressureTrend = getPressureTrend();
    prob += pressureTrend * 10;
  }
  
  // Limitar entre 0-100%
  if (prob < 0) prob = 0;
  if (prob > 100) prob = 100;
  
  return prob;
}

float getPressureTrend() {
  // Calcular tendencia de presión en las últimas horas
  float recent = 0, older = 0;
  int count = 0;
  
  for (int i = 0; i < 6; i++) {
    int idx = (historyIndex - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
    if (idx >= 0) {
      recent += pressureHistory[idx];
      count++;
    }
  }
  
  for (int i = 6; i < 12; i++) {
    int idx = (historyIndex - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
    if (idx >= 0) {
      older += pressureHistory[idx];
    }
  }
  
  if (count > 0) {
    return (older / 6) - (recent / count);
  }
  return 0;
}

String getWeatherForecast() {
  float prob = calculateRainProbability();
  float pressure = sensorData.pressure;
  
  if (prob > 70) return "Lluvia";
  else if (prob > 40) return "Nublado";
  else if (pressure > 1020) return "Soleado";
  else return "Variable";
}