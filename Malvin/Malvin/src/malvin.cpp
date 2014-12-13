/*
 * Bus stops - a preliminary study for development of Robocar City Emulator
 * Copyright (C) 2014 Norbert Bátfai, batfai.norbert@inf.unideb.hu
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <cstddef>
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include <set>
#include <vector>
#include <fstream>
#include <cstdio>

int id = 0;

class Bus {
public:

    Bus(std::string name, int id) {
        this->name = name;
        this->id = id;
    }
    int id;
    std::string name;
    std::vector<std::vector<osmium::Location> > locs;
    std::vector<std::string> busStopNames;
};

class BusHandler : public osmium::handler::Handler {
public:
    osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> locations;
    int stops = 0;
    std::vector<osmium::Location> locs;
    std::vector<Bus> buses;
    std::map<osmium::object_id_type, std::vector<osmium::object_id_type> > ways;

    void relation(osmium::Relation& rel) {
        const char* bus = rel.tags() ["route"];
        if (bus && !strcmp(bus, "bus")) {
            const char* busRef = rel.tags()["ref"];
            if (busRef) {
                buses.push_back(Bus(busRef, id));
                id++;

                osmium::RelationMemberList& rml = rel.members();
                for (osmium::RelationMember& rm : rml) {
                    if (rm.type() == osmium::item_type::way) {
                        std::vector<osmium::Location> tmpVec;
                        for (auto& nd : ways[rm.ref()]) {
                            osmium::Location loc = locations.get(nd);
                            tmpVec.push_back(loc);
                        }
                        buses.at(id - 1).locs.push_back(tmpVec);
                    }
                }
            }
        }
    }

    void way(osmium::Way& way) {
        for (auto& it : way.nodes()) {
            ways[way.id()].push_back(it.ref());
        }
    }

    void listStops() {
        std::ofstream fileLoc("/home/hpba/malvinOut.dat");
        for (auto &element : buses) {
            fileLoc << "NEWBUS\n" << element.name << std::endl;
            for (auto &locVec : element.locs) {
                fileLoc << "NEWWAY\n";
                for(auto &loc : locVec) {
                fileLoc << loc.y() << std::endl << loc.x() << std::endl;
                }
            }
        }
        fileLoc.close();
    }
};

int main(int argc, char* argv[]) {
    if (argc == 2) {
        osmium::io::File infile(argv[1]);
        osmium::io::Reader reader(infile, osmium::osm_entity_bits::all);

        BusHandler bus_handler;
        osmium::handler::NodeLocationsForWays<osmium::index::map::SparseTable < osmium::unsigned_object_id_type, osmium::Location >>
                node_locations(bus_handler.locations);

        osmium::apply(reader, node_locations, bus_handler);
        reader.close();
        bus_handler.listStops();

        google::protobuf::ShutdownProtobufLibrary();
    } else {
        std::cout << "Usage: " << argv[0] << "city.osm" << std::endl;
        std::exit(1);
    }
}
