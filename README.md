# Vitamin-CXX

A personal C/CPP practice ground.

## Build

```bash
./build.sh
```

Build examples:

```bash
./build.sh --examples
./build/examples/mpmc_queue_example
```

Link the library from another CMake target with `vitamin_cxx` only:

```cmake
target_link_libraries(your_app PRIVATE vitamin_cxx)
```
