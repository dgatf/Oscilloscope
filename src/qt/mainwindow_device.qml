import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0
import Qt.labs.platform 1.0

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
        property string backgroundColor
        property string gridColor
        property string signalColor
        property string textColor
    }

    FileDialog {
        id: fileDialogCapture
        fileMode:  FileDialog.SaveFile
        nameFilters: ["Bitmap image (*.bmp)","All Files (*)"]
        onAccepted: oscilloscope.exportImage(fileDialogCapture.currentFile)
    }

    FileDialog {
        id: fileDialogExport
        fileMode:  FileDialog.SaveFile
        nameFilters: ["Text CSV (*.csv)","All Files (*)"]
        onAccepted: oscilloscope.exportCsv(fileDialogExport.currentFile)
    }

    ColorDialog {
        id: colorDialog
        property string setting: ""
        onAccepted: {
            console.log(color)
            switch (setting) {
                case "background":
                    settingsItem.backgroundColor = color
                    break
                case "grid":
                    settingsItem.gridColor = color
                    break
                case "signal":
                    settingsItem.signalColor = color
                    break
                case "text":
                    settingsItem.textColor = color
                    break
            }
        }
    }

    Timer {
        id: timer
        onTriggered: mainItem.statusText =  mainItem.statusTextPermanent
    }

    Connections {
        target: oscilloscope

        onSendPixmap: {
            mainItem.imageSource =  ""
            mainItem.imageSource =  "image://imgProvider"
        }

        onSendMessage: {
            if (duration === 0)
                mainItem.statusTextPermanent = message
            else {
                timer.start()
                timer.interval = duration
            }
            mainItem.statusText =  message
        }

        onSendStatusConn: {
            if (status === 0) { // connected
                connectButton.status = "connected"
                connectMenu.status = "connected"
                pauseButton.enabled = true
                exportButton.enabled = true
                pauseMenu.enabled = true
                exportMenu.enabled = true
            }
            if (status === 1) { // disconnected
                connectButton.status = "disconnected"
                connectMenu.status = "disconnected"
                pauseButton.enabled = false
                exportButton.enabled = false
                pauseMenu.enabled = false
                exportMenu.enabled = false
            }
        }

        onIsPausedChanged: {
            if (isPaused) {
                pauseButton.isPaused = true
                pauseMenu.isPaused = true
            }
            else {
                pauseButton.isPaused = false
                pauseMenu.isPaused = false
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
                        pauseButton.visible = true
                        captureButton.visible = true
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
                    icon.source: (status === "disconnected") ? "qrc:res/disconnect.png" :"qrc:res/connect.png"
                    onClicked: (status === "disconnected")
                                ? oscilloscope.openSerialPort(settings.portname, settings.baudrate, settings.databits, settings.parity, settings.interval)
                                : oscilloscope.closeSerialPort()
                }
                ToolButton {
                    id: pauseButton
                    property bool isPaused: false
                    enabled: false
                    icon.height: 32
                    icon.width: 32
                    icon.source: (isPaused === false) ? "qrc:res/pause.png" : "qrc:res/play.png"
                    onClicked: oscilloscope.setIsPaused(!oscilloscope.getIsPaused())
                }
                ToolButton {
                    id: captureButton
                    icon.height: 32
                    icon.width: 32
                    icon.source: "qrc:res/capture.png"
                    onClicked: fileDialogCapture.open()
                }
                ToolButton {
                    id: exportButton
                    icon.height: 32
                    icon.width: 32
                    icon.source: "qrc:res/export.png"
                    onClicked: fileDialogExport.open()
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

                        MenuItem {
                            id: connectMenu
                            text: "Connect"
                            onTriggered: oscilloscope.openSerialPort(settings.portname, settings.baudrate, settings.databits, settings.parity, settings.interval)
                        }
                        MenuItem {
                            id: disconnectMenu
                            enabled: false
                            text: "Disconnect"
                            onTriggered: { oscilloscope.closeSerialPort(); /*settingsItem.fillPorts()*/ }
                        }
                        MenuItem {
                            text: "Export"
                            onTriggered: fileDialog.open()
                        }
                        MenuItem {
                            text: "Settings"
                            onTriggered: {
                                settingsItem.fillPorts()
                                stack.push(settingsItem)
                                connectButton.visible = false
                                pauseButton.visible = false
                                captureButton.visible = false
                                exportButton.visible = false
                                backButton.visible = true
                                menuButton.visible = false
                                toolBarLabel.text = "Settings"
                            }
                        }
                        MenuItem {
                            text: "About"
                            onTriggered: {
                                stack.push(aboutItem)
                                connectButton.visible = false
                                pauseButton.visible = false
                                captureButton.visible = false
                                exportButton.visible = false
                                backButton.visible = true
                                menuButton.visible = false
                                toolBarLabel.text = "Oscilloscope"
                            }
                        }
                        MenuItem {
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
