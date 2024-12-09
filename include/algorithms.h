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
    Kruskal(Graph& g) : MSTAlgorithm(g, "Kruskal") { }

    MST compute_mst() override;
};

class RandomKKT : public MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g, "randomKKT") { }

    MST compute_mst() override;
};

class PrimBinHeap : public MSTAlgorithm {
    public:
    PrimBinHeap(Graph &g) : MSTAlgorithm(g, "PrimBinHeap") { }

    MST compute_mst() override;
};

class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    MST compute_mst() override;
};

inline std::vector<std::unique_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::unique_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_unique<Kruskal>(g));
    algs.push_back(std::make_unique<Boruvka>(g));
    algs.push_back(std::make_unique<PrimBinHeap>(g));
    return algs;
}
