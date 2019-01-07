import QtQuick 2.0
import QtPositioning 5.6
import QtLocation 5.6
import "map_logic.js" as MapLogic

Item {
    id: item;

    Connections {
        target: mapCore

        onClearMap: {
            map.clearMapItems();
        }

        onAddPolyline: {
            map.track = Qt.createQmlObject('import QtLocation 5.6; MapPolyline {}', item);
            map.track.line.width = 6;
            map.track.line.color = 'red';
            MapLogic.tracks.push(map.track);
            map.addMapItem(MapLogic.tracks[MapLogic.tracks.length - 1]);
        }

        onAddPoint: {
            MapLogic.tracks[MapLogic.tracks.length - 1].addCoordinate(position);
            //console.log(position);
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
        }
        center: QtPositioning.coordinate(55.778274, 37.586859)
        zoomLevel: 17

        property MapPolyline track
    }
}
