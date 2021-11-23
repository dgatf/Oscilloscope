import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

ApplicationWindow {
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
        fileMode: Platform.FileDialog.SaveFile
        nameFilters: ["Bitmap image (*.bmp)","All Files (*)"]
        onAccepted: oscilloscope.exportImage(fileDialogCapture.currentFile)
    }

    FileDialog {
        id: fileDialogExport
        fileMode:  FileDialog.SaveFile
        nameFilters: ["Text CSV (*.csv)","All Files (*)"]
        onAccepted: oscilloscope.exportCsv(fileDialogExport.currentFile)
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

    header: ToolBar {
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
                enabled: false
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/export.png"
                onClicked: fileDialogExport.open()
            }
            Label {
                Layout.fillWidth: true
                rightPadding: 52
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
            }

            ToolButton {
                id: menuButton
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/menu.png"
                onClicked: {
                    menu.open()
                }

                Menu {
                    id: menu
                    MenuItem {
                        id: connectMenu
                        property string status: "disconnected"
                        text: (status === "disconnected") ? "Connect" : "Disconnect"
                        onTriggered: (status === "disconnected") ?
                                         oscilloscope.openSerialPort(settings.portname, settings.baudrate, settings.databits, settings.parity, settings.interval)
                                       : oscilloscope.closeSerialPort()
                    }
                    MenuItem {
                        id: pauseMenu
                        enabled: false
                        property bool isPaused: false
                        text: (isPaused === false) ? "Pause" : "Play"
                        onTriggered: oscilloscope.setIsPaused(!oscilloscope.getIsPaused())
                    }
                    MenuItem {
                        text: "Capture image"
                        onTriggered: fileDialogCapture.open()
                    }
                    MenuItem {
                        id: exportMenu
                        enabled: false
                        text: "Export CSV"
                        onTriggered: fileDialogExport.open()
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
        anchors.fill: parent
        Page {
            anchors.fill: parent
            MainItem {
                id: mainItem
                anchors.fill: parent
            }
        }
        Page {
            anchors.fill: parent
            SettingsItem {
                id: settingsItem
                colorsVisible: false
                visible: false
            }
        }
        Page {
            anchors.fill: parent
            AboutItem {
                id: aboutItem
                visible: false
            }
        }
    }

}

