#include "mst_algorithms.h"
#include "graph.h"
#include "lca.h"
#include "tree_path_maxima.h"

struct MSTVerify {
    GraphType span_tree;
    GraphType fbt; // fully branching tree
    Vertex fbt_root;
    // (vertex in span_tree, vertex in spantree, weight on edge between them)
    std::vector<std::tuple<Vertex, Vertex, double>> queries;
    std::vector<Vertex> leaf_map;

    MSTVerify(GraphType span_tree, std::vector<std::tuple<Vertex, Vertex, double>> queries)
        : span_tree(span_tree)
        , fbt_root()
        , queries(queries)
    {
        auto [graph, map, root] = st_to_fbt(span_tree);
        fbt = std::move(graph);
        leaf_map = std::move(map);
        fbt_root = root;
    }

    std::unordered_set<double> compute_heavy_edges() {
        auto lca = LCA(fbt, fbt_root);
        auto heavy_edges = std::unordered_set<double>{};
        auto path_maxima_queries = transform_queries(lca);
        auto tm = TreePathMaxima(path_maxima_queries, lca);
        for (size_t i = 0; i < queries.size(); i++) {
            auto a1 = tm.answers[2 * i];
            auto a2 = tm.answers[2 * i + 1];
            auto weight = std::get<2>(queries[i]);
            auto q_w = std::max(tm.weight(a1), tm.weight(a2));
            heavy_edges.insert(std::max(weight, q_w));
        }

        return heavy_edges;
    }

    std::vector<BottomUpQuery> transform_queries(LCA& lca) {
        auto tree_path_queries = std::vector<BottomUpQuery>{};
        for (auto [u, v, weight] : queries) {
            auto ancestor = lca.lca(u, v);
            // query i will be at (2i, 2i + 1)
            tree_path_queries.push_back({u, ancestor});
            tree_path_queries.push_back({v, ancestor});
        }
        return tree_path_queries;
    }
};
