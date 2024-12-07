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

class Kruskal : public MSTAlgorithm {
    public:
    Kruskal(Graph& g) : MSTAlgorithm(g, "Kruskal") { }

    MST compute_mst() override {
        auto mst = std::vector<Edge>{};
        auto weight_map = boost::get(boost::edge_weight, g.graph);
        size_t edges_in_mst = boost::num_vertices(g.graph) - 1;

        // sort the edges
        std::vector<std::pair<Edge, double>> edges_with_weights;
        for (auto e : boost::make_iterator_range(boost::edges(g.graph))) {
            edges_with_weights.emplace_back(e, weight_map[e]);
        }
        std::sort(edges_with_weights.begin(), edges_with_weights.end(),
                [](const std::pair<Edge, double>& a, const std::pair<Edge, double>& b) {
                return a.second < b.second;
                });

        // init union find
        std::vector<Vertex> paren(boost::num_vertices(g.graph));
        std::vector<size_t> rank(boost::num_vertices(g.graph));
        boost::disjoint_sets dsets(make_iterator_property_map(
                    rank.begin(), get(boost::vertex_index, g.graph)), make_iterator_property_map(
                    paren.begin(), get(boost::vertex_index, g.graph)));
        for (Vertex v : boost::make_iterator_range(boost::vertices(g.graph))) {
            dsets.make_set(v);
        }

        for (auto [edge, weight] : edges_with_weights) {
            auto u = dsets.find_set(edge.m_source);
            auto v = dsets.find_set(edge.m_target);
            if (u != v) {
                mst.emplace_back(edge);
                dsets.link(u, v);
            }
            if (mst.size() == edges_in_mst) {
                return mst;
            }
        }
        return mst;
    }
};

class RandomKKT : public MSTAlgorithm {
    RandomKKT(Graph &g) : MSTAlgorithm(g, "randomKKT") { }

    MST compute_mst() override {
        return {};
    }
};

class Boruvka : public MSTAlgorithm {
    public:
    Boruvka(Graph &g) : MSTAlgorithm(g, "boruvka") { }

    MST compute_mst() override {
        GraphType* current = &g.graph;
        GraphType tmp = GraphType();
        auto mst = std::vector<std::pair<Vertex, Vertex>>{};
        auto cur_to_old = std::optional<EdgeMap>{};
        while (boost::num_vertices(*current) > 1) {
            auto [edges, graph, map] = borůvka_step(*current, cur_to_old);
            tmp = std::move(graph);
            current = &tmp;
            cur_to_old = std::move(map);
            for (auto edge : edges) {
                mst.push_back(edge);
            }
        }
        return mst;
    }
};

inline std::vector<std::unique_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::unique_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_unique<Kruskal>(g));
    algs.push_back(std::make_unique<Boruvka>(g));
    return algs;
}
