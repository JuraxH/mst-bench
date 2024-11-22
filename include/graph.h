#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graphviz.hpp>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <ranges>


// undirected graph with weighted edges
using GraphType = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property, // no vertex property
    boost::property<boost::edge_weight_t, double>  // Edge weights
>;

struct Graph {
    GraphType graph;
    size_t id;

    void to_dot(std::ostream& os) {
        auto weightmap = get(boost::edge_weight, graph);
        boost::write_graphviz(os, graph,
                boost::default_writer(),
                [&](std::ostream& out, const auto& p) {
                    out << "[label=\"" << weightmap[p] << "\"]";
                });
    }
};

// who added ranges to c++20 without collect????????
std::vector<std::string> collect(auto iter) {
    std::vector<std::string> res{};
    for (const auto& part : iter) {
        std::string s{};
        // WTF
        for (auto e : part) {
            s.push_back(e);
        }
        res.push_back(s);
    }
    return res;
}

inline Graph parse_graph(std::filesystem::path file, size_t id) {
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
    size_t nodes = std::stoi(tmp[0]);
    [[maybe_unused]] size_t edges = std::stoi(tmp[1]);

    auto res = Graph { GraphType(nodes), id};

    auto weightmap = get(boost::edge_weight, res.graph);

    while (std::getline(is, line)) {
        auto tmp = collect(line | std::ranges::views::split(' '));
        assert(tmp.size() == 3);
        size_t src = std::stoi(tmp[0]);
        size_t dst = std::stoi(tmp[1]);
        double weight = std::stod(tmp[2]);
        auto e = boost::add_edge(src, dst, res.graph).first;
        weightmap[e] = weight;
    }

    return res;
}

