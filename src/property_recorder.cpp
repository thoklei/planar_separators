#include <property_recorder.h>

namespace fs = std::filesystem;

PropertyRecorder::PropertyRecorder(const std::string &file) : fileName{file} {

    // check if xml-property-file already exists
    // if yes, populate dictionary by scanning the file and setting the properties
    fs::path f(file);
    if(fs::exists(f)) {
        doc.LoadFile( file.c_str() );

        tinyxml2::XMLElement* instances = doc.FirstChildElement("instances");
        tinyxml2::XMLNode* it = instances->FirstChild();
        while(it != nullptr ) {
            readInstanceProperties(it);
            it = it->NextSibling();
        }
    } else { // if not, create the file, dict stays empty
        std::ofstream os(file, std::ios_base::out);
        os.close();

        doc.LoadFile( file.c_str() );
    }
}

void PropertyRecorder::readInstanceProperties(tinyxml2::XMLNode* inst) {

    Properties prop;
    std::string identifier = inst->FirstChildElement("identifier")->GetText();

    prop.name = inst->FirstChildElement("name")->GetText();
    prop.diameter = atoi(inst->FirstChildElement("diameter")->GetText());
    prop.diameter_lB = atoi(inst->FirstChildElement("diameter_lB")->GetText());
    prop.diameter_uB = atoi(inst->FirstChildElement("diameter_uB")->GetText());
    prop.radius = atoi(inst->FirstChildElement("radius")->GetText());

    propMap[identifier] = prop;
}

void PropertyRecorder::apply(std::string directory) {
    // for all files
    using rec_dir_it = std::filesystem::recursive_directory_iterator;

    // walk over all files/directories in target directory
    for (const auto &dirEntry : rec_dir_it(directory)) {

        if (!dirEntry.is_directory()) { // just skip directories

            std::string path = dirEntry.path().string();

            if (isGraphFile(path)) {

                processInstance(path);
            }
        }
    }
}

void PropertyRecorder::processInstance(std::string path) {
    std::cout << "Processing " << path << std::endl;

	std::string identifier = path;
    Graph G;
    readGraph(G, path);

    Properties prop;
    if(propMap.find(identifier) == propMap.end()) { // instance is unknown
        propMap[identifier] = prop;
    }

    if(propMap[identifier].diameter == -1 || propMap[identifier].radius == -1) {
        if(G.nodes.size() < 33000) {
            std::pair<int, int> distances = calculateDistances(G);
            int diameter = distances.first;
            int radius = distances.second;
            propMap[identifier].diameter = diameter;
            propMap[identifier].radius = radius;
        } else {
            propMap[identifier].diameter = -1;
            propMap[identifier].radius = -1;
        }
    }

    if(propMap[identifier].diameter_uB == -1) {
        auto bounds = calculateDiameterBounds(G);
        propMap[identifier].diameter_lB = bounds.first;
        propMap[identifier].diameter_uB = bounds.second;
    }

    if(propMap[identifier].name == "anonymous") {
        propMap[identifier].name = extractFullFileName(path);
    }

}

void PropertyRecorder::exportData() {
    doc.Clear();
    tinyxml2::XMLElement* instances = doc.NewElement("instances");
    for(const auto &prop : propMap) {
        tinyxml2::XMLElement* instance = instances->InsertNewChildElement("instance");
        instance->InsertNewChildElement("identifier")->SetText(prop.first.c_str());
        instance->InsertNewChildElement("name")->SetText(prop.second.name.c_str());
        instance->InsertNewChildElement("diameter")->SetText(to_string(prop.second.diameter).c_str());
        instance->InsertNewChildElement("radius")->SetText(to_string(prop.second.radius).c_str());
        instance->InsertNewChildElement("diameter_lB")->SetText(to_string(prop.second.diameter_lB).c_str());
        instance->InsertNewChildElement("diameter_uB")->SetText(to_string(prop.second.diameter_uB).c_str());
    }
    doc.InsertFirstChild(instances);
    doc.SaveFile(fileName.c_str());
}

PropertyRecorder::Properties PropertyRecorder::getProperties(const std::string &identifier) {
    // todo error catching!
    return propMap[identifier];
}
