// ARDUINO ESCLAVO - SISTEMA DE PREDICCIÓN METEOROLÓGICA CON IA
// Recibe datos del Arduino Maestro via I2C
// Procesa datos históricos para predicciones precisas
// Implementa algoritmos de IA para análisis meteorológico

#include <Wire.h>
#include <EEPROM.h>

// Direcciones I2C
#define SLAVE_ADDRESS 0x08
#define MASTER_ADDRESS 0x04

// Estructura de datos recibidos
struct WeatherData {
  float temp;
  float humidity;
  float pressure;
  int lightLevel;
  float tempHistory[6];
  float pressureHistory[6];
  float humidityHistory[6];
};

// Variables globales
WeatherData currentData;
bool dataReceived = false;
unsigned long lastPrediction = 0;
int predictionInterval = 300000; // 5 minutos

// Datos históricos para IA (últimas 48 horas)
float tempPattern[48];
float pressurePattern[48];
float humidityPattern[48];
int patternIndex = 0;
bool patternFull = false;

// Parámetros de la región (basados en datos proporcionados)
const float REGION_TEMP_MIN = 6.0;
const float REGION_TEMP_MAX = 29.0;
const float REGION_HUMIDITY_MIN = 75.0;
const float REGION_HUMIDITY_MAX = 91.0;
const float REGION_PRESSURE_SEA = 1013.25;
const float REGION_ALTITUDE = 2219.0;
const float REGION_PRESSURE_BASE = REGION_PRESSURE_SEA * pow(1 - 0.0065 * REGION_ALTITUDE / 288.15, 5.255);

// Coeficientes de IA (aprendizaje automático simple)
struct AICoefficients {
  float tempWeight;
  float pressureWeight;
  float humidityWeight;
  float trendWeight;
  float seasonalWeight;
} aiCoeffs = {0.3, 0.4, 0.2, 0.8, 0.1};

// Predicciones generadas
struct Predictions {
  float temp24h;
  float pressure24h;
  float humidity24h;
  int rainProb24h;
  int weatherType; // 0=soleado, 1=nublado, 2=lluvioso, 3=tormenta
  float confidence;
} predictions;

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  
  // Cargar coeficientes de IA desde EEPROM
  loadAICoefficients();
  
  // Inicializar patrones históricos
  initializePatterns();
  
  Serial.println("Arduino Esclavo - Sistema de Predicción IA");
  Serial.println("Esperando datos del maestro...");
}

void loop() {
  // Procesar datos si se recibieron nuevos
  if (dataReceived) {
    processWeatherData();
    dataReceived = false;
  }
  
  // Generar predicciones cada 5 minutos
  if (millis() - lastPrediction > predictionInterval) {
    generatePredictions();
    updateAILearning();
    saveAICoefficients();
    lastPrediction = millis();
  }
  
  delay(1000);
}

void receiveData(int bytes) {
  if (bytes == sizeof(WeatherData)) {
    Wire.readBytes((byte*)&currentData, sizeof(WeatherData));
    dataReceived = true;
    Serial.println("Datos recibidos del maestro");
  }
}

void processWeatherData() {
  // Almacenar datos en patrones históricos
  tempPattern[patternIndex] = currentData.temp;
  pressurePattern[patternIndex] = currentData.pressure;
  humidityPattern[patternIndex] = currentData.humidity;
  
  patternIndex = (patternIndex + 1) % 48;
  if (patternIndex == 0) patternFull = true;
  
  // Análisis de tendencias
  analyzeTrends();
  
  // Detección de patrones
  detectWeatherPatterns();
  
  Serial.print("Temp: "); Serial.print(currentData.temp);
  Serial.print(" °C, Presión: "); Serial.print(currentData.pressure);
  Serial.print(" hPa, Humedad: "); Serial.println(currentData.humidity);
}

void analyzeTrends() {
  if (!patternFull && patternIndex < 6) return;
  
  // Calcular tendencias de corto plazo (últimas 6 horas)
  float tempTrend = 0, pressureTrend = 0, humidityTrend = 0;
  int samples = min(6, patternIndex);
  
  for (int i = 1; i < samples; i++) {
    int idx = (patternIndex - i + 48) % 48;
    int prevIdx = (patternIndex - i - 1 + 48) % 48;
    
    tempTrend += tempPattern[idx] - tempPattern[prevIdx];
    pressureTrend += pressurePattern[idx] - pressurePattern[prevIdx];
    humidityTrend += humidityPattern[idx] - humidityPattern[prevIdx];
  }
  
  tempTrend /= (samples - 1);
  pressureTrend /= (samples - 1);
  humidityTrend /= (samples - 1);
  
  Serial.print("Tendencias - Temp: "); Serial.print(tempTrend);
  Serial.print(", Presión: "); Serial.print(pressureTrend);
  Serial.print(", Humedad: "); Serial.println(humidityTrend);
}

void detectWeatherPatterns() {
  if (!patternFull) return;
  
  // Detectar patrones de 24 horas
  float correlation = 0;
  int bestMatch = -1;
  
  for (int offset = 24; offset < 48; offset++) {
    float corr = calculateCorrelation(offset);
    if (corr > correlation) {
      correlation = corr;
      bestMatch = offset;
    }
  }
  
  if (bestMatch > 0 && correlation > 0.7) {
    Serial.print("Patrón detectado - Correlación: ");
    Serial.print(correlation);
    Serial.print(", Offset: ");
    Serial.println(bestMatch);
  }
}

float calculateCorrelation(int offset) {
  float sum1 = 0, sum2 = 0, sum1Sq = 0, sum2Sq = 0, pSum = 0;
  int n = 12; // Últimas 12 horas
  
  for (int i = 0; i < n; i++) {
    int idx1 = (patternIndex - i + 48) % 48;
    int idx2 = (patternIndex - i - offset + 48) % 48;
    
    float val1 = pressurePattern[idx1];
    float val2 = pressurePattern[idx2];
    
    sum1 += val1;
    sum2 += val2;
    sum1Sq += val1 * val1;
    sum2Sq += val2 * val2;
    pSum += val1 * val2;
  }
  
  float num = pSum - (sum1 * sum2 / n);
  float den = sqrt((sum1Sq - sum1 * sum1 / n) * (sum2Sq - sum2 * sum2 / n));
  
  return den != 0 ? num / den : 0;
}

void generatePredictions() {
  if (!patternFull && patternIndex < 6) return;
  
  // Predicción de temperatura usando IA
  predictions.temp24h = predictTemperature();
  
  // Predicción de presión
  predictions.pressure24h = predictPressure();
  
  // Predicción de humedad
  predictions.humidity24h = predictHumidity();
  
  // Predicción de probabilidad de lluvia
  predictions.rainProb24h = predictRainProbability();
  
  // Tipo de clima
  predictions.weatherType = predictWeatherType();
  
  // Nivel de confianza
  predictions.confidence = calculateConfidence();
  
  // Mostrar predicciones
  printPredictions();
}

float predictTemperature() {
  // Algoritmo de predicción basado en tendencias y patrones estacionales
  float currentTemp = currentData.temp;
  float trend = calculateTempTrend();
  float seasonal = calculateSeasonalFactor();
  
  // Aplicar pesos de IA
  float prediction = currentTemp + 
                    (trend * aiCoeffs.tempWeight) + 
                    (seasonal * aiCoeffs.seasonalWeight);
  
  // Limitar a rangos regionales
  prediction = constrain(prediction, REGION_TEMP_MIN, REGION_TEMP_MAX);
  
  return prediction;
}

float predictPressure() {
  float currentPressure = currentData.pressure;
  float trend = calculatePressureTrend();
  
  // Predicción basada en tendencias y ciclos diarios
  float prediction = currentPressure + (trend * aiCoeffs.pressureWeight);
  
  // Ajustar por altitud regional
  prediction = constrain(prediction, REGION_PRESSURE_BASE - 50, REGION_PRESSURE_BASE + 50);
  
  return prediction;
}

float predictHumidity() {
  float currentHumidity = currentData.humidity;
  float trend = calculateHumidityTrend();
  
  float prediction = currentHumidity + (trend * aiCoeffs.humidityWeight);
  
  // Limitar a rangos regionales
  prediction = constrain(prediction, REGION_HUMIDITY_MIN, REGION_HUMIDITY_MAX);
  
  return prediction;
}

int predictRainProbability() {
  float pressureTrend = calculatePressureTrend();
  float humidityLevel = currentData.humidity;
  
  int rainProb = 0;
  
  // Algoritmo de predicción de lluvia específico para la región
  if (pressureTrend < -3.0 && humidityLevel > 85) {
    rainProb = 90; // Muy probable
  } else if (pressureTrend < -2.0 && humidityLevel > 80) {
    rainProb = 75; // Probable
  } else if (pressureTrend < -1.0 && humidityLevel > 75) {
    rainProb = 50; // Posible
  } else if (humidityLevel > 85) {
    rainProb = 30; // Baja probabilidad
  } else {
    rainProb = 10; // Muy baja
  }
  
  // Ajustar por época del año (junio-septiembre más lluvioso)
  int month = getApproximateMonth();
  if (month >= 6 && month <= 9) {
    rainProb += 20; // Temporada de lluvias
  }
  
  return constrain(rainProb, 0, 100);
}

int predictWeatherType() {
  float pressure = predictions.pressure24h;
  int rainProb = predictions.rainProb24h;
  
  if (rainProb > 70) {
    return pressure < 995 ? 3 : 2; // Tormenta o lluvia
  } else if (pressure > 1020) {
    return 0; // Soleado
  } else {
    return 1; // Nublado
  }
}

float calculateConfidence() {
  // Calcular confianza basada en estabilidad de datos
  float tempVariance = calculateVariance(tempPattern, 6);
  float pressureVariance = calculateVariance(pressurePattern, 6);
  
  float stability = 1.0 / (1.0 + tempVariance + pressureVariance * 0.1);
  
  return constrain(stability, 0.3, 1.0);
}

float calculateTempTrend() {
  if (patternIndex < 3) return 0;
  
  float sum = 0;
  int samples = min(6, patternIndex);
  
  for (int i = 1; i < samples; i++) {
    int idx = (patternIndex - i + 48) % 48;
    int prevIdx = (patternIndex - i - 1 + 48) % 48;
    sum += tempPattern[idx] - tempPattern[prevIdx];
  }
  
  return sum / (samples - 1);
}

float calculatePressureTrend() {
  if (patternIndex < 3) return 0;
  
  float sum = 0;
  int samples = min(6, patternIndex);
  
  for (int i = 1; i < samples; i++) {
    int idx = (patternIndex - i + 48) % 48;
    int prevIdx = (patternIndex - i - 1 + 48) % 48;
    sum += pressurePattern[idx] - pressurePattern[prevIdx];
  }
  
  return sum / (samples - 1);
}

float calculateHumidityTrend() {
  if (patternIndex < 3) return 0;
  
  float sum = 0;
  int samples = min(6, patternIndex);
  
  for (int i = 1; i < samples; i++) {
    int idx = (patternIndex - i + 48) % 48;
    int prevIdx = (patternIndex - i - 1 + 48) % 48;
    sum += humidityPattern[idx] - humidityPattern[prevIdx];
  }
  
  return sum / (samples - 1);
}

float calculateSeasonalFactor() {
  // Aproximar factor estacional basado en tiempo
  int month = getApproximateMonth();
  
  if (month >= 4 && month <= 9) {
    return 2.0; // Época más cálida
  } else if (month == 1 || month == 12) {
    return -3.0; // Época más fría
  } else {
    return 0.0; // Época intermedia
  }
}

int getApproximateMonth() {
  // Aproximar mes basado en millis() (muy básico)
  return ((millis() / 86400000) % 365) / 30 + 1;
}

float calculateVariance(float* data, int size) {
  float mean = 0;
  for (int i = 0; i < size; i++) {
    int idx = (patternIndex - i + 48) % 48;
    mean += data[idx];
  }
  mean /= size;
  
  float variance = 0;
  for (int i = 0; i < size; i++) {
    int idx = (patternIndex - i + 48) % 48;
    variance += pow(data[idx] - mean, 2);
  }
  
  return variance / size;
}

void updateAILearning() {
  // Actualizar coeficientes de IA basado en precisión de predicciones anteriores
  // (Implementación simplificada de aprendizaje automático)
  
  if (patternIndex < 12) return;
  
  // Calcular error de predicción anterior
  float tempError = abs(currentData.temp - predictions.temp24h);
  float pressureError = abs(currentData.pressure - predictions.pressure24h);
  
  // Ajustar pesos basado en errores
  if (tempError > 2.0) {
    aiCoeffs.tempWeight *= 0.95; // Reducir peso si error alto
  } else if (tempError < 0.5) {
    aiCoeffs.tempWeight *= 1.05; // Aumentar peso si error bajo
  }
  
  if (pressureError > 5.0) {
    aiCoeffs.pressureWeight *= 0.95;
  } else if (pressureError < 1.0) {
    aiCoeffs.pressureWeight *= 1.05;
  }
  
  // Normalizar pesos
  float totalWeight = aiCoeffs.tempWeight + aiCoeffs.pressureWeight + 
                     aiCoeffs.humidityWeight + aiCoeffs.trendWeight;
  
  if (totalWeight > 0) {
    aiCoeffs.tempWeight /= totalWeight;
    aiCoeffs.pressureWeight /= totalWeight;
    aiCoeffs.humidityWeight /= totalWeight;
    aiCoeffs.trendWeight /= totalWeight;
  }
}

void initializePatterns() {
  // Inicializar patrones con valores típicos de la región
  for (int i = 0; i < 48; i++) {
    tempPattern[i] = 20.0;
    pressurePattern[i] = REGION_PRESSURE_BASE;
    humidityPattern[i] = 80.0;
  }
}

void loadAICoefficients() {
  // Cargar coeficientes desde EEPROM
  EEPROM.get(0, aiCoeffs);
  
  // Validar coeficientes
  if (aiCoeffs.tempWeight < 0 || aiCoeffs.tempWeight > 1) {
    // Restaurar valores por defecto
    aiCoeffs.tempWeight = 0.3;
    aiCoeffs.pressureWeight = 0.4;
    aiCoeffs.humidityWeight = 0.2;
    aiCoeffs.trendWeight = 0.8;
    aiCoeffs.seasonalWeight = 0.1;
  }
}

void saveAICoefficients() {
  // Guardar coeficientes en EEPROM
  EEPROM.put(0, aiCoeffs);
}

void printPredictions() {
  Serial.println("=== PREDICCIONES IA ===");
  Serial.print("Temp 24h: "); Serial.print(predictions.temp24h, 1); Serial.println("°C");
  Serial.print("Presión 24h: "); Serial.print(predictions.pressure24h, 1); Serial.println(" hPa");
  Serial.print("Humedad 24h: "); Serial.print(predictions.humidity24h, 1); Serial.println("%");
  Serial.print("Prob. lluvia: "); Serial.print(predictions.rainProb24h); Serial.println("%");
  
  Serial.print("Clima: ");
  switch (predictions.weatherType) {
    case 0: Serial.println("Soleado"); break;
    case 1: Serial.println("Nublado"); break;
    case 2: Serial.println("Lluvioso"); break;
    case 3: Serial.println("Tormenta"); break;
  }
  
  Serial.print("Confianza: "); Serial.print(predictions.confidence * 100, 1); Serial.println("%");
  Serial.println("=====================");
}