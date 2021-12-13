
#include <iostream>
#include <functional>
#include <chrono>
#include <filesystem>

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/deterministic.h>
#include <ogdf/planarity/PlanarSubgraphBoyerMyrvold.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_on_sphere_traits_2.h>
#include <CGAL/Delaunay_triangulation_on_sphere_2.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/function_objects.h>

#include <utils.h>

namespace fs = std::filesystem;

using namespace ogdf;

/**
 * Generates grid graphs.
 *
 * @param location the target parent directory
 */
void gen_grid(const std::string &location) {
    std::cout << "Generating grid..." << std::endl;

    std::vector<int> sizes = {100};

    for(const auto size : sizes) {
        Graph G;
        gridGraph(G, size, size, false, false);
        GraphIO::write(G, location+"grid_"+ to_string(size)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Generates rectangular graphs.
 *
 * @param location the target parent directory
 */
void gen_rect(const std::string &location) {
    std::cout << "Generating rect..." << std::endl;

    std::vector< std::pair<int, int> > sizes = {std::make_pair(500,20)};

    for(const auto p : sizes) {
        Graph G;
        gridGraph(G, p.first, p.second, false, false);
        GraphIO::write(G, location+"rect_"+ to_string(p.first)+"_"+to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}


// todo support horizontal and vertical wrapping, that would be cool!
/**
 * Core of sixgrid graph generation.
 * Creates a honeycomb with n hexagons per column and m columns in total.
 *
 * @param G the graph
 * @param n number of hexagons per column
 * @param m number of columns (even number)
 */
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

/**
 * Generates a sixgrid graph, i.e. a honeycomb.
 *
 * @param location the target parent directory
 */
void gen_sixgrid(const std::string &location) {
    std::cout << "Generating sixgrid..." << std::endl;

    std::vector< std::pair<int, int> > sizes = {std::make_pair(237, 20) };

    for(const auto p : sizes) {
        Graph G;
        honeycombGraph(G, p.first, p.second);
        GraphIO::write(G, location+"sixgrid_"+ to_string(p.first)+"_"+to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Core of triangle graph generation.
 *
 * @param G the graph
 * @param base the size of the base of the triangle
 */
void triangleGraph(Graph& G, int base) {

//    int base = pow(2.0, (double) iterations) + 1; // iterations used to be the input
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

/**
 * Generates a triangle graph.
 *
 * @param location the target parent directory
 */
void gen_triangular(const std::string &location) {
    std::cout << "Generating triangular..." << std::endl;

    std::vector< int > sizes = { 100 }; // describes number of iterations in tree-gen algorithm

    for(const auto size : sizes) {
        Graph G;
        triangleGraph(G, size);
        GraphIO::write(G, location+"triangular_"+to_string(size)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Core of globe generation.
 * Creates a globe graph with a node each crossing of a meridian and a latitude and a node at each pole.
 *
 * @param G the graph
 * @param meridians the number of meridians
 * @param latitudes the number of latitudes
 */
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

/**
 * Generates a globe graph.
 *
 * @param location the target parent directory
 */
void gen_globe(const std::string &location) {
    std::cout << "Generating globe..." << std::endl;

	// number of nodes will be t1 * t2 * 2 + 2
    std::vector< std::pair<int, int> > sizes = { std::make_pair(25, 25), // 1.200
												 std::make_pair(50, 50), // 5.000
												 std::make_pair(75, 75), // 11.000
                                                 std::make_pair(100, 100), // 20.000
												 std::make_pair(150, 150)}; // 45.000

    for(const auto p : sizes) {
        Graph G;
        globeGraph(G, p.first, p.second);
        GraphIO::write(G, location+"globe_"+to_string(p.first)+"_"+ to_string(p.second)+".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Auxiliary method for sphere graph generation.
 * Creates one half of an icosaeder.
 *
 * @param G the graph
 * @param row initially empty list that will contain the seam between the two halves
 */
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

/**
 * Core of sphere graph generation.
 * Creates a sphere graph by creating an icosaeder and splitting every face iteration many times.
 *
 * @param G the graph
 * @param iterations how many times to split each face
 */
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

/**
 * Generates a sphere graph.
 *
 * @param location the target parent directory
 */
void gen_sphere(const std::string &location) {
    std::cout << "Generating t-sphere..." << std::endl;

    std::vector< int > sizes = { 4, 5, 6, 7 };

    for(const int size : sizes) {
        Graph G;
        sphereGraph(G, size);
        GraphIO::write(G, location + "sphere_" + to_string(size) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Auxiliary structure for creating diameter graphs.
 */
struct DiameterModule {
    node left;
    node right;
    node middle;

    node ex;

	/**
	 * Constructor.
	 *
	 * @param G the graph
	 */
    explicit DiameterModule(Graph &G) {
        // create Nodes
        left = G.newNode();
        right = G.newNode();
        middle = G.newNode();

        ex = nullptr; // cannot be set yet

        // connect nodes
        G.newEdge(left, right);
        G.newEdge(right, middle);
        G.newEdge(left, middle);
    }

	/**
	 * Connects this module to the next one.
	 *
	 * @param G the graph
	 * @param other the previous diameter module
	 */
    void connectModule(Graph &G, DiameterModule other) {
        G.newEdge(middle, other.left);
        G.newEdge(middle, other.right);
        G.newEdge(left, other.left);
        G.newEdge(right, other.right);
        G.newEdge(left, other.ex);

        ex = other.right;
    }

};

/**
 * Core of diameter graph generation.
 *
 * @param G the graph
 * @param diameter the diameter (number of modules+1)
 */
void diameterGraph(Graph &G, int diameter) {

    node peak = G.newNode();
    DiameterModule mod(G);
    G.newEdge(peak, mod.middle);
    G.newEdge(peak, mod.right);
    mod.ex = peak;

    DiameterModule& module = mod;

    for(int i = 0; i < diameter - 1; i++) {
        DiameterModule nextMod(G);
        nextMod.connectModule(G, module);
        module = nextMod;
    }

    planarEmbedPlanarGraph(G);

}

/**
 * Generates a diameter graph.
 *
 * @param location the target parent directory
 */
void gen_diameter(const std::string &location) {
    std::cout << "Generating diameter..." << std::endl;

    std::vector< int > diameters = { 3333 };

    for(const int size : diameters) {
        Graph G;
        diameterGraph(G, size);
        GraphIO::write(G, location + "diameter_" + to_string(size) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Generates a maximal ogdf-graph (random graph generated by ogdf-generator).
 *
 * @param location the target parent directory
 */
void gen_ogdf_max(const std::string &location) {
    std::cout << "Generating ogdf..." << std::endl;

    std::vector< std::pair<int, int> > sizes = { std::make_pair(10000, 25000) };

    for(const auto &p : sizes) {
        Graph G;
        setSeed(p.first);
        randomPlanarConnectedGraph(G, p.first, p.second);
        assert(isSimple(G));
        GraphIO::write(G, location + "ogdf_" + to_string(p.first) + "_" + to_string(p.second) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;

        triangulate(G);
        GraphIO::write(G, location + "ogdf-max_" + to_string(p.first) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}

/**
 * Core of twin graph generation.
 * Duplicates a graph and connects the two copies via connectorSize many nodes.
 *
 * @param G the original graph
 * @param connectorSize the number of connector nodes
 */
void createTwin(Graph &G, int connectorSize) {

    assert(isPlanar(G));
    planarEmbedPlanarGraph(G);
    CombinatorialEmbedding E(G);

    face maxFace = E.maximalFace();
    while(maxFace->size() < connectorSize) {
        edge e = maxFace->firstAdj()->theEdge();
        node src = e->source();
        node trg = e->target();
        E.joinFaces(e);
        if(src->adjEntries.empty()) {
            G.delNode(src);
            E.init(G);
        }
        if(trg->adjEntries.empty()) {
            G.delNode(trg);
            E.init(G);
        }
        maxFace = E.maximalFace();
    }
    assert(maxFace->size() >= connectorSize);
    List<node> connectors;
    adjEntry adj = maxFace->firstAdj();
    for(int i = 0; i < connectorSize; ++i) {
        connectors.pushBack(adj->theNode());
        adj = adj->faceCycleSucc();
    }

    Graph newGraph;

    std::map<node, node> old_to_new; // maps original node to its copy

    // copy the graph
    for (node no : G.nodes) {
        node x = newGraph.newNode();
        old_to_new[no] = x;
    }
    for (edge e : G.edges) {
        node src = e->source();
        node trg = e->target();
        newGraph.newEdge(old_to_new[src], old_to_new[trg]);
    }

    // use the copy to create new nodes and edges in the original graph
    std::map<node, node> map;
    for(node no : newGraph.nodes) {
        node x = G.newNode();
        map[no] = x;
    }
    for(edge e : newGraph.edges) {
        G.newEdge(map[e->source()], map[e->target()]);
    }

    // find the counterparts of the connectors
    List<node> otherConnectors;
    for(node no : connectors) {
        node inNew = old_to_new[no]; // node in newGraph
        node inOriginal = map[inNew]; // node in original graph
        otherConnectors.pushBack(inOriginal);
    }

    // create the bridge
    List<node> bridge;
    for(int i = 0; i < connectorSize; i++) {
        node x = G.newNode();
        bridge.pushBack(x);
    }

    // connect connectors to bridge
    ListIterator<node> bIt = bridge.begin(), cIt = connectors.begin();
    bool increaseBridge = true;
    while(bIt != bridge.end()) {
        G.newEdge(*cIt, *bIt);
        if(increaseBridge) {
            bIt++;
        } else {
            cIt++;
        }
        increaseBridge = !increaseBridge;
    }

    // connect otherConnectors to bridge
    bIt = bridge.begin();
    cIt = otherConnectors.begin();
    increaseBridge = true;
    while(bIt != bridge.end()) {
        G.newEdge(*cIt, *bIt);
        if(increaseBridge) {
            bIt++;
        } else {
            cIt++;
        }
        increaseBridge = !increaseBridge;
    }

    assert(isPlanar(G));
}

/**
 * Generates twin graphs, i.e. two identical graphs connected by a small separator bridge.
 * (c-grid, c-ogdf, c-globe)
 *
 * @param location the target parent directory
 */
void gen_twin(const std::string &location) {
    std::cout << "Generating twins..." << std::endl;

    std::cout << "Generating c-grid..." << std::endl;
    Graph c_grid;
    gridGraph(c_grid, 100, 50, false, false);
    createTwin(c_grid, 87);
    GraphIO::write(c_grid, location + "c-grid_" + to_string(c_grid.numberOfNodes()) + ".gml", GraphIO::writeGML);
    std::cout << "Generated c-grid with " << c_grid.numberOfNodes() << " nodes and " << c_grid.numberOfEdges() << " edges." << std::endl;

    std::cout << "Generating c-ogdf..." << std::endl;
    Graph c_ogdf;
    setSeed(42);
    int size = 5000;
    int edges = randomNumber(size, 3*size-6);
    randomPlanarConnectedGraph(c_ogdf, size, edges);
    createTwin(c_ogdf, 5);
    GraphIO::write(c_ogdf, location + "c-ogdf_" + to_string(c_ogdf.numberOfNodes()) + ".gml", GraphIO::writeGML);
    std::cout << "Generated c-ogdf with " << c_ogdf.numberOfNodes() << " nodes and " << c_ogdf.numberOfEdges() << " edges." << std::endl;


    std::cout << "Generating c-globe..." << std::endl;
    Graph c_globe;
    globeGraph(c_globe, 50, 50);
    createTwin(c_globe, 90);
    GraphIO::write(c_globe, location + "c-globe_" + to_string(c_globe.numberOfNodes()) + ".gml", GraphIO::writeGML);
    std::cout << "Generated c-globe with " << c_globe.numberOfNodes() << " nodes and " << c_globe.numberOfEdges() << " edges." << std::endl;

}


/**
 * Walks over a directory full of graph files and planarizes them if necessary.
 * This is used for planarizing the quasi-planar city graphs extracted from openstreetmap.
 *
 * @param resource_path the path to the raw city graphs
 * @param target_path the target path where resulting planar graphs are stored
 */
void planarizeGraphs(const std::string &resource_path, const std::string &target_path) {

    using rec_dir_it = std::filesystem::recursive_directory_iterator;

    for (const auto& dirEntry : rec_dir_it (resource_path)) {

        if (!dirEntry.is_directory()) { // just skip directories
            std::string path = dirEntry.path().string();

            if (isGraphFile(path)) {

                std::string city_name = extractFileName(path);

                Graph G;
                List<edge> delEdges;
                readGraph(G, path);

                if(!isPlanar(G)) {

                    PlanarSubgraphBoyerMyrvold planarizer(1, 0);

                    planarizer.call(G, delEdges);

                    std::cout << "Planarizing " + city_name + " by deleting " << delEdges.size() << " / "
                              << G.edges.size() << " edges." << std::endl;

                    for (const edge &e : delEdges) {
                        G.delEdge(e);
                    }


                } else {
                    std::cout << city_name << " was planar." << std::endl;
                }

                if(!isSimple(G)) {
                    int edgeCountBefore = G.edges.size();
                    makeSimpleUndirected(G);
                    int edgeCountAfter = G.edges.size();
                    std::cout << "Simplifying " << city_name << " by deleting "
                              << (edgeCountBefore - edgeCountAfter) << " / " << edgeCountBefore << " edges." << std::endl;

                } else {
                    std::cout << city_name << " was simple." << std::endl;
                }

                assert(isPlanar(G));
                assert(isSimpleUndirected(G));

                GraphIO::write(G, target_path + city_name + ".gml", GraphIO::writeGML);

            }
        }
    }
}

/**
 * Generates increasingly large planar graphs.
 *
 * @param location the target parent directory
 */
void gen_random(const std::string &location) {
    std::cout << "Generating random..." << std::endl;

    int versions = 3;
    std::vector< int > sizes = { 125, 250, 500, 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000, 512000 };

    for(const int size : sizes) {
        for(int v = 0; v < versions; v++) {
            setSeed(v);
            Graph G;
            int edges = randomNumber(size, 3*size-6);
            randomPlanarConnectedGraph(G, size, edges);
            assert(isSimple(G));
            GraphIO::write(G, location + "random_" + to_string(size) + "_" + to_string(v) + ".gml", GraphIO::writeGML);
            std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
        }
    }
}

/* Generation of Delaunay graphs with CGAL. */

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_on_sphere_traits_2<K>  Traits;
typedef CGAL::Delaunay_triangulation_on_sphere_2<Traits>    DToS2;
typedef Traits::Point_3                                     Point_3;

/**
 * Converts a CGAL-delaunay triangulation of a point cloud on a sphere into an OGDF-graph.
 *
 * @tparam Gt
 * @tparam Tds
 * @param G the graph
 * @param dt the delaunay triangulation
 */
template <typename Gt, typename Tds>
void cgal_to_ogdf(Graph &G, const CGAL::Triangulation_on_sphere_2<Gt, Tds>& dt) {

    typedef CGAL::Triangulation_on_sphere_2<Gt,Tds>             Tr;
    typedef typename Tr::Vertex_handle                    Vertex_handle;
    typedef typename Tr::Vertices_iterator                Vertex_iterator;
    typedef typename Tr::All_faces_iterator               Face_iterator;

    // 1. map each vertex-handle to an ogdf node
    std::unordered_map<Vertex_handle, node> index_of_vertex;
    for(Vertex_iterator it = dt.vertices_begin(); it != dt.vertices_end(); ++it) {
        node x = G.newNode();
        index_of_vertex[it] = x;
    }

    // 2. for each face, create its three edges
    for(Face_iterator fit = dt.all_faces_begin() ; fit != dt.all_faces_end() ; ++fit) {

        node a = index_of_vertex[fit->vertex(0)];
        node b = index_of_vertex[fit->vertex(1)];
        node c = index_of_vertex[fit->vertex(2)];

        G.newEdge(a,b);
        G.newEdge(b,c);
        G.newEdge(a,c);
    }

    // 3. remove duplicate edges
    makeSimpleUndirected(G);
}

/**
 * Core of delaunay graph generation.
 * Creates a point cloud on the unit sphere and delaunay-triangulates it.
 *
 * @param G the graph
 * @param n the number of points to be sampled on the unit sphere
 */
void delaunayGraph(Graph &G, int n) {

    // 0. set random seed
    CGAL::get_default_random() = CGAL::Random(42);

    // 1. generate n points on the unit sphere
    std::vector<Point_3> points;
    points.reserve(n);
    CGAL::Random_points_on_sphere_3< Point_3> generator;
    std::copy_n( generator, n, std::back_inserter(points));

    // 2. create Delaunay Triangulation on Sphere
    Traits traits(CGAL::ORIGIN, 1); // sphere center on origin, with radius 10
    DToS2 dtos(traits);

    // 3. insert points into sphere
    for(const Point_3& pt : points) {
        assert(traits.is_on_sphere(pt));
        dtos.insert(pt);
    }

    assert(dtos.number_of_vertices() == n);
    assert(dtos.dimension() == 2);
    assert(dtos.number_of_ghost_faces() == 0);

    // 4. write triangulation as GML file
    cgal_to_ogdf(G, dtos);
}

/**
 * Generates a delaunay-triangulated graph.
 *
 * @param location the target parent directory
 */
void gen_delaunay(const std::string &location) {
    std::cout << "Generating delaunay..." << std::endl;

    int versions = 3;
    std::vector< int > sizes = { 64000 };

    for(const int size : sizes) {
        Graph G;
        delaunayGraph(G, size);
        GraphIO::write(G, location + "delaunay_" + to_string(size) + ".gml", GraphIO::writeGML);
        std::cout << "nodes: " << G.nodes.size() << " edges: " << G.edges.size() << std::endl;
    }
}


/**
 * Entry point for instance generation.
 *
 * This script generates 12 different types of planar graphs to evaluate the separator algorithms on.
 * See [Holzer et al.] for detailed explanations.
 *
 * Holzer, Prasinos, Schulz, Wagner, Zaroliagis (2005): "Engineering Planar Separator Algorithms". Journal of
 * Experimental Algorithmics.
 *
 * 	grid = a square grid
 * 	rect = a rectangular grid
 * 	sixgrid = a honeycomb-pattern
 * 	triangular = a Sierpinski-like triangle
 * 	globe = sphere approximation that is built like a globe, from meridians and latitudes
 * 	sphere = sphere approximation constructed from an iteratively split isocaeder
 * 	diameter = long graph with very small diameter
 * 	ogdf = random planar graphs
 * 	city = planar graphs derived from contracted road networks (see scripts/map_generator.py)
 * 	random = like ogdf, but growing in size from 125 to 512,000 nodes
 * 	delaunay = delaunay-triangulated graph generated by sampling points on the surface of a unit sphere
 * 	twin = graphs consisting of two identical graphs that are connected via a small separator-bottleneck
 *
 * @return 0 on success
 */
int main() {
    std::cout << "Generating directories..." << std::endl;

    std::string instance_dir = "../instances/";
    std::string resource_dir = "../resources/";

    // 1. generate directories for all types of graphs
    std::vector<std::string> subdirs = {"grid", "rect", "sixgrid","triangular", "globe", "sphere", "diameter",
                                        "ogdf", "city", "random", "delaunay", "twin", "europe" };
    for(const auto& sub : subdirs) {
        fs::create_directories(instance_dir + sub);
    }

    // 2. planarize city graphs
    std::cout << "Planarizing city graphs..." << std::endl;
    planarizeGraphs(resource_dir+"europe/", instance_dir + "europe/");

    // 3. generate other types of graphs
    std::cout << "Generating instances..." << std::endl;

//    gen_grid(instance_dir + "grid/");
//    gen_rect(instance_dir + "rect/");
//    gen_sixgrid(instance_dir + "sixgrid/");
//    gen_triangular(instance_dir + "triangular/");
//    gen_globe(instance_dir + "globe/");
//    gen_sphere(instance_dir + "sphere/");
//    gen_diameter(instance_dir + "diameter/");
//    gen_ogdf_max(instance_dir + "ogdf/");
//    gen_random(instance_dir + "random/");
//    gen_twin(instance_dir + "twin/");
//    gen_delaunay(instance_dir + "delaunay/");
}