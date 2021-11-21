import QtQuick 2.9
import QtQuick.Controls 2.2

Item {
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        padding: 40
        Image {
            source: "res/oscilloscope_120x64.bmp"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: "Oscilloscope Serial v" + oscilloscope.getVersion()
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: "DanielGeA, ©2021"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
