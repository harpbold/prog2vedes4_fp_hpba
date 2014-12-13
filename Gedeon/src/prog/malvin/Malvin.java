package prog.malvin;

import br.zuq.osm.parser.OSMParser;
import br.zuq.osm.parser.model.Member;
import br.zuq.osm.parser.model.OSMNode;
import br.zuq.osm.parser.model.OSM;
import br.zuq.osm.parser.model.Relation;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author hpba
 */
public class Malvin {

    public static class myNode {

        myNode(String lat, String lon) {
            this.lat = lat;
            this.lon = lon;
        }
        String lat, lon;
    }

    public static class myBus {

        String ref;
        ArrayList<String> nodes;

        public myBus(String s) {
            this.nodes = new ArrayList<String>();
            ref = s;
        }
    }

    private static Map nodes;
    private static ArrayList<myBus> buses;
    private static OSM osm;

    public static void main(String[] args) {
        try {
            osm = OSMParser.parse("/home/hpba/Downloads/debrecen.osm");
        } catch (Exception ex) {
            Logger.getLogger(Malvin.class.getName()).log(Level.SEVERE, null, ex);
        }

        nodes = new HashMap<Integer, ArrayList<String>>();
        buses = new ArrayList<myBus>();

        for (final OSMNode node : osm.getNodes()) {
            if (node.id != null && node.lat != null && node.lon != null) {
                nodes.put(node.id, new ArrayList<String>() {
                    {
                        add(node.lat);
                        add(node.lon);
                    }
                });
            }
        }

        for (Relation rel : osm.getRelations()) {
            if (rel.tags.get("route") != null && rel.tags.get("route").equals("bus")) {
                if (rel.tags.get("ref") != null) {
                    String name = rel.tags.get("ref");
                    if (name != null) {
                        buses.add(new myBus(name));
                        for (Member rm : rel.members) {
                            if (rm.role.equals("stop")) {
                                buses.get(buses.size() - 1).nodes.add(rm.ref);
                            }
                        }
                    }
                }
            }
        }

        PrintWriter writer = null;
        try {
            writer = new PrintWriter("/home/hpba/javaMalvin.dat", "UTF-8");
        } catch (FileNotFoundException ex) {
            Logger.getLogger(Malvin.class.getName()).log(Level.SEVERE, null, ex);
        } catch (UnsupportedEncodingException ex) {
            Logger.getLogger(Malvin.class.getName()).log(Level.SEVERE, null, ex);
        }

        for (myBus bus : buses) {
            writer.println(bus.ref);
            for (String node : bus.nodes) {
                ArrayList<String> loc = (ArrayList<String>) nodes.get(node);
                if (loc != null) {
                    writer.println(String.valueOf(loc.get(0)));
                    writer.println(String.valueOf(loc.get(1)));
                }
            }
        }

    }
}
