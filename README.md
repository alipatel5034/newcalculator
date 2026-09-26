# Scientific Calculator — C + WebAssembly

## Files
- index.html — UI
- style.css — styling + light/dark theme
- script.js — button interactions and C/WASM calls
- calculator.c — C calculation engine

## Build with Emscripten

Install/activate Emscripten first, then from this folder run:

emcc calculator.c -O3 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -s EXPORTED_FUNCTIONS='["_malloc","_free"]' -o calculator.js

This creates:
- calculator.js
- calculator.wasm

Keep both files beside index.html.

## Run locally

Do NOT open index.html directly with file:// because browsers may block WebAssembly loading.

Start a local server:

python -m http.server 8000

Then open:

http://localhost:8000

## Emscripten shell

Windows:
  emsdk_env.bat
  cd path\to\scientific_calculator
  emcc calculator.c -O3 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -s EXPORTED_FUNCTIONS='["_malloc","_free"]' -o calculator.js

Linux/macOS:
  source /path/to/emsdk/emsdk_env.sh
  cd /path/to/scientific_calculator
  emcc calculator.c -O3 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -s EXPORTED_FUNCTIONS='["_malloc","_free"]' -o calculator.js

## Notes
The original C program is terminal-oriented and uses scanf/printf. This project keeps the mathematical logic in C but exposes browser-callable C functions with EMSCRIPTEN_KEEPALIVE. The browser UI calls those exported functions through Module.ccall.
