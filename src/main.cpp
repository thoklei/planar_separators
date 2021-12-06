#include <ogdf/graphalg/SeparatorLiptonTarjan.h>
#include <ogdf/graphalg/SeparatorLiptonTarjanFC.h>
#include <ogdf/graphalg/SeparatorDual.h>
#include <ogdf/graphalg/SeparatorDualFC.h>
#include <ogdf/graphalg/SeparatorHarPeled.h>
#include <ogdf/basic/graph_generators/deterministic.h>

#include <iostream>
#include <functional>
#include <chrono>
#include <filesystem>
#include <regex>
#include <vector>
#include <unistd.h>
#include <cassert>

#include <property_recorder.h>

using namespace ogdf;

/**
 * Ecoding algorithms as const longs to select algorithms.
 */
const short LT      = 1 << 0;
const short LTFC    = 1 << 1;
const short D       = 1 << 2;
const short DFC     = 1 << 3;
const short HP      = 1 << 4;
const short all     = (1 << 5) - 1;

/**
 * Evaluates five different Separator modules on all test instances.
 */
class Experiment {

    /**
     * Container for one line of the results-csv.
     * Stores data on the instance, the algorithm and the solution - some of it redundant.
     */
    struct Result {

        std::string algorithm;
        PropertyRecorder::Properties prop;
        int nodes;
        int edges;
        long time;
        int sepSize;
        double balance;   // defined as A/B where A is the smaller half
        double ratio; // defined as S/A where A is the smaller half
		std::string exitPoint;


		/**
		 * Constructor.
		 *
		 * @param algorithm the name of the algorithm
		 * @param prop properties of the instance
		 * @param nodes number of nodes of the instance
		 * @param edges number of edges of the instance
		 * @param time time in microseconds it took to solve the instance
		 * @param sepSize separator size (nodes in the separator)
		 * @param firstSize size of the first list
		 * @param secondSize size of the second list
		 * @param exitPoint identifier of the termination point of the algorithm
		 */
        Result(const std::string &algorithm, const PropertyRecorder::Properties &prop, int nodes, int edges, long time, int sepSize, int firstSize, int secondSize, const std::string &exitPoint)
            : algorithm{algorithm}, prop{prop}, nodes{nodes}, edges{edges}, time{time}, sepSize{sepSize}, exitPoint{exitPoint} {

            int shortList = min(firstSize, secondSize);
            int longList = max(firstSize, secondSize);

            balance = shortList / (double) longList;
            ratio = sepSize / (double) shortList;
        }


		/**
		 * Exports the result to one csv-line.
		 *
		 * @return a string containing the data of this result
		 */
        std::string to_csv() const {

            std::vector<std::string> data = {
                    algorithm,
                    prop.name,
                    to_string(nodes),
                    to_string(edges),
                    to_string(prop.diameter),
                    to_string(prop.diameter_lB),
                    to_string(prop.diameter_uB),
                    to_string(prop.radius),
                    to_string(time),
                    to_string(sepSize),
                    to_string(balance),
                    to_string(ratio),
					exitPoint};

            std::string result;
            for(const std::string &val : data) {
                result += val + ",";
            }
            int length = result.size() - result.substr(result.rfind(',')).size();
            result = result.substr(0,length);

            return result + "\n";
        }


		/**
		 * Contains the headline of the csv-file.
		 *
		 * @return the headline
		 */
        static string get_head() {
            return "algorithm,instance,nodes,edges,diameter,diam_lB,diam_uB,radius,time,sep_size,balance,ratio,exit\n";
        }
    };

public:

	/**
	 * Constructor.
	 *
	 * @param res_file path to file in which results are written
	 * @param target_dir directory of instance files
	 * @param propertyFile path to property file (as generated by property recorder)
	 * @param limit size limit (in nodes) of the instances
	 * @param test whether to test the result for correctness
	 * @param attempts number of solving attempts (with different random seeds)
	 * @param algorithm which algorithm to use
	 * @param postprocessing whether to apply postprocessing or not
	 */
    Experiment(const std::string &res_file, const std::string &target_dir, const std::string &propertyFile, int limit, bool test, int attempts, short algorithm, bool postprocessing)
        : res_file{res_file}, instance_dir{target_dir}, limit{limit}, test{test}, attempts{attempts}, selectedAlgorithms{algorithm}, postProcessing{postprocessing}, recorder{propertyFile} {

        file.open(res_file);
        file << Result::get_head();
        file.close();

    }

	/**
	 * Runs the experiment.
	 */
    void run() {

        // all separators
        SeparatorLiptonTarjan sepLipTar;
        SeparatorDual sepDual;
        SeparatorLiptonTarjanFC sepLTFC;
        SeparatorDualFC sepDFC;
        SeparatorHarPeled sepHarPel;

        // collection of separators to use
        std::vector<PlanarSeparatorModule*> separators;
		if(selectedAlgorithms & LT)
			separators.push_back( &sepLipTar );
        if(selectedAlgorithms & D)
			separators.push_back( &sepDual );
        if(selectedAlgorithms & LTFC)
			separators.push_back( &sepLTFC );
        if(selectedAlgorithms & DFC)
			separators.push_back( &sepDFC );
        if(selectedAlgorithms & HP)
			separators.push_back( &sepHarPel );

        // walk over all files/directories in instance directory
        using rec_dir_it = std::filesystem::recursive_directory_iterator;
        for (const auto &dirEntry : rec_dir_it(instance_dir)) {

            if (!dirEntry.is_directory()) { // just skip directories

                std::string path = dirEntry.path().string();

                if (isGraphFile(path)) {

                    std::cout << "Working on " << extractFileName(path) << std::endl;

                    for(const auto sep : separators) {
                        apply(path, *sep);
                    }

                }
            }
        }
        std::cout << "Experiments ran successfully!" << std::endl;
    }

private:

    std::string res_file;
    std::string instance_dir;
    std::ofstream file;
    int limit;
    bool test; // whether to test results or not
    int attempts;
	short selectedAlgorithms;
	bool postProcessing; // whether to apply postprocessing

    PropertyRecorder recorder;


	/**
	 * Applies the separator to the instance at path.
	 *
	 * @param path the instance-path
	 * @param sep the separator to be used
	 */
    void apply(const std::string &path, PlanarSeparatorModule &sep) {


        PropertyRecorder::Properties prop = recorder.getProperties(path);

        Graph G;
        readGraph(G, path);

		// ensure that conditions hold
		makeSimpleUndirected(G);
		planarEmbedPlanarGraph(G);

        if(G.numberOfNodes() <= limit) {

            std::cout << "\t" << "with " << sep.getName() << std::endl;

			if(attempts <= 0) {
				for (node no: G.nodes) {
					// solve the instance with sep and all combinations of postprocessors
					setSeed(42);
					sep.setStartIndex(no->index());
					solve(sep, G, prop);
				}
			} else {
				sep.setStartIndex(-1);
				for(int i = 0; i < attempts; i++) {
					setSeed(i);
					solve(sep, G, prop);
				}
			}
        }

    }


	/**
	 * Solves the given graph with the given separator (basically the core of apply).
	 * Each graph is solved <attempts> many times with the pure separator, with each postprocessor and with all
	 * possible combinations of postprocessors.
	 *
	 * @param sep the separator
	 * @param G the graph
	 * @param prop properties of the graph
	 */
    void solve(PlanarSeparatorModule &sep, const Graph &G, const PropertyRecorder::Properties &prop) {

        List<node> separator;
        List<node> first;
        List<node> second;

        auto start = std::chrono::high_resolution_clock::now();
        sep.separate(G, separator, first, second);
        auto end = std::chrono::high_resolution_clock::now();

        // if test-flag is set, verify that the instance was solved correctly
        if(test) {
            assert(testSeparator(G, separator, first, second, sep.getMaxSeparatorSize(G.numberOfNodes())));
        }

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // create a pure result
        Result res(sep.getName(), prop, G.numberOfNodes(), G.numberOfEdges(), duration.count(), separator.size(), first.size(), second.size(), sep.getExitPoint());
        writeResults(res);

		if(postProcessing) {
			applyPostProcessors(G, sep.getName(), prop, separator, first, second);
		}
    }


	/**
	 * Applies all postprocessor and all possible combinations of postprocessors to a given solution.
	 * This is done separately instead of adding the postprocessors to the separators directly, because that would mean
	 * re-solving the instance for all combination of postprocessors.
	 * The solution is passed by value, not by reference, on purpose b
	 *
	 * @param G the graph
	 * @param name the name of the algorithm
	 * @param prop the properties of the graph
	 * @param separator the list of separator nodes
	 * @param first the first half of the graph
	 * @param second the second half of the graph
	 */
    void applyPostProcessors(const Graph &G, const std::string &name, const PropertyRecorder::Properties &prop, List<node> separator, List<node> first, List<node> second) {

        // currently, 2 post-processors
        NodeExpulsor expulsor;
        DMDecomposer decomposer;

        std::vector<Postprocessor*> postProcessors;
        postProcessors.push_back(nullptr);
        postProcessors.push_back( &expulsor );
        postProcessors.push_back( &decomposer );

        // 4. solve with all permutations of postProcessors
        do {
			if(postProcessors.front() == nullptr) continue;

			List<node> separatorCopy = separator;
			List<node> firstCopy = first;
			List<node> secondCopy = second;

            std::string postName = "";
            auto start = std::chrono::high_resolution_clock::now();
            for(const auto post : postProcessors) {
                if(post == nullptr) break;
                post->apply(G, separatorCopy, firstCopy, secondCopy);
                postName += "_" + post->getName();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            Result res(name+postName, prop, G.numberOfNodes(), G.numberOfEdges(), duration.count(), separatorCopy.size(), firstCopy.size(), secondCopy.size(), "post");
            writeResults(res);

        } while (std::next_permutation(postProcessors.begin(), postProcessors.end()));
    }


	/**
	 * Writes a result into the csv file.
	 *
	 * @param res the result-object
	 */
    void writeResults(const Result &res) {
        file.open(res_file, std::ios_base::app);
        file << res.to_csv();
        file.close();
    }

};

/**
 * Entry point for experiments.
 * Parses command line arguments and starts experiments as specified.
 *
 * === Command Line Arguments ===
 *      -r (results) = path to csv-file that will hold results
 *      -i (instances) = path to directory that contains instances (.gml, .stp)
 *      -p (properties) = path to xml-file as generated by record_properties.cpp that holds instance properties
 *      -l (limit) = size limit for instances in nodes, larger instances are skipped
 *      -t (test) = whether the generated results should be tested for correctness
 *      -a (attempts) = how many times to solve each instance with each algorithm
 *      -A (Algorithm) = which algorithm should be used, default is all
 *      -P (postprocessing) = whether to apply postprocessing or not
 * ==============================
 *
 * === Version ===
 * 		v1.0 = first version of result-file (no recorded details)
 * ===============
 *
 */
int main(int argc, char **argv) {

	std::string version = "_v1.0";

    /* default values for arguments */
    std::string res_file = "../results/data_" + currentTime() + version + ".csv"; // location to write results to
    std::string instance_path = "../instances/";                        // instance location
    std::string property_file = "../instances/properties.xml";          // where to look for properties
    int attempts = 20;
    int size_limit = 1000000;                                           // size limit (in nodes) up to which instances are attempted
    bool test_results = false;                                          // whether to test results to confirm correctness
	short algorithm = all;
	bool postprocessing = false;

    /* command line argument parsing */
    int opt;
    while ((opt = getopt(argc, argv, "r:i:p:l:a:A:tP")) != -1) { // : means arg takes a value
        switch (opt) {
            case 'r':
                res_file = optarg;
                break;
            case 'i':
                instance_path = optarg;
                break;
            case 'p':
                property_file = optarg;
                break;
            case 'l':
                size_limit = std::stoi(optarg);
                break;
            case 'a':
                attempts = std::stoi(optarg);
				break;
            case 't':
                test_results = true;
                break;
			case 'P':
				postprocessing = true;
				break;
			case 'A': {
				std::string names = optarg;
				algorithm = 0;
				if (names.find("LipTar") != std::string::npos) algorithm |= LT;
				if (names.find("LTFC")   != std::string::npos) algorithm |= LTFC;
				if (names.find("Dual")   != std::string::npos) algorithm |= D;
				if (names.find("DFC")    != std::string::npos) algorithm |= DFC;
				if (names.find("HP")     != std::string::npos) algorithm |= HP;
				break;
			}
            case '?':
				break;
            default:
                std::cout << "Could not parse command line arguments!" << std::endl;
                return 1;
        }
    }

	std::cout << all << std::endl;

    std::cout << "Running experiment with settings: \n"
		<< "Algorithm: 		 " << std::bitset<8*sizeof(algorithm)>(algorithm) << "\n"
        << "instance path:   " << instance_path << "\n"
        << "result file:     " << res_file << "\n"
        << "property file:   " << property_file << "\n"
        << "size limit:      " << size_limit << "\n"
        << "attempts:        " << attempts << "\n"
        << "testing results: " << (test_results ? "yes" : "no") << "\n"
		<< "postprocessing:  " << (postprocessing ? "yes" : "no") << "\n"
        << std::endl;


    /* experiments */
    setSeed(42);
    Experiment exp(res_file, instance_path, property_file, size_limit, test_results, attempts, algorithm, postprocessing);
    exp.run();

    return 0;
}

