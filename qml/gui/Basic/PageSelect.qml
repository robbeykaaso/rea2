import QtQuick 2.12
import QtQuick.Controls 2.5

FocusScope{
    id: fs
    property string page_text
    property string page_suffix
    property string page_title: "Page:"
    signal selectSpecificPage(var aIndex)
    signal selectNeighbourPage(var aNext)
    anchors.horizontalCenter: parent.horizontalCenter
    width: 130
    height: 30
    Row{
        anchors.fill: parent
        Edit{
            caption.text: page_title
            input.text: fs.page_text
            background.color: "lightskyblue"
            width: 60
            ratio: 0.5
            input.onAccepted: {
                selectSpecificPage(parseInt(input.text))
                fs.focus = false
            }
        }
        Label{
            text: fs.page_suffix
            font.pixelSize: 12
            color: "black"
            anchors.verticalCenter: parent.verticalCenter
        }
        Button{
            background: Rectangle{
                color: "transparent"
                border.color: "gray"
                radius: 3
            }
            anchors.verticalCenter: parent.verticalCenter
            height: 20
            width: 20
            text: "<"
            onClicked: selectNeighbourPage(false)
        }
        Button{
            background: Rectangle{
                color: "transparent"
                border.color: "gray"
                radius: 3
            }
            anchors.verticalCenter: parent.verticalCenter
            height: 20
            width: 20
            text: ">"
            onClicked: selectNeighbourPage(true)
        }
    }
}
