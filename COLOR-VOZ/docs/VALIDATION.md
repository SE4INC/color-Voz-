# Validacion real — 12 noviembre 2026

## Ejecutado aqui

Linux x86_64, GCC 14.2, CMake 4.4.4, JUCE 8.0.13 fijado al commit indicado en CMake. Compilacion Release de VST3 y Standalone completada. Test `cris_color_tests` pasado. Pluginval 1.0.4, strictness 5: SUCCESS. Ver logs completos en reports/.

- Mono y estereo: 44.1, 48 y 96 kHz, bloques de 64, 128 y 2048.
- NEUTRO coincide con original retrasado con error <1e-6.
- MIX=0 y ORIGINAL coinciden con original retrasado despues del suavizado.
- No se observaron asignaciones ordinarias ni alineadas en el hilo de audio durante los bloques comprobados, incluido processBlock con analisis solicitado. La sonda es thread-local, no cuenta memoria legitima del hilo de trabajo.
- Senal sinusoidal de 1 kHz con CRUNCH=0.8: tercer armonico de amplitud ~0.0565, demostrando saturacion real. No equivale a un criterio de calidad sonora.
- Presets reales, preview que no modifica parametros guardados, aplicar propuesta y recuperacion de estado comprobados.
- Procesamiento y analisis de `singing.ogg` del arbol de JUCE; muestra corta repetida hasta completar captura. RMS de analisis -14.13 dBFS. Esto verifica el circuito con audio vocal real; no certifica resultado perceptual ni representa todas las voces graves.
- Pluginval: apertura, editor, parametros, programas, estado, procesamiento y buses. Validador oficial Steinberg no configurado.
- Capturas de la interfaz real a 1220x760 y 1120x740 revisadas: interface.png e interface-small.png.

## Rendimiento orientativo

Procesar aproximadamente un segundo de audio estereo con RAP CON MORDIDA, bloques 2048, incluyendo generacion de seno y comprobaciones de salida del test:

| Frecuencia | Tiempo medido |
|---|---:|
| 44.1 kHz | 5.21 ms |
| 48 kHz | 5.70 ms |
| 96 kHz | 11.33 ms |

Una medicion de esta maquina Linux compartida. Excluye interfaz/analisis/host; no es el indicador de CPU de FL Studio ni una promesa para otros equipos. El sobremuestreo 2x funciona tambien cuando no hay captura. ORIGINAL conserva procesamiento interno para cambios suaves.

## Pendiente

Ejecutar el workflow de Windows y confirmar VST3 en FL Studio sobre Windows x64. Verificar voces del usuario, escucha con instrumental, niveles, automatizacion, sesiones con muchas instancias y CPU con interfaz abierta/cerrada y analisis activo. No se ha afirmado que exista una evaluacion auditiva profesional o un binario Windows compilado aqui.
