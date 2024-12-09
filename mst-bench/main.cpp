#include <exception>
#include <iostream>

#include "graph.h"
#include "algorithms.h"
#include <argparse/argparse.hpp>

#include "lca.h"

int main(int argc , char** argv) {
    argparse::ArgumentParser program("mst-bench");
    program.add_argument("command")
        .help("test");
    program.add_argument("graph")
        .help("path to the graph file")
        .default_value("");
    try {
        program.parse_args(argc, argv);
    } catch (std::exception const& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    auto command = program.get("command");
    if (command == std::string("test")) {
        auto graph = program.get("graph");
        auto g = parse_graph(graph);
        double ref = g.mst_weight();
        auto algs = get_algorithms(g);
        for (auto& alg : algs) {
            auto mst = alg->compute_mst();
            auto name = alg->name;
            double res = alg->mst_weight(mst);
            if (is_close(res, ref)) {
                std::cout << name << ": passed\n";
            } else {
                std::cout << name << ": failed expected: \n" << ref << " got: " << res << '\n';
            }
        }
    }
    return 0;
}
