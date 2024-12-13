# mst-bench
This repository contains code and data used in project for Graph Algorithms
course at Brno University of Technology, Faculty of Information Technology. 
The project is focused on comparing algorithms for finding minimum spanning trees.

## repository structure
```
.
├── bench_results/
├── CMakeLists.txt
├── eval_results.py
├── download_graphs.py
├── graph_generator.py
├── graphs/
├── include/
│   ├── graph.h
│   ├── lca.h
│   ├── mst_algorithms.h
│   ├── mst_verify.h
│   ├── tree_path_maxima.h
│   └── utils.h
├── mst-bench/
│   └── main.cpp
├── mst_bench.py
├── README.md
├── src/
│   ├── boruvka.cpp
│   ├── graph.cpp
│   ├── kruskal.cpp
│   ├── lca.cpp
│   ├── prim.cpp
│   ├── randomKKT.cpp
│   └── tree_path_maxima.cpp
└── tests/

```
The results of benchmarks that we mesured can be found in csv format in
the `bench_resutls/` directory, graphs used for these benchmarks
are in the `graphs/` directory.
The source files of the mst implementations are in the `src/` directory
and the header files in the `include/` directory.
The source files of the
executable used to run these algorithms is in the `mst-bench/` directory.
The `mst_bench.py` script is used to run benchmarks on the graph datasets
and collect results as csv.
The `download_graphs.py` is the script used to download the random_graphs
dataset, when the repository is downloaded without them.
The `graph_generator.py` is the script we used to generate the random dataset
of graphs.
The `eval_results.py` is used to plot graphs used for the report.


The code in `src/lca.cpp` was taken from [[1]](#1), which use a version of [[3]](#3)
and it was slightly modified to fit our use case.
The code in `src/tree_path_maxima.cpp` was taken from [[2]](#2) and rewrited to
c++ and modified to work on our graph type.

## Dependencies
Requires CMake 3.20+ and c++ compiler with c++20 support.

## Building 
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
## Running 
### mst-bench
To run custom benchmarks check the help menu.
```
./build/mst-bench --help
```
### Unit tests
To run unit tests use the following command
```
./build/tests
```
### Benchmark on random graphs dataset
First run the `download_graphs.py` to download the dataset used in the
report.
To reproduce the results, after building the mst-bench run
the following:
```
python3 mst_bench.py bench random_graphs results.csv
```
The measured results will be saved to `resutls.csv`.

## Used Libraries
- [Boost](https://www.boost.org/): graphs and fibonaci heap
- [Boost-ext μ(micro)/Unit](https://github.com/boost-ext/ut): unit tests 
- [Argparse](https://github.com/p-ranav/argparse): parsing command line arguments

## Authors
- Jiří Hrabovský (xhrabo17@vutbr.cz)
- Jan Vašák (xvasak01@vutbr.cz)

## References
<a id="1">[1]</a>
Algorithms for competetive programming [repository](https://github.com/cp-algorithms/cp-algorithms).

<a id="2">[2]</a>
Hagerup, Torben [2010](https://dl.acm.org/doi/10.5555/1120060.1712350)
An Even Simpler Linear-Time Algorithm for Verifying Minimum Spanning Trees

<a id="3">[3]</a>
Bender, Michael A. and Farach-Colton, Martin and Pemmasani, Giridhar and Skiena, Steven and Sumazin, Pavel [2005](https://dl.acm.org/doi/10.5555/1120060.1712350)
Lowest common ancestors in trees and directed acyclic graphs
