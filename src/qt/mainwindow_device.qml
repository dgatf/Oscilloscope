import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0
//import Qt.labs.platform

Window {
    id: window
    visibility: "Maximized"
    visible: true
    title: qsTr("Oscilloscope")

    Settings {
        id: settings
        property string portname
        property string baudrate
        property string databits
        property string parity
        property string interval
        property string background
        property string grid
        property string signal
        property string text
    }

    FileDialog {
        id:fileDialog
        fileMode:  FileDialog.SaveFile
        nameFilters: ["Bitmap image (*.bmp)","All Files (*)"]
        onAccepted: oscilloscope.exportData(fileDialog.currentFile)
    }
/*
    ColorDialog {
        id: colorDialog
        property string setting: ""
        onAccepted: {
            console.log(color)
            switch (setting) {
                case "background":
                    oscilloscope.setBackgroundColor(color)
                    break
                case "grid":
                    oscilloscope.setGridColor(color)
                    break
                case "signal":
                    oscilloscope.setSignalColor(color)
                    break
                case "text":
                    oscilloscope.setTextColor(color)
                    break
            }
        }
    }
*/
    Timer {
        id: timer
        onTriggered: mainItem.statusText =  mainItem.statusTextPermanent
    }

    Connections {
        target: oscilloscope

        function onSendPixmap() {
            mainItem.imageSource =  ""
            mainItem.imageSource =  "image://imgProvider"
        }

        function onSendMessage(message, duration) {
            if (duration === 0)
                mainItem.statusTextPermanent = message
            else {
                timer.start()
                timer.interval = duration
            }
            mainItem.statusText =  message
        }

        function onSendStatusConn(status) {
            if (status === 0) { // connected
                connectButton.icon.source = "qrc:res/connect.png"
                connectButton.status = "connected"
                disconnectMenu.enabled = true
                connectMenu.enabled = false
            }
            if (status === 1) { // disconnected
                connectButton.icon.source = "qrc:res/disconnect.png"
                connectButton.status = "disconnected"
                disconnectMenu.enabled = false
                connectMenu.enabled = true
            }
        }
    }

    ColumnLayout {
        //anchors.margins: 5
        anchors.fill: parent
        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    id: backButton
                    text: "<"
                    visible: false
                    onClicked: {
                        stack.pop()
                        connectButton.visible = true
                        exportButton.visible = true
                        menuButton.visible = true
                        backButton.visible = false
                        toolBarLabel.text = ""
                    }
                }
                ToolButton {
                    id: connectButton
                    property string status: "disconnected"
                    icon.height: 32
                    icon.width: 32
                    icon.source: "qrc:res/disconnect.png"
                    onClicked: {
                        if (status === "disconnected")
                            oscilloscope.openSerialPort(settings.portname, settings.baudrate, settings.databits, settings.parity, settings.interval)
                        else
                            oscilloscope.closeSerialPort()
                    }
                }
                ToolButton {
                    id: exportButton
                    icon.height: 32
                    icon.width: 32
                    icon.source: "qrc:android/res/save.png"
                    onClicked: fileDialog.open()
                }
                Label {
                    id: toolBarLabel
                    Layout.fillWidth: true
                    rightPadding: 52
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                }
                ToolButton {
                    id: menuButton
                    text: qsTr("⋮")
                    onClicked: {
                        menu.open()
                    }

                    Menu {
                        id: menu

                        Action {
                            id: connectMenu
                            text: "Connect"
                            onTriggered: oscilloscope.openSerialPort(settings.portname, settings.baudrate, settings.databits, settings.parity, settings.interval)
                        }
                        Action {
                            id: disconnectMenu
                            enabled: false
                            text: "Disconnect"
                            onTriggered: { oscilloscope.closeSerialPort(); /*settingsItem.fillPorts()*/ }
                        }
                        Action {
                            text: "Export"
                            onTriggered: fileDialog.open()
                        }
                        Action {
                            text: "Settings"
                            onTriggered: {
                                settingsItem.fillPorts()
                                stack.push(settingsItem)
                                connectButton.visible = false
                                exportButton.visible = false
                                backButton.visible = true
                                menuButton.visible = false
                                toolBarLabel.text = "Settings"
                            }
                        }
                        Action {
                            text: "About"
                            onTriggered: {
                                stack.push(aboutItem)
                                connectButton.visible = false
                                exportButton.visible = false
                                backButton.visible = true
                                menuButton.visible = false
                                toolBarLabel.text = "Oscilloscope"
                            }
                        }
                        Action {
                            text: "Quit"
                            onTriggered: Qt.quit()
                        }
                    }
                }
            }
        }

        StackView {
            id: stack
            initialItem: mainItem
            Layout.fillHeight: true
            Layout.fillWidth: true
            Page {
                MainItem {
                    id: mainItem
                }
            }
            Page {
                SettingsItem {
                    id: settingsItem
                    visible: false
                }
            }
            Page {
                AboutItem {
                    id: aboutItem
                    visible: false
                }
            }
        }

        /*
        property variant init: [ mainComp.createObject(), settingsComp.createObject() ]

        Component {
            id: mainComp
            MainItem {
                id: mainItem
                property string pr: "OK"
            }
        }
        Component {
            id: settingsComp
            SettingsItem {
                id: settingsItem
            }
        }
        Component {
            id: aboutComp
            AboutItem {
            }
        }*/


        /*StackLayout {
            id: stack
            Page {
                MainItem {
                    id: mainItem
                    anchors.fill: parent
                }
            }
            Page {
                SettingsItem {
                    id: settingsItem
                    anchors.fill: parent
                    Component.onCompleted: colors = false
                }
            }
            Page {
                AboutItem {
                    id: aboutItem
                    anchors.fill: parent
                }
            }
            currentIndex: 0
        }*/
    }

}



/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}
}
##^##*/
