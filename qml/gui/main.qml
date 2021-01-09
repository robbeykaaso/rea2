import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Controls.Universal 2.3
import "Basic"
import "Component"
import "TreeNodeView"
import QSGBoard 1.0
import Pipeline2 1.0

ApplicationWindow {
    id: mainwindow
    property var view_cfg
    visible: true
    width: 800
    height: 600
    //width: Screen.desktopAvailableWidth
    //height: Screen.desktopAvailableHeight
    //visibility: Window.Maximized

    Universal.theme: Universal.Dark
    //Universal.accent: Universal.Green
    //Universal.background: Universal.Cyan

    menuBar: MenuBar{
        id: mainmenu
        Menu{
            title: Pipeline2.tr("qsgShow")

            delegate: MenuItem {
                id: menuItem
                implicitWidth: 200
                implicitHeight: 40

                arrow: Canvas {
                    x: parent.width - width
                    implicitWidth: 40
                    implicitHeight: 40
                    visible: menuItem.subMenu
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.fillStyle = menuItem.highlighted ? "#ffffff" : "#21be2b"
                        ctx.moveTo(15, 15)
                        ctx.lineTo(width - 15, height / 2)
                        ctx.lineTo(15, height - 15)
                        ctx.closePath()
                        ctx.fill()
                    }
                }

                indicator: Item {
                    implicitWidth: 40
                    implicitHeight: 40
                    Rectangle {
                        width: 14
                        height: 14
                        anchors.centerIn: parent
                        visible: menuItem.checkable
                        border.color: "#21be2b"
                        radius: 3
                        Rectangle {
                            width: 8
                            height: 8
                            anchors.centerIn: parent
                            visible: menuItem.checked
                            color: "#21be2b"
                            radius: 2
                        }
                    }
                }

                contentItem: Text {
                    leftPadding: menuItem.indicator.width
                    rightPadding: menuItem.arrow.width
                    text: menuItem.text
                    font: menuItem.font
                    opacity: enabled ? 1.0 : 0.3
                    color: menuItem.highlighted ? "#ffffff" : "#21be2b"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: menuItem.highlighted ? "#21be2b" : "transparent"
                }
            }

            Menu{
                title: Pipeline2.tr("updateModel")
                MenuItem {
                    text: Pipeline2.tr("show")
                    onClicked: {
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }

                Action {
                    text: Pipeline2.tr("face")
                    checkable: true
                    shortcut: "Ctrl+F"
                    onTriggered: {
                        view_cfg["face"] = 100 - view_cfg["face"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: Pipeline2.tr("arrow")
                    checkable: true
                    shortcut: "Ctrl+A"
                    onTriggered: {
                        view_cfg["arrow"]["visible"] = !view_cfg["arrow"]["visible"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: Pipeline2.tr("text")
                    checkable: true
                    shortcut: "Ctrl+T"
                    onTriggered: {
                        view_cfg["text"]["visible"] = !view_cfg["text"]["visible"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: Pipeline2.tr("fps")
                    checkable: true
                    onTriggered: {
                        Pipeline2.run("testFPS", {})
                    }
                }
            }

            Menu{
                title: Pipeline2.tr("updateWholeAttr")

                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeArrowVisible")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["arrow", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeArrowPole")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["arrow", "pole"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeFaceOpacity")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["face"], val: checked ? 200 : 0})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeTextVisible")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["text", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeColor")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["color"], val: checked ? "yellow" : "green"})
                    }
                }

                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("wholeObjects")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["objects"], type: checked ? "add" : "del", tar: "shp_3", val: {
                                                                                                         type: "poly",
                                                                                                         points: [[500, 300, 700, 300, 700, 500, 500, 300]],
                                                                                                         color: "pink",
                                                                                                         caption: "new_obj",
                                                                                                         face: 200
                                                                                                     }})
                    }
                }

            }
            Menu{
                title: Pipeline2.tr("updateLocalAttr")

                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyArrowVisible")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyArrowPole")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "pole"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyFaceOpacity")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["face"], val: checked ? 200 : 0})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyTextVisible")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["text", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyColor")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["color"], val: checked ? "yellow" : "green"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyCaption")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["caption"], val: checked ? "poly_new" : "poly"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyWidth")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["width"], val: checked ? 0 : 10})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyPoints")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["points"], val: checked ? [[50, 50, 200, 50, 200, 200, 50, 200, 50, 50], [80, 70, 120, 100, 120, 70, 80, 70]] : [[50, 50, 200, 200, 200, 50, 50, 50]]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("polyStyle")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["style"], val: checked ? "dash" : "solid"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("ellipseAngle")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["angle"], val: checked ? 90 : 20})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("ellipseCenter")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["center"], val: checked ? [600, 400] : [400, 400]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("ellipseRadius")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["radius"], val: checked ? [200, 400] : [300, 200]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("ellipseCCW")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["ccw"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("imagePath")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["path"], val: checked ? "F:/3M/B4DT/DF Mark/V1-1.bmp" : "F:/3M/B4DT/DF Mark/V1-2.bmp"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: Pipeline2.tr("imageRange")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["range"], val: checked ? [0, 0, 600, 800] : [0, 0, 400, 300]})
                    }
                }
            }
        }

        Menu{
            title: Pipeline2.tr("gui")
            Menu{
                title: Pipeline2.tr("log")
                MenuItem{
                    text: Pipeline2.tr("addLogRecord")
                    onClicked:{
                        Pipeline2.run("addLogRecord", {type: "train", level: "info", msg: "train_info"})
                        Pipeline2.run("addLogRecord", {type: "train", level: "warning", msg: "train_warning"})
                        Pipeline2.run("addLogRecord", {type: "train", level: "error", msg: "train_error"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "info", msg: "system_info"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "warning", msg: "system_warning"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "error", msg: "system_error"})
                    }
                }
                MenuItem{
                    text: Pipeline2.tr("showLogPanel")
                    onClicked:{
                        Pipeline2.run("showLogPanel", {})
                    }
                }
            }
            Menu{
                title: Pipeline2.tr("list")
                MenuItem{
                    text: Pipeline2.tr("updateListView")
                    onClicked: {
                        Pipeline2.run("_updateListView", {title: ["cat", "dog", "sheep", "rat"],
                                                          selects: [1, 3, 5],
                                                          data: [
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]}
                                                          ]})
                    }
                }
                MenuItem{
                    text: Pipeline2.tr("modifyListView")
                    onClicked: {
                        Pipeline2.run("_updateListView", {index: [2, 4, 5],
                                                          fontclr: "red",
                                                          data: [
                                                            {entry: [1, 3, 2, 3]},
                                                            {},
                                                            {entry: [2, 3, 2, 3]}
                                                          ]})
                    }
                }
                MenuItem{
                    text: Pipeline2.tr("updatePageListView")
                    onClicked: {
                        Pipeline2.run("pageList_updateListView", {title: ["idx", "dog", "sheep", "rat"],
                                                          selects: [1, 3, 11],
                                                          entrycount: 3,
                                                          pageindex: 2,
                                                          data: [
                                                            {entry: [0, 6, 2, 3]},
                                                            {entry: [1, 6, 2, 3]},
                                                            {entry: [2, 6, 2, 3]},
                                                            {entry: [3, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [5, 6, 2, 3]},
                                                            {entry: [6, 6, 2, 3]},
                                                            {entry: [7, 6, 2, 3]},
                                                            {entry: [8, 6, 2, 3]},
                                                            {entry: [9, 6, 2, 3]},
                                                            {entry: [10, 6, 2, 3]},
                                                            {entry: [11, 6, 2, 3]}
                                                          ]})
                    }
                }

                Component.onCompleted: {
                    Pipeline2.find("_listViewSelected").next(function(aInput){
                        console.log(aInput)
                    }, {tag: "manual"}, {vtype: []})
                    Pipeline2.find("pageList_listViewSelected").next(function(aInput){
                        console.log(aInput)
                    }, {tag: "manual"}, {vtype: []})
                }
            }

            Menu{
                title: Pipeline2.tr("container")
                MenuItem{
                    text: Pipeline2.tr("TWindow")
                    onClicked: {
                        twin.show()
                    }
                }
                MenuItem{
                    text: Pipeline2.tr("swipe")
                    onClicked: swipe.show()
                }
                MenuItem{
                    text: Pipeline2.tr("gridder")
                    onClicked: gridder.show()
                }
                MenuItem{
                    text: Pipeline2.tr("flip")
                    onClicked: flip.show()
                }
                MenuItem{
                    text: Pipeline2.tr("nest")
                    onClicked:
                        nest.show()
                }
            }

            Menu{
                title: Pipeline2.tr("dialog")

                Menu{
                    title: Pipeline2.tr("file")
                    MenuItem{
                        text: Pipeline2.tr("files")
                        onClicked: {
                            Pipeline2.run("_selectFile", {folder: false, filter: ["Image files (*.jpg *.png *.jpeg *.bmp)"]})
                        }
                    }
                    MenuItem{
                        text: Pipeline2.tr("directory")
                        onClicked: {
                            Pipeline2.run("_selectFile", {folder: true, tag: {tag: "manual2"}})
                        }
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_selectFile").next(function(aInput){
                            console.log(aInput)
                        }, {tag: "manual2"}, {vtype: []})
                    }
                }

                MenuItem{
                    text: Pipeline2.tr("color")
                    onClicked: {
                        Pipeline2.run("_selectColor", {tag: {tag: "manual2"}})
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_selectColor").next(function(aInput){
                            console.log(aInput)
                        }, {tag: "manual2"}, {vtype: ""})
                    }
                }

                MenuItem{
                    text: Pipeline2.tr("MsgDialog")
                    onClicked:
                        Pipeline2.run("popMessage", {title: "hello4", text: "world"})
                }

            }

            Menu{
                title: Pipeline2.tr("diagram")

                MenuItem{
                    text: Pipeline2.tr("linechart")
                    onClicked: {
                        linechart.show()
                    }
                }

                MenuItem{
                    text: Pipeline2.tr("histogram")
                    onClicked: {
                        histogram.show()
                    }
                }

                MenuItem{
                    text: Pipeline2.tr("thistogram")
                    onClicked: {
                        thistogram.show()
                    }
                }
            }

            MenuItem{
                text: Pipeline2.tr("matrix")
                onClicked: {
                    matrix.show()
                }
            }

            MenuItem{
                text: Pipeline2.tr("status")
                onClicked:
                    Pipeline2.run("_updateStatus", ["hello", "world"])
            }
            Menu{
                title: Pipeline2.tr("navigation")
                MenuItem{
                    text: Pipeline2.tr("2")
                    onClicked: Pipeline2.run("_updateNavigation", ["first layer", "second layer"], {tag: "manual"})
                }
                MenuItem{
                    text: Pipeline2.tr("3")
                    onClicked: Pipeline2.run("_updateNavigation", ["first layer", "second layer", "third layer"], {tag: "manual"})
                }
                MenuItem{
                    text: Pipeline2.tr("4")
                    onClicked: Pipeline2.run("_updateNavigation", ["first layer", "second layer", "third layer", "forth layer"], {tag: "manual"})
                }
                Component.onCompleted: {
                    Pipeline2.find("_updateNavigation").next(function(aInput){
                        console.log(aInput)
                    }, {tag: "manual"}, {vtype: []})
                }
            }
            MenuItem{
                text: Pipeline2.tr("search")
                onClicked:
                    Pipeline2.run("_Searched", "", {tag: "manual"})
            }

            MenuItem{
                text: Pipeline2.tr("baseCtrl")
                onClicked: {
                    basectrl.show()
                }
            }
            MenuItem{
                text: Pipeline2.tr("treeNodeView")
                onClicked: {
                    treeview.show()
                }
            }
            MenuItem{
                text: Pipeline2.tr("treeView0")
                onClicked: {
                    treeview0.show()
                }
            }

            MenuItem{
                property var tag: {"tag": "testProgress"}
                property int cnt: 0
                property double hope
                text: Pipeline2.tr("progress")
                onClicked: {
                    if (cnt % 10 == 0){
                        hope = 0.0
                        Pipeline2.run("updateProgress", {title: "demo: ", sum: 10}, tag)
                        hope = 0.1
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 1){
                        hope = 0.2
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 2){
                        hope = 0.9
                        Pipeline2.run("updateProgress", {step: 7}, tag)
                        cnt += 7
                    }else if (cnt % 10 == 9){
                        hope = 1.0
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }
                }
                Component.onCompleted: {
                    Pipeline2.find("updateProgress").next(function(aInput){
                        console.assert(aInput === hope)
                    }, tag, {vtype: 0.1})
                }
            }
        }

        Menu{
            title: Pipeline2.tr("device")
            Action{
                text: Pipeline2.tr("camera")
                onTriggered: camera.show()
            }
            Action{
                text: Pipeline2.tr("io")
                onTriggered: io.show()
            }
        }

        Menu{
            title: Pipeline2.tr("integrate")
            Action{
                text: Pipeline2.tr("pack")
                onTriggered:
                    //Pipeline2.run("showPackWindow", {})
                    Pipeline2.run("packExe", {})
            }
        }

        /*Loader{
            source: "file:D:/mywork/build-app-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/plugin/menu/shape.qml"
            onLoaded: {
                mainmenu.addMenu(item)
            }
        }*/

        DynamicQML{
            name: "menu"
            onLoaded: function(aItem){
                mainmenu.addMenu(aItem)
            }
        }

        Component.onCompleted: {
            view_cfg = {
                face: 0,
                arrow: {
                    visible: false,
                    pole: true
                },
                text: {
                    visible: false,
                    location: "middle"
                }
            }
        }
    }
    contentData:
        Column{
            anchors.fill: parent
            Row{
                width: parent.width
                height: parent.height - 60
                Column{
                    width: parent.width * 0.3
                    height: parent.height
                    Search {
                        text: Pipeline2.tr("search")
                        width: parent.width
                        height: 30
                        prefix: "#"
                    }
                    Rectangle{
                        width: parent.width
                        height: (parent.height - 30) / 2
                        color: "white"
                        List{
                            anchors.fill: parent
                        }
                    }
                    Rectangle{
                        width: parent.width
                        height: (parent.height - 30) / 2
                        color: "white"
                        PageList{
                            name: "pageList"
                            anchors.fill: parent
                        }
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_Searched")
                        .next(function(aInput){
                            console.assert(aInput === Pipeline2.tr("search"))
                            console.log(aInput + " is searched")
                        }, {tag: "manual"}, {vtype: ""})
                    }
                }
                Column{
                    width: parent.width * 0.7
                    height: parent.height
                    QSGBoard{
                        id: testbrd
                        name: "testbrd"
                        plugins: [{type: "transform"}]
                        width: parent.width
                        height: parent.height * 0.7
                        Component.onDestruction: {
                            beforeDestroy()
                        }
                    }

                    Log{
                        width: parent.width
                        height: parent.height * 0.3
                    }
                }
            }
            Navigation{
                width: parent.width
                height: 30
            }
            Status{
                width: parent.width
                height: 30
            }
        }

    TWindow{
        id: twin
        caption: Pipeline2.tr("TWindow")
        content: Column{
            anchors.fill: parent
            Rectangle{
                width: parent.width
                height: parent.height
                color: "gray"
            }
        }
        titlebuttons: [{cap: "O", func: function(){console.log("hello")}},
                        {cap: "W", func: function(){console.log("world")}}]
        footbuttons: [{cap: "OK", func: function(){close()}},
                       {cap: "Cancel", func: function(){close()}}]
    }

    TWindow{
        id: basectrl
        caption: Pipeline2.tr("baseCtrl")
        content: Rectangle{
            anchors.fill: parent
            color: "gray"
            Column{
                /*Repeater{
                    delegate: T{

                    }
                }*/
                id: clm
                leftPadding: 5
                Edit{
                    width: 180
                    caption.text: Pipeline2.tr("attr1") + ":"
                    ratio: 0.4
                }
                Check{
                    width: 180
                    caption.text: Pipeline2.tr("attribu2") + ":"
                    ratio: 0.4
                }
                Spin{
                    width: 180
                    caption.text: Pipeline2.tr("attribute555") + ":"
                    ratio: 0.4
                }
                Combo{
                    width: 180
                    caption.text: Pipeline2.tr("attribute3") + ":"
                    ratio: 0.4
                }
                Track{
                    width: 180
                    caption.text: Pipeline2.tr("attri4") + ":"
                    ratio: 0.4
                    onIndexChanged: function(aIndex){
                        console.log("track index: " + aIndex)
                    }
                }
                Radio{
                    text: Pipeline2.tr("attribute4")
                }                
                AutoSize{

                }
            }
        }
        footbuttons: [{cap: "OK", func: function(){close()}}]
    }

    TWindow{
        id: gridder
        caption: Pipeline2.tr("gridder")
        content: Gridder{
            id: gridder_cld

            name: "demo"
            size: [2, 2]
            com: Component{
                Rectangle{
                    property string name
                    width: parent.width / parent.columns
                    height: parent.height / parent.rows

                    color: "transparent"
                    border.color: "red"
                    Component.onCompleted: {
                        console.log(name)
                    }
                }
            }

            padding: parent.width * 0.05
            width: parent.width * 0.9
            height: parent.height * 0.9
        }
        footbuttons: [
            {
                cap: "6",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: 6})
                }
            },
            {
                cap: "10",
                func: function(){
                    gridder_cld.updateViewCount(10)
                }
            },
            {
                cap: "1",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: 1})
                }
            },
            {
                cap: "5x5",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: [5, 5]})
                }
            }

        ]
    }

    TWindow{
        id: swipe
        caption: Pipeline2.tr("swipeview")
        content: TabView{
            anchors.fill: parent
            Tab{
                title: Pipeline2.tr("Red")
                Rectangle{
                    color: "red"
                }
            }
            Tab{
                title: Pipeline2.tr("Blue")
                Rectangle{
                    color: "blue"
                }
            }
            Tab{
                title: Pipeline2.tr("Swipe")
                Swipe{

                }
            }
        }
    }

    TWindow{
        id: flip
        caption: Pipeline2.tr("flipView")
        content: Flip{
            id: flipview
            anchors.fill: parent
            front: Rectangle{
                anchors.fill: parent
                color: "red"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipUp()
                }
            }
            back: Rectangle{
                anchors.fill: parent
                color: "blue"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipDown()
                }
            }
        }
    }

    TWindow{
        id: nest
        caption: Pipeline2.tr("nestView")
        content: Nest{
            rows: 10
            columns: 10
            anchors.fill: parent
            size: [1, 2, 7, 8, 8, 2, 2, 8, 1, 10]
            Rectangle {
                color: "red"
            }
            Rectangle {
                color: "green"
            }
            Rectangle {
                color: "blue"
            }
            Rectangle {
                color: "yellow"
            }
            Rectangle {
                color: "black"
            }
        }
    }

    TWindow{
        id: matrix
        caption: Pipeline2.tr("matrix")
        content: Matrix{
            id: mtx
            rowcap.text: "hello"
            colcap.text: "world"
            anchors.fill: parent
            content: [[1, 2], [3, 4], [5, 6]]
        }
        footbuttons: [
            {
                cap: "3x3",
                func: function(){
                    mtx.content = [[1, 2, 3], [4, 5, 6], [7, "8", 9]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "2x2",
                func: function(){
                    mtx.content = [[1, 2], [3, 4]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "5x4",
                func: function(){
                    Pipeline2.run("_updateMatrix", {rowcap: "hello2",
                                                    colcap: "world2",
                                                    content: [[1, 2, 3, 4],
                                                              [5, 6, 7, 8],
                                                              [9, 10, 11, 12],
                                                              [13, 14, 15, 16],
                                                              [17, 18, 19, 20]]})
                }
            }

        ]
        Component.onCompleted: {
            Pipeline2.find("_matrixSelected").next(function(aInput){
                console.log("_matrixSelected: " + aInput)
            }, {tag: "manual"}, {vtype: 0})
        }
    }

    TWindow{
        id: treeview
        property var sample: {
            "hello": "world",
            "he": [true, false],
            "hi": {
                "hi1": "hi2",
                "ge": {
                    "too": 3,
                    "heww": {
                        "ll": [3, 3, 4],
                        "dd": "dddd",
                        "ff": false
                    }
                }
            },
            "hi20": true,
            "hello2": "world",
            "he2": [0, {"kao": "gege"}, 1],
            "hi2": [
                {"hello": "world"},
                {"no": [true, false]}
            ],
            "hi22": 3
        }

        caption: Pipeline2.tr("treeview")
        content: Rectangle{
            color: "gray"
            anchors.fill: parent
            TreeNodeView{
                anchors.fill: parent
            }
        }
        footbuttons: [
            {cap: "modify", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "1"], val: true}, {tag: "modifyTreeView"})}},
            {cap: "add", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "2"], type: "add", val: 14}, {tag: "modifyTreeView"})}},
            {cap: "delete", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi"], type: "del"}, {tag: "modifyTreeView"})}},
            {cap: "load", func: function(){
                Pipeline2.run("loadTreeView", {data: sample}, {tag: "testTreeView"})}},
            {cap: "save", func: function(){
                Pipeline2.run("saveTreeView", {}, {tag: "testTreeView"})}},
            {cap: "style", func: function(){
                Pipeline2.run("saveTreeView", {}, {tag: "styleTreeView"})}}
            ]

        function sameObject(aTarget, aRef){
            for (var i in aTarget)
                if (typeof aRef[i] === "object"){
                    if (!sameObject(aTarget[i], aRef[i])){
                        //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                        return false
                    }
                }else if (aTarget[i] !== aRef[i]){
                    //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                    return false
                }
            return true
        }

        Component.onCompleted: {
            Pipeline2.find("loadTreeView")
            .nextL("saveTreeView", {tag: "testTreeView"})
            .next(function(aInput){
                console.assert(sameObject(aInput, sample))
                return {out: [{out: "Pass: save/load TreeView", next: "testSuccess"}]}
            })
            .next("testSuccess")

            Pipeline2.find("treeViewGUIModified")
            .next(function(aInput){
                console.log("treeViewGUIModified;" + aInput["key"] + ";" + aInput["val"] + ";" + aInput["type"])
            })

            Pipeline2.find("saveTreeView")
            .nextL("styleTreeView", {tag: "styleTreeView"})
            .nextL("loadTreeView")
            .nextL("saveTreeView")
            .next(function(aInput){
                return {data: {data: sample, style: aInput}, out: {}}
            })
            .nextL("loadTreeView")
            .next(function(aInput){
                return {out: [{
                            out: {data: aInput["style"], path: "style.json"},
                        }]}
            })
            .nextL("json2stg")
            .nextL("writeJson")
        }
    }

    TWindow{
        id: linechart
        caption: Pipeline2.tr("lineChart")
        content: LineChart{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateLineChart", [20, 30, 100, 125, 30, 10, 12, 30, 50])
                }
            }
        ]
    }

    TWindow{
        id: histogram
        caption: Pipeline2.tr("histogram")
        content: Histogram{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateHistogramGUI", {histogram: [40, 20, 15, 25, 14, 16, 13, 30]})
                }
            }

        ]
    }

    TWindow{
        id: treeview0
        content: TreeView0{
            anchors.fill: parent
            imagePath: {
                "image": 'image.png',
                "text": 'text.png',
                "shape": 'shape.png'
            }
            selectWay: 'background'
            selectColor: 'blue'
            openWay: 'all'
            onSetCurrentSelect: function (select) {
                console.log("treeview0 selected: " + select)
            }
            Component.onCompleted: {
                var tmp = [{
                               "type": "folder",
                               "name": "folder0",
                               "id": "folder0",
                               "children": [{
                                       "type": "page",
                                       "name": "page0",
                                       "id": "page0",
                                       "children": [{
                                               "id": "id0",
                                               "type": "image",
                                               "name": "image0",
                                               "position": [],
                                               "comment": "",
                                               "source": "" //url
                                           }, {
                                               "id": "id1",
                                               "type": "text",
                                               "name": "text0",
                                               "position": [],
                                               "comment": "",
                                               "relative_position": [],
                                               "content": "",
                                               "size": 16,
                                               "color": "green",
                                               "bold": ""
                                           }, {
                                               "id": "id2",
                                               "type": "shape",
                                               "name": "text0",
                                               "position": [],
                                               "comment": "",
                                               "relative_position": [],
                                               "direction": {
                                                   "color": "green",
                                                   "border": {
                                                       "type": "line",
                                                       "color": "red"
                                                   },
                                                   "radius": 30
                                               }
                                           }]
                                   }, {
                                       "type": "folder",
                                       "name": "folder0",
                                       "id": "folder1",
                                       "children": [{
                                               "id": "id3",
                                               "type": "text",
                                               "name": "text0_0",
                                               "range": [0, 0, 50, 50],
                                               "content": ""
                                           }]
                                   }]
                           }]
                buildDefaultTree('folder1', tmp)
            }
        }
    }

    TWindow{
        id: thistogram
        caption: Pipeline2.tr("thistogram")
        content: Column{
            anchors.fill: parent
            spacing: 5
            THistogram{
                id: histo
                width: parent.width
                height: parent.height * 0.9
                //oneThreshold: true
            }
            Track{
                property var intervals: []
                property var histogramdata: []
                property double value: 0.1
                property int idx: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: Pipeline2.tr("Interval") + ":"
                ratio: 0.2

                function updateInterval(){
                    if (intervals.length > 1 && histogramdata.length > 1){
                        histo.drawHistoGram(histogramdata[idx])
                    }
                }

                onIndexChanged: function(aIndex){
                    idx = aIndex
                    updateInterval()
                }
                Component.onCompleted: {
                    Pipeline2.add(function(aInput){
                        intervals = []
                        histogramdata = []
                        interval = Object.keys(aInput["histogram"]).length - 1
                        for (var i in aInput["histogram"]){
                            intervals.push(i)
                            histogramdata.push(aInput["histogram"][i])
                        }
                        updateInterval()
                        return {out: {}}
                    }, {name: "_updateTHistogramGUI"})
                }
            }

            Track{
                property double value: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: Pipeline2.tr("IOU") + ":"
                ratio: 0.2

                onIndexChanged: function(aIndex){
                    value = aIndex / 100
                    //thresholdChanged(maxthreshold.x, minthreshold.x)
                }
            }
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateTHistogramGUI", {histogram: {
                                      5: [10, 20, 15, 30, 25],
                                      10: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12],
                                      20: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12, 12, 10, 20, 42, 30, 15, 25, 20, 21, 23]
                                  }})
                }
            }

        ]
    }

    IO{
        id: io
        name: "io1"
    }
    Camera{
        id: camera
        name: "camera1"
    }
    Progress{

    }
    MsgDialog{

    }
    File{

    }
    ColorSelect{

    }
}
