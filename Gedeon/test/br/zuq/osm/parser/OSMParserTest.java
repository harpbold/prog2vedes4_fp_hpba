package br.zuq.osm.parser;

import br.zuq.osm.parser.model.OSM;
import br.zuq.osm.parser.model.Way;
import com.vividsolutions.jts.geom.Coordinate;
import com.vividsolutions.jts.geom.GeometryFactory;
import com.vividsolutions.jts.geom.LineString;
import com.vividsolutions.jts.geom.MultiLineString;
import org.junit.Test;
import static org.junit.Assert.*;

/**
 *
 * @author zuq
 */
public class OSMParserTest {

    public OSMParserTest() {
    }

    /**
     * Test of parse method, of class OSMParser.
     */
    @Test
    public void testParse() throws Exception {
        OSM osm = OSMParser.parse("test/testmap.osm");

        assertEquals(53, osm.getNodes().size());
        assertEquals(9, osm.getWays().size());
        assertEquals(1, osm.getRelations().size());

        boolean found = false;

        for (Way way : osm.getWays()) {
            if ("Rua Quintino Bocaiúva".equals(way.getName())) {
                found = true;
                assertEquals("R1;R2;R3", way.getAltNames());
                assertEquals("residential", way.getType());
            }
        }

        Coordinate c1 = new Coordinate(-35, -9);
        Coordinate c2 = new Coordinate(-34, -8);
        Coordinate c3 = new Coordinate(-33, -7);

        GeometryFactory fac = new GeometryFactory();
        MultiLineString mls;


        mls = fac.createMultiLineString(new LineString[] {
                    fac.createLineString(new Coordinate[]{c1, c2}),
                    fac.createLineString(new Coordinate[]{c2, c3}) });

        //assertTrue(mls.equals(osm.getRelations().iterator().next().getPolygon()));
        assertTrue(found);
    }
}
