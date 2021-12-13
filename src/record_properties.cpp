#include <property_recorder.h>
#include <ogdf/basic/Graph.h>
namespace fs = std::filesystem;

int main() {

    PropertyRecorder propRec("../instances/properties.xml");

    std::string instance_dir = "../instances/delaunay_small/";

    propRec.apply(instance_dir);

    propRec.exportData();

    return 0;
}