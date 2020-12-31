import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import Pipeline2 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 700

    Component.onCompleted: {
        var tmp = {}
        Pipeline2.find("test").next(function(aInput){
            console.log(aInput.data()["hello"])
            aInput.setData({"hello2": "world2"}).out({"ff": "dd"})
        })
    }
}
