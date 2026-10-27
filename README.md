<img width="1296" height="406" alt="изображение" src="https://github.com/user-attachments/assets/823bad90-7d0a-42d3-9188-20d0f9664ae5" />


# EN
### Niritech Labs Utils is a set of utilities for creating games and graphics applications.
## Current Modules
- Base(NLUtils) - Logger,Text Utilities, Timer, OS Requests, Thread Watchdog.
- EnderIO - Blocks format, Path Utiliti, Parsers JSON Blocks INI XML
- Extra - AssetLoader
- NGToolkit - Retaited Mode Toolkit

## How to Build
* Base CMakeLists.txt
```CMake
cmake_minimum_required(VERSION 3.30)
project(test VERSION 0.2.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(BUILD_NLUTILS_VENDORED_MODE ON CACHE INTERNAL "")
add_subdirectory(NLUtils ${CMAKE_CURRENT_BINARY_DIR}/nlutils_build)


add_executable(YourExe
    youfile.cpp
)

# Connect whatever you need
target_link_libraries(testic
    NLUtils-NLGToolkit
    NLUtils
    NLUtils-EnderIO
    NLUtils-Extra
)

target_include_directories(YourExe PRIVATE 
    NLUtils/include
) 

```

### Linux 
```Bash
mkdir build && cd build
cmake ..
make
```
### Windows
* For Windows, download MinGW or any other compiler that supports CMake.
- Official Site https://winlibs.com/
- For Win64 https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-23.1.3-14.0.0-ucrt-r3/winlibs-x86_64-posix-seh-gcc-16.2.0-mingw-w64ucrt-14.0.0-r3.7z
- For Win32 https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-23.1.3-14.0.0-ucrt-r3/winlibs-i686-posix-dwarf-gcc-16.2.0-mingw-w64ucrt-14.0.0-r3.7z
### Don't forget to run the MinGW path configuration script.
```PowerShell
mkdir build
cd build
cmake ..
cmake --build .
```

# RU
### Niritech Labs Utils - это набор утилит для создания игр и графических приложений.
## Текущие модули
- Base (NLUtils) — Логгер, утилиты для работы с текстом, таймер, взаимодействие с ОС, контроль потоков (Thread Watchdog).
- EnderIO — формат Blocks, класс для работы с путями, парсеры (JSON, Blocks, INI, XML).
- Extra — загрузчик ресурсов (AssetLoader).
- NGToolkit — инструментарий для построения интерфейсов в режиме сохранения состояния (Retained Mode Toolkit).

## Как собрать
* Базовый CMakeLists.txt
```CMake
cmake_minimum_required(VERSION 3.30)
project(test VERSION 0.2.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(BUILD_NLUTILS_VENDORED_MODE ON CACHE INTERNAL "")
add_subdirectory(NLUtils ${CMAKE_CURRENT_BINARY_DIR}/nlutils_build)


add_executable(YourExe
    youfile.cpp
)

# Подключайте те модули что вам нужны
target_link_libraries(testic
    NLUtils-NLGToolkit
    NLUtils
    NLUtils-EnderIO
    NLUtils-Extra
)

target_include_directories(YourExe PRIVATE 
    NLUtils/include
) 

```

### Linux 
```Bash
mkdir build && cd build
cmake ..
make
```
### Windows
* Для Windows, скачайте MinGW или любой другой компилятор поддерживающий CMake.
- Оффициальный сайт https://winlibs.com/
- Для Win64 https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-23.1.3-14.0.0-ucrt-r3/winlibs-x86_64-posix-seh-gcc-16.2.0-mingw-w64ucrt-14.0.0-r3.7z
- Для Win32 https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-23.1.3-14.0.0-ucrt-r3/winlibs-i686-posix-dwarf-gcc-16.2.0-mingw-w64ucrt-14.0.0-r3.7z
### Не забудьте запустить скрипт настройки путей для MinGW.
```PowerShell
mkdir build
cd build
cmake ..
cmake --build .
```

