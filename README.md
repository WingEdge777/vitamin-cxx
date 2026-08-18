# Vitamin-CXX

A personal C/CPP practice ground.

## Build

```bash
./build.sh
```

Build examples:

```bash
bash build.sh
./build/examples/lockfree_queue_example
./build/examples/thread_pool_example
./build/examples/object_pool_example
```

Link the library from another CMake target with `vitamin_cxx` only:

```cmake
target_link_libraries(your_app PRIVATE vitamin_cxx)
```
