# Benchmarking Planar Separator Algorithms

# Content
This repository contains supporting code for my Master's Thesis, "Applied Algorithms for the Planar Separator Theorem".
The actual separator algorithms were implemented as part of the <a href="https://ogdf.uos.de">OGDF</a>, but are not yet
contained in the latest release, so the core of this repo only works with the internal, unreleased version of OGDF.

* instance generation: see the readme in /instances/
* instance properties: for all instances, properties like diameter etc. are recorded and stored under a hash
* main experiment: applies all algorithms and postprocessors to all instances

# Dependencies

1. OGDF: download and install as described on <a href="https://ogdf.uos.de">ogdf.uos.de</a> (see disclaimer above)
2. tinyxml: is contained in this repository, no need to install anything
3. CGAL: for parts of the instance generation, CGAL is needed - download from <a href="https://www.cgal.org">cgal.org</a> (or via homebrew)
