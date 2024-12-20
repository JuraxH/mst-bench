#include <boost/ut.hpp>
#include "mst_algorithms.h"
#include "graph.h"
#include "lca.h"
#include "tree_path_maxima.h"
#include "utils.h"
#include "mst_verify.h"

#include <limits>
#include <stdexcept>

using namespace boost::ut;



GraphType test_tree() {
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}
    };
    std::vector<double> weights = {
        1.5, 2.3, 0.9, 1.2, 3.1, 2.8
    };
    return GraphType(edges.begin(), edges.end(), weights.begin(), 7);
}

int main() {
    // test to test build
    "1 == 1"_test = [] {
        expect(1 == 1);
    };

    "throws"_test = [] {
        expect(throws([] { throw 0; }));
    };

    "doesn't throw"_test = [] {
        expect(nothrow([]{}));
    };

    // lca tests
    "lca/euler_tour"_test = [] {
        auto t = test_tree();
        auto expected_nodes = std::vector<size_t>{0,1,3,1,4,1,0,2,5,2,6,2,0};
        auto expected_height = std::vector<size_t>{0,1,1,2,2,2,2};
        auto expected_first_visit = std::vector<size_t>{0,1,7,2,4,8,10};
        auto lca = LCA(t, 0);
        expect(expected_nodes == lca.euler_tour);
        expect(expected_height == lca.height);
        expect(expected_first_visit == lca.first_visit);
    };

    "lca/query"_test = [] {
        auto t = test_tree();
        auto lca = LCA(t, 0);
        auto queries = std::vector<std::tuple<size_t, size_t>>{{3,0}, {3,1}, {3,2}, {3,5}, {3,6}, {4,5}, {1, 2}, {5, 6}};
        auto expected_lca = std::vector<size_t>{0, 1, 0, 0, 0, 0, 0, 2};
        for (size_t i = 0; i < queries.size(); i++) {
            auto [u, v] = queries[i];
            auto res = lca.lca(u, v);
            auto naive_res = naive_lca(t, u, v, 0);
            if (res != expected_lca[i] && naive_res == expected_lca[i]) {
                std::cerr << "failed at query: " << std::get<0>(queries[i]) << ", " << std::get<1>(queries[i])  << " got: " << res << " expected: " << expected_lca[i] << '\n';
                expect(false);
            }
        }
    };

    "lca/naive"_test = [] {
        auto t = test_tree();
        Vertex root = 0;

        auto max = find_path_maxima_naive(t, 3, 6, root);
        auto src = boost::source(max, t);
        auto dst = boost::target(max, t);
        expect(src == 6);
        expect(dst == 2);
        max = find_path_maxima_naive(t, 3, 4, root);
        src = boost::source(max, t);
        dst = boost::target(max, t);
        expect(src == 4);
        expect(dst == 1);
        max = find_path_maxima_naive(t, 3, 5, root);
        src = boost::source(max, t);
        dst = boost::target(max, t);
        expect(src == 5);
        expect(dst == 2);
        max = find_path_maxima_naive(t, 4, 6, root);
        src = boost::source(max, t);
        dst = boost::target(max, t);
        expect(src == 6);
        expect(dst == 2);
    };

    "lca/depth"_test = [] {
        auto t = test_tree();
        auto lca = LCA(t, 0);
        expect(lca.depth(0) == 0);
        expect(lca.depth(3) == 2);
        expect(lca.depth(4) == 2);
        expect(lca.depth(5) == 2);
        expect(lca.depth(6) == 2);
        expect(lca.depth(1) == 1);
        expect(lca.depth(2) == 1);
    };

    "lca/parent"_test = [] {
        auto t = test_tree();
        auto lca = LCA(t, 0);
        expect(lca.parrent(0) == boost::graph_traits<GraphType>::null_vertex());
        expect(lca.parrent(1) == 0);
        expect(lca.parrent(2) == 0);
        expect(lca.parrent(3) == 1);
        expect(lca.parrent(4) == 1);
        expect(lca.parrent(5) == 2);
        expect(lca.parrent(6) == 2);
    };

    "lca/leafs"_test = [] {
        auto t = test_tree();
        auto lca = LCA(t, 0);
        expect(lca.max_depth() == 2);
        auto expected_leafs = std::vector<Vertex>{3, 4, 5, 6};
        auto leafs = lca.leafs();
        expect(leafs == expected_leafs);
    };

    "TreePathMaxima/query_mapping"_test = [] {
        auto t = test_tree();
        auto queries = std::vector<BottomUpQuery>{{3, 0}, {3, 1}, {4, 1}, {5, 0}, {6, 2}, {4, 0}};
        auto lca = LCA(t, 0);
        auto tm = TreePathMaxima(queries, lca);
        auto expected_mapping = std::vector<std::vector<size_t>>{{1, 0}, {5, 2, }, {3, }, {4, },};
        auto res = tm.query_per_leaf();
        for (size_t i = 0; i < expected_mapping.size(); i++) {
            expect(res[i].size() == expected_mapping[i].size());
            for (size_t j = 0; j < res[i].size(); j++) {
                expect(res[i][j] == expected_mapping[i][j]);
            }
        }
    };

    "TreePathMaxima/query_set_propagation"_test = [] {
        auto t = test_tree();
        auto queries = std::vector<BottomUpQuery>{{3, 0}, {3, 1}, {4, 1}, {5, 0}, {6, 2}, {4, 0}};
        auto lca = LCA(t, 0);
        auto tm = TreePathMaxima(queries, lca);
        expect(tm.query_sets[0] == 0);
        expect(tm.query_sets[1] == n_bit(0));
        expect(tm.query_sets[2] == n_bit(0));
        expect(tm.query_sets[3] == n_bits({0, 1}));
        expect(tm.query_sets[4] == n_bits({0, 1}));
        expect(tm.query_sets[5] == n_bit(0));
        expect(tm.query_sets[6] == n_bit(1));
    };

    "st_to_fbt/simple_tree"_test = [] {
        auto t = test_tree();
        auto [graph, leaf_map, root] = st_to_fbt(t);
        auto expected_leaf_weights = std::vector<double>{1.5, 0.9, 2.3, 0.9, 1.2, 3.1, 2.8};
        for (size_t i = 0; i < expected_leaf_weights.size(); i++) {
            auto edge = *boost::out_edges(leaf_map[i], graph).first;
            auto weight_map = get(boost::edge_weight, graph);
            expect(weight_map[edge] == expected_leaf_weights[i]);
        }
        expect(root == 7);
    };

    "st_to_fbt/2_iter"_test = [] {
        auto t = test_tree();
        auto old_weight_map = get(boost::edge_weight, t);
        old_weight_map[boost::edge(0, 1, t).first] = 4.0;
        auto [graph, leaf_map, root] = st_to_fbt(t);
        auto expected_leaf_weights = std::vector<double>{2.3, 0.9, 2.3, 0.9, 1.2, 3.1, 2.8};
        for (size_t i = 0; i < expected_leaf_weights.size(); i++) {
            auto edge = *boost::out_edges(leaf_map[i], graph).first;
            auto weight_map = get(boost::edge_weight, graph);
            expect(weight_map[edge] == expected_leaf_weights[i]);
        }
        expect(root == 9);
    };

    "mst_verify/transform_queries"_test = [] {
        auto t = test_tree();
        auto old_weight_map = get(boost::edge_weight, t);
        old_weight_map[boost::edge(0, 1, t).first] = 4.0;
        auto queries = std::vector<std::tuple<Vertex, Vertex, double>>{{3, 4, 5.1}, {4, 5, 6.0}, };
        // auto expected_res = std::vector<double>{1.2, 3.1,};
        auto expected_res = std::vector<BottomUpQuery>{{3, 8}, {4,8}, {4, 9}, {5, 9},};
        auto mv = MSTVerify(t, queries);
        auto lca = LCA(mv.fbt, mv.fbt_root);
        auto res = mv.transform_queries(lca);
        expect(res.size() == expected_res.size());
        for (size_t i = 0; i < res.size(); i++) {
            expect(res[i].leaf == expected_res[i].leaf);
            // std::cerr << "from leaf   :" << res[i].leaf << std::endl;
            // std::cerr << "from leaf   :" << expected_res[i].leaf << std::endl;
            // std::cerr << "expected ans: " << expected_res[i].ancestor << std::endl;
            // std::cerr << "res ans     :" << res[i].ancestor << std::endl;
            expect(res[i].ancestor == expected_res[i].ancestor);
        }
    };

    "mst_verify/heavy_edges"_test = [] {
        auto t = test_tree();
        auto old_weight_map = get(boost::edge_weight, t);
        old_weight_map[boost::edge(0, 1, t).first] = 4.0;
        auto queries = std::vector<std::tuple<Vertex, Vertex, double>>{{3, 4, 0.1}, {4, 5, 0.0}, {3, 0, 0.1}};
        auto expected_res = std::unordered_set<double>{1.2, 4.0, 4.0};
        auto mv = MSTVerify(t, queries);
        auto heavy = mv.compute_heavy_edges();
        std::cout << "teseting " << std::endl;
        expect(expected_res == heavy);
    };

    "randomKKT/remove_heavy_edges"_test = [] {
        auto t = test_tree();
        auto old_weight_map = get(boost::edge_weight, t);
        old_weight_map[boost::edge(0, 1, t).first] = 4.0;
        boost::add_edge(3, 4, 0.2, t);
        boost::add_edge(5, 0, 0.2, t);
        auto forest = std::unordered_set<double>{0.9, 1.2, 4.0, 3.1, 2.8, 2.3};
        auto removed = std::unordered_set<double>{3.1, 1.2};
        // dump_as_dot(std::cout, t);
        auto g = remove_heavy_edges(t, forest);
        // dump_as_dot(std::cout, g);
        auto weight_map = get(boost::edge_weight, g);
        for (auto edge : boost::make_iterator_range(boost::edges(g))) {
            expect(!removed.contains(weight_map[edge]));
        }
    };
}
