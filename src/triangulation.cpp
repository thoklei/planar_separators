/**
 * This file serves as a workbench to try things out.
 */

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/extended_graph_alg.h>

using namespace ogdf;

void speedtest() {

	for(int i = 1000; i < 65000; i*=2) {
		Graph G; // = getRandomPlanarGraph(1000);
		GraphIO::read(G, "/Users/thomas/Uni/Masterarbeit/planar_separators/instances/delaunay/delaunay_" + std::to_string(i) + ".gml",
					  GraphIO::readGML);
//		CombinatorialEmbedding E;
//
		makeSimpleUndirected(G);
		planarEmbedPlanarGraph(G);

		std::cout << "size of instance: " << G.numberOfNodes() << std::endl;

		auto start = std::chrono::high_resolution_clock::now();
//		int counter = 0;
//		NodeArray<int> array(G, 0);
//		for(int j = 0; j < i; j++) {
//			node n = G.newNode();
//			array[n] = randomNumber(0,100);
//		}

//		planarEmbedPlanarGraph(G); // embedding the graph can at times look a little superlinear
//		E.init(G);
		GraphCopy copy(G);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "Duration for " << G.numberOfNodes() << ": " << duration.count() / 1000.0 << std::endl;
	}
}

int main(int, char**)
{
	speedtest();
    return 0;
}