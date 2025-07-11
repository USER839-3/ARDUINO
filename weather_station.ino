#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <EEPROM.h>

// LCD Configuration (pins 13, 12, 11, 10, 9, 8)
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Sensors
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

// Pin definitions
const int LDR_PIN = A0;

// Custom characters (icons)
byte tempIcon[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x1F, 0x1F, 0x0E};
byte humidIcon[8] = {0x04, 0x0A, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
byte sunIcon[8] = {0x04, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x04, 0x00};
byte rainIcon[8] = {0x00, 0x0E, 0x1F, 0x00, 0x12, 0x12, 0x12, 0x00};
byte cloudIcon[8] = {0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00};
byte stormIcon[8] = {0x0E, 0x1F, 0x1F, 0x04, 0x08, 0x10, 0x04, 0x00};
byte pressIcon[8] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00};
byte uvIcon[8] = {0x15, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x15, 0x00};

// Global variables
float temperature = 0, humidity = 0, pressure = 0, altitude = 0;
float tempMin = 50, tempMax = -50, dewPoint = 0, heatIndex = 0;
int uvIndex = 0, rainProb = 0, ldrValue = 0;
byte weatherCondition = 0; // 0=sunny, 1=cloudy, 2=rain, 3=storm
byte currentScreen = 0;
unsigned long lastUpdate = 0, screenTimer = 0;
const unsigned long UPDATE_INTERVAL = 1000;
const unsigned long SCREEN_INTERVAL = 3000;

// Weather prediction variables
float pressHistory[12]; // 12 hour history
byte historyIndex = 0;
float pressureTrend = 0;

// AI variables for prediction improvement
int predictionAccuracy = 50; // Start at 50% accuracy
float learningRate = 0.1;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  // Initialize sensors
  if (!bmp.begin(0x76)) {
    lcd.print("BMP280 Error!");
    while(1);
  }
  
  if (!aht.begin()) {
    lcd.print("AHT20 Error!");
    while(1);
  }
  
  // Create custom characters
  lcd.createChar(0, tempIcon);
  lcd.createChar(1, humidIcon);
  lcd.createChar(2, sunIcon);
  lcd.createChar(3, rainIcon);
  lcd.createChar(4, cloudIcon);
  lcd.createChar(5, stormIcon);
  lcd.createChar(6, pressIcon);
  lcd.createChar(7, uvIcon);
  
  // Initialize pressure history
  for(int i = 0; i < 12; i++) {
    pressHistory[i] = 1013.25;
  }
  
  // Welcome screen
  lcd.clear();
  lcd.print("Weather Station");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update sensor readings
  if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
    readSensors();
    calculateWeatherData();
    updatePredictions();
    lastUpdate = currentTime;
  }
  
  // Rotate screens
  if (currentTime - screenTimer >= SCREEN_INTERVAL) {
    displayScreen(currentScreen);
    currentScreen = (currentScreen + 1) % 8;
    screenTimer = currentTime;
  }
}

void readSensors() {
  // Read BMP280
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0; // Convert to hPa
  altitude = bmp.readAltitude(1013.25); // Sea level pressure
  
  // Read AHT20
  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);
  humidity = hum.relative_humidity;
  
  // Average temperature from both sensors
  temperature = (temperature + temp.temperature) / 2.0;
  
  // Read LDR for UV approximation
  ldrValue = analogRead(LDR_PIN);
  
  // Update min/max temperatures
  if (temperature < tempMin) tempMin = temperature;
  if (temperature > tempMax) tempMax = temperature;
}

void calculateWeatherData() {
  // Calculate dew point
  dewPoint = temperature - ((100 - humidity) / 5.0);
  
  // Calculate heat index (feels like temperature)
  if (temperature >= 26.7) {
    heatIndex = -42.379 + 2.04901523 * temperature + 10.14333127 * humidity;
    heatIndex -= 0.22475541 * temperature * humidity;
    heatIndex -= 6.83783e-3 * temperature * temperature;
    heatIndex -= 5.481717e-2 * humidity * humidity;
    heatIndex += 1.22874e-3 * temperature * temperature * humidity;
    heatIndex += 8.5282e-4 * temperature * humidity * humidity;
    heatIndex -= 1.99e-6 * temperature * temperature * humidity * humidity;
  } else {
    heatIndex = temperature;
  }
  
  // Calculate UV index based on LDR and time
  uvIndex = map(ldrValue, 0, 1023, 0, 11);
  
  // Determine weather condition based on pressure and humidity
  if (pressure < 995) {
    weatherCondition = 3; // Storm
  } else if (pressure < 1005) {
    if (humidity > 80) weatherCondition = 2; // Rain
    else weatherCondition = 1; // Cloudy
  } else if (pressure > 1015) {
    weatherCondition = 0; // Sunny
  } else {
    weatherCondition = 1; // Cloudy
  }
  
  // Calculate rain probability
  rainProb = calculateRainProbability();
}

int calculateRainProbability() {
  int prob = 0;
  
  // Base probability on pressure
  if (pressure < 995) prob += 80;
  else if (pressure < 1005) prob += 40;
  else if (pressure > 1020) prob -= 20;
  
  // Add humidity factor
  if (humidity > 85) prob += 30;
  else if (humidity > 75) prob += 15;
  
  // Add pressure trend
  if (pressureTrend < -2) prob += 35;
  else if (pressureTrend < -1) prob += 20;
  else if (pressureTrend > 1) prob -= 15;
  
  return constrain(prob, 0, 100);
}

void updatePredictions() {
  // Update pressure history every hour (simplified to every 12 readings)
  static int readingCount = 0;
  readingCount++;
  
  if (readingCount >= 12) {
    pressHistory[historyIndex] = pressure;
    historyIndex = (historyIndex + 1) % 12;
    
    // Calculate pressure trend
    float sum = 0;
    for (int i = 0; i < 12; i++) {
      sum += pressHistory[i];
    }
    float avgPressure = sum / 12.0;
    pressureTrend = pressure - avgPressure;
    
    readingCount = 0;
    
    // AI learning - improve prediction accuracy
    improvePredictionAccuracy();
  }
}

void improvePredictionAccuracy() {
  // Simple AI: adjust predictions based on historical accuracy
  static float lastPrediction = 0;
  static unsigned long lastPredictionTime = 0;
  
  if (lastPredictionTime != 0 && millis() - lastPredictionTime > 3600000) { // 1 hour
    // Check if our prediction was accurate
    float actualChange = pressure - lastPrediction;
    float error = abs(actualChange - pressureTrend);
    
    if (error < 1.0) {
      predictionAccuracy = min(95, predictionAccuracy + 1);
    } else {
      predictionAccuracy = max(30, predictionAccuracy - 1);
    }
  }
  
  lastPrediction = pressure;
  lastPredictionTime = millis();
}

void displayScreen(byte screen) {
  lcd.clear();
  
  switch(screen) {
    case 0: // Temperature & Heat Index
      lcd.write(0); // Temperature icon
      lcd.print("T:");
      lcd.print(temperature, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("FL:");
      lcd.print(heatIndex, 1);
      lcd.print("C");
      break;
      
    case 1: // Min/Max Temperature
      lcd.write(0);
      lcd.print("L:");
      lcd.print(tempMin, 1);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("H:");
      lcd.print(tempMax, 1);
      lcd.print("C");
      break;
      
    case 2: // Humidity & Dew Point
      lcd.write(1); // Humidity icon
      lcd.print("HR:");
      lcd.print(humidity, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("DP:");
      lcd.print(dewPoint, 1);
      lcd.print("C");
      break;
      
    case 3: // Current Weather
      lcd.write(weatherCondition + 2); // Weather icon
      lcd.print("Weather:");
      lcd.setCursor(0, 1);
      switch(weatherCondition) {
        case 0: lcd.print("Sunny"); break;
        case 1: lcd.print("Cloudy"); break;
        case 2: lcd.print("Rainy"); break;
        case 3: lcd.print("Storm"); break;
      }
      break;
      
    case 4: // Pressure & Altitude
      lcd.write(6); // Pressure icon
      lcd.print("P:");
      lcd.print(pressure, 1);
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print("Alt:");
      lcd.print(altitude, 0);
      lcd.print("m");
      break;
      
    case 5: // UV Index
      lcd.write(7); // UV icon
      lcd.print("UV Index:");
      lcd.print(uvIndex);
      lcd.setCursor(0, 1);
      if (uvIndex <= 2) lcd.print("Low Risk");
      else if (uvIndex <= 5) lcd.print("Moderate");
      else if (uvIndex <= 7) lcd.print("High Risk");
      else lcd.print("Very High");
      break;
      
    case 6: // Rain Probability
      lcd.write(3); // Rain icon
      lcd.print("Rain Prob:");
      lcd.setCursor(0, 1);
      lcd.print(rainProb);
      lcd.print("% Acc:");
      lcd.print(predictionAccuracy);
      lcd.print("%");
      break;
      
    case 7: // Weather Prediction
      lcd.print("Forecast:");
      lcd.setCursor(0, 1);
      if (pressureTrend < -2) lcd.print("Worsening");
      else if (pressureTrend < -1) lcd.print("Clouding");
      else if (pressureTrend > 1) lcd.print("Improving");
      else lcd.print("Stable");
      break;
  }
}