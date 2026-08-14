cmake -B build -S . && cmake --build build -j$(nproc)
# cmake -B build -S . -DVITAMIN_CXX_BUILD_EXAMPLES=ON && cmake --build build -j$(nproc)
