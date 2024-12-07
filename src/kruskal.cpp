#include "algorithms.h"

MST Kruskal::compute_mst() {
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
