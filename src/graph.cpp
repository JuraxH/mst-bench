#include "graph.h"
#include "algorithms.h"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/subgraph.hpp>
#include <limits>
#include <ranges>


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

std::tuple<EdgeSet, GraphType, EdgeMap>
borůvka_step(GraphType& graph, std::optional<EdgeMap> cur_to_old) {
    auto weight_map = get(boost::edge_weight, graph);
    std::vector<Vertex> paren(boost::num_vertices(graph));
    std::vector<size_t> rank(boost::num_vertices(graph));
    boost::disjoint_sets dsets(make_iterator_property_map(
                rank.begin(), get(boost::vertex_index, graph)), make_iterator_property_map(
                paren.begin(), get(boost::vertex_index, graph)));
    for (Vertex v : boost::make_iterator_range(boost::vertices(graph))) {
        dsets.make_set(v);
    }
    std::unordered_set<std::pair<Vertex, Vertex>, PairHash<Vertex, Vertex>> min_edges{};

    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        auto min_edge = std::optional<Edge>();
        for (auto edge : boost::make_iterator_range(boost::out_edges(vertex, graph))) {
            if (!min_edge.has_value() || weight_map[min_edge.value()] > weight_map[edge]) {
                min_edge = edge;
            }
        }
        if (min_edge.has_value()) {
            auto dst = boost::target(min_edge.value(), graph);
            auto first = std::min(vertex, dst);
            auto second = std::max(vertex, dst);
            if (cur_to_old.has_value()) {
                min_edges.insert(cur_to_old.value()[std::make_pair(first, second)]);
            } else {
                min_edges.insert(std::make_pair(first, second));
            }
            dsets.union_set(vertex, boost::target(min_edge.value(), graph));
        }
    }
    std::unordered_map<Vertex, Vertex> set_to_new{};
    auto components = GraphType();
    std::unordered_map<std::pair<Vertex, Vertex>, std::tuple<double, Vertex, Vertex>, PairHash<Vertex, Vertex>> components_edges{};

    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        auto src = boost::source(edge, graph);
        auto dst = boost::target(edge, graph);
        auto src_set = dsets.find_set(src);
        auto dst_set = dsets.find_set(dst);
        if (src_set != dst_set) {
            if (!set_to_new.contains(src_set)) {
                set_to_new[src_set] = boost::add_vertex(components); 
            }
            if (!set_to_new.contains(dst_set)) {
                set_to_new[dst_set] = boost::add_vertex(components);
            }
            auto src_v = set_to_new[src_set];
            auto dst_v = set_to_new[dst_set];
            // the key has a specified order so (u, v) == (v, u)
            auto first = std::min(src, dst);
            auto second = std::max(src, dst);
            auto key = std::make_pair(std::min(src_v, dst_v), std::max(src_v, dst_v));
            if (!components_edges.contains(key) ||  weight_map[edge] < std::get<0>(components_edges[key])) {
                if (cur_to_old.has_value()) {
                    auto [old_src, old_dst] = cur_to_old.value()[std::make_pair(first, second)];
                    components_edges[key] = {weight_map[edge], old_src, old_dst};
                } else {
                    components_edges[key] = {weight_map[edge], first, second};
                }
            }
        }
    }
    auto new_to_old = EdgeMap{};
    for (auto [key, val] : components_edges) {
        auto [src, dst] = key;
        auto [weight, old_src, old_dst] = val;
        boost::add_edge(src, dst, weight, components);
        new_to_old[std::make_pair(src, dst)] = std::make_pair(old_src, old_dst);
    }

    return {std::move(min_edges), std::move(components), std::move(new_to_old)};
}

std::vector<std::unique_ptr<MSTAlgorithm>> get_algorithms(Graph& g) {
    std::vector<std::unique_ptr<MSTAlgorithm>> algs{};
    algs.push_back(std::make_unique<Kruskal>(g));
    algs.push_back(std::make_unique<Boruvka>(g));
    return algs;
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
