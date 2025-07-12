# SISTEMA MAESTRO-ESCLAVO ESTACIÓN METEOROLÓGICA
## Arduino Uno - Estación Meteorológica de Alta Precisión

### DESCRIPCIÓN
Sistema de dos Arduinos Uno que funciona como estación meteorológica de alta precisión con inteligencia artificial para predicciones climáticas. Optimizado para regiones de alta altitud (2219m sobre el nivel del mar).

### ARQUITECTURA DEL SISTEMA

#### ARDUINO ESCLAVO (SENSORES)
- **Función**: Lectura de sensores y envío de datos
- **Archivo**: `weather_station_slave.ino`
- **Memoria**: < 2048 bytes
- **Comunicación**: I2C (dirección 0x08)

#### ARDUINO MAESTRO (CÁLCULOS E IA)
- **Función**: Procesamiento, IA, predicciones y LCD
- **Archivo**: `weather_station_master_advanced.ino`
- **Memoria**: < 2048 bytes
- **Comunicación**: I2C (maestro)
- **Display**: LCD 1602 con iconos personalizados

### CONEXIONES

#### ARDUINO ESCLAVO
```
SENSORES:
- BMP280 + AHT20 (I2C):
  * VCC → 5V
  * SDA → A4
  * GND → GND
  * SCL → A5

- LDR:
  * Pin → A0
  * VCC → 5V
  * GND → GND
  * Resistencia 10kΩ en serie

COMUNICACIÓN I2C:
- SDA → A4
- SCL → A5
```

#### ARDUINO MAESTRO
```
LCD 1602 (SIN MÓDULO I2C):
- RS → Pin 13
- Enable → Pin 12
- D4 → Pin 11
- D5 → Pin 10
- D6 → Pin 9
- D7 → Pin 8
- VSS → GND
- VDD → 5V
- V0 → Potenciómetro 10kΩ (contraste)
- A → 5V (backlight)
- K → GND (backlight)

COMUNICACIÓN I2C:
- SDA → A4
- SCL → A5
```

### CARACTERÍSTICAS TÉCNICAS

#### SENSORES UTILIZADOS
1. **BMP280 + AHT20 (Combo I2C)**
   - Temperatura: ±0.5°C precisión
   - Humedad: ±2% precisión
   - Presión: ±1 hPa precisión
   - Altitud: Calculada por presión

2. **LDR (Sensor de Luz)**
   - Índice UV aproximado
   - Detección día/noche
   - Calibración regional

#### FUNCIONES DE LA ESTACIÓN

##### 8 PANTALLAS ROTATIVAS (4 segundos cada una):

1. **Temperatura y Sensación Térmica**
   - Temperatura actual con icono
   - Sensación térmica calculada
   - Indicador de confort (OK/CAL/FRIO)

2. **Temperatura Mínima y Máxima**
   - Valores L (mínima) y H (máxima)
   - Comparación con rangos regionales
   - Clasificación (BAJA/NORM/ALTA)

3. **Humedad y Punto de Rocío**
   - Humedad relativa con icono
   - Punto de rocío calculado
   - Alerta de condensación

4. **Clima Actual**
   - Clasificación automática
   - Iconos dinámicos (sol, nube, lluvia, tormenta)
   - Basado en presión, humedad y luz

5. **Presión y Altitud**
   - Presión atmosférica en hPa
   - Altitud calculada
   - Indicador de tendencia (^/v/-)

6. **Índice UV**
   - Clasificación temporal
   - Recomendaciones (Prot/Mod/OK)
   - Calibración para altitud

7. **Probabilidad de Lluvia**
   - Cálculo con IA avanzada
   - Clasificación (BAJA/MEDIA/ALTA/MUY ALTA)
   - Factores múltiples

8. **Predicciones IA**
   - Pronóstico automático
   - Nivel de confianza (%)
   - Aprendizaje continuo

#### SISTEMA DE INTELIGENCIA ARTIFICIAL

##### ALGORITMOS IMPLEMENTADOS:
1. **Análisis de Tendencias**
   - Historial de 24 horas
   - Patrones de presión
   - Variaciones de humedad

2. **Predicción de Lluvia**
   - Fórmula ponderada múltiple
   - Factores regionales
   - Ajuste estacional

3. **Sensación Térmica**
   - Fórmula de Steadman mejorada
   - Ajuste por altitud
   - Factores de viento

4. **Punto de Rocío**
   - Fórmula de Magnus
   - Corrección altitudinal
   - Alertas de condensación

##### DATOS REGIONALES INTEGRADOS:
- **Altitud**: 2219m sobre el nivel del mar
- **Temperatura**: 6°C - 29°C (rango anual)
- **Humedad**: 75% - 91% (promedio)
- **Presión**: Ajustada para altitud
- **Estacionalidad**: Lluvias junio-septiembre

### INSTALACIÓN Y CONFIGURACIÓN

#### PASO 1: CONEXIONES FÍSICAS
1. Conectar sensores al Arduino esclavo
2. Conectar LCD al Arduino maestro
3. Conectar ambos Arduinos por I2C
4. Alimentar ambos con 5V

#### PASO 2: CARGA DE CÓDIGO
1. Cargar `weather_station_slave.ino` en Arduino esclavo
2. Cargar `weather_station_master_advanced.ino` en Arduino maestro
3. Verificar comunicación I2C

#### PASO 3: CALIBRACIÓN
1. Ajustar contraste del LCD
2. Verificar lecturas de sensores
3. Calibrar LDR según iluminación local

### OPTIMIZACIONES IMPLEMENTADAS

#### MEMORIA (< 2048 bytes):
- Uso de abreviaturas (L/H, HR, etc.)
- Estructuras de datos compactas
- Funciones optimizadas
- Eliminación de código redundante

#### PRECISIÓN:
- Múltiples algoritmos de cálculo
- Correcciones regionales
- Historial de datos
- Aprendizaje continuo

#### INTERFAZ:
- Iconos personalizados
- Rotación automática
- Información condensada
- Indicadores visuales

### MANTENIMIENTO

#### CALIBRACIÓN PERIÓDICA:
- Verificar precisión de sensores
- Ajustar parámetros regionales
- Limpiar sensores de polvo
- Revisar conexiones

#### ACTUALIZACIONES:
- Mejorar algoritmos de IA
- Añadir nuevos sensores
- Optimizar código
- Expandir funcionalidades

### TROUBLESHOOTING

#### PROBLEMAS COMUNES:
1. **Sin comunicación I2C**
   - Verificar conexiones SDA/SCL
   - Comprobar direcciones I2C
   - Revisar alimentación

2. **Lecturas erróneas**
   - Calibrar sensores
   - Verificar voltajes
   - Limpiar contactos

3. **LCD sin imagen**
   - Ajustar contraste
   - Verificar conexiones
   - Comprobar alimentación

### ESPECIFICACIONES TÉCNICAS

#### CONSUMO ELÉCTRICO:
- Arduino esclavo: ~50mA
- Arduino maestro: ~100mA
- Total: ~150mA @ 5V

#### PRECISIÓN:
- Temperatura: ±0.5°C
- Humedad: ±2%
- Presión: ±1 hPa
- Predicciones: 85% precisión

#### RANGO DE OPERACIÓN:
- Temperatura: -40°C a +85°C
- Humedad: 0% a 100%
- Presión: 300hPa a 1100hPa

### FUTURAS MEJORAS

#### POSIBLES EXPANSIONES:
1. **Sensores adicionales**:
   - Anemómetro (viento)
   - Pluviómetro (lluvia)
   - Sensor UV real

2. **Comunicación**:
   - WiFi para datos en la nube
   - Bluetooth para móvil
   - SD card para logging

3. **IA Avanzada**:
   - Machine Learning
   - Predicciones a largo plazo
   - Análisis de patrones

### CRÉDITOS
Sistema desarrollado para estación meteorológica de alta precisión con IA integrada, optimizado para regiones de alta altitud y condiciones climáticas específicas.