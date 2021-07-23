#include <ogdf/basic/graph_generators.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/graphalg/SeparatorLiptonTarjan.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <ogdf/basic/graph_generators/deterministic.h>
#include <ogdf/graphalg/SeparatorLiptonTarjan.h>

#include <iostream>
#include <functional>
#include <chrono>

using namespace ogdf;


Graph getRandomPlanarGraph(int n) {
    Graph graph;
    int edges = randomNumber(n, 3*n-6);
    randomPlanarConnectedGraph(graph, n, edges);

    return graph;
}

// tests whether a node appears twice in any of the lists
bool testListCompleteness(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second) {

    NodeArray<bool> marked;
    marked.init(G, false);

    auto listCheck = [&marked](const List<node> &list) -> bool {
        for(node no : list) {
            if(marked[no]) return false;
            marked[no] = true;
        }
        return true;
    };

    // making sure that no node was mentioned twice
    if( !listCheck(sep) || !listCheck(first) || !listCheck(second) ) {
        return false;
    }

    // making sure that no node was forgotten
    for(node no : G.nodes) {
        if(!marked[no]) return false;
    }

    return true;
}

// tests whether the size constraints on the separation are fulfilled
bool testListSizes(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second) {
    int n = G.numberOfNodes();
    bool sizes = n == sep.size() + first.size() + second.size();
    bool sepSize = sep.size() < 2 * sqrt(2) * sqrt(n);
    bool firstSize = first.size() <= 2.0 / 3.0 * n;
    bool secondSize = second.size() <= 2.0 / 3.0 * n;

    return sizes && sepSize && firstSize && secondSize;
}

// tests whether the separator actually separates the graph
bool testSeparatorCorrectness(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second) {

    GraphCopy graphCopy(G);

    for(node no : sep) {
        graphCopy.delNode(graphCopy.copy(no));
    }

    NodeArray<bool> marked;
    marked.init(graphCopy, false);

    ArrayBuffer<node> buffer;

    // start BFS at each node of first
    for(node v : first) {
        node v_copy = graphCopy.copy(v);
        if (marked[v_copy]) continue;

        buffer.push(v_copy);
        marked[v_copy] = true;

        while(!buffer.empty()) {
            node w = buffer.popRet();
            for(adjEntry adj : w->adjEntries) {
                node x = adj->twinNode();
                if (!marked[x]) {
                    marked[x] = true;
                    buffer.push(x);
                }
            }
        }
    }

    // now make sure that none of the nodes in second were visited
    for(node no : second) {
        if(marked[graphCopy.copy(no)]) return false;
    }

    return true;
}

// runs all standard tests on a separation
bool testSeparator(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second) {
    bool listSizesOk = testListSizes(G, sep, first, second);
    bool listComplete = testListCompleteness(G, sep, first, second);
    bool separatorCorrect = testSeparatorCorrectness(G, sep, first, second);

    return listSizesOk && listComplete && separatorCorrect;
}

double run_experiment(int tries, int n) {

    long sum = 0;

    for(int i = 0; i < tries; ++i) {

        //std::cout << "=========" << " working on " << i << " ============" << std::endl;
        setSeed(i);

        Graph g = getRandomPlanarGraph(n);

//        std::cout << "Created Graph" << std::endl;

        List<node> separator;
        List<node> first;
        List<node> second;

        SeparatorLiptonTarjan sep;

        auto start = std::chrono::high_resolution_clock::now();
        sep.separate(g, separator, first, second);
        auto end = std::chrono::high_resolution_clock::now();

        //std::cout << "Separator: " << separator << " Size: " << separator.size() << std::endl;
        //std::cout << "A: " << first << " Size: " << first.size() <<std::endl;
        //std::cout << "B: " << second << " Size: " << second.size() <<std::endl;

        //OGDF_ASSERT(testSeparator(g, separator, first, second));

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

//        std::cout << "Took " << duration.count() << " ms" << std::endl;

        sum += duration.count();
    }

    return sum / (double) tries;
}

int main() {

    std::vector<int> sizes = {300000};//{1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000, 60000};
    for(int size : sizes) {
        double avg = run_experiment(1, size);
        std::cout << "Average duration for size " << size << ": " << avg << " milliseconds." << std::endl;
    }

    return 0;
}

