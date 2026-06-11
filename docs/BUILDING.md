# Compilacion de PhotoStudio Premium

## Requisitos

| Componente | Version minima |
|---|---|
| CMake | 3.20 |
| Compilador C++20 | MSVC 2022 / GCC 11 / Clang 15 |
| Qt | 6.2 (Core, Gui, Widgets, Svg) |
| OpenCV | 4.x (core, imgproc, imgcodecs, photo) |

GoogleTest se descarga automaticamente via FetchContent cuando los tests estan activados.

## Linux (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install cmake ninja-build build-essential \
    qt6-base-dev libqt6svg6-dev libopencv-dev libgl1-mesa-dev

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/PhotoStudio
```

## Windows (MSVC 2022 + vcpkg)

```powershell
git clone https://github.com/Microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = (Resolve-Path .\vcpkg)

# vcpkg.json en la raiz instala qtbase, qtsvg y opencv4 automaticamente
cmake --preset windows-msvc
cmake --build --preset windows-msvc
.\build\msvc\src\Release\PhotoStudio.exe
```

La primera configuracion compila Qt y OpenCV desde fuentes; puede tardar bastante.

## macOS

```bash
brew install cmake ninja qt opencv
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
open build/src/PhotoStudio.app
```

## Conan (alternativa)

```bash
pip install conan
conan profile detect --force
conan install . --output-folder=build --build=missing
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Opciones de CMake

| Opcion | Por defecto | Descripcion |
|---|---|---|
| `PHOTOSTUDIO_BUILD_TESTS` | `ON` | Compila los tests unitarios (GoogleTest) |
| `PHOTOSTUDIO_ENABLE_ASAN` | `OFF` | Activa AddressSanitizer/UBSan |

## Tests

```bash
ctest --test-dir build --output-on-failure
```
