#include "mst_algorithms.h"

MST Boruvka::compute_mst() {
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

std::tuple<EdgeSet, GraphType, EdgeMap>
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
        // edge is key in components_edges set which prevents multiedges
        boost::add_edge(src, dst, weight, components);
        new_to_old[std::make_pair(src, dst)] = std::make_pair(old_src, old_dst);
    }

    return {std::move(min_edges), std::move(components), std::move(new_to_old)};
}

std::tuple<GraphType, std::unordered_set<double>> borůvka_step2(GraphType& graph) {
    auto weight_map = get(boost::edge_weight, graph);
    std::vector<Vertex> paren(boost::num_vertices(graph));
    std::vector<size_t> rank(boost::num_vertices(graph));
    boost::disjoint_sets dsets(make_iterator_property_map(
                rank.begin(), get(boost::vertex_index, graph)), make_iterator_property_map(
                paren.begin(), get(boost::vertex_index, graph)));
    for (Vertex v : boost::make_iterator_range(boost::vertices(graph))) {
        dsets.make_set(v);
    }
    std::unordered_set<double> min_edges{};

    for (auto u : boost::make_iterator_range(boost::vertices(graph))) {
        auto min_edge = std::optional<Edge>();
        for (auto edge : boost::make_iterator_range(boost::out_edges(u, graph))) {
            if (!min_edge.has_value() || weight_map[min_edge.value()] > weight_map[edge]) {
                min_edge = edge;
            }
        }
        if (min_edge.has_value()) {
            auto v = boost::target(min_edge.value(), graph);
            min_edges.insert(weight_map[min_edge.value()]);
            dsets.union_set(u, v);
        }
    }

    auto components = GraphType();
    std::unordered_map<std::pair<Vertex, Vertex>, std::tuple<double, Vertex, Vertex>, PairHash<Vertex, Vertex>> components_edges{};
    auto set_to_new = std::vector<Vertex>(boost::num_vertices(graph), graph.null_vertex());

    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        auto src = boost::source(edge, graph);
        auto dst = boost::target(edge, graph);
        auto src_set = dsets.find_set(src);
        auto dst_set = dsets.find_set(dst);
        if (src_set != dst_set) {
            if (set_to_new[src_set] == graph.null_vertex()) {
                set_to_new[src_set] = boost::add_vertex(components); 
            }
            if (set_to_new[dst_set] == graph.null_vertex()) {
                set_to_new[dst_set] = boost::add_vertex(components);
            }
            auto src_v = set_to_new[src_set];
            auto dst_v = set_to_new[dst_set];
            // the key has a specified order so (u, v) == (v, u)
            auto first = std::min(src, dst);
            auto second = std::max(src, dst);
            auto key = std::make_pair(std::min(src_v, dst_v), std::max(src_v, dst_v));
            if (!components_edges.contains(key) ||  weight_map[edge] < std::get<0>(components_edges[key])) {
                components_edges[key] = {weight_map[edge], first, second};
            }
        }
    }
    for (auto [key, val] : components_edges) {
        auto [src, dst] = key;
        auto [weight, old_src, old_dst] = val;
        // edge is key in components_edges set which prevents multiedges
        boost::add_edge(src, dst, weight, components);
    }

    return {std::move(components), std::move(min_edges)};
}

