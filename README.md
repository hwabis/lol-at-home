```
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build .
```
