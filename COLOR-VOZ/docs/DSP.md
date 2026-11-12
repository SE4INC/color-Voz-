# Decisiones de procesamiento

## Motor

Sobremuestreo JUCE 2x IIR polifasico con latencia entera. Fijo en esta version para evitar cambios de latencia al automatizar. No hay selector Eco/Calidad en 1.0. La latencia devuelta por JUCE se informa al host. Memoria y filtros se preparan en prepareToPlay; processBlock se divide en bloques preasignados si el host envia uno mayor.

En el dominio sobremuestreado, dos filtros de primer orden a 250 y 3500 Hz producen componentes complementarios low, mid y high. Solo se procesa la diferencia de color; la senal original se retrasa y se suma despues. En NEUTRO la diferencia es cero y la salida coincide con el original retrasado. El sobremuestreo IIR tiene fase dependiente de frecuencia: compensar retardo no equivale a fase lineal.

BODY anade una saturacion tanh suave al componente bajo. PRESENCE refuerza medios. CRUNCH usa tanh con drive dependiente de su control en medios. AIR combina realce y armonicos altos. El color anadido pasa un bloqueador de DC a 12 Hz. SMOOTH detecta envolvente aguda (1 ms ataque / 70 ms relajacion) y reduce solo el color alto anadido. No elimina sibilancia que ya estaba en la grabacion y no es un de-esser completo.

Controles con suavizado de 25 ms; bypass con 10 ms. Mezcla seca/humeda lineal. Medicion RMS estereo enlazada con ventana exponencial de 500 ms, compensacion limitada ±6 dB y suavizada 400 ms. No es LUFS ni normalizacion instantanea. OUTPUT despues de mezcla y bypass. Sin limitador; puede haber picos >0 dBFS. No usar para diagnosticar clipping en grabacion.

## Analisis

SPSC FIFO de 65536 muestras, solo habilitada cuando el usuario solicita analizar. Se analiza el canal L (mono si hay una sola entrada), antes del color. Un hilo de baja prioridad consume audio; FFT Hann 2048 sin solapamiento. Cuatro segundos de audio recibido, minimo un segundo de ventanas activas RMS >-50 dBFS; timeout 15 s de reloj. No es un detector de voz entrenado. Respiraciones y ruido fuerte pueden contar como actividad. Si se pierde audio por FIFO lleno se descarta el resultado.

Cuatro regiones: 80-250, 250-2000, 2000-6000, 6000-16000 Hz. Se calcula media de potencia por bin y por ventana activa. Reglas conservadoras ajustan el punto inicial elegido: entrada brillante reduce AIR y aumenta SMOOTH; exceso relativo de graves reduce BODY; RMS alto reduce CRUNCH. El crest factor se muestra en resultados de prueba, pero no se usa para diagnosticar compresion. Espectro representa la ultima ventana activa de la captura, no una curva de respuesta ni analizador en vivo.

Las recomendaciones no son una optimizacion perceptual ni comparan con instrumental. Calida/Crocante/Brillante determinan la direccion; Sutil/Equilibrada/Intensa modifican dosis. OUTPUT se conserva al proponer. Vista previa usa atomicos y no cambia el estado guardado; APLICAR escribe parametros reales automatizables. Cambiar controles detiene la escucha temporal. Cambiar intencion/intensidad tambien la detiene. Cerrar interfaz cancela escucha temporal; captura ya solicitada puede terminar.

## Tiempo real

No se usan locks, I/O, FFT ni asignaciones en processBlock. El lock de resultados solo conecta el hilo de analisis con la interfaz. El motor usa buffers fijos, una instancia sobremuestreada y dos canales como maximo. UI 20 Hz. No se promete un porcentaje fijo de CPU en FL Studio; depende de CPU, frecuencia, numero de instancias y tamano de bloque.
