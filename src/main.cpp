#include <iostream>

#include "graph.h"


int main() {
    auto g = parse_graph("graphs/xavierwoo/dtp_small/dtp_15_30_1.txt", 0);

    g.to_dot(std::cout);
}
