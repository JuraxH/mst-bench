#pragma once

#include "graph.h"
#include "utils.h"

#include <algorithm>
#include <boost/container_hash/hash_fwd.hpp>
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



using EdgeMap = std::unordered_map<std::pair<Vertex, Vertex>, std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>>;
inline static std::tuple<std::unordered_set<std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>>, GraphType, EdgeMap>
borůvka_step(GraphType& graph, std::optional<EdgeMap> cur_to_old) {
    auto weight_map = get(boost::edge_weight, graph);
    std::vector<Vertex> paren(boost::num_vertices(graph));
    std::vector<size_t> rank(boost::num_vertices(graph));
    boost::disjoint_sets dsets(make_iterator_property_map(
                rank.begin(), get(boost::vertex_index, graph)), make_iterator_property_map(
                paren.begin(), get(boost::vertex_index, graph)));
    for (Vertex v : boost::make_iterator_range(boost::vertices(graph))) {
        dsets.make_set(v);
    }
    std::unordered_set<std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>> min_edges{};

    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        auto min_edge = std::optional<Edge>();
        for (auto edge : boost::make_iterator_range(boost::out_edges(vertex, graph))) {
            if (!min_edge.has_value() || weight_map[min_edge.value()] > weight_map[edge]) {
                min_edge = edge;
            }
        }
        if (min_edge.has_value()) {
            auto dst = boost::target(min_edge.value(), graph);
            auto first = std::min(vertex, dst);
            auto second = std::max(vertex, dst);
            if (cur_to_old.has_value()) {
                min_edges.insert(cur_to_old.value()[std::make_pair(first, second)]);
            } else {
                min_edges.insert(std::make_pair(first, second));
            }
            dsets.union_set(vertex, boost::target(min_edge.value(), graph));
        }
    }
    std::unordered_map<Vertex, Vertex> set_to_new{};
    auto components = GraphType();
    std::unordered_map<std::pair<Vertex, Vertex>, std::tuple<double, Vertex, Vertex>, PairHash<Vertex, Vertex>> components_edges{};

    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        auto src = boost::source(edge, graph);
        auto dst = boost::target(edge, graph);
        auto src_set = dsets.find_set(src);
        auto dst_set = dsets.find_set(dst);
        if (src_set != dst_set) {
            if (!set_to_new.contains(src_set)) {
                set_to_new[src_set] = boost::add_vertex(components); 
            }
            if (!set_to_new.contains(dst_set)) {
                set_to_new[dst_set] = boost::add_vertex(components);
            }
            auto src_v = set_to_new[src_set];
            auto dst_v = set_to_new[dst_set];
            // the key has a specified order so (u, v) == (v, u)
            auto first = std::min(src, dst);
            auto second = std::max(src, dst);
            auto key = std::make_pair(std::min(src_v, dst_v), std::max(src_v, dst_v));
            if (!components_edges.contains(key) ||  weight_map[edge] < std::get<0>(components_edges[key])) {
                if (cur_to_old.has_value()) {
                    auto [old_src, old_dst] = cur_to_old.value()[std::make_pair(first, second)];
                    components_edges[key] = {weight_map[edge], old_src, old_dst};
                } else {
                    components_edges[key] = {weight_map[edge], first, second};
                }
            }
        }
    }
    auto new_to_old = EdgeMap{};
    for (auto [key, val] : components_edges) {
        auto [src, dst] = key;
        auto [weight, old_src, old_dst] = val;
        boost::add_edge(src, dst, weight, components);
        new_to_old[std::make_pair(src, dst)] = std::make_pair(old_src, old_dst);
    }

    return {std::move(min_edges), std::move(components), std::move(new_to_old)};
}

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
    Kruskal(Graph& g) : MSTAlgorithm(g, "Kruskal") { }

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

class RandomKKT : public MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g, "randomKKT") { }

    void compute_mst() override {
    }
};

class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    void compute_mst() override {
        GraphType* current = &g.graph;
        GraphType tmp = GraphType();
        g.mst_edges_boruvka.clear();
        auto cur_to_old = std::optional<EdgeMap>{};
        while (boost::num_vertices(*current) > 1) {
            auto [edges, graph, map] = borůvka_step(*current, cur_to_old);
            tmp = std::move(graph);
            current = &tmp;
            cur_to_old = std::move(map);
            for (auto edge : edges) {
                g.mst_edges_boruvka.push_back(edge);
            }
        }
    }

    double sum() override {
        double res = 0;
        auto weight_map = get(boost::edge_weight, g.graph);
        for (auto [u, v] : g.mst_edges_boruvka) {
            auto [edge, valid] = boost::edge(u, v, g.graph);
            if (!valid) {
                std::cerr << "Edge does not exist between " << u << " and " << v << '\n';
                continue;
            }
            res += weight_map[edge];
        }
        return res;
    }
};

inline std::vector<std::unique_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::unique_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_unique<Kruskal>(g));
    algs.push_back(std::make_unique<Boruvka>(g));
    return algs;
}
