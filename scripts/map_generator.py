import overpy
import os


def write_chaco(filename, nodes, edges, graph_dict, corr_func):
    with open(filename+".chaco", 'w') as file:
        file.write(str(nodes) + " " + str(edges))
        for key in graph_dict:
            file.write("\n" + " ".join(str(corr_func(v)) for v in graph_dict[key]))


def query_osm(path):

    if not os.path.exists(path):
        os.makedirs(path)

    cities = ["Osnabrück"]

    api = overpy.Overpass()

    for city in cities:

        print("Working on city:", city)
        real_query = "area[\"name\"=\""+city+"\"]->.b; way(area.b);(._;>;);out skel;"
        result = api.query(real_query)

        id_max = 1
        edges = 0
        osm_to_id = {}
        neighbours = {}

        # we only care about nodes on ways
        for way in result.ways:

            last = None
            for node in way.nodes:

                # create new number and empty neighbour list for each node
                if node.id not in osm_to_id:
                    osm_to_id[node.id] = id_max
                    neighbours[id_max] = []
                    id_max += 1

                # if this node was not the first on the path, connect it to its neighbour and vice versa
                if last is not None:
                    if not osm_to_id[node.id] in neighbours[last]:  # unless they already saw each other
                        neighbours[last].append(osm_to_id[node.id])
                        edges += 1
                    if last not in neighbours[osm_to_id[node.id]]:
                        neighbours[osm_to_id[node.id]].append(last)
                        edges += 1

                last = osm_to_id[node.id]  # this node is the new last

        # removing chains
        print("Postprocessing", city)
        key_list = list(neighbours.keys())

        for key in key_list:

            while key in neighbours[key]:  # remove self-loops
                neighbours[key].remove(key)

            if len(neighbours[key]) == 0:  # remove isolated nodes
                neighbours.pop(key)

            if key in neighbours.keys() and len(neighbours[key]) == 2:  # bridge chain-nodes

                a = neighbours[key][0]
                b = neighbours[key][1]

                neighbours[a].append(b)
                neighbours[a].remove(key)

                neighbours[b].append(a)
                neighbours[b].remove(key)

                neighbours.pop(key)

        # at this point, the graph rep in the dict is topologically correct, but the indices are ugly
        correction_dict = {}
        correction_idx = 1
        edge_count = 0

        for key in neighbours.keys():
            correction_dict[key] = correction_idx
            correction_idx += 1
            edge_count += len(neighbours[key])

        write_chaco(os.path.join(path, city), correction_idx-1, edge_count, neighbours, lambda x: correction_dict[x])

        # write_chaco(os.path.join(path, city+"test"), correction_idx-1, edge_count, neighbours, lambda x: x)


if __name__ == "__main__":
    path = "../instances/city"
    query_osm(path)
