#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/deterministic.h>
#include <ogdf/planarity/PlanarizationLayout.h>

#include <iostream>
#include <functional>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

using namespace ogdf;

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
//    fmmm.useHighLevelOptions(true);
//    fmmm.unitEdgeLength(15.0);
//    fmmm.newInitialPlacement(true);
//    fmmm.qualityVersusSpeed(FMMMOptions::QualityVsSpeed::NiceAndIncredibleSpeed);

    layout.call(GA);
    GraphIO::write(GA, name+".svg", GraphIO::drawSVG);
}

void gen_grid(std::string location) {
    std::cout << "Generating grid..." << std::endl;

    std::vector<size_t> sizes = {100};

    for(const auto size : sizes) {
        Graph G;
        gridGraph(G, size, size, false, false);
        GraphIO::write(G, location+"grid_"+ to_string(size)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

void gen_rect(std::string location) {
    std::cout << "Generating rect..." << std::endl;

    std::vector< std::pair<size_t, size_t> > sizes = {std::make_pair(500,20)};

    for(const auto p : sizes) {
        Graph G;
        gridGraph(G, p.first, p.second, false, false);
        GraphIO::write(G, location+"rect_"+ to_string(p.first)+"_"+to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

// generates a honeycomb with n hexagons per column and m columns in total
// where m has to be an even number
// todo support horizontal and vertical wrapping, that would be cool!
void honeycombGraph(Graph& G, int n, int m) {

    if(m % 2 != 0) m++; // m has to be even, otherwise wrapping won't work

    SListPure<node> lateralConnections;

    for(int i = 0; i < m+1; ++i) {

        node earlierNode = nullptr;
        SListPure<node> newLateralConnections;

        int length = (i == 0 || i == m) ? 2*n+1 : 2*n+2;

        for(int j = 0; j < length; ++j) {

            node no = G.newNode();
            if(earlierNode != nullptr) {
                G.newEdge(no, earlierNode);
            }
            earlierNode = no;

            if(  (i == 0 && (j % 2) == 0)  ||  ( i != 0 && (j % 2) != (i % 2) )  ) {
                newLateralConnections.pushBack(no);
            } else if(!lateralConnections.empty()) {
                G.newEdge(lateralConnections.popFrontRet(), no);
            }
        }
        lateralConnections = std::move(newLateralConnections);
    }
}

void gen_sixgrid(std::string location) {
    std::cout << "Generating sixgrid..." << std::endl;

    std::vector< std::pair<size_t, size_t> > sizes = {std::make_pair(237, 20) };

    for(const auto p : sizes) {
        Graph G;
        honeycombGraph(G, p.first, p.second);
        GraphIO::write(G, location+"sixgrid_"+ to_string(p.first)+"_"+to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

void triangleGraph(Graph& G, int iterations) {

    int base = pow(2.0, (double) iterations) + 1;
    SListPure<node> lastRow;

    while(base > 0) {
        SListPure<node> newRow;
        node leftNeighbour = nullptr;
        for(int i = 0; i < base; ++i) {
            node no = G.newNode();
            newRow.pushBack(no);

            if(leftNeighbour != nullptr) {
                G.newEdge(leftNeighbour, no);
            }
            leftNeighbour = no;

            if(!lastRow.empty()) {
                G.newEdge(no, lastRow.popFrontRet());
                G.newEdge(no, lastRow.front());
            }

        }
        lastRow = std::move(newRow);
        base--;
    }
}

void gen_triangular(std::string location) {
    std::cout << "Generating triangular..." << std::endl;

    std::vector< int > sizes = { 7 }; // describes number of iterations in tree-gen algorithm

    for(const auto size : sizes) {
        Graph G;
        triangleGraph(G, size);
        GraphIO::write(G, location+"triangular_"+to_string(size)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

void globeGraph(Graph& G, int meridians, int latitudes) {

    // first, create latitude rings of nodes
    List<List<node>> rings;
    for(int lat = 0; lat < latitudes; lat++) {
        List<node> equator;
        node previous = nullptr;
        for(int cross = 0; cross < 2*meridians; ++cross) {
            node no = G.newNode();
            equator.pushBack(no);

            if(previous != nullptr) {
                G.newEdge(previous, no);
            }
            previous = no;
        }
        G.newEdge(equator.front(), equator.back());
        rings.pushBack(equator);
    }

    // create north and south poles
    node north = G.newNode();
    for(node no : rings.front()) {
        G.newEdge(north, no);
    }
    node south = G.newNode();
    for(node no : rings.back()) {
        G.newEdge(south, no);
    }

    // connect nodes from one latitude to the next
    std::vector< ListIterator<node> > iterators;
    for(List<node> &ring : rings) {
        iterators.push_back(ring.begin());
    }

    while(iterators.front() != rings.front().end()) {
        node previous = nullptr;
        for(auto &it : iterators) {
            node no = *it;
            if(previous != nullptr) {
                G.newEdge(previous, no);
            }
            previous = no;
            ++it;
        }
    }

}

void gen_globe(std::string location) {
    std::cout << "Generating globe..." << std::endl;

    std::vector< std::pair<int, int> > sizes = { std::make_pair(50, 100) };

    for(const auto p : sizes) {
        Graph G;
        globeGraph(G, p.first, p.second);
        GraphIO::write(G, location+"globe_"+to_string(p.first)+"_"+ to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

void constructIcoHalf(Graph &G, List<node> &row) {
    node previous = nullptr;
    for(int i = 0; i < 5; i++) {
        node no = G.newNode();
        row.pushBack(no);
        if(previous != nullptr) {
            G.newEdge(no, previous);
        }
        previous = no;
    }
    G.newEdge(row.front(), row.back());

    node north = G.newNode();
    for(node no : row) {
        G.newEdge(north, no);
    }
}

void sphereGraph(Graph &G, int iterations) {

    // 1. construct icosaeder by first constructing two halves and then connecting them

    List<node> topRow;
    constructIcoHalf(G, topRow);

    List<node> bottomRow;
    constructIcoHalf(G, bottomRow);

    auto topIt = topRow.begin();
    auto bottomIt = bottomRow.begin();

    while(bottomIt != bottomRow.end()) {
        G.newEdge(*topIt, *bottomIt);
        topIt = ++topIt == topRow.end() ? topRow.begin() : topIt;
        G.newEdge(*bottomIt, *topIt);
        bottomIt++;
    }


    // 2. iterate over faces and split them 'iteration' many times
    for(int iter = 0; iter < iterations; iter++) {

        planarEmbedPlanarGraph(G);
        CombinatorialEmbedding E(G);

        // copy edges - we can't loop over edges directly because container is changed
        List<edge> edges;
        for (edge &e : G.edges) { edges.pushBack(e); }

        std::map<face, List<node> > faceMap;
        // for each edge, split it
        for (edge &e: edges) {
            List<face> faces;
            faces.pushBack(E.rightFace(e->adjSource()));
            faces.pushBack(E.leftFace(e->adjSource()));

            edge newEdge = E.split(e);
            node newNode = newEdge->source();

            for(face &f : faces) {
                if (faceMap.find(f) == faceMap.end()) {
                    faceMap[f] = {newNode};
                } else {
                    faceMap[f].pushBack(newNode);
                }
            }
        }

        for(const auto &item : faceMap) {
            assert(item.second.size() == 3);
            node a = *item.second.get(0);
            node b = *item.second.get(1);
            node c = *item.second.get(2);

            G.newEdge(a, b);
            G.newEdge(a, c);
            G.newEdge(b, c);
        }

    }

}

void gen_sphere(std::string location) {
    std::cout << "Generating t-sphere..." << std::endl;

    std::vector< int > sizes = { 5 };

    for(const int size : sizes) {
        Graph G;
        sphereGraph(G, size);
        GraphIO::write(G, location + "sphere_" + to_string(size) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

struct DiameterModule {
    node left;
    node right;
    node middle;

    node ex;

    DiameterModule(Graph &G) {
        // create Nodes
        left = G.newNode();
        right = G.newNode();
        middle = G.newNode();

        // connect nodes
        G.newEdge(left, right);
        G.newEdge(right, middle);
        G.newEdge(left, middle);
    }

    void connectModule(Graph &G, DiameterModule other) {
        G.newEdge(middle, other.left);
        G.newEdge(middle, other.right);
        G.newEdge(left, other.left);
        G.newEdge(right, other.right);
        G.newEdge(left, other.ex);

        ex = other.right;
    }

};

// see figure 1 in Holzer et al, 2005
void diameterGraph(Graph &G, int diameter) {

    node peak = G.newNode();
    DiameterModule mod(G);
    G.newEdge(peak, mod.middle);
    G.newEdge(peak, mod.right);
    mod.ex = peak;

    for(int i = 0; i < diameter - 1; i++) {
        DiameterModule nextMod(G);
        nextMod.connectModule(G, mod);
        mod = nextMod;
    }

    planarEmbedPlanarGraph(G);
    triangulate(G);

}

void gen_diameter(std::string location) {
    std::cout << "Generating diameter..." << std::endl;

    std::vector< int > diameters = { 3333 };

    for(const int size : diameters) {
        Graph G;
        diameterGraph(G, size);
        GraphIO::write(G, location + "diameter_" + to_string(size) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

void gen_ogdf_max(std::string location) {
    std::cout << "Generating ogdf..." << std::endl;

    std::vector< std::pair<int, int> > sizes = { std::make_pair(10000, 25000) };

    for(const auto &p : sizes) {
        Graph G;
        randomPlanarConnectedGraph(G, p.first, p.second);
        assert(isSimple(G));
        GraphIO::write(G, location + "ogdf_" + to_string(p.first) + "_" + to_string(p.second) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;

        triangulate(G);
        GraphIO::write(G, location + "ogdf-max_" + to_string(p.first) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

// copies a graph twice and connects the copies via the connectors
void twinGraph(Graph &newGraph, Graph &G, List<node> &connectors) {

    List<node> newConnectors;

    for(int i = 0; i < 2; ++i) {
        std::map<node, node> map;
        for (node no : G.nodes) {
            node newNo = newGraph.newNode();
            map[no] = newNo;
        }

        for (edge e : G.edges) {
            newGraph.newEdge(map[e->source()], map[e->target()]);
        }

        for(node no : connectors) {
            newConnectors.pushBack(map[no]);
        }
    }

    std::cout << "Size: " << newGraph.nodes.size() << std::endl;
    for(node x : newGraph.nodes) {
        std::cout << x << std::endl;
    }

    SListPure<node> sep;
    for(int i = 0; i < connectors.size()-1; ++i) {
        sep.pushBack(newGraph.newNode());
    }

    auto it = newConnectors.begin();
    for(node sepNode : sep) {
        newGraph.newEdge(*it, sepNode);
        it++;
        newGraph.newEdge(*it, sepNode);
    }
    it++;
    for(node sepNode : sep) {
        newGraph.newEdge(*it, sepNode);
        it++;
        newGraph.newEdge(*it, sepNode);
    }

    assert(newGraph.nodes.size() == 2 * G.nodes.size() + connectors.size() - 1);

}

void gen_twin(std::string location) {
    std::cout << "Generating twin..." << std::endl;

    Graph G;
    gridGraph(G, 10, 10, false, false);
    assert(isPlanar(G));
    planarEmbedPlanarGraph(G);
    CombinatorialEmbedding E(G);

    face ext = E.externalFace();

    List<node> connectors;
    adjEntry adj = ext->firstAdj();
    std::cout << adj << std::endl;
    for(int i = 0; i < ext->size() / 2; ++i) {
        connectors.pushBack(adj->theNode());
        adj = adj->faceCycleSucc();
    }

    std::cout << connectors.size() << std::endl;

    Graph newGraph;
    twinGraph(newGraph, G, connectors);

    GraphIO::write(newGraph, location + "c_grid.gml", GraphIO::writeGML);
    std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;

}

int main() {
    std::cout << "Generating directories..." << std::endl;
    std::string path = "../instances/";

    std::vector<std::string> subdirs = {"grid", "rect", "sixgrid", "triangular", "globe", "sphere", "diameter", "ogdf" };
    for(const auto sub : subdirs) {
        fs::create_directories(path+sub);
    }

    std::cout << "Generating Instances..." << std::endl;

    gen_grid(path+"grid/");
    gen_rect(path+"rect/");
    gen_sixgrid(path+"sixgrid/");
    gen_triangular(path+"triangular/");
    gen_globe(path+"globe/");
    gen_sphere(path+"sphere/");
    gen_diameter(path+"diameter/");
    gen_ogdf_max(path+"ogdf/");
    //gen_twin(path+"twin/");
}