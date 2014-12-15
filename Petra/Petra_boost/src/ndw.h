#include <vector>
#include <stdio>

bool debug = false;

class NodeWithNeighbour {
private:
    double distfromorigin = 9999;
    int current_neighbours = 0;
    int id;

public:
    static int counter;
    std::vector<NodeWithNeighbour*> neighbours; //szomszédos node-ok listája
    std::vector<int> route;
    std::vector<double> dist; // ezektől való távolságuk

    bool isvisited = false;

    NodeWithNeighbour() {

    }

    NodeWithNeighbour(int n) {
        this->counter += 1;
        this->setid(counter);
        route.push_back(this->getid());
    }

    void add(NodeWithNeighbour& node_to_add, double dist_from_this) {
        neighbours.push_back(&node_to_add);
        dist.push_back(dist_from_this);
        this->gainneigh();
    }

    void print() { //debug volt a kezdethez
        std::cout << this->getid() << " " << this->getneigh() << std::endl;
        for (int i = 0; i<this->getneigh(); ++i) {
            std::cout << this->neighbours.at(i)->getid() << " " << this->dist.at(i) << std::endl;
        }
        std::cout << std::endl;
    }

    void setdist(double d) {
        this->distfromorigin = d;
    }

    double getdist() {
        return this->distfromorigin;
    }

    void gainneigh() {
        this->current_neighbours += 1;
    }

    int getneigh() {
        return this->current_neighbours;
    }

    void setid(int i) {
        this->id = i;
    }

    int getid() {
        return this->id;
    }

    static double di(NodeWithNeighbour& start, NodeWithNeighbour& end) {
        NodeWithNeighbour* smallest; //melyik node a szomszédosak közül a legközelebbi a jelenlegi(start) nodehoz?
        double smallest_dist = 9999;
        for (int i = 0; i < start.getneigh(); ++i) {
            if (start.neighbours.at(i)->isvisited) continue;
            if (start.getdist() + start.dist.at(i) < start.neighbours.at(i)->getdist()) {
                start.neighbours.at(i)->setdist(start.getdist() + start.dist.at(i));
                start.neighbours.at(i)->route.clear();
                for (int j = 0; j < (int) start.route.size(); ++j) {
                    if (debug) std::cout << "Element " << j << ": " << start.route.at(j) << std::endl;
                    start.neighbours.at(i)->route.push_back(start.route.at(j));
                }
                start.neighbours.at(i)->route.push_back(start.getid());
            }
            if (start.neighbours.at(i)->getdist() <= smallest_dist) {
                if (debug) std::cout << "P1" << std::endl;
                smallest_dist = start.neighbours.at(i)->getdist();
                if (debug) std::cout << "P1.5" << std::endl;
                smallest = start.neighbours.at(i); //itt hal meg elvileg - ha kisebb a távolsága a vizsgálttól mint az eddigi akkor rá mutatok
                if (debug) std::cout << "LID2: " << smallest->getid() << " NUM: " << smallest->getneigh() << std::endl; //létezik az amire mutatok?
                if (debug) std::cout << "P2" << std::endl; /**/
            }
        }
        start.isvisited = true;
        if (start.getid() == end.getid()) {
            for (int j = 1; j < (int) start.route.size(); ++j) {
                std::cout << "Node  No." << j << ": " << start.route.at(j) << std::endl;
            }
            std::cout << "Node Last " << ": " << start.getid() << std::endl;
            return start.getdist();
        }
        di(*smallest, end);
    }
};


