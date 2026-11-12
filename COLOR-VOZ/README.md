# CRIS VOX COLOR 1.0

Saturacion vocal selectiva con asistente local, C++17 / JUCE 8.0.13 / VST3 / Standalone. Disenado para cuerpo, presencia y textura crocante en rap y canto. Identidad propia, independiente de CRIS VOX EQ, TUNER y ENHANCER.

**Para conseguir el VST3 de Windows:** lee [INSTALAR_EN_FL_STUDIO.md](INSTALAR_EN_FL_STUDIO.md). Incluye pasos de GitHub y alternativa de compilacion local. La entrega no contiene un binario Windows ya validado. Consulta [docs/VALIDATION.md](docs/VALIDATION.md) para resultados reales y limites.

## Funciones

- BODY, PRESENCE, CRUNCH, AIR, SMOOTH, MIX y OUTPUT automatizables.
- Saturacion de medios/agudos y cuerpo grave con sobremuestreo fijo 2x.
- IGUALAR VOLUMEN RMS, ORIGINAL con latencia compensada y suavizado de controles.
- Analisis de cuatro segundos, intenciones Calida/Crocante/Brillante y tres intensidades.
- Escucha temporal, aplicar o descartar. Sin cuenta, servidor ni modelo generativo.
- Seis presets reales; estado y preset guardados por el host.
- Mono/estereo; 44.1, 48 y 96 kHz; interfaz negra/dorada redimensionable.

## Estructura

- `Source/PluginProcessor.h`, `.cpp`: host, parametros, estado, presets y escucha temporal.
- `Source/PluginEditor.h`, `.cpp`: interfaz y recomendaciones.
- `Source/ColorEngine.h`, `.cpp`: DSP, sobremuestreo, mezcla y compensacion.
- `Source/AnalysisWorker.h`, `.cpp`: FIFO, analisis y reglas musicales.
- `tests/Tests.cpp`: audio sintetico, voz real corta, estado y asignaciones.
- `.github/workflows/build.yml`: construccion Windows/Linux y artefactos.
- `scripts/package-windows.ps1`: empaquetado del VST3 y standalone.
- `docs/`: decisiones DSP, limitaciones y validacion.

## Compilar en Windows x64

Requisitos: Visual Studio 2022 con Desarrollo para el escritorio con C++, Windows SDK, Git y CMake >=3.24. Internet para descargar JUCE, fijado a un commit exacto. No requiere Projucer.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
./scripts/package-windows.ps1
```

Salida: `build/CrisVoxColor_artefacts/Release/VST3/CRIS VOX COLOR.vst3`. Para compilar sin ejecutar PowerShell usa `COMPILAR_WINDOWS.cmd`.

## Linux

Instala compilador C++, CMake, Ninja, Git y paquetes de desarrollo ALSA, X11/Xext/Xinerama/Xrandr/Xcursor, FreeType, Fontconfig y OpenGL.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
xvfb-run -a ctest --test-dir build --output-on-failure
```

Para una copia local de JUCE anade `-DFETCHCONTENT_SOURCE_DIR_JUCE=/ruta/JUCE` al configurar. El plugin de Linux no se puede instalar en FL Studio para Windows.

## Presets

NEUTRO, MEDIOS DORADOS, CROCANTE SUAVE, AIRE SEDOSO, RAP CON MORDIDA, MELODICO CALIDO. Son ajustes iniciales, no modelos de artistas ni promesas de resultado.

## Licencia

Codigo de este proyecto: AGPL-3.0-only, texto en LICENSE. Dependencias y avisos: THIRD_PARTY.md y licenses/. No se incluye una licencia comercial de JUCE.
