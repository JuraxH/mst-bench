#pragma once

#include "utils.h"

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>


// undirected graph with weighted edges
using GraphType = boost::adjacency_list<
    boost::listS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property, // vertex property
    boost::property<boost::edge_weight_t, double>  // Edge weights
>;

using Edge = boost::graph_traits<GraphType>::edge_descriptor;
using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;

struct Graph {
    GraphType graph;
    boost::property_map<GraphType, boost::edge_weight_t>::type weight_map;

    Graph(size_t vertexes) : graph(vertexes), weight_map(get(boost::edge_weight, graph)) { }

    bool is_connected();

    // for testing of implementations
    double mst_weight();
};

inline std::pair<Vertex, Vertex> ordered(Vertex u, Vertex v) {
    return {std::min(u, v), std::max(u, v)};
}

Graph parse_graph(std::filesystem::path file);
void dump_as_dot(std::ostream& os, GraphType const& graph);

bool all_edge_weights_unique(GraphType const& g);
std::vector<Vertex> find_path(const GraphType& g, Vertex start, Vertex end);

// slow, only for testing
// graph needs to be tree and u, v list nodes
Edge find_path_maxima_naive(GraphType& g, Vertex u, Vertex v, Vertex root);

// slow, only for testing
size_t naive_lca(GraphType& g, size_t u, size_t v, size_t root);
