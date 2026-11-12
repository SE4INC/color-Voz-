# Obtener el VST3 de Windows

El ZIP del proyecto contiene codigo fuente. GitHub Actions construye el VST3 de Windows x64. La compilacion realizada aqui fue en Linux; el binario Windows lo genera el workflow.

1. Descomprime el ZIP en una carpeta nueva llamada `CRIS-VOX-COLOR`. Los archivos del proyecto estan directamente en la raiz del ZIP.
2. Crea un repositorio nuevo. Sube **el contenido** de esa carpeta a su raiz: `CMakeLists.txt`, `Source`, `tests`, `scripts`, `LICENSE`, etc. Incluye `.github/workflows/build.yml` (las carpetas que empiezan por punto pueden estar ocultas). No subas solo el ZIP ni una carpeta contenedora adicional.
3. En GitHub abre **Actions > Build CRIS VOX COLOR**. El push inicia la compilacion. Tambien puedes usar **Run workflow**.
4. Espera a que el trabajo **windows** termine en verde. Abre esa ejecucion y descarga **CRIS-VOX-COLOR-Windows-x64** en **Artifacts**. Debes iniciar sesion en GitHub.
5. Extrae el artefacto. Copia la carpeta completa `VST3/CRIS VOX COLOR.vst3` a `C:\Program Files\Common Files\VST3\`. Conserva su carpeta `Contents`. Windows puede pedir permisos de administrador.
6. En FL Studio abre **Options > Manage plugins**, verifica esa ruta y ejecuta **Find installed plugins**. Busca **CRIS VOX COLOR** y cargalo en el Mixer de tu voz.

## Primera escucha

Empieza con NEUTRO. Reproduce voz aislada, preferiblemente antes de reverb y delay. Pulsa ANALIZAR VOZ, selecciona Crocante/Sutil y pulsa ESCUCHAR. Usa DETENER para volver a tu ajuste y APLICAR para guardarlo. Los controles no se mueven durante la escucha temporal; los valores propuestos aparecen en el panel. ORIGINAL compara con la entrada con la misma latencia; OUTPUT sigue actuando.

IGUALAR VOLUMEN compensa energia RMS con limite de ±6 dB: deja estabilizar unos segundos y comprueba el nivel. No es un limitador. Si OUT supera 0 dBFS reduce OUTPUT. Compara tambien con la instrumental.

Si la compilacion sale roja, abre el paso que fallo y conserva el error. No descargues un artefacto de otra ejecucion suponiendo que corresponde al codigo nuevo. Si no ves Actions, comprueba que `.github/workflows/build.yml` esta en la raiz correcta.

Alternativa sin GitHub: instala Visual Studio 2022 con C++ y CMake, y ejecuta `COMPILAR_WINDOWS.cmd` en la carpeta del proyecto.

Referencia GitHub: https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts
