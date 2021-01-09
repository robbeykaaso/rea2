import QtQuick 2.0
import "../Basic"
import Pipeline 1.0

PWindow0 {
    tr: Pipeline.tr

    onAccept: {
        Pipeline.run("_objectNew", outputModel(), service_tag, false)
        close()
    }

    onReject: {
        close()
    }

    Component.onCompleted: {
        Pipeline.add(function(aInput){
            aInput.out()
        }, {name: "_objectNew", type: "Partial"})

        Pipeline.add(function(aInput){
            showModel(aInput.data())
        }, {name: "_newObject", type: "Delegate", delegate: "_objectNew"})
    }
}
