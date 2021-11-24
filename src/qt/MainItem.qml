import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.15
import QtQuick.Window 2.3

Item {
    property alias statusText : statusLabel.text
    property string statusTextPermanent: statusLabel.textPermanent
    property alias imageSource : image.source
    property alias columns: controls.columns
    ColumnLayout {
        id: column
        anchors.fill: parent
        GridLayout {
            id: grid
            Layout.fillWidth: true
            columns: 2
            Image {
                id: image
                source: "image://imgProvider"
                Layout.fillWidth: true
                Layout.fillHeight: true
                cache: false
                sourceSize.width: width
                sourceSize.height: height
            }
            Slider {
                id: verticalSlider
                orientation: Qt.Vertical
                Layout.maximumWidth: 30
                Layout.fillHeight: true
                from: -30
                to: 30
                stepSize: 5
                snapMode: "SnapAlways"
                background: Rectangle {
                    color: "#c0bfbc"
                    width: 5
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                onValueChanged: oscilloscope.setVAdjust(200 * value / (to - from))
            }
            Slider {
                value: 0
                Layout.fillWidth: true
                from: -55
                to: 55
                stepSize: 5
                snapMode: "SnapAlways"
                background: Rectangle {
                    color: "#c0bfbc"
                    height: 5
                    width: parent.width
                    anchors.verticalCenter: parent.verticalCenter
                }
                onValueChanged: oscilloscope.setHAdjust(200 * value / (to - from))
            }
        }

        // desktop / device horiz: 2,2,2
        // device vertical: 1,2,2
        GridLayout {
            id: controls
            columns: Screen.width > Screen.height ? 2 : 1
            GridLayout {
                columns: 2
                ComboBox {
                    id: vDivCombo
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                    model: ListModel {
                        id: vDivList
                        ListElement { text: "2V/d"; value: 2000 }
                        ListElement { text: "1V/d"; value: 1000 }
                        ListElement { text: "500mV/d"; value: 500 }
                        ListElement { text: "100mV/d"; value: 100 }
                    }
                    currentIndex: 1
                    onCurrentValueChanged: oscilloscope.setVDiv(currentValue)
                }

                ComboBox {
                    id: hDivCombo
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                    model: ListModel {
                        id: hDivList
                        ListElement { text: "10ms/d"; value: 10000 }
                        ListElement { text: "5ms/d"; value: 5000 }
                        ListElement { text: "1ms/d"; value: 1000 }
                        ListElement { text: "500μs/d"; value: 500 }
                        ListElement { text: "100μs/d"; value: 100 }
                    }
                    currentIndex: 2
                    onCurrentValueChanged: oscilloscope.setHDiv(currentValue)
                }
            }
            GridLayout {
                columns: 3
                ComboBox {
                    id: triggerTypeCombo
                    Layout.fillWidth: true
                    model: ListModel {
                        id: triggerTypeList
                        ListElement { text: "None" }
                        ListElement { text: "Rising" }
                        ListElement { text: "Falling" }
                    }
                    currentIndex: 1
                    onCurrentTextChanged: oscilloscope.setTriggerType(currentIndex)
                }

                SpinBox {
                    id: triggerValueSpin
                    editable: true
                    Layout.fillWidth: true
                    from: 0
                    to: 5000
                    value: 1000
                    onValueChanged: oscilloscope.setTriggerValue(value)
                }
                Label {
                    text: "mV"
                }
            }
        }

        Label {
            id: statusLabel
            Layout.preferredWidth: parent.width
            elide: Text.ElideRight
            property string textPermanent: "Disconnected"
            text: "Disconnected" }
    }
}
