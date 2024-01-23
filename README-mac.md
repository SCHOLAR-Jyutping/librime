# RIME for TypeDuck Development with Mac

## Prerequisites

- Xcode
- Xcode Command Line Tools

## Install cmake

``` sh
brew install cmake
```

## Checkout source code

``` sh
git clone --recursive https://github.com/TypeDuck-HK/librime.git
cd librime
```

## Install Boost

``` sh
source install-boost.sh
```

This should set the `BOOST_ROOT` variable to the installation path.

## Build third-party libraries

``` sh
make xcode/deps
```

## Build librime

``` sh
make xcode
```

Or, create a debug build:

``` sh
make xcode/debug
```

## Run unit tests

``` sh
make xcode/test
```

Or, test the debug build:

``` sh
make xcode/test-debug
```

## Try it in the console

`librime` comes with a REPL application which can be used to test if the library
is working.

``` sh
cd debug/bin
./Debug/rime_api_console
```
