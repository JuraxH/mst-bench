#pragma once

#include "utils.h"

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <filesystem>
#include <ostream>
#include <string>
#include <variant>
#include <vector>


// undirected graph with weighted edges
using GraphType = boost::adjacency_list<
    boost::vecS,
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

Graph parse_graph(std::filesystem::path file);
void dump_as_dot(std::ostream& os, GraphType const& graph);

using EdgeMap = std::unordered_map<std::pair<Vertex, Vertex>, std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>>;
using EdgeSet = std::unordered_set<std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>>;
std::tuple<EdgeSet, GraphType, EdgeMap>
borůvka_step(GraphType& graph, std::optional<EdgeMap> cur_to_old);

using MST = std::variant<std::vector<Edge>, std::vector<std::pair<Vertex, Vertex>>>;

class MSTAlgorithm {
    public:
    Graph& g;
    std::string name;

    MSTAlgorithm(Graph& g, std::string name) : g(g), name(name) { }

    virtual MST compute_mst() = 0;

    double mst_weight(MST mst);
    virtual ~MSTAlgorithm() = default;
};

std::vector<std::unique_ptr<MSTAlgorithm>> get_algorithms(Graph& g);

std::vector<Vertex> find_path(const GraphType& g, Vertex start, Vertex end);
