@echo off
node preprocess.js
rmdir /S /Q .\build
xcopy /E .\src\static .\build\ >nul
(for /F "delims=" %%I in ('where /R src *.c 2^>nul') do set "FileName=%%I" & setlocal EnableDelayedExpansion & echo !FileName:\=/!& endlocal) > sources.txt
(for /F "delims=" %%I in ('where /R src *.S 2^>nul') do set "FileName=%%I" & setlocal EnableDelayedExpansion & echo !FileName:\=/!& endlocal) >> sources.txt
clang ^
    -Wall -Wextra -Wpedantic ^
    -Wno-unused-parameter -Wno-strict-prototypes -Wno-sign-compare -Wno-gnu-empty-struct ^
    --target=wasm32 -nostdlib -fno-builtin ^
    -matomics -mbulk-memory -msimd128 -mrelaxed-simd ^
    -O3 -ffast-math -flto ^
    -Wl,--lto-O3 -Wl,--error-limit=0 ^
    -Wl,--no-entry -Wl,--export-dynamic ^
    -Wl,--import-memory -Wl,--shared-memory -Wl,--initial-memory=98304000 -Wl,--max-memory=98304000 ^
    -Wl,-z,stack-size=65536 ^
    -o ./build/game.wasm ^
    @sources.txt
del sources.txt
npx tsc
