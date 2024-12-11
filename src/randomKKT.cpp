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
