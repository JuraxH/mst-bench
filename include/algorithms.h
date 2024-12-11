#pragma once

#include "boost/graph/kruskal_min_spanning_tree.hpp"
#include "boost/graph/prim_minimum_spanning_tree.hpp"
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

// for comparing with boost impl to test quality of our implementation
class KruskalBoost : public MSTAlgorithm {
    public:
    KruskalBoost(Graph& g) : MSTAlgorithm(g, "kruskal_boost") { }

    MST compute_mst() override {
        auto mst = std::vector<Edge>{};
        boost::kruskal_minimum_spanning_tree(g.graph, std::back_inserter(mst));
        return mst;
    }
};

class RandomKKT : public MSTAlgorithm {
    public:
    // We abuse here the fact that the edge weights are unique, to map
    // the edges in the subgraphs to the original graph.
    // If the edges were not unique we could instead assing each edge id
    // and keep it as edge property in each subgraph, and order dedges by it,
    // but that would make the code even more complicated, so we do this
    // instead.
    std::unordered_map<double, Edge> weight_to_edge;

    RandomKKT(Graph &g);

    MST compute_mst() override;
    std::unordered_set<double> compute_mst_impl(GraphType& graph);
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

// for comparing with boost impl to test quality of our implementation
class PrimBoost : public MSTAlgorithm {
    public:
    PrimBoost(Graph &g) : MSTAlgorithm(g, "prim_boost") { }

    MST compute_mst() override {
        auto preds = std::vector<Vertex>(boost::num_vertices(g.graph));
        boost::prim_minimum_spanning_tree(g.graph, preds.data());
        return preds;
    }
};


class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    MST compute_mst() override;
};

inline std::vector<std::shared_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::shared_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_shared<Kruskal>(g));
    algs.push_back(std::make_shared<KruskalBoost>(g));
    algs.push_back(std::make_shared<Boruvka>(g));
    algs.push_back(std::make_shared<PrimBinHeap>(g));
    algs.push_back(std::make_shared<PrimFibHeap>(g));
    algs.push_back(std::make_shared<PrimBoost>(g));
    algs.push_back(std::make_shared<RandomKKT>(g));
    return algs;
}
