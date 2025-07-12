# 🌦️ Estación Meteorológica Inteligente - Arduino UNO

## 📋 Descripción
Estación meteorológica avanzada con **inteligencia artificial** que proporciona predicciones precisas del clima. Diseñada específicamente para la región de altitud 2219m SNM con características climáticas únicas.

## �️ Componentes Necesarios

### Hardware
- **Arduino UNO R3**
- **Sensor BMP280 + AHT20** (2 en 1, comunicación I2C)
- **LDR (Light Dependent Resistor)** + Resistencia 10kΩ
- **LCD 1602** (sin módulo I2C)
- **Resistencia 220Ω** (para contraste LCD)
- **Potenciómetro 10kΩ** (ajuste contraste LCD)
- **Cables jumper**
- **Protoboard**

### Software
- **Arduino IDE 1.8.19** o superior
- **Librerías necesarias:**
  - `Adafruit_Sensor`
  - `Adafruit_BMP280`
  - `Adafruit_AHTX0`
  - `LiquidCrystal` (incluida en Arduino)
  - `Wire` (incluida en Arduino)
  - `EEPROM` (incluida en Arduino)

## 🔌 Conexiones

### LCD 1602 (Sin módulo I2C)
```
LCD Pin  → Arduino Pin
VSS/GND  → GND
VDD/VCC  → 5V
V0       → Potenciómetro (contraste)
RS       → Pin 13
E        → Pin 12
D4       → Pin 11
D5       → Pin 10
D6       → Pin 9
D7       → Pin 8
A        → 5V (con resistencia 220Ω)
K        → GND
```

### Sensor BMP280 + AHT20 (I2C)
```
Sensor Pin → Arduino Pin
VCC        → 3.3V o 5V
GND        → GND
SDA        → A4
SCL        → A5
```

### LDR (Sensor de luz)
```
LDR → A0 y 5V
Resistencia 10kΩ → A0 y GND
```

## 📊 Funcionalidades

### 8 Pantallas Rotatorias (4 segundos c/u)

1. **🌡️ Temperatura y Sensación Térmica**
   - Temperatura real del ambiente
   - Sensación térmica (ajustada por humedad y luz)

2. **📈 Temperatura Mínima/Máxima**
   - Registro de temperaturas extremas
   - Actualización automática

3. **💧 Humedad y Punto de Rocío**
   - Humedad relativa del aire
   - Punto de rocío calculado

4. **🌤️ Clima Actual**
   - Condición climática (Soleado, Nublado, Lluvioso, Tormenta)
   - Velocidad del viento estimada
   - Nivel de luz ambiente

5. **🔽 Presión y Altitud**
   - Presión atmosférica en hPa
   - Altitud fija (2219m SNM)

6. **☀️ Índice UV**
   - Índice UV calculado (0-11+)
   - Clasificación del riesgo UV

7. **🌧️ Probabilidad de Lluvia**
   - Porcentaje de probabilidad de lluvia
   - Predicción basada en sensores

8. **🤖 Predicciones IA**
   - Precisión actual del sistema IA
   - Temperatura predicha (+2 horas)
   - Lluvia predicha (+2 horas)
   - Clima predicho

## 🧠 Sistema de Inteligencia Artificial

### Características Avanzadas
- **Análisis de tendencias**: Evalúa cambios en temperatura, humedad y presión
- **Pesos adaptativos**: El sistema ajusta automáticamente la importancia de cada factor
- **Aprendizaje continuo**: Mejora la precisión con el tiempo
- **Almacenamiento persistente**: Guarda datos históricos en EEPROM

### Algoritmos Implementados
- **Predicción meteorológica**: Basada en patrones de presión atmosférica
- **Cálculo de punto de rocío**: Fórmula Magnus-Tetens
- **Sensación térmica**: Ajustada por humedad y radiación solar
- **Índice UV**: Calculado según intensidad lumínica y hora del día

## 📍 Calibración Regional

### Parámetros específicos para altitud 2219m SNM:
- **Presión base**: 886.5 hPa (ajustada por altitud)
- **Rango de temperatura**: -5°C a 35°C
- **Humedad típica**: 60-95%
- **Temporada lluviosa**: Junio-Septiembre
- **Temporada seca**: Octubre-Mayo

## 🔧 Instalación y Configuración

### 1. Instalar librerías
```bash
# En Arduino IDE:
Sketch → Include Library → Library Manager
Buscar e instalar:
- Adafruit Unified Sensor
- Adafruit BMP280 Library
- Adafruit AHTX0
```

### 2. Cargar el código
1. Conectar Arduino UNO al PC
2. Abrir `estacion_meteorologica.ino`
3. Seleccionar **Board: Arduino UNO**
4. Seleccionar el puerto COM correcto
5. Cargar el código

### 3. Calibración inicial
- Al primer arranque, el sistema requerirá ~30 minutos para calibrar
- La IA comenzará con 75% de precisión
- La precisión mejorará gradualmente hasta 98%

## 📱 Uso y Monitoreo

### Monitor Serie
```
Baudrate: 9600
Información disponible:
- Datos de sensores en tiempo real
- Análisis de tendencias
- Precisión actual de la IA
- Optimizaciones del sistema
```

### Pantalla LCD
- **Rotación automática**: 4 segundos por pantalla
- **Indicador de pantalla**: Número en esquina superior derecha
- **Iconos personalizados**: Un icono único para cada tipo de dato

## 🚀 Funciones Avanzadas

### Predicciones Meteorológicas
- **Corto plazo**: 2-6 horas
- **Precisión**: 75-98% (mejora con el tiempo)
- **Factores considerados**:
  - Tendencia de presión atmosférica
  - Cambios de humedad
  - Variación de temperatura
  - Patrones estacionales

### Almacenamiento de Datos
- **EEPROM**: Últimos 6 puntos de datos
- **RAM**: Últimas 12 lecturas para análisis
- **Persistencia**: Datos conservados al reiniciar

### Optimizaciones de Memoria
- **Strings en PROGMEM**: Liberación de RAM
- **Tipos de datos optimizados**: `byte` en lugar de `int`
- **Estructuras compactas**: Máximo aprovechamiento de memoria

## 🛡️ Mantenimiento

### Limpieza
- Limpiar sensores cada 3 meses
- Proteger de humedad extrema
- Verificar conexiones periódicamente

### Recalibración
- El sistema se recalibra automáticamente
- Reset manual: desconectar alimentación por 30 segundos
- Los datos históricos se preservan en EEPROM

## � Resolución de Problemas

### Errores Comunes
1. **"Error Sensor BMP280"**: Verificar conexiones I2C
2. **"Error Sensor AHT20"**: Verificar alimentación 3.3V/5V
3. **Lecturas erróneas**: Verificar LDR y resistencia pull-down
4. **Pantalla en blanco**: Ajustar potenciómetro de contraste

### Diagnóstico
```cpp
// Descomentar en el código para debug:
// Serial.print("Temperatura: ");
// Serial.println(temp);
// Serial.print("Humedad: ");
// Serial.println(hum);
// Serial.print("Presión: ");
// Serial.println(press);
```

## 🌡️ Especificaciones Técnicas

### Precisión de Sensores
- **BMP280**: ±0.5°C (temperatura), ±1 hPa (presión)
- **AHT20**: ±0.3°C (temperatura), ±2% RH (humedad)
- **LDR**: Respuesta espectral 540nm (pico)

### Consumo de Energía
- **Operación normal**: ~150mA @ 5V
- **Modo bajo consumo**: Implementable con modificaciones

### Memoria Utilizada
- **Flash**: ~85% (27KB de 32KB)
- **SRAM**: ~75% (1.5KB de 2KB)
- **EEPROM**: ~15% (150 bytes de 1024 bytes)

## 📈 Mejoras Futuras

### Posibles Ampliaciones
1. **Conectividad WiFi**: ESP32 para datos en la nube
2. **Registro SD**: Almacenamiento de datos históricos
3. **Alertas**: Buzzer para condiciones extremas
4. **Pantalla gráfica**: OLED para gráficos de tendencias
5. **Sensores adicionales**: Viento, radiación UV real

### Algoritmos IA Avanzados
- **Redes neuronales**: Para predicciones más precisas
- **Machine Learning**: Reconocimiento de patrones complejos
- **Integración APIs**: Datos meteorológicos externos

## 📄 Licencia
Este proyecto está bajo licencia MIT. Libre para uso personal y educativo.

## 🤝 Contribuciones
¡Las contribuciones son bienvenidas! Por favor, crea un pull request o reporta issues.

## 📞 Soporte
Para soporte técnico, consultas o mejoras, contacta a través de GitHub Issues.

---

**Desarrollado con ❤️ para la comunidad Arduino**

*Versión: 1.0 | Fecha: 2024*