#include "algorithms.h"

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
