#pragma once

#include "utils.h"

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <ranges>
#include <variant>
#include <vector>


// undirected graph with weighted edges
using GraphType = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property, // no vertex property
    boost::property<boost::edge_weight_t, double>  // Edge weights
>;

using Edge = boost::graph_traits<GraphType>::edge_descriptor;
using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;

inline void dump_as_dot(std::ostream& os, GraphType const& graph) {
    auto weightmap = get(boost::edge_weight, graph);
    boost::write_graphviz(os, graph,
            boost::default_writer(),
            [&](std::ostream& out, const auto& p) {
            out << "[label=\"" << weightmap[p] << "\"]";
            });
}

struct Graph {
    GraphType graph;
    boost::property_map<GraphType, boost::edge_weight_t>::type weight_map;

    Graph(size_t vertexes) : graph(vertexes), weight_map(get(boost::edge_weight, graph)) { }

    bool is_connected() {
        std::vector<bool> visited(boost::num_vertices(graph), false);
        boost::default_bfs_visitor vis{};
        boost::breadth_first_search(graph, vertex(0, graph), visitor(vis)
            .color_map(make_iterator_property_map(visited.begin(),
                get(boost::vertex_index, graph))));
        return std::all_of(visited.begin(), visited.end(), std::identity());
    }

    // for testing of implementations
    double mst_weight() {
        std::vector<Edge> mst{};
        boost::kruskal_minimum_spanning_tree(graph, std::back_inserter(mst));
        double res = 0;
        for (auto e : mst) {
            res += weight_map[e];
        }
        return res;
    }
};

using MST = std::variant<std::vector<Edge>, std::vector<std::pair<Vertex, Vertex>>>;

class MSTAlgorithm {
    public:
    Graph& g;
    std::string name;

    MSTAlgorithm(Graph& g, std::string name) : g(g), name(name) { }

    virtual MST compute_mst() = 0;

    virtual double mst_weight(MST mst) {
        double res = 0;
        if (std::holds_alternative<std::vector<Edge>>(mst)) {
            for (auto e : std::get<std::vector<Edge>>(mst)) {
                res += g.weight_map[e];
            }
        } else if (std::holds_alternative<std::vector<std::pair<Vertex, Vertex>>>(mst)) {
            for (auto [u, v] : std::get<std::vector<std::pair<Vertex, Vertex>>>(mst)) {
                auto [edge, valid] = boost::edge(u, v, g.graph);
                res += g.weight_map[edge];
            }
        }
        return res;
    }
    virtual ~MSTAlgorithm() = default;
};

inline Graph parse_graph(std::filesystem::path file) {
    auto is = std::ifstream(file);
    if (!is) {
        throw std::runtime_error("failed to open file: " + file.string() + "\n");
    }
    std::string line;
    if (!std::getline(is, line)) {
        throw std::runtime_error(
                "missing the first line declaring number of nodes and edges in " +
                file.string() + "\n");
    }
    auto tmp = collect(line | std::ranges::views::split(' '));
    assert(tmp.size() == 2);
    size_t vertexes = std::stoi(tmp[0]);
    [[maybe_unused]] size_t edges = std::stoi(tmp[1]);

    auto res = Graph(vertexes);

    while (std::getline(is, line)) {
        auto tmp = collect(line | std::ranges::views::split(' '));
        assert(tmp.size() == 3);
        size_t src = std::stoi(tmp[0]);
        size_t dst = std::stoi(tmp[1]);
        double weight = std::stod(tmp[2]);
        boost::add_edge(src, dst, weight, res.graph);
    }

    return res;
}

