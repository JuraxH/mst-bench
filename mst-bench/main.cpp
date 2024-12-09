#include "graph.h"
#include "algorithms.h"
#include "lca.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_set>


struct AlgRunner {
    std::filesystem::path graph_file;
    Graph graph;
    std::vector<std::shared_ptr<MSTAlgorithm>> algs_to_run;

    AlgRunner(std::filesystem::path graph_file, std::vector<std::string> filter)
        : graph_file(graph_file)
        , graph(parse_graph(graph_file))
        , algs_to_run(get_algorithms(graph))
    {
        if (!filter.empty()) {
            auto filter_s = std::unordered_set<std::string>{};
            filter_s.insert(filter.begin(), filter.end());
            for (auto it = algs_to_run.begin(); it != algs_to_run.end(); ) {
                if (filter_s.contains(it->get()->name)) {
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
    std::vector<bool> results;

    TestRunner(std::filesystem::path graph_file, std::vector<std::string> filter)
        : AlgRunner(graph_file, filter)
        , ref_res(graph.mst_weight())
        , results()
    { }
    
    virtual void run() override {
        std::cerr << "running tests on " << graph_file << ":\n";
        AlgRunner::run();
    }

    void run_on_alg(MSTAlgorithm &alg) override {
            auto mst = alg.compute_mst();
            double res = alg.mst_weight(mst);
            if (is_close(res, ref_res)) {
                results.push_back(true);
                std::cerr << alg.name << ": passed\n";
            } else {
                results.push_back(false);
                std::cerr << alg.name << ": failed expected: \n" << ref_res << " got: " << res << '\n';
            }
    }

    std::string res_as_json() {
        auto dict = std::vector<std::pair<std::string, std::string>>{};
        for (size_t i = 0; i < algs_to_run.size(); i++) {
            dict.emplace_back(algs_to_run[i]->name, bool_to_str(results[i]));
        }
        return to_json(dict);
    }
};

struct BenchRunner : public AlgRunner {
    double ref_res;
    std::vector<size_t> results;

    BenchRunner(std::filesystem::path graph_file, std::vector<std::string> filter)
        : AlgRunner(graph_file, filter)
        , ref_res(graph.mst_weight())
        , results()
    { }

    virtual void run() override {
        std::cerr << "running bench on " << graph_file << ":\n";
        AlgRunner::run();
    }

    void run_on_alg(MSTAlgorithm &alg) override {
        size_t runs = 10;
        using Clc = std::chrono::steady_clock;
        auto start = Clc::now();
        for (size_t i = 0; i < runs; i++) {
            alg.compute_mst();
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(Clc::now() - start);
        results.push_back(elapsed.count() / runs);
    }

    std::string res_as_json() {
        auto dict = std::vector<std::pair<std::string, std::string>>{};
        for (size_t i = 0; i < algs_to_run.size(); i++) {
            dict.emplace_back(algs_to_run[i]->name, std::to_string(results[i]));
        }
        return to_json(dict);
    }
};

int main(int argc , char** argv) {
    argparse::ArgumentParser program("mst-bench");

    auto test_command = argparse::ArgumentParser("test");
    test_command.add_description("verifies that impls of mst algs get the same result as boost impl");
    test_command.add_argument("graph")
        .help("path to the file of the graph");
    test_command.add_argument("--filter")
        .help("only run on the specified algorithms")
        .nargs(1, 10)
        .default_value(std::vector<std::string>{});

    auto ls_command = argparse::ArgumentParser("ls");
    ls_command.add_description("list runable algorithms for computing mst");

    auto info_command = argparse::ArgumentParser("info");
    info_command.add_description("prints info about given graph in json");
    info_command.add_argument("graph")
        .help("path to the file of the graph");

    auto bench_command = argparse::ArgumentParser("bench");
    bench_command.add_description("messures the expected runtime on the graph for each algorithm");
    bench_command.add_argument("graph")
        .help("path to the file of the graph");
    bench_command.add_argument("--filter")
        .help("only run on the specified algorithms")
        .nargs(1, 10)
        .default_value(std::vector<std::string>{});

    program.add_subparser(test_command);
    program.add_subparser(ls_command);
    program.add_subparser(info_command);
    program.add_subparser(bench_command);

    try {
        program.parse_args(argc, argv);
    } catch (std::exception const& err) {
        std::cerr << "failed to parse args" << std::endl;
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.is_subcommand_used(test_command)) {
        auto graph = test_command.get("graph");
        auto filter = test_command.get<std::vector<std::string>>("filter");
        auto test_runner = TestRunner(graph, filter);
        test_runner.run();
        std::cout << test_runner.res_as_json();
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
    if (program.is_subcommand_used(info_command)) {
        auto graph = info_command.get("graph");
        auto g = parse_graph(graph);
        std::vector<std::pair<std::string, std::string>> info;
        info.emplace_back("connected", bool_to_str(g.is_connected()));
        info.emplace_back("unique_weights", bool_to_str(all_edge_weights_unique(g.graph)));
        info.emplace_back("vertices", std::to_string(boost::num_vertices(g.graph)));
        info.emplace_back("edges", std::to_string(boost::num_edges(g.graph)));
        std::cout << to_json(info);
    }
    if (program.is_subcommand_used(bench_command)) {
        auto graph = bench_command.get("graph");
        auto filter = bench_command.get<std::vector<std::string>>("filter");
        auto bench_runner = BenchRunner(graph, filter);
        bench_runner.run();
        std::cout << bench_runner.res_as_json();
    }
    return 0;
}
