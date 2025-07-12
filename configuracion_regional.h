// CONFIGURACIÓN REGIONAL - ESTACIÓN METEOROLÓGICA
// Personalizar según tu ubicación específica

#ifndef CONFIGURACION_REGIONAL_H
#define CONFIGURACION_REGIONAL_H

// ========================================
// DATOS GEOGRÁFICOS DE LA REGIÓN
// ========================================

// Altitud sobre el nivel del mar (metros)
#define ALTITUD_REGIONAL 2219.0

// Coordenadas aproximadas (para cálculos)
#define LATITUD_REGIONAL 19.0  // Aproximada
#define LONGITUD_REGIONAL -99.0 // Aproximada

// Presión atmosférica estándar al nivel del mar (hPa)
#define PRESION_NIVEL_MAR 1013.25

// ========================================
// RANGOS CLIMÁTICOS REGIONALES
// ========================================

// Temperaturas (en grados Celsius)
#define TEMP_MIN_REGIONAL 6.0    // Enero (mes más frío)
#define TEMP_MAX_REGIONAL 29.0   // Abril-Mayo (meses más cálidos)
#define TEMP_MEDIA_ANUAL 20.0    // Temperatura media anual

// Humedad relativa (en porcentaje)
#define HUMEDAD_MIN_REGIONAL 75.0  // Mínima promedio
#define HUMEDAD_MAX_REGIONAL 91.0  // Máxima en temporada de lluvias
#define HUMEDAD_MEDIA_REGIONAL 83.0 // Promedio anual

// Presión atmosférica (hPa) - ajustada para altitud
#define PRESION_MIN_REGIONAL 980.0   // Durante tormentas
#define PRESION_MAX_REGIONAL 1030.0  // Durante anticiclones
#define PRESION_MEDIA_REGIONAL 1005.0 // Promedio para la altitud

// ========================================
// PATRONES ESTACIONALES
// ========================================

// Meses de temporada de lluvias (0-11, donde 0=enero)
#define MES_INICIO_LLUVIAS 5  // Junio
#define MES_FIN_LLUVIAS 8     // Septiembre

// Meses más cálidos
#define MES_INICIO_CALOR 3    // Abril
#define MES_FIN_CALOR 5       // Mayo

// Meses más fríos
#define MES_INICIO_FRIO 11    // Diciembre
#define MES_FIN_FRIO 1        // Enero

// ========================================
// CONFIGURACIÓN DE SENSORES
// ========================================

// Dirección I2C del esclavo
#define DIRECCION_ESCLAVO 0x08

// Pines LCD (maestro)
#define PIN_LCD_RS 13
#define PIN_LCD_EN 12
#define PIN_LCD_D4 11
#define PIN_LCD_D5 10
#define PIN_LCD_D6 9
#define PIN_LCD_D7 8

// Pin LDR (esclavo)
#define PIN_LDR A0

// ========================================
// PARÁMETROS DE IA Y PREDICCIÓN
// ========================================

// Tamaño del historial para IA (horas)
#define TAMANO_HISTORIAL 24

// Duración de cada pantalla (milisegundos)
#define DURACION_PANTALLA 4000

// Intervalo de actualización de sensores (milisegundos)
#define INTERVALO_ACTUALIZACION 1000

// Factores de ponderación para probabilidad de lluvia
#define PESO_PRESION 0.4
#define PESO_HUMEDAD 0.3
#define PESO_TENDENCIA 0.2
#define PESO_ESTACIONAL 0.1

// Umbrales para clasificación de clima
#define UMBRAL_PRESION_BAJA 1000.0
#define UMBRAL_PRESION_MEDIA 1010.0
#define UMBRAL_PRESION_ALTA 1020.0

#define UMBRAL_HUMEDAD_ALTA 85.0
#define UMBRAL_HUMEDAD_MEDIA 80.0
#define UMBRAL_HUMEDAD_BAJA 70.0

// Umbrales LDR para detección de luz
#define UMBRAL_LDR_NOCHE 100
#define UMBRAL_LDR_AMANECER 300
#define UMBRAL_LDR_DIA 600
#define UMBRAL_LDR_TARDE 800

// ========================================
// CORRECCIONES ALTITUDINALES
// ========================================

// Factor de corrección de temperatura por altitud (°C/1000m)
#define FACTOR_CORRECCION_TEMP -6.5

// Factor de corrección de presión por altitud
#define FACTOR_CORRECCION_PRESION 0.12

// Factor de corrección de punto de rocío por altitud
#define FACTOR_CORRECCION_ROCIO -2.0

// ========================================
// CONFIGURACIÓN DE ALERTAS
// ========================================

// Umbrales de alerta
#define ALERTA_TEMP_BAJA 5.0
#define ALERTA_TEMP_ALTA 30.0
#define ALERTA_HUMEDAD_BAJA 60.0
#define ALERTA_HUMEDAD_ALTA 95.0
#define ALERTA_PRESION_BAJA 990.0
#define ALERTA_PRESION_ALTA 1040.0

// Umbral de proximidad al punto de rocío (°C)
#define UMBRAL_CONDENSACION 2.0

// ========================================
// CONFIGURACIÓN DE CALIBRACIÓN
// ========================================

// Valores de calibración para LDR
#define LDR_VALOR_OSCURO 50
#define LDR_VALOR_LUZ_DIRECTA 900

// Offset de calibración de sensores
#define OFFSET_TEMPERATURA 0.0
#define OFFSET_HUMEDAD 0.0
#define OFFSET_PRESION 0.0

// ========================================
// CONFIGURACIÓN DE INTERFAZ
// ========================================

// Textos personalizables
#define TEXTO_ESTACION "Estacion Meteo"
#define TEXTO_IA "IA Avanzada"
#define TEXTO_TEMPERATURA "Temp:"
#define TEXTO_SENSACION "Sens:"
#define TEXTO_HUMEDAD "HR:"
#define TEXTO_PRESION "Pres:"
#define TEXTO_ALTITUD "Alt:"
#define TEXTO_LLUVIA "Prob Lluvia"
#define TEXTO_PREDICCION "Prediccion IA"

// Abreviaturas
#define ABREV_MINIMA "L:"
#define ABREV_MAXIMA "H:"
#define ABREV_ACTUAL "Act:"
#define ABREV_ROCIO "Rocio:"

// Clasificaciones
#define CLAS_BAJA "BAJA"
#define CLAS_MEDIA "MEDIA"
#define CLAS_ALTA "ALTA"
#define CLAS_MUY_ALTA "MUY ALTA"
#define CLAS_NORMAL "NORM"
#define CLAS_CALOR "CAL"
#define CLAS_FRIO "FRIO"
#define CLAS_OK "OK"

// Estados del clima
#define CLIMA_SOLEADO "Soleado"
#define CLIMA_NUBLADO "Nublado"
#define CLIMA_LLUVIA "Lluvia"
#define CLIMA_TORMENTA "Tormenta"
#define CLIMA_DESPJADO "Despejado"
#define CLIMA_PARCIAL "Parcial"
#define CLIMA_VARIABLE "Variable"

// Estados UV
#define UV_NOCHE "Noche"
#define UV_AMANECER "Amanecer"
#define UV_DIA "Dia"
#define UV_TARDE "Tarde"
#define UV_ATARDECER "Atardecer"

// Recomendaciones UV
#define UV_PROTECCION "Prot"
#define UV_MODERADA "Mod"
#define UV_OK "OK"

// ========================================
// FUNCIONES DE CONFIGURACIÓN
// ========================================

// Función para obtener el mes actual (0-11)
inline int obtenerMesActual() {
  // Simplificado - en implementación real usar RTC
  return (millis() / (24 * 60 * 60 * 1000)) % 12;
}

// Función para verificar si estamos en temporada de lluvias
inline bool esTemporadaLluvias() {
  int mes = obtenerMesActual();
  return (mes >= MES_INICIO_LLUVIAS && mes <= MES_FIN_LLUVIAS);
}

// Función para verificar si estamos en temporada cálida
inline bool esTemporadaCalor() {
  int mes = obtenerMesActual();
  return (mes >= MES_INICIO_CALOR && mes <= MES_FIN_CALOR);
}

// Función para verificar si estamos en temporada fría
inline bool esTemporadaFrio() {
  int mes = obtenerMesActual();
  return (mes == MES_INICIO_FRIO || mes == MES_FIN_FRIO);
}

// Función para obtener factor estacional de lluvia
inline float obtenerFactorEstacionalLluvia() {
  if (esTemporadaLluvias()) {
    return 10.0; // Aumentar probabilidad en temporada de lluvias
  }
  return 0.0;
}

// Función para corregir temperatura por altitud
inline float corregirTemperaturaAltitud(float temp) {
  return temp + (ALTITUD_REGIONAL / 1000.0) * FACTOR_CORRECCION_TEMP;
}

// Función para corregir presión por altitud
inline float corregirPresionAltitud(float presion) {
  return presion + (ALTITUD_REGIONAL / 1000.0) * FACTOR_CORRECCION_PRESION;
}

// Función para corregir punto de rocío por altitud
inline float corregirRocioAltitud(float rocio) {
  return rocio + (ALTITUD_REGIONAL / 1000.0) * FACTOR_CORRECCION_ROCIO;
}

#endif // CONFIGURACION_REGIONAL_H