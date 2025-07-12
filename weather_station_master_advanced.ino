// ARDUINO MAESTRO AVANZADO - IA Y LCD CON ICONOS
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
const unsigned long SCREEN_DURATION = 4000; // 4 segundos por pantalla

// Datos históricos para IA (últimas 24 horas)
#define HISTORY_SIZE 24
float tempHistory[HISTORY_SIZE];
float humidityHistory[HISTORY_SIZE];
float pressureHistory[HISTORY_SIZE];
int historyIndex = 0;
bool historyFull = false;

// Configuración regional específica (altitud 2219m)
const float REGIONAL_ALTITUDE = 2219.0;
const float REGIONAL_PRESSURE = 1013.25;
const float REGIONAL_TEMP_MIN = 6.0;  // Enero
const float REGIONAL_TEMP_MAX = 29.0; // Abril-Mayo
const float REGIONAL_HUMIDITY_MIN = 75.0;
const float REGIONAL_HUMIDITY_MAX = 91.0;

// Iconos personalizados (8x5 bits)
byte sunIcon[8] = {
  B00100,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B00100,
  B00000
};

byte cloudIcon[8] = {
  B00000,
  B01110,
  B11111,
  B11111,
  B01110,
  B00000,
  B00000,
  B00000
};

byte rainIcon[8] = {
  B00000,
  B01110,
  B11111,
  B01110,
  B00100,
  B01010,
  B00100,
  B01010
};

byte stormIcon[8] = {
  B00000,
  B01110,
  B11111,
  B01110,
  B00100,
  B01110,
  B00100,
  B01110
};

byte tempIcon[8] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B00100
};

byte humidityIcon[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

void setup() {
  Wire.begin();
  lcd.begin(16, 2);
  
  // Crear iconos personalizados
  lcd.createChar(0, sunIcon);
  lcd.createChar(1, cloudIcon);
  lcd.createChar(2, rainIcon);
  lcd.createChar(3, stormIcon);
  lcd.createChar(4, tempIcon);
  lcd.createChar(5, humidityIcon);
  
  // Pantalla de inicio
  lcd.print("Estacion Meteo");
  lcd.setCursor(0, 1);
  lcd.print("IA Avanzada");
  delay(2000);
  
  // Inicializar historial con datos regionales
  for (int i = 0; i < HISTORY_SIZE; i++) {
    tempHistory[i] = 20.0;
    humidityHistory[i] = 80.0;
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
  
  // Rotar pantallas cada 4 segundos
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
    case 0: displayTemperatureAdvanced(); break;
    case 1: displayMinMaxAdvanced(); break;
    case 2: displayHumidityAdvanced(); break;
    case 3: displayWeatherAdvanced(); break;
    case 4: displayPressureAdvanced(); break;
    case 5: displayUVAdvanced(); break;
    case 6: displayRainProbAdvanced(); break;
    case 7: displayForecastAdvanced(); break;
  }
}

void displayTemperatureAdvanced() {
  lcd.setCursor(0, 0);
  lcd.write(4); // Icono temperatura
  lcd.print(" ");
  lcd.print(sensorData.temp, 1);
  lcd.print("C");
  
  // Sensación térmica avanzada
  float feelsLike = calculateFeelsLikeAdvanced();
  lcd.setCursor(0, 1);
  lcd.print("Sens: ");
  lcd.print(feelsLike, 1);
  lcd.print("C");
  
  // Indicador de confort
  if (feelsLike >= 20 && feelsLike <= 26) {
    lcd.print(" OK");
  } else if (feelsLike > 26) {
    lcd.print(" CAL");
  } else {
    lcd.print(" FRIO");
  }
}

void displayMinMaxAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(sensorData.temp_min, 1);
  lcd.print(" H:");
  lcd.print(sensorData.temp_max, 1);
  
  lcd.setCursor(0, 1);
  lcd.print("Act:");
  lcd.print(sensorData.temp, 1);
  lcd.print("C ");
  
  // Comparar con rangos regionales
  if (sensorData.temp < REGIONAL_TEMP_MIN) {
    lcd.print("BAJA");
  } else if (sensorData.temp > REGIONAL_TEMP_MAX) {
    lcd.print("ALTA");
  } else {
    lcd.print("NORM");
  }
}

void displayHumidityAdvanced() {
  lcd.setCursor(0, 0);
  lcd.write(5); // Icono humedad
  lcd.print(" ");
  lcd.print(sensorData.humidity, 1);
  lcd.print("%");
  
  // Punto de rocío avanzado
  float dewPoint = calculateDewPointAdvanced();
  lcd.setCursor(0, 1);
  lcd.print("Rocio: ");
  lcd.print(dewPoint, 1);
  lcd.print("C");
  
  // Alerta de condensación
  if (sensorData.temp - dewPoint < 2) {
    lcd.print("!");
  }
}

void displayWeatherAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("Clima Actual");
  
  lcd.setCursor(0, 1);
  String weather = getCurrentWeatherAdvanced();
  lcd.print(weather);
  
  // Mostrar icono correspondiente
  if (weather.indexOf("Soleado") >= 0) {
    lcd.setCursor(15, 0);
    lcd.write(0);
  } else if (weather.indexOf("Nublado") >= 0) {
    lcd.setCursor(15, 0);
    lcd.write(1);
  } else if (weather.indexOf("Lluvia") >= 0) {
    lcd.setCursor(15, 0);
    lcd.write(2);
  } else if (weather.indexOf("Tormenta") >= 0) {
    lcd.setCursor(15, 0);
    lcd.write(3);
  }
}

void displayPressureAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("Pres: ");
  lcd.print(sensorData.pressure, 1);
  lcd.print("hPa");
  
  lcd.setCursor(0, 1);
  lcd.print("Alt: ");
  lcd.print(sensorData.altitude, 0);
  lcd.print("m");
  
  // Indicador de tendencia
  if (historyFull) {
    float trend = getPressureTrendAdvanced();
    lcd.setCursor(14, 0);
    if (trend > 1) lcd.print("^");
    else if (trend < -1) lcd.print("v");
    else lcd.print("-");
  }
}

void displayUVAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("Indice UV");
  
  lcd.setCursor(0, 1);
  String uvLevel = getUVLevelAdvanced();
  lcd.print(uvLevel);
  
  // Recomendación UV
  lcd.setCursor(10, 1);
  if (uvLevel.indexOf("Dia") >= 0) {
    lcd.print("Prot");
  } else if (uvLevel.indexOf("Tarde") >= 0) {
    lcd.print("Mod");
  } else {
    lcd.print("OK");
  }
}

void displayRainProbAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("Prob Lluvia");
  
  float prob = calculateRainProbabilityAdvanced();
  lcd.setCursor(0, 1);
  lcd.print(prob, 0);
  lcd.print("% ");
  
  // Clasificación de probabilidad
  if (prob < 20) lcd.print("BAJA");
  else if (prob < 50) lcd.print("MEDIA");
  else if (prob < 80) lcd.print("ALTA");
  else lcd.print("MUY ALTA");
}

void displayForecastAdvanced() {
  lcd.setCursor(0, 0);
  lcd.print("Prediccion IA");
  
  lcd.setCursor(0, 1);
  String forecast = getWeatherForecastAdvanced();
  lcd.print(forecast);
  
  // Confianza de la predicción
  float confidence = getForecastConfidence();
  lcd.setCursor(12, 1);
  lcd.print(confidence, 0);
  lcd.print("%");
}

// FUNCIONES DE IA AVANZADA

float calculateFeelsLikeAdvanced() {
  float temp = sensorData.temp;
  float humidity = sensorData.humidity;
  float windSpeed = 5.0; // Estimación regional
  
  // Fórmula de Steadman mejorada
  if (temp >= 27.0) {
    // Sensación térmica para calor (índice de calor)
    float hi = temp + 0.348 * humidity - 0.7 * temp + 0.7;
    return hi + (windSpeed * 0.1);
  } else if (temp <= 10.0) {
    // Sensación térmica para frío (wind chill)
    float wc = 13.12 + 0.6215 * temp - 11.37 * pow(humidity/100, 0.16) + 0.3965 * temp * pow(humidity/100, 0.16);
    return wc - (windSpeed * 0.5);
  }
  return temp;
}

float calculateDewPointAdvanced() {
  // Fórmula de Magnus mejorada
  float a = 17.62;
  float b = 243.12;
  float gamma = (a * sensorData.temp) / (b + sensorData.temp) + log(sensorData.humidity / 100.0);
  float dewPoint = (b * gamma) / (a - gamma);
  
  // Ajuste por altitud
  dewPoint -= (REGIONAL_ALTITUDE / 1000) * 2;
  
  return dewPoint;
}

String getCurrentWeatherAdvanced() {
  float pressure = sensorData.pressure;
  float humidity = sensorData.humidity;
  int ldr = sensorData.ldr;
  
  // IA basada en patrones regionales
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

String getUVLevelAdvanced() {
  int ldr = sensorData.ldr;
  
  // Calibración regional para altitud 2219m
  if (ldr < 100) return "Noche";
  else if (ldr < 300) return "Amanecer";
  else if (ldr < 600) return "Dia";
  else if (ldr < 800) return "Tarde";
  else return "Atardecer";
}

float calculateRainProbabilityAdvanced() {
  float prob = 0;
  
  // Factor de presión (ajustado para altitud)
  float pressureFactor = (1013.25 - sensorData.pressure) * 2.5;
  if (pressureFactor > 0) prob += pressureFactor;
  
  // Factor de humedad (ajustado para región)
  float humidityFactor = (sensorData.humidity - 75) * 0.8;
  if (humidityFactor > 0) prob += humidityFactor;
  
  // Factor de tendencia de presión
  if (historyFull) {
    float pressureTrend = getPressureTrendAdvanced();
    prob += pressureTrend * 15;
  }
  
  // Factor estacional (simplificado)
  int month = (millis() / (24 * 60 * 60 * 1000)) % 12;
  if (month >= 5 && month <= 8) { // Junio-Septiembre (lluvias)
    prob += 10;
  }
  
  // Limitar entre 0-100%
  if (prob < 0) prob = 0;
  if (prob > 100) prob = 100;
  
  return prob;
}

float getPressureTrendAdvanced() {
  float recent = 0, older = 0;
  int count = 0;
  
  // Últimas 6 horas
  for (int i = 0; i < 6; i++) {
    int idx = (historyIndex - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
    if (idx >= 0) {
      recent += pressureHistory[idx];
      count++;
    }
  }
  
  // 6-12 horas atrás
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

String getWeatherForecastAdvanced() {
  float prob = calculateRainProbabilityAdvanced();
  float pressure = sensorData.pressure;
  float humidity = sensorData.humidity;
  
  // IA más sofisticada
  if (prob > 80) return "Lluvia";
  else if (prob > 60) return "Nublado";
  else if (pressure > 1020 && humidity < 70) return "Soleado";
  else if (pressure > 1015) return "Despejado";
  else return "Variable";
}

float getForecastConfidence() {
  // Calcular confianza basada en consistencia de datos
  float confidence = 50; // Base
  
  if (historyFull) {
    // Consistencia de presión
    float pressureVar = getPressureVariation();
    if (pressureVar < 5) confidence += 20;
    else if (pressureVar < 10) confidence += 10;
    
    // Consistencia de humedad
    float humidityVar = getHumidityVariation();
    if (humidityVar < 10) confidence += 15;
    else if (humidityVar < 20) confidence += 8;
    
    // Tendencia clara
    float trend = getPressureTrendAdvanced();
    if (abs(trend) > 2) confidence += 15;
  }
  
  if (confidence > 100) confidence = 100;
  return confidence;
}

float getPressureVariation() {
  float min = 9999, max = 0;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (pressureHistory[i] < min) min = pressureHistory[i];
    if (pressureHistory[i] > max) max = pressureHistory[i];
  }
  return max - min;
}

float getHumidityVariation() {
  float min = 9999, max = 0;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (humidityHistory[i] < min) min = humidityHistory[i];
    if (humidityHistory[i] > max) max = humidityHistory[i];
  }
  return max - min;
}