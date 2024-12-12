#include "mst_algorithms.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <limits>
#include <queue>

struct Node {
    Vertex vertex;
    double distance;

    bool operator>(Node const& other) const {
        return distance > other.distance;
    }
};

MST PrimBinHeap::compute_mst() {
    auto& graph = g.graph;
    auto weight_map = g.weight_map;
    auto null_vertex = boost::graph_traits<GraphType>::null_vertex();
    auto pred = std::vector<Vertex>(boost::num_vertices(graph), null_vertex);
    auto min_dist = std::vector<double>(boost::num_vertices(graph), std::numeric_limits<double>::infinity());
    auto in_mst = std::vector<bool>(boost::num_vertices(graph), false);


    auto queue = std::priority_queue<Node, std::vector<Node>, std::greater<>>{};

    auto start = *boost::vertices(graph).first;

    min_dist[start] = 0;
    queue.emplace(start, 0.0);

    while (!queue.empty()) {
        auto u = queue.top().vertex;
        queue.pop();

        in_mst[u] = true;

        for (auto edge : boost::make_iterator_range(boost::out_edges(u, graph))) {
            auto v = boost::target(edge, graph);
            auto weight = weight_map[edge];
            if (!in_mst[v] && weight < min_dist[v]) {
                min_dist[v] = weight;
                pred[v] = u;
                queue.emplace(v, weight);
            }
        }
    }

    return pred;
}

MST PrimFibHeap::compute_mst() {
    auto& graph = g.graph;
    auto weight_map = g.weight_map;
    auto null_vertex = boost::graph_traits<GraphType>::null_vertex();
    auto pred = std::vector<Vertex>(boost::num_vertices(graph), null_vertex);
    auto min_dist = std::vector<double>(boost::num_vertices(graph), std::numeric_limits<double>::infinity());
    auto in_mst = std::vector<bool>(boost::num_vertices(graph), false);

    auto start = *boost::vertices(graph).first;

    using FibHeap = boost::heap::fibonacci_heap<Node, boost::heap::compare<std::greater<>>>;
    using FibHandle = FibHeap::handle_type;
    auto heap = FibHeap{};
    auto handles = std::vector<std::optional<FibHandle>>(boost::num_vertices(graph));

    min_dist[start] = 0;
    handles[start] = heap.push({start, 0.0});

    while (!heap.empty()) {
        auto u = heap.top().vertex;
        heap.pop();

        in_mst[u] = true;

        for (auto edge : boost::make_iterator_range(boost::out_edges(u, graph))) {
            auto v = boost::target(edge, graph);
            double weight = weight_map[edge];

            if (!in_mst[v] && weight < min_dist[v]) {
                min_dist[v] = weight;
                pred[v] = u;
                
                if (handles[v].has_value()) {
                    heap.update(handles[v].value(), {v, weight});
                } else {
                    handles[v] = heap.push({v, weight});
                }
            }
        }
    }

    return pred;
}
