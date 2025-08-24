# Reactor
used for test reactor network model

# Compile Option

``` cmd

 1. BUILD_SHARED_REACTOR_LIB: build shared library. default is OFF.
 2. BUILD_STATIC_REACTOR_LIB: build static library. default is ON.
 3. BUILD_TESTS: build test program. default is ON.
 4. BUILD_EXAMPLES: build examples. default is ON.

``` 

# Build

``` cmake

 1. cmake -B build -DBUILD_SHARED_REACTOR_LIB=ON
 2. cmake --build build -j4

```

# Test

``` cmd

 1. cd bin/tests
 2. ./TestLog

```

# Example

``` cmd

 1. cd bin/examples
 2. ./EchoServer

```

# Clear

``` cmd
 
 rm -rf build bin lib

```