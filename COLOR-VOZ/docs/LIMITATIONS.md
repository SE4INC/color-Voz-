# Alcance de 1.0

- Este es un efecto de color, no un afinador, compresor completo o clon de voces.
- Asistente local de reglas, no modelo de IA entrenado ni evaluacion de calidad absoluta.
- Analiza entrada mono/canal izquierdo y captura aislada. No escucha instrumental, sidechain ni referencias.
- El espectro muestra la ultima captura; no consume FFT continuamente.
- Una sola calidad 2x. Sobremuestreo reduce aliasing, no lo elimina por completo.
- Compensacion RMS limitada; no garantiza igualdad de volumen percibido.
- ORIGINAL mantiene OUTPUT activo. MIX=0 deja solo la ruta seca retrasada.
- El preset seleccionado permanece como nombre del ultimo preset aunque edites sus controles.
- No hay presets personales externos, undo global ni modo ARA.
- Guardado de estado incluye controles y preset, no captura ni grafico ni elecciones del asistente.
- Las pruebas sinteticas y una muestra vocal corta no certifican calidad en todas las voces. Hace falta escuchar tus tomas y medir CPU en tu PC/FL Studio.
- El build de Windows se ejecuta en tu GitHub Actions o Visual Studio. No confundir artefactos Linux con Windows.
