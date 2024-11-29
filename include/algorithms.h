#pragma once

#include <algorithm>
#include <boost/graph/subgraph.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <memory>
#include <utility>
#include "graph.h"

class EdgeHeapQueue {
    public:
    Graph& g;
    size_t size;
    EdgeHeapQueue(Graph& g) : g(g), size(boost::num_edges(g.graph)) {
        size_t i = 0;
        for (auto edge : boost::make_iterator_range(boost::edges(g.graph))) {
            g.edges[i++] = edge;
        }
        auto greater = [&] (Edge a, Edge b) { return g.weight_map[a] > g.weight_map[b]; };
        std::make_heap(g.edges.begin(), g.edges.end(), greater);
    }

    Edge pop_min() {
        assert(size != 0);
        auto greater = [&] (Edge a, Edge b) { return g.weight_map[a] > g.weight_map[b]; };
        std::pop_heap(g.edges.begin(), g.edges.begin() + size, greater);
        size--;
        return g.edges[size];
    }
};

// krushkal implementation computing smallest edge on demand
class Kruskal : public MSTAlgorithm {
    public:
    Kruskal(Graph& g) : MSTAlgorithm(g) { }

    void compute_mst() override {
        size_t edges = 0;
        size_t edges_in_mst = boost::num_vertices(g.graph) - 1;
        // init edge queue
        auto queue = EdgeHeapQueue(g);
        // init union find
        boost::disjoint_sets dsets(make_iterator_property_map(
            g.rank.begin(), get(boost::vertex_index, g.graph)), make_iterator_property_map(
            g.paren.begin(), get(boost::vertex_index, g.graph)));
        for (Vertex v : boost::make_iterator_range(boost::vertices(g.graph))) {
            dsets.make_set(v);
        }
        while (edges != edges_in_mst) {
            auto e = queue.pop_min();
            auto u = dsets.find_set(e.m_source);
            auto v = dsets.find_set(e.m_target);
            if (u != v) {
                g.mst_edges[edges++] = e;
                dsets.link(u, v);
            }
        }
    }
    double sum() override {
        double res = 0;
        for (auto e : g.mst_edges) {
            res += g.weight_map[e];
        }
        return res;
    }
};

inline std::vector<std::pair<std::string, std::unique_ptr<MSTAlgorithm>>> get_algorithms(Graph& g) {
    std::vector<std::pair<std::string, std::unique_ptr<MSTAlgorithm>>> algs{};
    algs.push_back(std::make_pair<std::string, std::unique_ptr<MSTAlgorithm>>(std::string("krushkal_on_demand_bin_heap"), std::make_unique<Kruskal>(g)));
    return algs;
}
