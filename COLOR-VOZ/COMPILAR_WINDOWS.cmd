@echo off
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
 echo Instala CMake y Visual Studio 2022 con Desarrollo para el escritorio con C++.
 pause
 exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
cmake --build build --config Release --parallel 2
if errorlevel 1 goto fail
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 goto fail
echo VST3 listo en build\CrisVoxColor_artefacts\Release\VST3
explorer "build\CrisVoxColor_artefacts\Release\VST3"
pause
exit /b 0
:fail
echo La compilacion o una prueba fallo. Conserva el mensaje de error.
pause
exit /b 1
