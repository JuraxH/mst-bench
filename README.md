# mst-bench
## deps
Requires CMake 3.20+ and c++ compiler with c++20 support.
## building 
Because the build needs boost, takes about half an hour to configure and build.
```
cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
cmake --build build
```
If you have boost installed locally use the following instead for faster build:
```
cmake -DBOOST_LOCAL=1 -DCMAKE_BUILD_TYPE=Release -Bbuild -S.
cmake --build build
```
## running 
To run custom benchmarks check the help menu.
```
./build/mst-bench --help
```
To run unit test use the following command
```
./build/tests
```

