import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Oscilloscope")
    minimumWidth: 400
    minimumHeight: 200

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
        property string averagingElements

        //property alias x: window.x
        //property alias y: window.y
        //property alias width: window.width
        //property alias height: window.height
    }

    FileDialog {
        id: fileDialogCapture
        title: "Export image"
        fileMode:  FileDialog.SaveFile
        nameFilters: ["Bitmap image (*.bmp)","All Files (*)"]
        onAccepted: oscilloscope.exportImage(fileDialogCapture.currentFile)
    }

    FileDialog {
        id: fileDialogExport
        title: "Export data"
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

    header: ToolBar {
        Layout.fillWidth: true
        RowLayout {
            anchors.fill: parent

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
            ToolButton {
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/settings.png"
                onClicked:  { settingsItem.fillPorts(); settingsWindow.show() }
            }
            ToolButton {
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/info.png"
                onClicked: aboutWindow.show()
            }
            ToolButton {
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/application-exit.png"
                onClicked: Qt.quit()
            }
            Label {
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                icon.height: 32
                icon.width: 32
                icon.source: "qrc:res/menu.png"
                onClicked: menu.open()

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
                        onTriggered: { settingsItem.fillPorts(); settingsWindow.show() }
                    }
                    MenuItem {
                        text: "About"
                        onTriggered: aboutWindow.show()
                    }
                    MenuItem {
                        text: "Quit"
                        onTriggered: Qt.quit()
                    }
                }
            }
        }
    }

    GridLayout {
        anchors.margins: 5
        anchors.fill: parent
        MainItem {
            id: mainItem
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Window {
        id: settingsWindow
        width: 300
        height: settingsItem.childrenRect.height
        maximumHeight: height
        maximumWidth: width
        minimumHeight: height
        minimumWidth: width
        modality: Qt.ApplicationModal
        flags: Qt.WindowTitleHint | Qt.WindowCloseButtonHint | Qt.Dialog
        title: "Settings"
        SettingsItem {
            id: settingsItem
            anchors.fill: parent
        }
    }

    Window {
        id: aboutWindow
        width: aboutItem.childrenRect.width
        height: aboutItem.childrenRect.height
        maximumHeight: height
        maximumWidth: width
        minimumHeight: height
        minimumWidth: width
        modality: Qt.ApplicationModal
        flags: Qt.WindowTitleHint | Qt.WindowCloseButtonHint | Qt.Dialog
        title: "About"
        AboutItem {
            id: aboutItem
            anchors.fill: parent
        }
    }

}



/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}
}
##^##*/
