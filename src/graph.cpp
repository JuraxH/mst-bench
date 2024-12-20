#include "graph.h"
#include "mst_algorithms.h"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/subgraph.hpp>
#include <limits>
#include <ranges>
#include <unordered_set>


void dump_as_dot(std::ostream& os, GraphType const& graph) {
    auto weightmap = get(boost::edge_weight, graph);
    boost::write_graphviz(os, graph,
            boost::default_writer(),
            [&](std::ostream& out, const auto& p) {
            out << "[label=\"" << weightmap[p] << "\"]";
            });
}

bool Graph::is_connected() {
    std::vector<bool> visited(boost::num_vertices(graph), false);
    boost::default_bfs_visitor vis{};
    boost::breadth_first_search(graph, vertex(0, graph), visitor(vis)
            .color_map(make_iterator_property_map(visited.begin(),
                    get(boost::vertex_index, graph))));
    return std::all_of(visited.begin(), visited.end(), std::identity());
}

double Graph::mst_weight() {
    std::vector<Edge> mst{};
    boost::kruskal_minimum_spanning_tree(graph, std::back_inserter(mst));
    double res = 0;
    for (auto e : mst) {
        res += weight_map[e];
    }
    return res;
}

double MSTAlgorithm::mst_weight(MST mst) {
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
    } else if (std::holds_alternative<PredecessorMap>(mst)) {
        auto null_vertex = boost::graph_traits<GraphType>::null_vertex();
        auto& parent = std::get<PredecessorMap>(mst);
        for (size_t u = 0; u < parent.size(); u++) {
            auto v = parent[u];
            // skip the root
            if (v == null_vertex || v == u) {
                continue;
            }
            auto edge = boost::edge(u, v, g.graph).first;
            res += g.weight_map[edge];
        }
    }
    return res;
}

Graph parse_graph(std::filesystem::path file) {
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

    // to remove duplicates
    auto inserted_edges = std::unordered_set<std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>>{};

    while (std::getline(is, line)) {
        auto tmp = collect(line | std::ranges::views::split(' '));
        if (tmp.size() != 3) {
            continue;
        }
        size_t src = std::stoi(tmp[0]);
        size_t dst = std::stoi(tmp[1]);
        double weight = std::stod(tmp[2]);
        assert(src < vertexes);
        assert(dst < vertexes);
        auto in_order = ordered(src, dst);
        if (!inserted_edges.contains(in_order)) {
            inserted_edges.insert(in_order);
            // set inserted edges is used to preven multi edges
            boost::add_edge(src, dst, weight, res.graph);
        }
    }

    return res;
}

bool all_edge_weights_unique(GraphType const& g) {
    auto weight_map = get(boost::edge_weight, g);
    std::unordered_set<double> weights{};
    for (Edge e : boost::make_iterator_range(boost::edges(g))) {
        auto weight = weight_map[e];
        if (weights.contains(weight)) {
            return false;
        }
        weights.insert(weight);
    }
    return true;
}

std::vector<Vertex> find_path(const GraphType& g, Vertex start, Vertex end) {
    std::vector<Vertex> predecessors(boost::num_vertices(g), std::numeric_limits<Vertex>::max());

    auto bfs_visitor = boost::make_bfs_visitor(
        boost::record_predecessors(predecessors.data(), boost::on_tree_edge{})
    );

    boost::breadth_first_search(g, start, boost::visitor(bfs_visitor));

    std::vector<Vertex> path;
    for (Vertex v = end; v != start; v = predecessors[v]) {
        if (v == std::numeric_limits<Vertex>::max()) {
            return {};
        }
        path.push_back(v);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

size_t naive_lca(GraphType& g, size_t u, size_t v, size_t root) {
    auto to_v = find_path(g, u, v);
    auto to_root = find_path(g, u, root);
    auto len = std::max(to_v.size(), to_root.size());
    size_t i = 0;
    for (; i < len; i++) {
        if (to_v[i] != to_root[i]) {
            break;
        }
    }
    return to_root[i - 1];
}

Edge find_path_maxima_naive(GraphType& g, Vertex u, Vertex v, Vertex root) {
    auto lca = naive_lca(g, u, v, root);
    auto weight_map = get(boost::edge_weight, g);
    auto u_path = find_path(g, u, lca);
    auto v_path = find_path(g, v, lca);

    auto max_edge = [&] (Edge a, Edge b) {
        return weight_map[a] > weight_map[b] ? a : b;
    };

    auto find_max = [&] (auto path) {
        Edge max = boost::edge(path[0], path[1], g).first;
        for (size_t i = 2; i < path.size(); i++) {
            auto prev = i - 1;
            auto edge = boost::edge(path[prev], path[i], g).first;
            max = max_edge(max, edge);
        }
        return max;
    };
    auto u_max = find_max(u_path); 
    auto v_max = find_max(v_path); 
    return max_edge(u_max, v_max);
}
