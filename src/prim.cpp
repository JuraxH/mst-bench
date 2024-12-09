#include "algorithms.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <limits>
#include <queue>

struct CmpSecond {
    bool operator() (std::pair<Vertex, double> const& a, std::pair<Vertex, double> const&b) {
        return a.second > b.second;
    }
};

MST PrimBinHeap::compute_mst() {
    auto& graph = g.graph;
    auto weight_map = g.weight_map;
    auto null_vertex = boost::graph_traits<GraphType>::null_vertex();
    auto predecessors = std::vector<Vertex>(boost::num_vertices(graph), null_vertex);
    auto key = std::vector<double>(boost::num_vertices(graph), std::numeric_limits<double>::infinity());
    auto in_mst = std::vector<bool>(boost::num_vertices(graph), false);


    auto queue = std::priority_queue<
        std::pair<Vertex, double>,
        std::vector<std::pair<Vertex, double>>,
        CmpSecond
            >{};

    auto start = *boost::vertices(graph).first;

    key[start] = 0;
    queue.emplace(start, 0.0);

    while (!queue.empty()) {
        auto u = queue.top().first;
        queue.pop();

        in_mst[u] = true;

        for (auto edge : boost::make_iterator_range(boost::out_edges(u, graph))) {
            auto v = boost::target(edge, graph);
            auto weight = weight_map[edge];
            if (!in_mst[v] && weight < key[v]) {
                key[v] = weight;
                predecessors[v] = u;
                queue.emplace(v, weight);
            }
        }
    }

    return predecessors;
}
