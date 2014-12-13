#include <iostream>
#include <cstddef>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/geom/haversine.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/osm.hpp>

#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>

#include <boost/property_map/property_map.hpp>

#include <vector>
#include <fstream>
#include <cstdio>
#include <string>
#include <set>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <utility>
#include <sstream>

class Adj {
public:

    Adj() {
    };

    Adj(double dist, std::vector<osmium::object_id_type> nodes) {
        this->dist = dist;
        this->nodes = nodes;
    }
    double dist;
    std::vector<osmium::object_id_type> nodes;
};

class NodeJunction {
public:

    NodeJunction() {
    }

    std::map<osmium::object_id_type, Adj> adjacents;
};

class Routing : public osmium::handler::Handler {
public:
    osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> locations;
    std::map<osmium::object_id_type, osmium::Location> nodes;
    std::map<osmium::object_id_type, std::vector<osmium::object_id_type> > ways;
    std::map<osmium::object_id_type, NodeJunction> nj;
    std::vector<osmium::object_id_type> myRoute;
    std::set<osmium::object_id_type> junctions, added;

    void boostPath(osmium::object_id_type from, osmium::object_id_type to) {
        typedef float Weight;
        typedef boost::property<boost::edge_weight_t, Weight> WeightProperty;
        typedef boost::property<boost::vertex_name_t, std::string> NameProperty;

        typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS,
                NameProperty, WeightProperty > Graph;

        typedef boost::graph_traits < Graph >::vertex_descriptor Vertex;

        typedef boost::property_map < Graph, boost::vertex_index_t >::type IndexMap;
        typedef boost::property_map < Graph, boost::vertex_name_t >::type NameMap;

        typedef boost::iterator_property_map < Vertex*, IndexMap, Vertex, Vertex& > PredecessorMap;
        typedef boost::iterator_property_map < Weight*, IndexMap, Weight, Weight& > DistanceMap;
        std::vector<std::pair<int, int> > csucsok;
        std::vector<double> suly;
        std::vector<std::pair<osmium::object_id_type, osmium::object_id_type> > edges;

        Graph g;
        std::vector<Vertex> ve;
        std::map<osmium::object_id_type, Vertex> vMap;
        int acti, acto;
        int iFrom, iTo;

        // Fill up graph
        for (auto& i : nj) {

            //vMap[i.first] = boost::add_vertex(std::to_string(i.first), g);


            for (auto& j : i.second.adjacents) {
                if (added.find(j.first) != added.end())
                    continue;

                edges.push_back(std::make_pair(i.first, j.first));
                //vMap[j.first] = boost::add_vertex(std::to_string(j.first), g);
                //boost::add_edge(vMap[i.first], vMap[j.first], 12, g);
            }
            added.insert(i.first);
        }

        std::set<osmium::object_id_type> vAdded;
        for (auto& i : edges) {
            if (vAdded.find(i.first) == vAdded.end()) {
                vMap[i.first] = boost::add_vertex(std::to_string(i.first), g);
                vAdded.insert(i.first);
            }
            if (vAdded.find(i.second) == vAdded.end()) {
                vMap[i.second] = boost::add_vertex(std::to_string(i.second), g);
                vAdded.insert(i.second);
            }
        }

        for (auto& i : edges) {
            boost::add_edge(vMap[i.first], vMap[i.second], nj[i.first].adjacents[i.second].dist, g);
        }

        std::vector<Vertex> predecessors(boost::num_vertices(g));
        std::vector<Weight> distances(boost::num_vertices(g));

        IndexMap indexMap = boost::get(boost::vertex_index, g);
        PredecessorMap predecessorMap(&predecessors[0], indexMap);
        DistanceMap distanceMap(&distances[0], indexMap);

        std::cout << "from: " << from << std::endl;
        boost::dijkstra_shortest_paths(g, vMap[from], boost::distance_map(distanceMap).predecessor_map(predecessorMap));

        std::cout << "distances and parents:" << std::endl;
        NameMap nameMap = boost::get(boost::vertex_name, g);

        int i = 0;
        Vertex vf = vMap[from];

        BGL_FORALL_VERTICES(v, g, Graph) {
            std::cout << "dist(" << nameMap[vf] << ", " << nameMap[v] << ") = " << distanceMap[v] << ", ";
            std::cout << "pre(" << nameMap[v] << ") = " << nameMap[predecessorMap[v]] << std::endl;
        }

        // Extract a shortest path
        std::cout << std::endl;
        typedef std::vector<Graph::edge_descriptor> PathType;
        PathType path;

        Vertex v = vMap[to]; // We want to start at the destination and work our way back to the source
        for (Vertex u = predecessorMap[v]; u != v; v = u, u = predecessorMap[v]) {
            std::pair < Graph::edge_descriptor, bool> edgePair = boost::edge(u, v, g);
            Graph::edge_descriptor edge = edgePair.first;
            path.push_back(edge);
        }

        std::stringstream ss;
        osmium::object_id_type intVar;
        std::cout << "Shortest path from v0 to v5:" << std::endl;
        for (PathType::reverse_iterator pathIterator = path.rbegin(); pathIterator != path.rend(); ++pathIterator) {
            std::cout << nameMap[boost::source(*pathIterator, g)] << " -> "
                    << nameMap[boost::target(*pathIterator, g)]
                    << " = " << boost::get(boost::edge_weight, g, *pathIterator)
                    << std::endl;

            ss.clear();
            ss << nameMap[boost::source(*pathIterator, g)];
            ss >> intVar;
            myRoute.push_back(intVar);
        }
        std::cout << "\nDistance: " << distanceMap[vMap[to]] << std::endl;
    }

    void node(osmium::Node& nd) {
        nodes[nd.id()] = locations.get(nd.id());
    }

    void way(osmium::Way& way) {
        const char* isNormal = way.tags()["highway"];
        if (isNormal) {
            for (auto& nd : way.nodes()) {
                ways[way.id()].push_back(nd.ref());
            }
        }
    }

    double getDist(osmium::object_id_type id1, osmium::object_id_type id2) {
        osmium::Location loc1 = locations.get(id1);
        osmium::Location loc2 = locations.get(id2);
        double dist = osmium::geom::haversine::distance(
                osmium::geom::Coordinates(loc1),
                osmium::geom::Coordinates(loc2));
        return dist;
    }

    double getDist(double lat1, double lon1, osmium::object_id_type id) {
        osmium::Location loc1(lat1, lon1);
        osmium::Location loc2 = locations.get(id);
        double dist = osmium::geom::haversine::distance(
                osmium::geom::Coordinates(loc1),
                osmium::geom::Coordinates(loc2));
        return dist;
    }

    double getDist(double lat1, double lon1, double lat2, double lon2) {
        osmium::Location loc1(lat1, lon1);
        osmium::Location loc2(lat2, lon2);
        double dist = osmium::geom::haversine::distance(
                osmium::geom::Coordinates(loc1),
                osmium::geom::Coordinates(loc2));
        return dist;
    }

    void calcJunctions() {
        for (auto& w1 : ways) {
            for (auto& w2 : ways) {
                if (w1.first == w2.first)
                    continue;
                for (auto& nd1 : w1.second) {
                    for (auto& nd2 : w2.second) {
                        if (nd1 == nd2) {
                            junctions.insert(nd1);
                        }
                    }
                }
            }
        }
        std::cout << "junctions done\n";
    }

    void addRel(osmium::object_id_type current, std::vector<osmium::object_id_type> way) {
        osmium::object_id_type prev;

        std::vector<osmium::object_id_type> tmpVec;

        prev = current;
        for (auto& nd : way) {
            tmpVec.push_back(nd);
            if (nd == current && current != prev) {
                nj[current].adjacents[prev] = Adj(getDist(current, prev), tmpVec);
                break;
            }

            if (junctions.find(nd) != junctions.end()) {
                prev = nd;
                tmpVec.clear();
                tmpVec.push_back(prev);
            }
        }

        tmpVec.clear();
        prev = current; //way.at(way.size() - 1);
        for (auto nd = way.rbegin(); nd != way.rend(); ++nd) {
            tmpVec.push_back(*nd);
            if (*nd == current && current != prev) {
                nj[current].adjacents[prev] = Adj(getDist(current, prev), tmpVec);
                break;
            }

            if (junctions.find(*nd) != junctions.end()) {
                prev = *nd;
                tmpVec.clear();
                tmpVec.push_back(prev);
            }
        }
    }

    double calcDist(std::vector<osmium::object_id_type> nds) {
        double sum = 0;
        bool first = true;
        osmium::object_id_type prev;
        for (auto& it : nds) {
            if (first) {
                first = false;
                prev = it;
            } else {
                sum += getDist(it, prev);
                prev = it;
            }
        }
        return sum;
    }

    void calcAdj() {
        for (auto& w1 : ways) {
            for (auto& w2 : ways) {
                if (w1.first == w2.first) {
                    continue;
                }
                for (auto& nd1 : w1.second) {
                    if (junctions.find(nd1) == junctions.end()) {
                        continue;
                    }
                    for (auto& nd2 : w2.second) {
                        if (nd1 == nd2) {
                            addRel(nd1, w2.second);
                        }
                    }
                }
            }
        }
        std::cout << "mapped nodes\n";
    }

    void output() {
        std::ofstream fileLoc("/home/hpba/junctions.dat");
        osmium::object_id_type prev;
        bool first = true;

        fileLoc << "NEWBUS" << std::endl << "THE ROUTE" << std::endl;
        for (auto& nd : myRoute) {
            if (first) {
                first = false;
                prev = nd;
            } else {
                fileLoc << "NEWWAY\n";
                for (auto& wt : nj[prev].adjacents[nd].nodes) {
                    fileLoc << locations.get(wt).y() << std::endl;
                    fileLoc << locations.get(wt).x() << std::endl;
                }
                prev = nd;
            }
        }
        fileLoc.close();
    }

    void printNode(osmium::object_id_type id) {
        std::cout << id << " has " << nj[id].adjacents.size() << " adjacents!\n";
    }

    double airDist = 0, min = INFINITY, nextDist;
    std::vector<osmium::object_id_type> minVector;
    std::set<osmium::object_id_type> tmpDenied;
    int limit;

    void fromAtoB(osmium::object_id_type A, osmium::object_id_type B,
            double dist,
            std::vector<osmium::object_id_type> currentRoute) {

        currentRoute.push_back(A);
        bool isA = false, isB = true;
        if (A == B && dist < min) {
            min = dist;
            currentRoute.push_back(A);
            myRoute = currentRoute;
            std::cout << dist << " yeah\n";
            isA = true;
        }

        if (dist > limit) {
            isB = false;
        }

        if (!isA && isB) {
            for (auto& nd : nj[A].adjacents) {
                // If we already visited it, then skip it
                if (std::find(currentRoute.begin(), currentRoute.end(), nd.first)
                        != currentRoute.end()) continue;

                fromAtoB(nd.first, B, nj[A].adjacents[nd.first].dist + dist,
                        currentRoute);
            }
        }
    }

    osmium::object_id_type getNode(double lat, double lon) {
        double min = INFINITY, tmp1, tmp2;
        osmium::object_id_type tmp;

        for (auto& nd : junctions) {
            tmp1 = locations.get(nd).lat() + locations.get(nd).lon();
            tmp2 = lat + lon;
            if (std::abs(tmp1 / 2 - tmp2 / 2) < min) {
                min = std::abs(tmp1 / 2 - tmp2 / 2);
                tmp = nd;
            }
        }
        return tmp;
    }

    void check() {
        for (auto& it : nj) {
            for (auto& jt : it.second.adjacents) {
                if (jt.first == it.first) {
                    std::cout << it.first << " = " << jt.first << "\n";
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        double fromLat = 47.5522196, fromLon = 21.6094983;
        double toLat = 47.5247333, toLon = 21.6416533;
        if (argc == 6) {
            fromLat = std::stod(argv[2]);
            fromLon = std::stod(argv[3]);
            toLat = std::stod(argv[4]);
            toLon = std::stod(argv[5]);
        } else if (argc != 2) {
            std::cout << "Ivalid arguments!\n";
        }
        osmium::io::File infile(argv[1]);
        osmium::io::Reader reader(infile, osmium::osm_entity_bits::all);

        Routing rt;
        osmium::handler::NodeLocationsForWays<osmium::index::map::SparseTable < osmium::unsigned_object_id_type, osmium::Location >>
                node_locations(rt.locations);

        osmium::apply(reader, node_locations, rt);
        reader.close();
        rt.calcJunctions();
        rt.calcAdj();

        std::cout << std::setprecision(8)
                << fromLat << " " << fromLon << "\n"
                << toLat << " " << toLon << "\n";

        osmium::object_id_type from, to;
        from = rt.getNode(fromLat, fromLon);
        to = rt.getNode(toLat, toLon);

        rt.airDist = rt.getDist(from, to);
        rt.limit = rt.airDist * 6;
        //220807690;220807695;
        rt.printNode(from);
        rt.printNode(to);
        //std::set<osmium::object_id_type> denied;
        std::vector<osmium::object_id_type> empty;
        rt.check();

        time_t startTime = time(NULL);

        //rt.fromAtoB(from, to, 0, empty);
        rt.boostPath(from, to);

        std::cout << "Routing time: "
                << difftime(startTime, time(NULL)) / 60
                << " minute(s)\n";

        rt.output();

        google::protobuf::ShutdownProtobufLibrary();
    } else {
        std::cout << "Usage: " << argv[0] << "city.osm" << std::endl;
        std::exit(1);
    }
}
