import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import Pipeline2 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 700

    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            console.assert(aInput.data()["test8"] === "test8")
            console.assert(aInput.varData("test8_var", "string") === "test8_var")
            aInput.out(aInput.data(), "test8_0", {}, false).var("test8_var", "test8_var_");
        }, {name: "test8_"})

        Pipeline2.add(function(aInput){
            console.assert(aInput.data()["56"] === "56")
            aInput.out("Pass: test4_")
        }, {name: "test4_1"}).nextL("testFail")

        Pipeline2.add(null, {name: "test7_", type: "Buffer", count: 2})
        .next(function(aInput){
            var dt = aInput.data()
            console.assert(dt["0"]["hello"] === "world")
            console.assert(dt["00"]["hello"] === "world2")
            aInput.out("Pass: test7_")
        })
        .nextL("testSuccess")
    }
}
