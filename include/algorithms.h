#pragma once

#include "graph.h"

#include <algorithm>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

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

class RandomKKT : MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g) { }

    void compute_mst() override {
    }

    std::pair<std::vector<Edge>, GraphType> borůvka_step(GraphType& graph) {
        auto weight_map = get(boost::edge_weight, graph);
        std::vector<Vertex> paren(boost::num_vertices(graph));
        std::vector<size_t> rank(boost::num_vertices(graph));
        boost::disjoint_sets dsets(make_iterator_property_map(
            rank.begin(), get(boost::vertex_index, graph)), make_iterator_property_map(
            paren.begin(), get(boost::vertex_index, graph)));
        for (Vertex v : boost::make_iterator_range(boost::vertices(graph))) {
            dsets.make_set(v);
        }
        std::vector<Edge> min_edges{};
        for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
            auto min_edge = std::optional<Edge>();
            for (auto edge : boost::make_iterator_range(boost::out_edges(vertex, graph))) {
                if (!min_edge.has_value() || weight_map[min_edge.value()] > weight_map[edge]) {
                    min_edge = edge;
                }
            }
            if (min_edge.has_value()) {
                min_edges.push_back(min_edge.value());
                dsets.union_set(vertex, boost::source(min_edge.value(), graph));
            }
        }
        // TODO think of better mapping
        std::unordered_map<Vertex, Vertex> set_to_new{};
        auto components = GraphType();
        for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
            auto set = dsets.find_set(vertex);
            if (!set_to_new.contains(set)) {
                set_to_new[set] = boost::add_vertex(graph);
            }
        }
        for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
            auto src = boost::source(edge, graph);
            auto dst = boost::target(edge, graph);
            auto src_set = dsets.find_set(src);
            auto dst_set = dsets.find_set(dst);
            if (src_set != dst_set) {
                auto src_v = set_to_new[src_set];
                auto dst_v = set_to_new[dst_set];
                boost::add_edge(src_v, dst_v, weight_map[edge], components);
            }
        }
        auto is_valid = std::vector<Vertex>(boost::num_vertices(components), std::numeric_limits<Vertex>::max());
        auto current_edge = std::vector<std::optional<Edge>>(boost::num_vertices(components));
        auto isolated = std::vector<Vertex>();
        auto redundant_edge = std::vector<Edge>();
        auto component_weight_map = get(boost::edge_weight, components);
        for (auto vertex : boost::make_iterator_range(boost::vertices(components))) {
            if (boost::degree(vertex, components)) {
                isolated.push_back(vertex);
                continue;
            }
            for (auto edge : boost::make_iterator_range(boost::out_edges(vertex, components))) {
                auto dst = boost::target(edge, components);
                if (is_valid[dst] == vertex) {
                    if (component_weight_map[current_edge[dst].value()] > component_weight_map[edge]) {
                        redundant_edge.push_back(current_edge[dst].value());
                        current_edge[dst] = edge;
                    } else {
                        redundant_edge.push_back(edge);
                    }
                } else {
                    is_valid[dst] = vertex;
                    current_edge[dst] = edge;
                }
            }
        }
        for (auto edge : redundant_edge) {
            boost::remove_edge(edge, components);
        }
        for (auto vertex : isolated) {
            boost::remove_vertex(vertex, components);
        }

        return std::make_pair(std::move(min_edges), std::move(components));
    }
};

inline std::vector<std::pair<std::string, std::unique_ptr<MSTAlgorithm>>> get_algorithms(Graph& g) {
    std::vector<std::pair<std::string, std::unique_ptr<MSTAlgorithm>>> algs{};
    algs.push_back(std::make_pair<std::string, std::unique_ptr<MSTAlgorithm>>(std::string("krushkal_on_demand_bin_heap"), std::make_unique<Kruskal>(g)));
    return algs;
}
