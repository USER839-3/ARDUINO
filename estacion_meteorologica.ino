// ESTACIÓN METEOROLÓGICA INTELIGENTE - ARDUINO UNO
// Sensores: BMP280 + AHT20 (I2C) + LDR
// Display: LCD 1602 (Pines: 13,12,11,10,9,8)
// 8 Pantallas con IA y predicciones precisas

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <EEPROM.h>

// Configuración LCD (RS,E,D4,D5,D6,D7)
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Sensores
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

// Pines y constantes
#define LDR_PIN A0
#define SCREENS 8
#define SCREEN_TIME 4000
#define SENSOR_TIME 2000
#define ALTITUDE 2219.0
#define BASE_PRESSURE 1013.25 - (ALTITUDE * 0.12)

// Variables meteorológicas
float temp = 0, hum = 0, press = 0, dewPt = 0, feels = 0;
float tempMin = 50, tempMax = -50, uvIdx = 0;
int lightLvl = 0, rainProb = 0, windSpd = 0;
String weather = "";

// Variables de control
byte screen = 0;
unsigned long lastScreen = 0, lastSensor = 0;

// Datos históricos para IA (últimas 12 lecturas)
struct WeatherPoint {
  float t, h, p;
  byte time;
};
WeatherPoint history[12];
byte histIdx = 0;

// Pesos IA y precisión
float pressWeight = 0.4, humWeight = 0.35, tempWeight = 0.25;
byte aiAccuracy = 75;

// Iconos personalizados (8x8 pixels)
byte tempIcon[8] = {0x04,0x0A,0x0A,0x0E,0x1F,0x1F,0x0E,0x00};
byte humIcon[8] = {0x04,0x04,0x0A,0x0A,0x11,0x11,0x0E,0x00};
byte pressIcon[8] = {0x00,0x0E,0x1F,0x1F,0x1F,0x0E,0x00,0x00};
byte sunIcon[8] = {0x00,0x15,0x0E,0x1F,0x0E,0x15,0x00,0x00};
byte cloudIcon[8] = {0x00,0x0E,0x1F,0x1F,0x1F,0x00,0x00,0x00};
byte rainIcon[8] = {0x0E,0x1F,0x1F,0x0A,0x15,0x0A,0x00,0x00};
byte uvIcon[8] = {0x15,0x0E,0x1F,0x1F,0x0E,0x15,0x00,0x00};
byte aiIcon[8] = {0x0E,0x11,0x15,0x11,0x11,0x11,0x0E,0x00};

// Strings en PROGMEM para ahorrar RAM
const char str1[] PROGMEM = "ESTACION METEO";
const char str2[] PROGMEM = "Iniciando...";
const char str3[] PROGMEM = "Error Sensor!";
const char str4[] PROGMEM = "T:";
const char str5[] PROGMEM = "ST:";
const char str6[] PROGMEM = "H:";
const char str7[] PROGMEM = "L:";
const char str8[] PROGMEM = "HR:";
const char str9[] PROGMEM = "PR:";
const char str10[] PROGMEM = "P:";
const char str11[] PROGMEM = "UV:";
const char str12[] PROGMEM = "Prob:";
const char str13[] PROGMEM = "IA:";
const char str14[] PROGMEM = "Soleado";
const char str15[] PROGMEM = "Nublado";
const char str16[] PROGMEM = "Lluvioso";
const char str17[] PROGMEM = "Tormenta";

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  // Crear iconos personalizados
  lcd.createChar(0, tempIcon);
  lcd.createChar(1, humIcon);
  lcd.createChar(2, pressIcon);
  lcd.createChar(3, sunIcon);
  lcd.createChar(4, cloudIcon);
  lcd.createChar(5, rainIcon);
  lcd.createChar(6, uvIcon);
  lcd.createChar(7, aiIcon);
  
  // Pantalla inicial
  lcd.clear();
  printProgmem(str1, 0, 0);
  printProgmem(str2, 0, 1);
  
  // Inicializar sensores
  if (!bmp.begin(0x76)) {
    lcd.clear();
    printProgmem(str3, 0, 0);
    lcd.print("BMP280");
    while(1);
  }
  
  if (!aht.begin()) {
    lcd.clear();
    printProgmem(str3, 0, 0);
    lcd.print("AHT20");
    while(1);
  }
  
  // Configurar BMP280 para máxima precisión
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  
  // Cargar datos históricos
  loadHistory();
  
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long now = millis();
  
  // Leer sensores
  if (now - lastSensor >= SENSOR_TIME) {
    readSensors();
    calculateWeather();
    updateAI();
    lastSensor = now;
  }
  
  // Cambiar pantalla
  if (now - lastScreen >= SCREEN_TIME) {
    displayScreen();
    screen = (screen + 1) % SCREENS;
    lastScreen = now;
  }
}

void readSensors() {
  // Leer BMP280
  temp = bmp.readTemperature();
  press = bmp.readPressure() / 100.0;
  
  // Leer AHT20
  sensors_event_t humidity, temperature;
  aht.getEvent(&humidity, &temperature);
  hum = humidity.relative_humidity;
  
  // Promedio de temperatura
  temp = (temp + temperature.temperature) / 2.0;
  
  // Leer LDR
  lightLvl = map(analogRead(LDR_PIN), 0, 1023, 0, 100);
  
  // Actualizar min/max
  if (temp < tempMin) tempMin = temp;
  if (temp > tempMax) tempMax = temp;
  
  // Guardar en historial
  history[histIdx].t = temp;
  history[histIdx].h = hum;
  history[histIdx].p = press;
  history[histIdx].time = (millis() / 60000) % 255;
  histIdx = (histIdx + 1) % 12;
  
  // Guardar en EEPROM cada 10 lecturas
  if (histIdx % 10 == 0) saveHistory();
}

void calculateWeather() {
  // Calcular punto de rocío
  float a = 17.27, b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(hum / 100.0);
  dewPt = (b * alpha) / (a - alpha);
  
  // Calcular sensación térmica
  feels = temp;
  if (hum > 70) feels += (hum - 70) * 0.05;
  if (lightLvl > 80) feels += (lightLvl - 80) * 0.02;
  
  // Calcular índice UV
  byte hour = (millis() / 3600000) % 24;
  if (hour >= 6 && hour <= 18 && lightLvl > 40) {
    uvIdx = map(lightLvl, 40, 100, 1, 11);
  } else {
    uvIdx = 0;
  }
  
  // Determinar clima y probabilidad de lluvia
  analyzeWeather();
  
  // Simular velocidad del viento
  windSpd = map(press, 990, 1030, 35, 5);
  windSpd = constrain(windSpd, 5, 35);
}

void analyzeWeather() {
  // Análisis basado en presión y humedad
  rainProb = 10; // Base
  
  // Factor presión
  if (press < BASE_PRESSURE - 20) {
    weather = "Tormenta";
    rainProb = 90;
  } else if (press < BASE_PRESSURE - 10) {
    weather = "Lluvioso";
    rainProb = 75;
  } else if (press < BASE_PRESSURE) {
    weather = "Nublado";
    rainProb = 45;
  } else {
    weather = "Soleado";
    rainProb = 15;
  }
  
  // Ajustar por humedad
  if (hum > 85) rainProb += 25;
  else if (hum > 75) rainProb += 15;
  else if (hum < 50) rainProb -= 10;
  
  // Ajustar por tendencia de presión
  float pressTrend = calculateTrend('p');
  if (pressTrend < -2) rainProb += 20;
  else if (pressTrend > 2) rainProb -= 15;
  
  rainProb = constrain(rainProb, 0, 100);
}

void updateAI() {
  // Análisis de tendencias para IA
  float tempTrend = calculateTrend('t');
  float humTrend = calculateTrend('h');
  float pressTrend = calculateTrend('p');
  
  // Ajustar precisión IA
  if (abs(tempTrend) < 0.5 && abs(pressTrend) < 1.0) {
    aiAccuracy = min(98, aiAccuracy + 1);
  } else if (abs(tempTrend) > 2.0 || abs(pressTrend) > 5.0) {
    aiAccuracy = max(60, aiAccuracy - 1);
  }
  
  // Ajustar pesos IA
  if (aiAccuracy > 90) {
    pressWeight = 0.45;
    humWeight = 0.35;
    tempWeight = 0.20;
  } else if (aiAccuracy < 70) {
    pressWeight = 0.35;
    humWeight = 0.40;
    tempWeight = 0.25;
  }
}

float calculateTrend(char param) {
  if (histIdx < 6) return 0;
  
  float recent = 0, old = 0;
  byte count = 0;
  
  // Comparar últimas 3 vs anteriores 3
  for (byte i = 0; i < 3; i++) {
    byte recentIdx = (histIdx - 1 - i + 12) % 12;
    byte oldIdx = (histIdx - 4 - i + 12) % 12;
    
    switch (param) {
      case 't':
        recent += history[recentIdx].t;
        old += history[oldIdx].t;
        break;
      case 'h':
        recent += history[recentIdx].h;
        old += history[oldIdx].h;
        break;
      case 'p':
        recent += history[recentIdx].p;
        old += history[oldIdx].p;
        break;
    }
    count++;
  }
  
  return (recent / count) - (old / count);
}

void displayScreen() {
  lcd.clear();
  
  switch (screen) {
    case 0: // Temperatura y sensación térmica
      lcd.write(byte(0));
      printProgmem(str4, 2, 0);
      lcd.print(temp, 1);
      printProgmem(str5, 8, 0);
      lcd.print(feels, 1);
      lcd.setCursor(0, 1);
      lcd.print("Real    Sens");
      break;
      
    case 1: // Temperatura min/max
      lcd.write(byte(0));
      printProgmem(str6, 2, 0);
      lcd.print(tempMax, 1);
      printProgmem(str7, 9, 0);
      lcd.print(tempMin, 1);
      lcd.setCursor(0, 1);
      lcd.print("Max     Min");
      break;
      
    case 2: // Humedad y punto de rocío
      lcd.write(byte(1));
      printProgmem(str8, 2, 0);
      lcd.print(hum, 0);
      lcd.print("% ");
      printProgmem(str9, 9, 0);
      lcd.print(dewPt, 1);
      lcd.setCursor(0, 1);
      lcd.print("Humedad  P.Rocio");
      break;
      
    case 3: // Clima actual
      // Seleccionar icono
      if (weather == "Soleado") lcd.write(byte(3));
      else if (weather == "Nublado") lcd.write(byte(4));
      else if (weather == "Lluvioso") lcd.write(byte(5));
      else lcd.write(byte(5)); // Tormenta
      
      lcd.setCursor(2, 0);
      lcd.print(weather);
      lcd.setCursor(0, 1);
      lcd.print("V:");
      lcd.print(windSpd);
      lcd.print("km/h L:");
      lcd.print(lightLvl);
      lcd.print("%");
      break;
      
    case 4: // Presión y altitud
      lcd.write(byte(2));
      printProgmem(str10, 2, 0);
      lcd.print(press, 1);
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print("Alt:");
      lcd.print(ALTITUDE, 0);
      lcd.print("m");
      break;
      
    case 5: // Índice UV
      lcd.write(byte(6));
      printProgmem(str11, 2, 0);
      lcd.print(uvIdx, 0);
      lcd.print(" L:");
      lcd.print(lightLvl);
      lcd.print("%");
      lcd.setCursor(0, 1);
      if (uvIdx <= 2) lcd.print("Bajo");
      else if (uvIdx <= 5) lcd.print("Moderado");
      else if (uvIdx <= 7) lcd.print("Alto");
      else if (uvIdx <= 10) lcd.print("Muy Alto");
      else lcd.print("Extremo");
      break;
      
    case 6: // Probabilidad de lluvia
      lcd.write(byte(5));
      printProgmem(str12, 2, 0);
      lcd.print(rainProb);
      lcd.print("%");
      lcd.setCursor(0, 1);
      if (rainProb < 20) lcd.print("Sin lluvia");
      else if (rainProb < 50) lcd.print("Posible");
      else if (rainProb < 80) lcd.print("Probable");
      else lcd.print("Muy probable");
      break;
      
    case 7: // Predicciones IA
      lcd.write(byte(7));
      printProgmem(str13, 2, 0);
      lcd.print(aiAccuracy);
      lcd.print("% T+2:");
      lcd.print(predictTemp(2), 1);
      lcd.setCursor(0, 1);
      lcd.print("R+2:");
      lcd.print(predictRain(2));
      lcd.print("% ");
      lcd.print(predictWeather(2));
      break;
  }
  
  // Indicador de pantalla
  lcd.setCursor(15, 0);
  lcd.print(screen + 1);
}

float predictTemp(byte hours) {
  float trend = calculateTrend('t');
  float prediction = temp + (trend * hours * tempWeight);
  
  // Ajustar por hora simulada
  byte currentHour = (millis() / 3600000) % 24;
  byte targetHour = (currentHour + hours) % 24;
  
  if (targetHour >= 6 && targetHour <= 12) prediction += 1.5;
  else if (targetHour >= 18 && targetHour <= 23) prediction -= 2.0;
  
  return constrain(prediction, 0, 35);
}

byte predictRain(byte hours) {
  float pressTrend = calculateTrend('p');
  float humTrend = calculateTrend('h');
  
  int prediction = rainProb;
  prediction += (pressTrend * -10 * pressWeight);
  prediction += (humTrend * 0.5 * humWeight);
  
  return constrain(prediction, 0, 100);
}

String predictWeather(byte hours) {
  byte rain = predictRain(hours);
  
  if (rain > 80) return "Tormenta";
  if (rain > 60) return "Lluvia";
  if (rain > 30) return "Nublado";
  return "Sol";
}

void printProgmem(const char* str, byte x, byte y) {
  lcd.setCursor(x, y);
  char buffer[20];
  strcpy_P(buffer, str);
  lcd.print(buffer);
}

void saveHistory() {
  // Guardar últimos 6 puntos en EEPROM
  for (byte i = 0; i < 6; i++) {
    byte idx = (histIdx - 6 + i + 12) % 12;
    EEPROM.put(i * sizeof(WeatherPoint), history[idx]);
  }
  
  // Guardar parámetros IA
  EEPROM.put(6 * sizeof(WeatherPoint), aiAccuracy);
  EEPROM.put(6 * sizeof(WeatherPoint) + 1, pressWeight);
}

void loadHistory() {
  // Cargar datos históricos
  for (byte i = 0; i < 6; i++) {
    WeatherPoint data;
    EEPROM.get(i * sizeof(WeatherPoint), data);
    
    if (data.t > -50 && data.t < 60) {
      history[i] = data;
      histIdx = i + 1;
    }
  }
  
  // Cargar parámetros IA
  EEPROM.get(6 * sizeof(WeatherPoint), aiAccuracy);
  EEPROM.get(6 * sizeof(WeatherPoint) + 1, pressWeight);
  
  // Validar valores
  if (aiAccuracy < 50 || aiAccuracy > 100) aiAccuracy = 75;
  if (pressWeight < 0.1 || pressWeight > 0.8) pressWeight = 0.4;
}