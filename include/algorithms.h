#pragma once

#include "graph.h"
#include "utils.h"

#include <algorithm>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>

using PredecessorMap = std::vector<Vertex>;
using MST = std::variant<std::vector<Edge>, std::vector<std::pair<Vertex, Vertex>>, PredecessorMap>;

class MSTAlgorithm {
    public:
    Graph& g;
    std::string name;

    MSTAlgorithm(Graph& g, std::string name) : g(g), name(name) { }

    virtual MST compute_mst() = 0;

    double mst_weight(MST mst);
    virtual ~MSTAlgorithm() = default;
};


class Kruskal : public MSTAlgorithm {
    public:
    Kruskal(Graph& g) : MSTAlgorithm(g, "kruskal") { }

    MST compute_mst() override;
};

class RandomKKT : public MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g, "random_KKT") { }

    MST compute_mst() override;
};

class PrimBinHeap : public MSTAlgorithm {
    public:
    PrimBinHeap(Graph &g) : MSTAlgorithm(g, "prim_bin_heap") { }

    MST compute_mst() override;
};

class PrimFibHeap : public MSTAlgorithm {
    public:
    PrimFibHeap(Graph &g) : MSTAlgorithm(g, "prim_fib_heap") { }

    MST compute_mst() override;
};

class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    MST compute_mst() override;
};

inline std::vector<std::shared_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::shared_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_shared<Kruskal>(g));
    algs.push_back(std::make_shared<Boruvka>(g));
    algs.push_back(std::make_shared<PrimBinHeap>(g));
    algs.push_back(std::make_shared<PrimFibHeap>(g));
    return algs;
}
