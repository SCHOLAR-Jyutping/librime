# RIME for TypeDuck Development with Windows

## Prerequisites

  - **Visual Studio** for *Desktop development in C++*
  - **[cmake](http://www.cmake.org/)**

## Checkout source code

``` batch
git clone --recursive https://github.com/TypeDuck-HK/librime.git
cd librime
```

## Install Boost

``` batch
install-boost.bat
```

## Setup a build environment

Copy `env.bat.template` to `env.bat` and edit the file according to your setup.
Specifically, make sure `BOOST_ROOT` is set to the root directory of Boost
source tree; modify `BJAM_TOOLSET`, `CMAKE_GENERATOR` and `PLATFORM_TOOLSET` if
using a different version of Visual Studio; also set `DEVTOOLS_PATH` for build
tools installed to custom location.

When prepared, do the following in a *Developer Command Prompt* window.

## Build Boost

This is already handled by `install-boost.bat`.

``` batch
build.bat boost
```

## Build third-party libraries

``` batch
build.bat deps
```

## Build librime

``` batch
build.bat librime
```

Or, create a debug build:

``` batch
build.bat librime debug
```

## Run unit tests

``` batch
build.bat test
```

Or, test the debug build:

``` batch
build.bat test debug
```

## Try it in the console

`librime` comes with a REPL application which can be used to test if the library
is working.

``` batch
copy /Y build\lib\Release\rime.dll build\bin
cd build\bin
Release\rime_api_console.exe
```
