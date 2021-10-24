
#include <utils.h>
#include <ogdf/basic/simple_graph_alg.h>

bool isGraphFile(std::string path) {
    std::string extension = path.substr(path.rfind("."));
    return extension == ".gml" || extension == ".chaco" || extension == ".stp";
}

void readGraph(Graph &G, std::string path) {
	G.clear();
    std::string extension = path.substr(path.rfind("."));

    if(extension == ".gml") {
        GraphIO::read(G, path, GraphIO::readGML);
    } else if(extension == ".chaco") {
        GraphIO::read(G, path, GraphIO::readChaco);
    } else if(extension == ".stp") {
        GraphIO::read(G, path, GraphIO::readSTP);
    } else {
        throw std::invalid_argument("Could not understand graph format");
    }
}

unsigned long getHashCode(std::string path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return std::hash<std::string>()(buffer.str());
}

std::string extractFileName(std::string path) {
    size_t name_start_pos = path.rfind("/")+1;
    size_t name_end_pos = path.rfind(".");
    size_t name_length = path.size() - name_start_pos - (path.size() - name_end_pos);

    return path.substr(name_start_pos, name_length);
}

std::string extractFullFileName(std::string path) {
	size_t name_start_pos = path.rfind("instances/")+10;
	size_t name_end_pos = path.rfind(".");
	size_t name_length = path.size() - name_start_pos - (path.size() - name_end_pos);

	return path.substr(name_start_pos, name_length);
}

std::string currentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::string res(std::ctime(&time));

    res = std::regex_replace(res, std::regex(" "), "_");
    res = std::regex_replace(res, std::regex(":"), "-");
    res = res.substr(0, res.rfind("_")); // year is overkill
    return res;
}

void drawGraph(const Graph& graph, std::string name) {

    GraphAttributes GA(graph, GraphAttributes::all);

    for (node v : graph.nodes) {
        GA.width(v) = GA.height(v) = 20.0;
        GA.label(v) = to_string(v->index());
        GA.shape(v) = Shape::Ellipse;
        GA.fillColor(v) = Color::Name::Aliceblue;
    }

    for(edge e : graph.edges) {
        GA.arrowType(e) = EdgeArrow::None;
    }

    PlanarizationLayout layout;

//    FMMMLayout layout;
//    layout.useHighLevelOptions(true);
//    layout.unitEdgeLength(15.0);
//    layout.newInitialPlacement(true);
//    layout.qualityVersusSpeed(FMMMOptions::QualityVsSpeed::NiceAndIncredibleSpeed);

    layout.call(GA);
    GraphIO::write(GA, name+".svg", GraphIO::drawSVG);
}

// warning: is super expensive
std::pair<int, int> calculateDistances(const Graph &G) {

    NodeArray<NodeArray<int>> distances;
    distances.init(G);
    for(auto& na : distances) {
        na.init(G, 0);
    }
    int edgeCost = 1;
    bfs_SPAP(G, distances, edgeCost);

    int diameter = -1;
    int radius = G.nodes.size() + 1;

    for(node n : G.nodes) {
        int rad = -1;
        for(node m : G.nodes) {
            int d = distances[n][m];
            if(d > diameter) {
                diameter = d;
            }
            if(d > rad) {
                rad = d;
            }
        }
        if(rad < radius) {
            radius = rad;
        }
    }

    return std::make_pair(diameter, radius);
}

Graph getRandomPlanarGraph(int n) {
    Graph graph;
    int edges = randomNumber(n, 3*n-6);
    randomPlanarConnectedGraph(graph, n, edges);

    return graph;
}

bool checkSizeAgainstDiameter(int sepSize, const Graph &G) {

    NodeArray<int> distance;
    distance.init(G);
    int edgeCosts = 1;

    for (node v : G.nodes) {
        bfs_SPSS(v, G, distance, edgeCosts);
        for(node w : G.nodes) {
            if(2*distance[w] + 1 > sepSize) {
                return true;
            }
        }
    }
    return false;
}

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

bool testListSizes(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second, double maxSize) {
    int n = G.numberOfNodes();
    bool sizes = n == sep.size() + first.size() + second.size();
    bool firstSize = first.size() <= 2.0 / 3.0 * n;
    bool secondSize = second.size() <= 2.0 / 3.0 * n;

    bool sepSize;
    if(maxSize > 0) {
        sepSize = sep.size() < maxSize;
    } else {
        sepSize = checkSizeAgainstDiameter(sep.size(), G);
    }

    return sizes && sepSize && firstSize && secondSize;
}

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

bool testSeparator(const Graph &G, const List<node> &sep, const List<node> &first, const List<node> &second, double maxSize) {

    bool listComplete = testListCompleteness(G, sep, first, second);
    bool separatorCorrect = testSeparatorCorrectness(G, sep, first, second);
    bool listSizesOk = testListSizes(G, sep, first, second, maxSize);

    return listSizesOk && listComplete && separatorCorrect;
}

std::pair<int, int> calculateDiameterBounds(const Graph &G) {

    if(!isPlanar(G)) throw std::invalid_argument("Graph has to be planar!");

    if(G.nodes.size() < 20) {
        int diam = calculateDistances(G).second;
        return std::make_pair(diam, diam);
    }

    if(!isConnected(G)) {
        // run this algorithm for every component of the graph, take maxima over all components for both bounds
        NodeArray<int> comps;
        comps.init(G);
        int numComps = connectedComponents(G, comps);

        int maxLower = -1;
        int maxUpper = -1;

        for(int i = 0; i < numComps; ++i) {
            GraphCopy g;
            g.init(G);
            g.clear();
            List<node> nodes;
            for(node n : G.nodes) {
                if(comps[n] == i) {
                    g.newNode(n);
                    nodes.pushBack(n);
                }
            }
            for(node n : nodes) {
                for(adjEntry adj : n->adjEntries) {
                    g.newEdge(g.copy(n), g.copy(adj->twinNode()));
                }
            }

            // If the number of nodes in this component is smaller than the largest currently known separator,
            // there is no way this component can influence the currently known bounds.
            if(g.nodes.size() <= maxLower) {
                continue;
            }

            auto res = calculateDiameterBounds(g);

            maxLower = max(maxLower, res.first);
            maxUpper = max(maxUpper, res.second);
        }

        return std::make_pair(maxLower, maxUpper);
    }


    SeparatorLiptonTarjanFC sep;
    NodeExpulsor post(false);

    List<node> separator;
    List<node> first;
    List<node> second;

    sep.separate(G, separator, first, second);
    post.apply(G, separator, first, second);

    if(separator.empty()) { // can happen if the graph was really small / had huge diameter
        int diam = calculateDistances(G).second;
        return std::make_pair(diam, diam);
    }

    NodeArray<short> assignments;
    assignments.init(G, -1);
    for(node s : separator) {
        assignments[s] = 0;
    }
    for(node f : first) {
        assignments[f] = 1;
    }
    for(node sec : second) {
        assignments[sec] = 2;
    }

    int lowerBound = 0;  // longest known shortest path
    int upperBound = INT_MAX;  // sum of longest and second longest shortest path of each separator-node

    for(node n : separator) {

        int maxDist = 0; // max dist from n to any other node
        int secondMaxDist = 0; // second largest dist from n to any other node

        NodeArray<int> distanceArray;
        distanceArray.init(G, -1);
        bfs_SPSS(n, G,distanceArray, 1);

        for(node x : G.nodes) {

            if(x == n) continue;

            int dist = distanceArray[x];

            lowerBound = max(lowerBound, dist);

            if(dist > maxDist) {
                maxDist = dist;
            } else {
                if(dist > secondMaxDist) {
                    secondMaxDist = dist;
                }
            }
        }
        upperBound = min(upperBound, maxDist + secondMaxDist);
    }

    return std::make_pair(lowerBound, upperBound);
}