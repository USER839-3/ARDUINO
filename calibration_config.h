/*
 * ARCHIVO DE CALIBRACIÓN REGIONAL Y CONFIGURACIÓN AVANZADA
 * Personaliza estos valores según tu ubicación geográfica
 */

#ifndef CALIBRATION_CONFIG_H
#define CALIBRATION_CONFIG_H

// ==================== CONFIGURACIÓN REGIONAL ====================

// Datos de tu región específica (Ya configurados para tu ubicación)
#define REGION_NAME "Mi Región"
#define REGION_ALTITUDE 2219.0          // Altitud en metros sobre el nivel del mar
#define REGION_LATITUDE 0.0             // Latitud (opcional, para cálculos UV)
#define REGION_LONGITUDE 0.0            // Longitud (opcional)

// Temperaturas características de tu región
#define TEMP_MIN_ANNUAL 6.0             // Mínima anual típica (°C)
#define TEMP_MAX_ANNUAL 27.0            // Máxima anual típica (°C)
#define TEMP_AVG_COLD_MONTH 12.0        // Promedio mes más frío (°C)
#define TEMP_AVG_HOT_MONTH 23.5         // Promedio mes más caluroso (°C)

// Humedad típica de tu región
#define HUMIDITY_MIN_TYPICAL 75.0       // Mínima típica (%)
#define HUMIDITY_MAX_TYPICAL 91.0       // Máxima típica (%)
#define HUMIDITY_AVG_DRY_SEASON 75.0    // Promedio temporada seca (%)
#define HUMIDITY_AVG_WET_SEASON 85.0    // Promedio temporada lluviosa (%)

// Presión atmosférica base (calculada automáticamente)
#define SEA_LEVEL_PRESSURE 1013.25
#define REGION_PRESSURE_BASE (SEA_LEVEL_PRESSURE * pow(1 - (0.0065 * REGION_ALTITUDE) / 288.15, 5.255))

// Patrones de lluvia regionales
#define RAINY_SEASON_START 6            // Mes de inicio (junio)
#define RAINY_SEASON_END 9              // Mes de fin (septiembre)
#define PEAK_RAIN_MONTH 9               // Mes con más lluvia (septiembre)
#define DRY_SEASON_MONTH 1              // Mes más seco (enero)

// Velocidad del viento típica
#define WIND_SPEED_MIN 5.0              // km/h
#define WIND_SPEED_MAX 35.0             // km/h
#define WIND_SPEED_AVG 15.0             // km/h

// ==================== CONFIGURACIÓN DEL SISTEMA ====================

// Intervalos de actualización (en milisegundos)
#define SENSOR_UPDATE_INTERVAL 1000     // 1 segundo
#define SCREEN_ROTATION_INTERVAL 3000   // 3 segundos
#define AI_UPDATE_INTERVAL 5000         // 5 segundos para procesamiento IA
#define HISTORY_UPDATE_INTERVAL 300000  // 5 minutos para histórico

// Configuración de precisión
#define TEMP_PRECISION 1                // Decimales para temperatura
#define HUMIDITY_PRECISION 1            // Decimales para humedad
#define PRESSURE_PRECISION 1            // Decimales para presión
#define ALTITUDE_PRECISION 0            // Decimales para altitud

// Configuración de pantalla
#define DISPLAY_CONTRAST 128            // Contraste LCD (0-255)
#define SCREEN_COUNT 8                  // Número total de pantallas
#define SHOW_ICONS true                 // Mostrar iconos personalizados
#define USE_ABBREVIATIONS true          // Usar abreviaturas para ahorrar espacio

// ==================== CONFIGURACIÓN DE SENSORES ====================

// Direcciones I2C
#define BMP280_ADDRESS 0x76             // Dirección I2C del BMP280
#define AHT20_ADDRESS 0x38              // Dirección I2C del AHT20

// Configuración del LDR
#define LDR_PIN A0                      // Pin analógico para LDR
#define LDR_RESISTANCE 10000            // Resistencia de pull-down (ohms)
#define LDR_DARK_THRESHOLD 100          // Umbral para considerar "oscuro"
#define LDR_BRIGHT_THRESHOLD 900        // Umbral para considerar "brillante"

// Calibración del LDR para UV
#define UV_CALIBRATION_FACTOR 11.0      // Factor de calibración UV
#define UV_NIGHT_OFFSET 0               // Offset para horario nocturno

// ==================== CONFIGURACIÓN DE IA ====================

// Parámetros del sistema de aprendizaje
#define AI_INITIAL_ACCURACY 50          // Precisión inicial (%)
#define AI_MAX_ACCURACY 95              // Precisión máxima (%)
#define AI_MIN_ACCURACY 30              // Precisión mínima (%)
#define AI_LEARNING_RATE 0.1            // Tasa de aprendizaje
#define AI_HISTORY_SIZE 12              // Tamaño del histórico (horas)

// Umbrales para predicción del clima
#define STORM_PRESSURE_THRESHOLD 995.0  // hPa
#define RAIN_PRESSURE_THRESHOLD 1005.0  // hPa
#define CLEAR_PRESSURE_THRESHOLD 1015.0 // hPa

// Factores para cálculo de probabilidad de lluvia
#define PRESSURE_FACTOR 0.4             // Peso de la presión
#define HUMIDITY_FACTOR 0.3             // Peso de la humedad
#define TREND_FACTOR 0.2                // Peso de la tendencia
#define ALTITUDE_FACTOR 0.1             // Peso de la altitud

// ==================== CONFIGURACIÓN DE TEXTOS ====================

// Textos para condiciones climáticas
#define TEXT_SUNNY "Soleado"
#define TEXT_CLOUDY "Nublado"
#define TEXT_RAINY "Lluvioso"
#define TEXT_STORM "Tormenta"

// Textos para predicciones
#define TEXT_IMPROVING "Mejorando"
#define TEXT_WORSENING "Empeorando"
#define TEXT_CLOUDING "Nublando"
#define TEXT_STABLE "Estable"

// Textos para niveles UV
#define TEXT_UV_LOW "Bajo"
#define TEXT_UV_MODERATE "Moderado"
#define TEXT_UV_HIGH "Alto"
#define TEXT_UV_VERY_HIGH "Muy Alto"

// Abreviaturas para ahorrar espacio
#define ABBR_TEMPERATURE "T"
#define ABBR_HUMIDITY "HR"
#define ABBR_PRESSURE "P"
#define ABBR_ALTITUDE "Alt"
#define ABBR_UV_INDEX "UV"
#define ABBR_RAIN_PROB "Lluvia"
#define ABBR_FORECAST "Pred"
#define ABBR_MIN_TEMP "L"
#define ABBR_MAX_TEMP "H"
#define ABBR_DEW_POINT "PR"
#define ABBR_HEAT_INDEX "ST"
#define ABBR_ACCURACY "IA"

// ==================== CONFIGURACIÓN DE DEPURACIÓN ====================

// Habilitar/deshabilitar depuración
#define DEBUG_MODE true                 // Activar mensajes de depuración
#define DEBUG_SENSORS true              // Depurar lecturas de sensores
#define DEBUG_AI true                   // Depurar sistema de IA
#define DEBUG_PREDICTIONS true          // Depurar predicciones

// Baudrate para comunicación serie
#define SERIAL_BAUDRATE 9600

// ==================== CONFIGURACIÓN AVANZADA ====================

// Configuración de memoria
#define OPTIMIZE_MEMORY true            // Optimizar uso de memoria
#define USE_PROGMEM true                // Usar PROGMEM para datos constantes
#define REDUCE_PRECISION false          // Reducir precisión para ahorrar memoria

// Configuración de alimentación
#define POWER_SAVE_MODE false           // Modo de ahorro de energía
#define SLEEP_BETWEEN_READINGS false    // Dormir entre lecturas

// Configuración de comunicación
#define ENABLE_SERIAL_OUTPUT true       // Habilitar salida por serie
#define SERIAL_OUTPUT_INTERVAL 10000    // Intervalo de salida (ms)

// ==================== FUNCIONES DE UTILIDAD ====================

// Función para obtener la presión base regional
inline float getRegionalPressureBase() {
    return REGION_PRESSURE_BASE;
}

// Función para determinar si estamos en temporada de lluvias
inline bool isRainySeason(byte month) {
    return (month >= RAINY_SEASON_START && month <= RAINY_SEASON_END);
}

// Función para obtener la humedad típica según la estación
inline float getTypicalHumidity(byte month) {
    return isRainySeason(month) ? HUMIDITY_AVG_WET_SEASON : HUMIDITY_AVG_DRY_SEASON;
}

// Función para calibrar la lectura del LDR
inline int calibrateLDR(int rawValue) {
    return map(rawValue, 0, 1023, 0, UV_CALIBRATION_FACTOR);
}

// ==================== CONFIGURACIÓN DE VALIDACIÓN ====================

// Validar que los valores estén en rangos lógicos
#if REGION_ALTITUDE < 0 || REGION_ALTITUDE > 10000
#error "Altitud fuera del rango válido (0-10000 metros)"
#endif

#if TEMP_MIN_ANNUAL > TEMP_MAX_ANNUAL
#error "Temperatura mínima no puede ser mayor que la máxima"
#endif

#if HUMIDITY_MIN_TYPICAL > HUMIDITY_MAX_TYPICAL
#error "Humedad mínima no puede ser mayor que la máxima"
#endif

#if RAINY_SEASON_START > 12 || RAINY_SEASON_END > 12
#error "Meses de temporada lluviosa fuera del rango válido (1-12)"
#endif

#endif // CALIBRATION_CONFIG_H