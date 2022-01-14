#include <property_recorder.h>
#include <ogdf/basic/Graph.h>
namespace fs = std::filesystem;

int main() {

	// pass path to where xml should be located
    PropertyRecorder propRec("../instances/properties.xml");

	// pass path to directory from which to take instances
    std::string instance_dir = "../instances/random";

    propRec.apply(instance_dir); // record properties

    propRec.exportData(); // write to file

    return 0;
}