import QtQuick 2.12
import QtQuick.Dialogs 1.2
import Pipeline 1.0

MessageDialog {
    property bool decided: false
    property string service_tag
    property var cache
    title: "Hello"
    text: "World"

    onAccepted: {
        if (!decided){
            decided = true
            Pipeline.run("messagePoped", {}, service_tag)
        }
    }

    Component.onCompleted: {
        Pipeline.add(function(aInput){
            cache = aInput.data()
            title = cache["title"]
            text = cache["text"]
            service_tag = cache["tag"] || ""
            decided = false
            open()
        }, {name: "popMessage", type: "Delegate", delegate: "messagePoped"})

        Pipeline.add(function(aInput){
            aInput.setData(cache).out()
        }, {name: "messagePoped", type: "Partial"})
    }
}
