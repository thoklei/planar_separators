#ifndef UTILS_H
#define UTILS_H

#include <regex>
#include <sstream>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/graphalg/ShortestPathAlgorithms.h>
#include <ogdf/graphalg/PlanarSeparatorModule.h>
#include <ogdf/graphalg/SeparatorLiptonTarjan.h>
#include <ogdf/graphalg/SeparatorLiptonTarjanFC.h>

using namespace ogdf;

// ========== diameter calculations ========== //

/**
 * WARNING: very expensive method!
 * Calculates the diameter and the radius of a given graph.
 *
 * @param G the graph
 * @return a pair <diameter, radius>
 */
std::pair<int, int> calculateDistances(const Graph &G);


/**
 * Calculates a lower and an upper bound for the size of the diameter of a planar graph.
 *
 * @param G the graph
 * @return a pair <lower bound, upper bound>
 */
std::pair<int, int> calculateDiameterBounds(const Graph &G);


/**
 * Tests whether the size of the separator is smaller than 2*d+1, where d = diameter of the graph.
 * Calculating the actual diameter by calculating all shortest paths is too expensive, so the algorithm terminates
 * as soon as a shortest path P is found that fulfills sepSize < 2 * |P| + 1.
 *
 * @param sepSize the size of the separator
 * @param G the graph
 * @return true if the separator is small enough
 */
bool checkSizeAgainstDiameter(int sepSize, const Graph &G);

// ========== IO stuff ========== //

/**
 * Tests if a file is a graph-file that we can parse.
 * (Only by checking the file extension, not by actually parsing the file!
 *
 * @param path the path to be tested
 * @return true if the file is a .gml, .stp or .chaco file
 */
bool isGraphFile(std::string path);


/**
 * Reads a graph into G from a given path.
 *
 * @param G the graph to be read into
 * @param path the path from which we read the file
 */
void readGraph(Graph &G, std::string path);


/**
 * Calculates a hash code for a given file (probably pretty inefficiently).
 *
 * @param path the file to be hashed
 * @return a hash value
 */
unsigned long getHashCode(std::string path);


/**
 * Extracts the file name from a path.
 *
 * @param path the path to the file
 * @return the filename (between last / and .extension)
 */
std::string extractFileName(std::string path);

/**
 * Extracts the full file name from a path, including sudirectories.
 *
 * @param path the path to the file
 * @return the filename (between "instances/" and .extension)
 */
std::string extractFullFileName(std::string path);

/**
 * Returns a human-readable specification of the current time stamp.
 *
 * @return a string like Sat_Oct_16_16-08-48
 */
std::string currentTime();

// ========== visualization stuff ========== //

/**
 * WARNING: this is really expensive, don't do this for large graphs (more than 100 nodes)
 * Creates a svg-file depicting a planar embedding of a given planar graph.
 *
 * @param graph the graph
 * @param name the filename (without extension)
 */
void drawGraph(const Graph& graph, std::string name);

// ========== separator correctness testing ========== //

/**
 * Tests whether a node appears twice in any of the lists.
 *
 * @param G the graph
 * @param sep list of nodes in the separator
 * @param first list of nodes in the first half
 * @param second list of nodes in the second half
 * @return true if every node appeared exactly once
 */
bool testListCompleteness(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second);


/**
 * Tests whether the size constraints on the separation are fulfilled, ie
 * 	1. the separator is not larger than the value guaranteed by the algorithm
 * 	2. no list contains more than 2/3 of the nodes
 * 	3. sep.size + first.size + second.size = n
 *
 * @param G the graph
 * @param sep the separator
 * @param first the first half
 * @param second the second half
 * @param maxSize the maximal size of the separator as guaranteed by the algorithm
 * @return true if all conditions are fulfilled
 */
bool testListSizes(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second, double maxSize);


/**
 * Tests whether the separator actually separates the graph.
 *
 * @param G the graph
 * @param sep the separator
 * @param first the first half
 * @param second the second half
 * @return true if the graph is separated
 */
bool testSeparatorCorrectness(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second);


/**
 * Runs all standard tests on a separation.
 *
 * @param G the graph
 * @param sep the separator
 * @param first the first half
 * @param second the second half
 * @param maxSize the maximal size of the separator as guaranteed by the algorithm
 * @return true if the separation was legitimate
 */
bool testSeparator(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second, double maxSize);

#endif /* UTILS_H */