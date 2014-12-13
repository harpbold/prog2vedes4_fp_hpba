package Bruno_Koppany;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.FileReader;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.Point2D;
import org.jxmapviewer.*;
import org.jxmapviewer.viewer.*;
import org.jxmapviewer.input.*;
import org.jxmapviewer.painter.CompoundPainter;
import org.jxmapviewer.painter.Painter;

public class Map {

    public static class Bus {

        public Bus(String n, int i) {
            name = n;
            id = i;
        }

        public int id, cnt = 0;
        public String name;
        public ArrayList<ArrayList<Point2D.Double>> stops = new ArrayList<ArrayList<Point2D.Double>>();
    }

    private static ArrayList<Bus> buses;
    private static ArrayList<Point2D.Double> al1;
    private static ArrayList<String> strList;
    private static ArrayList<ArrayList<GeoPosition>> track;
    private static List<TileFactory> factories;
    private static Set<MyWaypoint> waypoints;

    private static JFrame frame;
    private static JMenuBar menuBar;
    private static JMenu colorMenu, fileMenu, mapMenu, tileMenu, resizeMenu;
    private static JMenuItem selectRouteColor, routeWidthItem, selectFile;
    private static JRadioButtonMenuItem osmItem, veItem, vesItem, vehItem, resizeOnItem, resizeOffItem;

    private static JPanel mainPanel, controlPanel, actionPanel, cbPanel;
    private static JList busList;
    private static JXMapViewer mapViewer;
    private static JButton selectButton;
    private static List<Painter<JXMapViewer>> painters;
    private static JFileChooser fileDialog;

    private static WaypointPainter<MyWaypoint> waypointPainter;
    private static DefaultListModel lm;
    private static CompoundPainter<JXMapViewer> painter;
    private static JCheckBox cbDrawWay, cbDrawMarker;
    private static JScrollPane scrollPane;
    private static ArrayList<RoutePainter> routePainter;
    private static boolean isDrawWay = true, isDrawMarker = true,
            hasData = false, isResize = true;
    private static Color routeColor = Color.ORANGE;
    private static int id = 0, routeWidth = 5;

    public static <T> void main(String[] args) {
        // Panels
        mainPanel = new JPanel();
        controlPanel = new JPanel();
        actionPanel = new JPanel();
        cbPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout());
        controlPanel.setLayout(new BorderLayout());
        actionPanel.setLayout(new BorderLayout());
        cbPanel.setLayout(new BorderLayout());
        // ArrayLists
        al1 = new ArrayList<Point2D.Double>();
        strList = new ArrayList<String>();
        track = new ArrayList<ArrayList<GeoPosition>>();
        // Checkboxes
        cbDrawWay = new JCheckBox("Way", true);
        cbDrawMarker = new JCheckBox("WayPoints", true);
        cbDrawWay.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                isDrawWay = !isDrawWay;
                resetPainter();
            }
        });
        cbDrawMarker.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                isDrawMarker = !isDrawMarker;
                resetPainter();
            }
        });
        // JList, DefaultListModel and JScrollPane
        lm = new DefaultListModel();
        busList = new JList(lm);
        scrollPane = new JScrollPane(busList);
        scrollPane.setPreferredSize(new Dimension(100, 0));
        // Other
        waypoints = new HashSet<MyWaypoint>();
        mapViewer = new JXMapViewer();
        frame = new JFrame("JXMapviewer2 Debrecen busStops");
        menuBar = new JMenuBar();
        fileDialog = new JFileChooser();

        colorMenu = new JMenu("Route");
        fileMenu = new JMenu("File");

        selectRouteColor = new JMenuItem("Route Color");
        selectRouteColor.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Color color = JColorChooser.showDialog(frame, "Choose route color", Color.yellow);
                if (color != null) {
                    routeColor = color;
                    resetPainter();
                }
            }
        });
        colorMenu.add(selectRouteColor);

        routeWidthItem = new JMenuItem("Route Width");
        routeWidthItem.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String res = JOptionPane.showInputDialog(frame, "Route width:");
                try {
                    routeWidth = Integer.parseInt(res);
                    routeWidth = routeWidth < 3 ? 3 : routeWidth;
                    resetPainter();
                } catch (NumberFormatException ex) {
                }
            }
        });
        colorMenu.add(routeWidthItem);

        selectFile = new JMenuItem("Select Input File");
        selectFile.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                int returnVal = fileDialog.showOpenDialog(frame);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    java.io.File file = fileDialog.getSelectedFile();
                    input(file.getAbsolutePath());
                }
            }
        });
        fileMenu.add(selectFile);

        mapMenu = new JMenu("Map");
        tileMenu = new JMenu("Tileset");
        osmItem = new JRadioButtonMenuItem("OSM TileSet", true);
        osmItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (e.getStateChange() == ItemEvent.SELECTED) {
                    veItem.setSelected(!osmItem.isSelected());
                    vesItem.setSelected(!osmItem.isSelected());
                    vehItem.setSelected(!osmItem.isSelected());
                    changeTiles(0);
                }
            }
        });
        veItem = new JRadioButtonMenuItem("Virtual Earth Map", false);
        veItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (e.getStateChange() == ItemEvent.SELECTED) {
                    osmItem.setSelected(!veItem.isSelected());
                    vesItem.setSelected(!veItem.isSelected());
                    vehItem.setSelected(!veItem.isSelected());
                    changeTiles(1);
                }
            }
        });
        vesItem = new JRadioButtonMenuItem("Virtual Earth Satelit", false);
        vesItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (e.getStateChange() == ItemEvent.SELECTED) {
                    osmItem.setSelected(!vesItem.isSelected());
                    veItem.setSelected(!vesItem.isSelected());
                    vehItem.setSelected(!vesItem.isSelected());
                    changeTiles(2);
                }
            }
        });
        vehItem = new JRadioButtonMenuItem("Virtual Earth Hybrid", false);
        vehItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (e.getStateChange() == ItemEvent.SELECTED) {
                    osmItem.setSelected(!vehItem.isSelected());
                    veItem.setSelected(!vehItem.isSelected());
                    vesItem.setSelected(!vehItem.isSelected());
                    changeTiles(3);
                }
            }
        });

        tileMenu.add(osmItem);
        tileMenu.add(veItem);
        tileMenu.add(vesItem);
        tileMenu.add(vehItem);
        mapMenu.add(tileMenu);

        resizeMenu = new JMenu("Resize To Best Fit");
        resizeOnItem = new JRadioButtonMenuItem("Resize On", true);
        resizeOnItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                resizeOffItem.setSelected(!resizeOnItem.isSelected());
                isResize = true;
            }
        });
        resizeOffItem = new JRadioButtonMenuItem("Resize Off", false);
        resizeOffItem.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                resizeOnItem.setSelected(!resizeOffItem.isSelected());
                isResize = false;
            }
        });
        resizeMenu.add(resizeOnItem);
        resizeMenu.add(resizeOffItem);
        mapMenu.add(resizeMenu);

        menuBar.add(fileMenu);
        menuBar.add(colorMenu);
        menuBar.add(mapMenu);

        frame.setJMenuBar(menuBar);

        MouseInputListener mia = new PanMouseInputListener(mapViewer);
        mapViewer.addMouseListener(mia);
        mapViewer.addMouseMotionListener(mia);
        mapViewer.addMouseListener(new CenterMapListener(mapViewer));
        mapViewer.addMouseWheelListener(new ZoomMouseWheelListenerCursor(
                mapViewer));
        mapViewer.addKeyListener(new PanKeyListener(mapViewer));

        selectButton = new JButton("Select");
        selectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (hasData) {
                    id = busList.getSelectedIndex();
                    load();
                }
            }
        });

        cbPanel.add(cbDrawWay, BorderLayout.PAGE_START);
        cbPanel.add(cbDrawMarker, BorderLayout.PAGE_END);
        actionPanel.add(cbPanel, BorderLayout.LINE_END);
        actionPanel.add(selectButton, BorderLayout.PAGE_START);
        controlPanel.add(scrollPane, BorderLayout.CENTER);
        controlPanel.add(actionPanel, BorderLayout.PAGE_END);
        mainPanel.add(mapViewer, BorderLayout.CENTER);
        mainPanel.add(controlPanel, BorderLayout.LINE_END);

        frame.setSize(500, 200);
        frame.setVisible(true);

        frame.add(mainPanel);
        frame.setSize(800, 600);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setVisible(true);

        // Create a TileFactoryInfo for OpenStreetMap
        TileFactoryInfo osmInfo = new OSMTileFactoryInfo();
        TileFactoryInfo veInfo = new VirtualEarthTileFactoryInfo(VirtualEarthTileFactoryInfo.MAP);
        TileFactoryInfo vesInfo = new VirtualEarthTileFactoryInfo(VirtualEarthTileFactoryInfo.SATELLITE);
        TileFactoryInfo vehInfo = new VirtualEarthTileFactoryInfo(VirtualEarthTileFactoryInfo.HYBRID);

        factories = new ArrayList<TileFactory>();
        factories.add(new DefaultTileFactory(osmInfo));
        factories.add(new DefaultTileFactory(veInfo));
        factories.add(new DefaultTileFactory(vesInfo));
        factories.add(new DefaultTileFactory(vehInfo));

        mapViewer.setTileFactory(factories.get(0));

        waypointPainter = new WaypointPainter<MyWaypoint>();
        waypointPainter.setRenderer(new FancyWaypointRenderer());

        if (args.length > 0) {
            input(args[0]);
        }
    }

    private static void changeTiles(int index) {
        mapViewer.setTileFactory(factories.get(index));
    }

    private static void resetPainter() {
        routePainter = new ArrayList<RoutePainter>();
        for (ArrayList<GeoPosition> it : track) {
            routePainter.add(new RoutePainter(it));
        }
        for (RoutePainter i : routePainter) {
            i.color1 = routeColor;
            i.width = routeWidth;
        }

        painters = new ArrayList<Painter<JXMapViewer>>();
        waypointPainter.setWaypoints(waypoints);
        if (isDrawWay) {
            for (RoutePainter i : routePainter) {
                painters.add(i);
            }
        }
        if (isDrawMarker) {
            painters.add(waypointPainter);
        }
        painter = new CompoundPainter<JXMapViewer>(painters);
        mapViewer.setOverlayPainter(painter);
    }

    private static void input(String fileName) {
        System.out.println("Reading from " + fileName);
        strList.clear();

        try {
            readFile(fileName);
        } catch (Exception e) {
            e.getMessage();
        }
        calcWaypoints();
        hasData = true;
        load();
    }

    private static void load() {
        track.clear();
        waypoints.clear();

        setTrack();
        resetPainter();
        HashSet<GeoPosition> sizer = new HashSet<GeoPosition>();
        for (ArrayList<GeoPosition> it : track) {
            for (GeoPosition jt : it) {
                sizer.add(jt);
            }
        }
        if (isResize) {
            mapViewer.zoomToBestFit(new HashSet<GeoPosition>(sizer), 1);
        }
    }

    public static void setTrack() {

        for (int i = 0; i < buses.get(id).stops.size(); ++i) {
            // Create a track from the geo-positions
            track.add(new ArrayList<GeoPosition>());

            for (int j = 0; j < buses.get(id).stops.get(i).size(); j++) {
                track.get(i).add(
                        new GeoPosition(
                                buses.get(id).stops.get(i).get(j).x,
                                buses.get(id).stops.get(i).get(j).y));


                waypoints.add(new MyWaypoint("", Color.ORANGE, track.get(i).get(j)));
            }
        }
    }

    public static void calcWaypoints() {
        lm.removeAllElements();
        int i = 0;
        al1.clear();
        double x = 0, x1, y, y1;
        id = 0;
        buses = new ArrayList<Bus>();
        boolean newBus = false;

        for (String s : strList) {
            if (s.equals("NEWBUS")) {
                newBus = true;
                continue;
            }
            if (s.equals("NEWWAY")) {
                buses.get(id - 1).stops.add(new ArrayList<Point2D.Double>());
                buses.get(id - 1).cnt++;
                continue;
            }

            if (newBus) {
                buses.add(new Bus(s, id + 1));
                lm.addElement(s);
                newBus = false;
                id++;
                continue;
            }

            i++;
            try {
                if (i % 2 == 0) {
                    y = Integer.parseInt(s);
                    i = 0;
                    x1 = x / 10000000.0;
                    y1 = y / 10000000.0;
                    buses.get(id - 1).stops.get(buses.get(id - 1).cnt - 1).add(new Point2D.Double(x1, y1));
                }
                x = Integer.parseInt(s);
            } catch (NumberFormatException ex) {
                System.out.println("ex: "+s);
            }
        }

        id = 0;
        busList.setSelectedIndex(id);
    }

    // Beolvassa az osszes buszmegallot
    public static void readFile(String fileName) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(fileName));
        try {
            String line = br.readLine();

            while (line != null) {
                strList.add(line);
                line = br.readLine();
            }
        } finally {
            br.close();
        }
    }
}
