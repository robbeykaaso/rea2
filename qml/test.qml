import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Pipeline 1.0

Window {
    width: 400
    height: 200
    visible: true

    Row{
        anchors.fill: parent
        spacing: 5
        Button{
            width: 60
            height: 30
            anchors.verticalCenter: parent.verticalCenter
            text: "UnitTest"
            onClicked: Pipeline.run("doUnitTest", 0, "", false)
        }
        Button{
            width: 60
            height: 30
            anchors.verticalCenter: parent.verticalCenter
            text: "Routine"
            onClicked: Pipeline.run("logRoutine", 0, "", false)
        }
    }

    Component.onCompleted: {
        Pipeline.add(function(aInput){
            console.assert(aInput.data()["test8"] === "test8")
            console.assert(aInput.varData("test8_var", "string") === "test8_var")
            aInput.outs(aInput.data(), "test8_0", {}, false).var("test8_var", "test8_var_");
        }, {name: "test8_"})

        Pipeline.add(function(aInput){
            console.assert(aInput.data()["56"] === "56")
            aInput.outs("Pass: test4_")
        }, {name: "test4_1"}).nextL("testFail")

        Pipeline.add(function(aInput){
            aInput.outs({"hello": "world"})
            aInput.outs({"hello": "world2"})
        }, {name: "test7_"})
        .nextP(Pipeline.add(null, {type: "Buffer", count: 2}))
        .next(function(aInput){
            var dt = aInput.data()
            console.assert(dt[0]["hello"] === "world")
            console.assert(dt[1]["hello"] === "world2")
            aInput.outs("Pass: test7_")
        }, {}, {vtype: []})
        .nextL("testSuccess")
    }
}
