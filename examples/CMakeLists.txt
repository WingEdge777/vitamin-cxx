find_package(Threads REQUIRED)

add_executable(mpmc_queue_example mpmc_queue_example.cpp)
target_link_libraries(mpmc_queue_example PRIVATE vitamin_cxx Threads::Threads)
