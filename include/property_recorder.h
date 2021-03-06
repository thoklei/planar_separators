
#pragma once
#include <utils.h>
#include <map>
#include <iostream>
#include <tinyxml2.h>
#include <filesystem>

/**
 * Records the properties of all instances, i.e. metadata on size, diameters etc. and stores them
 * in a xml-file. This allows us to not re-calculate the properties every time.
 * Changes in the file contents are noticed, because a hash of the file is used as an identifier.
 */
class PropertyRecorder {

public:

	/**
	 * The actual properties that we store. If a graph is too larger, the diameter is estimated.
	 * nodes = the number of nodes in the graph
	 * edges = the number of edges in the graph
	 * diameter = the longest shortest path in the graph
	 * radius = the minimum eccentricity of any vertex
	 * diameter_lB = lower bound of the diameter
	 * diameter_uB = upper bound of the diameter
	 * name = name of the instance (filename)
	 */
    struct Properties {
		int nodes = 0;
		int edges = 0;
        int diameter = -1;
        int radius = -1;
        int diameter_lB = -1;
        int diameter_uB = -1;
        std::string name = "anonymous";
    };

	/**
	 * Constructor.
	 *
	 * @param file the path to the xml file that will contain the properties
	 */
    PropertyRecorder(const std::string &file);

	/**
	 * Applies the PR recursively to a directory of instance files.
	 *
	 * @param directory the target directory (of instances)
	 */
    void apply(std::string directory);

	/**
	 * Dumps all stored properties into an xml-file.
	 */
    void exportData();

	/**
	 * Gives access to the properties of a file.
	 *
	 * @param hash the hash to identify the file with
	 * @return a properties-object with all the data of the file
	 */
    Properties getProperties(const std::string &identifier);

private:

	/* maps file contents-hash to properties */
    std::map<std::string, Properties> propMap;

    tinyxml2::XMLDocument doc; // handle for xml document

    std::string fileName; // name of the xml document

	/**
	 * Reads a single xml-node corresponding to an instance and containing the data into a properties-object that is
	 * then stored in the propMap. (Used when reading in an xml-file)
	 *
	 * @param inst the XML-object
	 */
    void readInstanceProperties(tinyxml2::XMLNode* inst);

	/**
	 * Processes a file: Reads the file, extracts all data, calculates the hash, and stores the properties in the
	 * propMap under that hash. (Used when recording properties initially)
	 *
	 * @param path the path to the instance
	 */
    void processInstance(std::string path);

};
