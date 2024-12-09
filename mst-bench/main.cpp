#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>

#include "graph.h"
#include "algorithms.h"
#include <argparse/argparse.hpp>
#include <memory>
#include <unordered_set>

#include "lca.h"
#include "utils.h"

struct AlgRunner {
    std::filesystem::path graph_file;
    Graph graph;
    std::vector<std::shared_ptr<MSTAlgorithm>> algs_to_run;

    AlgRunner(std::filesystem::path graph_file, std::unordered_set<std::string> filter)
        : graph_file(graph_file)
        , graph(parse_graph(graph_file))
        , algs_to_run(get_algorithms(graph))
    {
        if (!filter.empty()) {
            for (auto it = algs_to_run.begin(); it != algs_to_run.end(); ) {
                if (filter.contains(it->get()->name)) {
                    it = algs_to_run.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    virtual void run() {
        for (auto alg : algs_to_run) {
            run_on_alg(*alg);
        }
    }

    virtual void run_on_alg(MSTAlgorithm& alg) = 0;
};

struct TestRunner : public AlgRunner {
    double ref_res;
    TestRunner(std::filesystem::path graph_file, std::unordered_set<std::string> filter) : AlgRunner(graph_file, filter), ref_res(graph.mst_weight()) { }
    
    virtual void run() override {
        std::cerr << "running tests on " << graph_file << ":\n";
        AlgRunner::run();
    }

    void run_on_alg(MSTAlgorithm &alg) override {
            auto mst = alg.compute_mst();
            double res = alg.mst_weight(mst);
            if (is_close(res, ref_res)) {
                std::cerr << alg.name << ": passed\n";
            } else {
                std::cerr << alg.name << ": failed expected: \n" << ref_res << " got: " << res << '\n';
            }
    }
};

int main(int argc , char** argv) {
    argparse::ArgumentParser program("mst-bench");

    std::cerr << "building the test command" << std::endl;
    auto test_command = argparse::ArgumentParser("test");
    test_command.add_description("verifies that impls of mst algs get the same result as boost impl");
    test_command.add_argument("graph")
        .help("path to the file of the tested graph");
    test_command.add_argument("--filter")
        .help("only run on the specied algorithms")
        .nargs(1, 10)
        .default_value(std::vector<std::string>{});

    auto ls_command = argparse::ArgumentParser("ls");
    ls_command.add_description("list runable algorithms for computing mst");

    program.add_subparser(test_command);
    program.add_subparser(ls_command);

    try {
        std::cerr << "starting the parsing" << std::endl;
        program.parse_args(argc, argv);
    } catch (std::exception const& err) {
        std::cerr << "failed to parse args" << std::endl;
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::cerr << "checking test_command" << std::endl;
    if (program.is_subcommand_used(test_command)) {
        auto graph = test_command.get("graph");
        auto algs_to_run = test_command.get<std::vector<std::string>>("filter");
        auto filter = std::unordered_set<std::string>{};
        filter.insert(algs_to_run.begin(), algs_to_run.end());

        auto test_runner = TestRunner(graph, filter);
        test_runner.run();
        return 0;
    }
    if (program.is_subcommand_used(ls_command)) {
        auto g = parse_graph("graphs/xavierwoo/dtp_small/dtp_15_30_1.txt");
        auto algs = get_algorithms(g);
        for (auto& alg : algs) {
            auto name = alg->name;
            std::cout << name << std::endl;
        }
    }
    return 0;
}
