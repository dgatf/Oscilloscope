import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0

Item {

    property alias backgroundColor: backgroundRect.color
    property alias gridColor: gridRect.color
    property alias signalColor: signalRect.color
    property alias textColor: textRect.color

    function fillPorts() {
        portnames.clear()
        const infos = oscilloscope.fillPortsInfo()
        for (var i = 0; i < infos.length; i++) {
            portnames.append({"text":infos[i]})
        }
        if (portname.find(settings.portname) === -1)
            portname.currentIndex = 0
        else
            portname.currentIndex = portname.find(settings.portname)
    }

    Component.onCompleted: {

        fillPorts()

        if (baudrate.find(settings.baudrate) === -1)
            settings.baudrate = baudrate.currentText
        else
            baudrate.currentIndex = baudrate.find(settings.baudrate)

        if (databits.find(settings.databits) === -1)
            settings.databits = databits.currentText
        else
            databits.currentIndex = databits.find(settings.databits)

        if (parity.find(settings.parity) === -1)
            settings.parity = parity.currentText
        else
            parity.currentIndex = parity.find(settings.parity)

        if (settings.interval === "")
            settings.interval = interval.value
        else
            interval.value = parseInt(settings.interval)

        if (settings.backgroundColor === "")
            settings.backgroundColor = backgroundRect.color
        else
            backgroundRect.color = settings.backgroundColor

        if (settings.gridColor === "")
            settings.gridColor = gridRect.color
        else
            gridRect.color = settings.gridColor

        if (settings.signalColor === "")
            settings.signalColor = signalRect.color
        else
            signalRect.color = settings.signalColor

        if (settings.textColor === "")
            settings.textColor = textRect.color
        else
            textRect.color = settings.textColor

        settings.portname = Qt.binding(function() { return portname.currentText })
        settings.baudrate = Qt.binding(function() { return baudrate.currentText })
        settings.databits = Qt.binding(function() { return databits.currentText })
        settings.parity = Qt.binding(function() { return parity.currentText })
        settings.interval = Qt.binding(function() { return interval.value })
        settings.backgroundColor = Qt.binding(function() { return backgroundRect.color })
        settings.gridColor = Qt.binding(function() { return gridRect.color })
        settings.signalColor = Qt.binding(function() { return signalRect.color })
        settings.textColor = Qt.binding(function() { return textRect.color })
    }
    GridLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        columns: 1
        GroupBox {
            title: "Serial Port"
            Layout.fillWidth: true
            GridLayout {
                anchors.fill: parent
                columns: 2
                Label { text: "Port " }
                ComboBox {
                    id: portname
                    Layout.fillWidth: true
                    model: ListModel {
                        id: portnames
                    }
                    currentIndex: 0
                }
                Label { text: "Baudrate" }
                ComboBox {
                    id: baudrate
                    Layout.fillWidth: true
                    model: ListModel {    
                        ListElement { text: "9600" }
                        ListElement { text: "19200" }
                        ListElement { text: "38400" }
                        ListElement { text: "57600" }
                        ListElement { text: "115200" }
                        ListElement { text: "230400" }
                        ListElement { text: "500000" }
                        ListElement { text: "1000000" }
                    }
                    currentIndex: 7
                }
                Label { text: "Data bits" }
                ComboBox {
                    id: databits
                    Layout.fillWidth: true
                    model: ListModel {
                        ListElement { text: "5" }
                        ListElement { text: "6" }
                        ListElement { text: "7" }
                        ListElement { text: "8" }
                    }
                    currentIndex: 3
                }
                Label { text: "Parity" }
                ComboBox {
                    id: parity
                    Layout.fillWidth: true
                    model: ListModel {
                        ListElement { text: "None" }
                        ListElement { text: "Even" }
                        ListElement { text: "Odd" }
                    }
                    currentIndex: 0
                }
                Label { text: "Interval (μs)" }
                SpinBox {
                    id: interval
                    Layout.fillWidth: true
                    value: 26
                }
            }
        }
        GroupBox {
           id: colors
           title: "Colors"
           Layout.fillWidth: true
            GridLayout {
                anchors.fill: parent
                columns: 2
                Button {
                    text: "Default colors"
                    Layout.fillWidth: true
                    onClicked: {
                        backgroundRect.color = '#000000'
                        gridRect.color = '#77767b'
                        signalRect.color = '#33d17a'
                        textRect.color = '#f9f06b'
                    }
                }
                Label {}
                Button {
                    id: background
                    text: "Background"
                    Layout.fillWidth: true
                    onClicked: { colorDialog.open(); colorDialog.setting = "background" }
                }
                Rectangle {
                    id: backgroundRect
                    color: '#000000'
                    Layout.fillWidth: true
                    Layout.minimumWidth: 50
                    Layout.minimumHeight: background.height
                    Component.onCompleted: oscilloscope.setBackgroundColor(color)
                    onColorChanged: oscilloscope.setBackgroundColor(color)
                }
                Button {
                    id: grid
                    text: "Grid"
                    Layout.fillWidth: true
                    onClicked: { colorDialog.open(); colorDialog.setting = "grid" }
                }
                Rectangle {
                   id: gridRect
                   color: '#77767b'
                   Layout.fillWidth: true
                   Layout.minimumWidth: 50
                   Layout.minimumHeight: grid.height
                   Component.onCompleted: oscilloscope.setGridColor(color)
                   onColorChanged: oscilloscope.setGridColor(color)
               }
               Button {
                   id: signal
                   text: "Signal"
                   Layout.fillWidth: true
                   onClicked: { colorDialog.open(); colorDialog.setting = "signal" }
               }
               Rectangle {
                   id: signalRect
                   color: '#33d17a'
                   Layout.fillWidth: true
                   Layout.minimumWidth: 50
                   Layout.minimumHeight: signal.height
                   Component.onCompleted: oscilloscope.setSignalColor(color)
                   onColorChanged: oscilloscope.setSignalColor(color)
               }
               Button {
                   id: text
                   text: "Text"
                   Layout.fillWidth: true
                   onClicked: { colorDialog.open(); colorDialog.setting = "text" }
               }
               Rectangle {
                   id: textRect
                   color: '#f9f06b'
                   Layout.fillWidth: true
                   Layout.minimumWidth: 50
                   Layout.minimumHeight: text.height
                   Component.onCompleted: oscilloscope.setTextColor(color)
                   onColorChanged: oscilloscope.setTextColor(color)
               }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.5;height:480;width:640}
}
##^##*/
