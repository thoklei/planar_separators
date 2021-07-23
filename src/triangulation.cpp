// This is just to confirm that the regular triangulation function takes quadratic time for planar graphs

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>

#include <iostream>
#include <chrono>

using namespace ogdf;

Graph getRandomPlanarGraph(int n) {
    Graph graph;
    int edges = randomNumber(n, 3*n-6);
    randomPlanarConnectedGraph(graph, n, edges);

    return graph;
}


int main() {

    std::vector<int> sizes = {5000, 10000, 20000, 40000, 80000};
    int tries = 10;

    for(int size : sizes) {

        long avg = 0;

        for(int i = 0; i < tries; ++i) {
            Graph g = getRandomPlanarGraph(size);

            auto start = std::chrono::high_resolution_clock::now();
            triangulate(g);
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            avg += duration.count();
        }
        std::cout << "Average duration for size " << size << " was: " << avg / (double) tries << " ms" << std::endl;
    }


    return 0;
}