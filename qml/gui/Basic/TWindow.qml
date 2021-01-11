import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Controls.Universal 2.3

Window{
    id: root
    //flags: Qt.Dialog
    property string caption
    property alias content: mid.data
    property var titlebuttons
    property var footbuttons
    property bool titlevisible: true
    flags: Qt.Window | Qt.CustomizeWindowHint //Qt.FramelessWindowHint
    modality: Qt.WindowModal
    width: 400
    height: 300

    MouseArea{
        property int coor_x
        property int coor_y
        width: parent.width
        height: Screen.desktopAvailableHeight * 0.03
        z: - 1
        onPressed: function(aInput){
            coor_x = aInput["x"]
            coor_y = aInput["y"]
        }
        onPositionChanged: function(aInput){
            root.x += aInput["x"] - coor_x
            root.y += aInput["y"] - coor_y
        }
    }

    Column{
        anchors.fill: parent
        Rectangle{
            width: parent.width
            height: Screen.desktopAvailableHeight * 0.03
            color: "lightskyblue"
            visible: !titlevisible
        }
        Row{
            width: parent.width
            height: Screen.desktopAvailableHeight * 0.03
            visible: titlevisible
            Rectangle{
                x: parent.height * 0.2
                y: parent.height * 0.2
                height: parent.height * 0.6
                width: height
                color: "lightskyblue"
                ShaderEffect{
                    id: logo
                    anchors.fill: parent
                    property variant source: Image{
                        anchors.fill: parent
                        source: "file:resource/logo.png"
                    }
                    property real amplitude: 0
                    property real frequency: 20
                    property real time: 0
                    NumberAnimation on time {
                        id: animation
                        running: false
                        loops: Animation.Infinite; from: 0; to: Math.PI * 2; duration: 600
                    }
                    /*fragmentShader: "
                        varying highp vec2 qt_TexCoord0;
                        uniform sampler2D source;
                        uniform lowp float qt_Opacity;
                        uniform lowp float redChannel;
                        void main(){
                            gl_FragColor = texture2D(source, qt_TexCoord0) * vec4(redChannel, 1.0, 1.0, 1.0) * qt_Opacity;
                        }
                    "*/
                    fragmentShader: "
                        uniform sampler2D source;
                        varying highp vec2 qt_TexCoord0;
                        uniform lowp float qt_Opacity;
                        uniform highp float amplitude;
                        uniform highp float frequency;
                        uniform highp float time;
                        void main()
                        {
                            highp vec2 p = sin(time + frequency * qt_TexCoord0);
                            gl_FragColor = texture2D(source, qt_TexCoord0 + amplitude * vec2(p.y, -p.x)) * qt_Opacity;
                        }
                    "
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            animation.running = !animation.running
                            if (animation.running)
                                logo.amplitude = 0.02
                            else
                                logo.amplitude = 0
                        }
                    }
                }
            }
            Rectangle{
                id: titbar
                height: parent.height
                width: parent.width - btns.parent.width - logo.width
                Text{
                    leftPadding: parent.width * 0.05
                    anchors.verticalCenter: parent.verticalCenter
                    text: caption
                }
            }
            Row{
                width: childrenRect.width
                height: parent.height

                Repeater {
                    id: btns
                    model: ListModel {
                    }

                    delegate: Button {
                        width: parent.height
                        height: parent.height
                        text: cap
                        background: Rectangle{
                            border.color: parent.hovered ? "gray" : "transparent"
                        }
                    }
                }

                Component.onCompleted: {
                    if (titlebuttons){
                        for (var i in titlebuttons){
                            btns.model.append({cap: titlebuttons[i]["cap"]})
                            btns.itemAt(btns.count - 1).clicked.connect(titlebuttons[i]["func"])
                        }
                    }
                    btns.model.append({cap: "X"})
                    btns.itemAt(btns.count - 1).clicked.connect(function(){
                        close()
                    })
                }
            }
        }
        Item{
            id: mid
            width: parent.width
            height: parent.height - Screen.desktopAvailableHeight * (0.045 + (footbuttons ? 0.03 : 0))
        }
        Row{
            anchors.right: parent.right
            anchors.rightMargin: height * 0.4
            width: childrenRect.width
            height: footbuttons ? Screen.desktopAvailableHeight * 0.045 : 0
            spacing: 5

            Repeater{
                id: btns2
                model: ListModel{

                }
                delegate: Button{
                    height: parent.height * 0.5
                    width: height * 2.5
                    text: cap
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle{
                        color: parent.hovered ? "transparent" : "gray"
                        border.color: parent.hovered ? "gray" : "transparent"
                    }
                }
            }

            Component.onCompleted: {
                if (footbuttons)
                    for (var i in footbuttons){
                        btns2.model.append({cap: footbuttons[i]["cap"]})
                        btns2.itemAt(btns2.count - 1).clicked.connect(footbuttons[i]["func"])
                    }
            }
        }
    }
    onHeightChanged: {
        mid.height = height - Screen.desktopAvailableHeight * (0.045 + (footbuttons ? 0.03 : 0))
    }

    function setHeight(aHeight){
        height = aHeight
        mid.height = height - Screen.desktopAvailableHeight * (0.045 + (footbuttons ? 0.03 : 0))
    }
}
