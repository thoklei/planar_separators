# Contents

This directory contains instances for the separator problem, consisting of some real-world data or common datasets as
well as custom graphs with interesting properties to test the algorithms on.
Many instance types were directly taken from [Holzer et al. 2005].


| Type          |                                                                                     Description                                                                                    |
|:-------------:|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| city          | Road networks of selected cities. Sourced from [openstreetmap].                                                                                                                    |
| delaunay      | Delaunay-triangulations of point clouds on the unit sphere.                                                                                                                        | 
| diameter      | Diameter graphs as described by Holzer et al.  (Graphs with large diameter but small separator)                                                                                    |
| globe         | Globe graphs as described by Holzer et al.  (Nodes on a network of meridians and latitudes, with north- and southpole)                                                             |
| grid          | Grid graphs as described by Holzer et al.                                                                                                                                          |
| ogdf          | Random graphs to replace Holzer's LEDA-graphs.  (Random maximal planar graph and random graph with less than maximum number of edges)                                              |
| rect          | Rectangular graphs as described by Holzer et al.                                                                                                                                   |
| sixgrid       | Honeycomb graphs as described by Holzer et al.                                                                                                                                     |
| sphere        | Sphere approximation graphs generated from splitting an icosaeder as described in Holzer et al.                                                                                    |
| triangular    | Graphs derived from subdividing triangles as described in Holzer et al. (See notes below)                                                                                          |
| twin          | Graphs consisting of two identical sub-graphs connected by a small separator as described in Holzer et al.                                                                         |
| vlsi          | VLSI derived grid graphs taken from the Steinlib-library [Koch et al. 2000]                                                                                                        |


# Notes

## Triangular Graphs
In [Holzer et al. 2005], the construction of triangular graphs is described as a recursive process that splits a 
triangle in a Sierpinski-like fashion. This results in a triangular graph that looks like a pyramid, with n nodes in the 
bottom layer, n-1 nodes in the layer above etc. which results in (n^2 + n) / 2 nodes in total. 
It is perfectly possible to create such a graph with 5050 nodes and 14850 edges, as stated in the paper - just create a
pyramid with a base size of 100. But this has to be done in an iterative fashion, as is done here. The recursive 
algorithm described in the paper "skips" the graph of this size. See code for details.

## Random Graphs
The largest random instances contain 512,000 nodes, resulting in files that exceed the size limit recommended by GitHub.
Therefore, these instances are not included here but should be generated locally instead.


# References
Holzer, Prasinos, Schulz, Wagner, Zaroliagis (2005): "Engineering Planar Separator Algorithms". Journal of Experimental Algorithmics.

Koch, Martin, Voß (2000): "SteinLib: An Updated Library on Steiner Tree Problems in Graphs". ZIP-Report 00-37. See http://steinlib.zib.de/testset.php

<a href="https://www.openstreetmap.org/">OpenStreetMap</a> - Published under <a href="https://opendatacommons.org/licenses/odbl/">ODbL</a>