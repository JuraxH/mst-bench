#include "algorithms.h"
#include "graph.h"
#include <unordered_set>

RandomKKT::RandomKKT(Graph &g) : MSTAlgorithm(g, "random_KKT"), weight_to_edge() {
    for (auto edge : boost::make_iterator_range(boost::edges(g.graph))) {
        weight_to_edge[g.weight_map[edge]] = edge;
    }
}

MST RandomKKT::compute_mst() {
    auto mst_weights = compute_mst_impl(g.graph);
    auto mst = std::vector<Edge>{};
    for (auto weight : mst_weights) {
        mst.push_back(weight_to_edge.at(weight));
    }
    return mst;
}

std::unordered_set<double> RandomKKT::compute_mst_impl(GraphType& graph) {
    auto* current = &graph;
    auto tmp = GraphType();
    auto mst = std::unordered_set<double>{};
    while (boost::num_vertices(*current) > 1) {
        auto [graph, edges] = borůvka_step2(*current);
        tmp = std::move(graph);
        current = &tmp;
        mst.insert(edges.begin(), edges.end());
    }
    return mst;
}

// expects tree as input
std::tuple<GraphType, std::vector<Vertex>, Vertex> st_to_fbt(GraphType& graph) {
    auto old_to_leafs = std::vector<Vertex>(boost::num_vertices(graph), graph.null_vertex());
    auto fbt = GraphType();
    auto reduced_prev_to_fbt = std::vector<Vertex>{};

    for (auto u : boost::make_iterator_range(boost::vertices(graph))) {
        auto u_fbt = boost::add_vertex(fbt);
        reduced_prev_to_fbt.push_back(u_fbt);
        old_to_leafs[u] = u_fbt;
    }
    auto tmp = GraphType();
    auto* cur = &graph;
    auto last_added = fbt.null_vertex(); // after loop ends thsi will be the root
    while (boost::num_vertices(*cur) > 1) {
        auto [reduced, edges] = boruvka_step_fbt(*cur);
        tmp = std::move(reduced);
        cur = &tmp;
        // add edges to the merged vertex in boruvka step
        auto reduced_to_fbt = std::vector<Vertex>(boost::num_vertices(reduced), fbt.null_vertex());
        for (auto [src_prev_reduced, dst_reduced, weight] : edges) {
            auto src_fbt = reduced_prev_to_fbt[src_prev_reduced];
            auto dst_fbt = reduced_to_fbt[dst_reduced];
            if (dst_fbt == fbt.null_vertex()) {
                dst_fbt = boost::add_vertex(fbt);
                reduced_to_fbt[dst_reduced] = dst_fbt;
                last_added = dst_fbt;
            }
            boost::add_edge(src_fbt, dst_fbt, weight, fbt);
        }
        reduced_prev_to_fbt.swap(reduced_to_fbt);
    }
    return {fbt, old_to_leafs, last_added};
}

std::tuple<GraphType, std::vector<std::tuple<Vertex, Vertex, double>>> boruvka_step_fbt(GraphType& graph) {
    auto weight_map = get(boost::edge_weight, graph);
    std::vector<Vertex> paren(boost::num_vertices(graph));
    std::vector<size_t> rank(boost::num_vertices(graph));
    boost::disjoint_sets dsets(make_iterator_property_map(
                rank.begin(), get(boost::vertex_index, graph)), make_iterator_property_map(
                paren.begin(), get(boost::vertex_index, graph)));
    for (Vertex v : boost::make_iterator_range(boost::vertices(graph))) {
        dsets.make_set(v);
    }
    auto merge_edge_of_vertex = std::vector<std::tuple<Vertex, double>>{};

    for (auto u : boost::make_iterator_range(boost::vertices(graph))) {
        auto min_edge = std::optional<Edge>();
        for (auto edge : boost::make_iterator_range(boost::out_edges(u, graph))) {
            if (!min_edge.has_value() || weight_map[min_edge.value()] > weight_map[edge]) {
                min_edge = edge;
            }
        }
        if (min_edge.has_value()) {
            auto v = boost::target(min_edge.value(), graph);
            dsets.union_set(u, v);
            merge_edge_of_vertex.push_back({u, weight_map[min_edge.value()]});
        }
    }

    auto components = GraphType();
    std::unordered_map<std::pair<Vertex, Vertex>, std::tuple<double, Vertex, Vertex>, PairHash<Vertex, Vertex>> components_edges{};
    auto set_to_new = std::vector<Vertex>(boost::num_vertices(graph), graph.null_vertex());

    for (auto v : boost::make_iterator_range(boost::vertices(graph))) {
        auto v_set = dsets.find_set(v);
        if (set_to_new[v_set] == graph.null_vertex()) {
            set_to_new[v_set] = boost::add_vertex(components); 
        }
    }

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

    auto merge_edges = std::vector<std::tuple<Vertex, Vertex, double>>{};
    for (auto [u, weight] : merge_edge_of_vertex) {
        auto u_set = dsets.find_set(u);
        auto v = set_to_new[u_set];
        merge_edges.push_back({u, v, weight});
    }

    for (auto [key, val] : components_edges) {
        auto [src, dst] = key;
        auto [weight, old_src, old_dst] = val;
        boost::add_edge(src, dst, weight, components);
    }

    return {std::move(components), std::move(merge_edges)};
}
