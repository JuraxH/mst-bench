#include <boost/graph/subgraph.hpp>
#include <boost/ut.hpp>
#include "graph.h"
#include "lca.h"

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
}
