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

#include <vector>
#include <fstream>
#include <cstdio>
#include <string>
#include <set>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class Adj {
public:

    Adj() {
    };

    Adj(double dist, std::vector<osmium::object_id_type> nodes) {
        this->dist = dist;
        this->nodes = nodes;
    }

    Adj(double dist) {
		this->dist = dist;
	}
    double dist;
    std::vector<osmium::object_id_type> nodes;
};

class NodeJunction {
public:

    NodeJunction() {
    }

    std::map<osmium::object_id_type, Adj> adjacents;
    int current_neighbours=0;
    double distfromorigin=9999999;
    std::vector<osmium::object_id_type> route;
    bool isvisited=false;
    osmium::object_id_type last;
    osmium::object_id_type self; //need to set!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bool isInit = false;
};

class Routing : public osmium::handler::Handler {
public:
    osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> locations;
    std::map<osmium::object_id_type, osmium::Location> nodes;
    std::set<osmium::object_id_type> junctions;
    std::map<osmium::object_id_type, std::vector<osmium::object_id_type> > ways;
    std::map<osmium::object_id_type, NodeJunction> nj;
    std::vector<osmium::object_id_type> myRoute;

    std::map<osmium::object_id_type, NodeJunction> test;

    void testFn() {
		test[1].adjacents[3] = Adj(14);
		//A.add(C, 14);
		test[1].adjacents[4] = Adj(9);
		//A.add(D, 9);
		test[1].adjacents[5] = Adj(7);
		//A.add(E, 7);

		//B.add(C, 9);
		test[2].adjacents[3] = Adj(9);
		//B.add(F, 6);
		test[2].adjacents[6] = Adj(6);

		//C.add(B, 9);
		test[3].adjacents[2] = Adj(9);
		//C.add(D, 2);
		test[3].adjacents[4] = Adj(2);
		//C.add(A, 14);
		test[3].adjacents[1] = Adj(14);

		//D.add(A, 9);
		test[4].adjacents[1] = Adj(9);
		//D.add(C, 2);
		test[4].adjacents[3] = Adj(2);
		//D.add(E, 10);
		test[4].adjacents[5] = Adj(10);
		//D.add(F, 11);
		test[4].adjacents[6] = Adj(11);

		//E.add(A, 7);
		test[5].adjacents[1] = Adj(7);
		//E.add(D, 10);
		test[5].adjacents[4] = Adj(10);
		//E.add(F, 15);
		test[5].adjacents[6] = Adj(15);

		//F.add(B, 6);
		test[6].adjacents[2] = Adj(6);
		//F.add(D, 11);
		test[6].adjacents[4] = Adj(11);
		//F.add(E, 15);
		test[6].adjacents[5] = Adj(15);

		for(auto& i : test) {
			std::cout << "parent: " << i.first << std::endl;
			for(auto& j : i.second.adjacents) {
				std::cout << j.first << std::endl;
			}
		}
	}

	void initdi(osmium::object_id_type from) {
		nj[from].distfromorigin = 0; //!!!
		//test[1].distfromorigin=0; //???
	}

    //double di(NodeWithNeighbour& start, NodeWithNeighbour& end)
    //int cnt = 0;
    //osmium::object_id_type lastid=1234567;
    double di(osmium::object_id_type start, osmium::object_id_type end)
	{
		//if(nj.find(start) != nj.end())
		//std::cout << cnt << " _c_ " ;
		//std::cout << start << " _s_ " ;
		//std::cout << end << " _e_ ";
		//std::cout << nj[start].distfromorigin << std::endl;
		//cnt++;
		//NodeWithNeighbour* smallest; //melyik node a szomszédosak közül a legközelebbi a jelenlegi(nj[start]) nodehoz?
		osmium::object_id_type smallest;
		double smallest_dist=9999999;
		//for (int i=0; i<nj[start].adjacents.size(); ++i)
		//std::cout<<"Entering first for"<<std::endl;
		for (auto & i : nj[start].adjacents)
		{
			//int co=1;
			if (nj[i.first].isvisited) continue; //if (nj[start].neighbours.at(i)->isvisited) continue;
			//if (nj[start].distfromorigin + nj[start].dist.at(i) < nj[i.first].distfromorigin)
			//std::cout<<nj[start].distfromorigin<<" + ";
			//std::cout<<i.second.dist<<" <? ";
			//std::cout<<nj[i.first].distfromorigin<<std::endl;
			if (nj[start].distfromorigin + i.second.dist < nj[i.first].distfromorigin)
			{
				nj[i.first].distfromorigin=(nj[start].distfromorigin + i.second.dist);
				nj[i.first].route.clear();
				//for (int j=0; j<(int)nj[start].route.size(); ++j)
				//std::cout<<"Entering second for "<<co<<" times / "<<nj[start].route.size()<<std::endl;
				//++co;
				for(auto &j : nj[start].route)
				{
					nj[i.first].route.push_back( j );
				}
				nj[i.first].route.push_back( start );
			}
			//std::cout<<"End second for"<<std::endl;
			//std::cout<<"---"<<nj[i.first].distfromorigin<<" "<<smallest_dist<<"---"<<std::endl;
			if (nj[i.first].distfromorigin <= smallest_dist)
			{
				smallest_dist=nj[i.first].distfromorigin;
				smallest=i.first;
			}
		}
		//std::cout<<"End first for"<<std::endl;
		nj[start].isvisited=true;
		if (start == end)
		{
			osmium::object_id_type prev;
			//for (int j=1; j<(int)nj[start].route.size(); ++j)
			int count=1;
			for(auto &j : nj[start].route)
			{
				
				/*std::cout << "Node  No." << count << ": " << j << std::endl;
				myRoute.push_back(j);
				++count;*/
				
				if (count==1) std::cout << "Node No." << count << ": " << j << " 0"<<std::endl;
				else std::cout << "Node No." << count << ": " << j << " "<< nj[prev].adjacents[j].dist << std::endl;
				myRoute.push_back(j);
				++count;
				prev=j;
			}
			std::cout << "Node No." << count << ": " << start << std::endl<<std::endl;;
			std::cout << "### Distance: " << nj[start].distfromorigin << std::endl;
			std::cout << "### Traveled " << count << " edges" << std::endl;
			return nj[start].distfromorigin;
		}
		/*if (smallest == lastid) 
		{
			std::cout<<"Failure"<<std::endl;
			return 0;
		}
		lastid=smallest;*/
		//di(smallest, end);
		if (smallest_dist!=9999999)  // !!! ->
		{
			//smallest.last=start.self; //need to correct !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			nj[smallest].last=nj[start].self;
			di(smallest, end);
		}
		else if(smallest_dist==9999999)
		{
			di(nj[start].last, end); //need to correct !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		} // !!! <-
	}

	double ditest(osmium::object_id_type start, osmium::object_id_type end)
	{
		//if (start>6) return 0;

		//std::cout << cnt << " " << start << " " << end << " " << test[start].distfromorigin << std::endl;
		//cnt++;
		//NodeWithNeighbour* smallest; //melyik node a szomszédosak közül a legközelebbi a jelenlegi(test[start]) nodehoz?
		osmium::object_id_type smallest;
		double smallest_dist=9999999;
		//for (int i=0; i<test[start].adjacents.size(); ++i)
		for (auto & i : test[start].adjacents)
		{
			if (test[i.first].isvisited) continue; //if (test[start].neighbours.at(i)->isvisited) continue;
			//if (test[start].distfromorigin + test[start].dist.at(i) < test[i.first].distfromorigin)
			if (test[start].distfromorigin + i.second.dist < test[i.first].distfromorigin)
			{
				test[i.first].distfromorigin=(test[start].distfromorigin + i.second.dist);
				test[i.first].route.clear();
				//for (int j=0; j<(int)test[start].route.size(); ++j)
				for(auto &j : test[start].route)
				{
					test[i.first].route.push_back( j );
				}
				test[i.first].route.push_back( start );
			}
			if (test[i.first].distfromorigin <= smallest_dist)
			{
				smallest_dist=test[i.first].distfromorigin;
				smallest=i.first;
			}
		}
		test[start].isvisited=true;
		if (start == end)
		{
			osmium::object_id_type prev;
			//for (int j=1; j<(int)test[start].route.size(); ++j)
			int count=1;
			test[start].route.push_back(start);
			for(auto &j : test[start].route)
			{
				if (count==1) std::cout << "Node  No." << count << ": " << j << "0"<<std::endl;
				else std::cout << "Node  No." << count << ": " << j << " "<< nj[prev].adjacents[j].dist << std::endl;
				++count;
				prev=j;
			}
			//std::cout << "Node Last " << ": " << start << std::endl;
			return test[start].distfromorigin;
		}
		ditest(smallest, end);
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
                if(!nj[current].isInit) {
                    nj[current].self = current;
					nj[current].route.push_back(current);
				}
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
                if(!nj[current].isInit) {
                    nj[current].self = current;
					nj[current].route.push_back(current);
				}
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
        std::ofstream fileLoc("junctions.dat");
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
            std::vector<osmium::object_id_type> currentRoute,
            std::set<osmium::object_id_type> denied) {

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
            denied.insert(A);
            for (auto& nd : nj[A].adjacents) {
                // If we already visited it, then skip it
                if (denied.find(nd.first) != denied.end())
                    continue;

                fromAtoB(nd.first, B, nj[A].adjacents[nd.first].dist + dist, currentRoute, denied);
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
		time_t startTime = time(NULL);
        //double fromLat = 47.5522196, fromLon = 21.6094983;
        //double toLat = 47.5247333, toLon = 21.6416533;
        double fromLat = 47.5247333, fromLon = 21.6416533;
        double toLat = 47.5522196, toLon = 21.6094983;
        if (argc == 6) {
            fromLat = std::stod(argv[2]);
            fromLon = std::stod(argv[3]);
            toLat = std::stod(argv[4]);
            toLon = std::stod(argv[5]);
        } else if (argc != 2) {
            std::cout << "Invalid arguments!\n";
        }
        osmium::io::File infile(argv[1]);
        osmium::io::Reader reader(infile, osmium::osm_entity_bits::all);

        Routing rt;
        osmium::handler::NodeLocationsForWays<osmium::index::map::SparseTable < osmium::unsigned_object_id_type, osmium::Location >>
                node_locations(rt.locations);

        osmium::apply(reader, node_locations, rt);
        reader.close();
        rt.calcJunctions();  //!!!
        rt.calcAdj();  //!!!

        std::cout << std::setprecision(8)  //!!!
                << fromLat << " " << fromLon << "\n"  //!!!
                << toLat << " " << toLon << "\n";  //!!!

        osmium::object_id_type from, to;  //!!!
        from = rt.getNode(fromLat, fromLon);  //!!!
        to = rt.getNode(toLat, toLon);  //!!!

        rt.airDist = rt.getDist(from, to);  //!!!
        rt.limit = rt.airDist * 6;  //!!!
        //220807690;220807695;
        rt.printNode(from);  //!!!
        rt.printNode(to);  //!!!
        
        std::set<osmium::object_id_type> denied;
        std::vector<osmium::object_id_type> empty;
        rt.check();
        //rt.fromAtoB(from, to, 0, empty, denied);
        //rt.testFn();   //???
        rt.initdi(from);   //!!!
        //rt.ditest(1, 2);   //???
        //time_t startTime = time(NULL);
        //std::cout<<std::setprecision(2)<<time(NULL)<<std::endl;
        rt.di(from, to);   //!!!
        std::cout<<std::setprecision(2)<<"### Routing Time: "<<(time(NULL) - startTime)/60.0<<" minute(s)"<<std::endl;
        //std::cout<<std::setprecision(2)<<time(NULL)<<std::endl;
        rt.output();

        google::protobuf::ShutdownProtobufLibrary();
    } else {
        std::cout << "Usage: " << argv[0] << "city.osm" << std::endl;
        std::exit(1);
    }
}
