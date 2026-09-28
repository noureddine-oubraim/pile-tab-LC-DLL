@echo off
setlocal

where g++ >nul 2>nul
if errorlevel 1 (
    echo Erreur : g++ est introuvable dans PATH.
    exit /b 1
)

g++ -std=c++17 -O2 -Wall -Wextra -pedantic -DVERIFICATION_DLL_BUILD -shared verification_validite.cpp -o verification_validite.dll -Wl,--out-implib,libverification_validite.dll.a
if errorlevel 1 exit /b 1

g++ -std=c++17 -O2 -Wall -Wextra -pedantic client_verification.cpp -L. -lverification_validite -o client_verification.exe
if errorlevel 1 exit /b 1

echo Compilation terminee : verification_validite.dll et client_verification.exe
endlocal