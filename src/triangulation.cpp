/**
 * This file serves as a workbench to try things out.
 */

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_on_sphere_traits_2.h>
#include <CGAL/Delaunay_triangulation_on_sphere_2.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/function_objects.h>

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include <ogdf/fileformats/GraphIO.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_on_sphere_traits_2<K>  Traits;
typedef CGAL::Delaunay_triangulation_on_sphere_2<Traits>    DToS2;
typedef Traits::Point_3                                     Point_3;


template <typename Gt, typename Tds>
void writeDelaunay(const CGAL::Triangulation_on_sphere_2<Gt, Tds>& dt) {

    typedef CGAL::Triangulation_on_sphere_2<Gt,Tds>             Tr;
    typedef typename Tr::Vertex_handle                    Vertex_handle;
    typedef typename Tr::Vertices_iterator                Vertex_iterator;
    typedef typename Tr::All_faces_iterator               Face_iterator;

    ogdf::Graph G;

    // 1. map each vertex-handle to an ogdf node
    std::unordered_map<Vertex_handle, ogdf::node> index_of_vertex;
    for(Vertex_iterator it = dt.vertices_begin(); it != dt.vertices_end(); ++it) {
        ogdf::node x = G.newNode();
        index_of_vertex[it] = x;
    }

    // 2. for each face, create its three edges
    for(Face_iterator fit = dt.all_faces_begin() ; fit != dt.all_faces_end() ; ++fit) {

        ogdf::node a = index_of_vertex[fit->vertex(0)];
        ogdf::node b = index_of_vertex[fit->vertex(1)];
        ogdf::node c = index_of_vertex[fit->vertex(2)];

        G.newEdge(a,b);
        G.newEdge(b,c);
        G.newEdge(a,c);

    }

    // 3. remove duplicate edges
    ogdf::makeSimple(G);

    ogdf::GraphIO::write(G, "delaunay.gml", ogdf::GraphIO::writeGML);
}

void generateDelaunay(int n) {

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
    writeDelaunay(dtos);
}


int main(int, char**)
{
    generateDelaunay(10);

    return 0;
}