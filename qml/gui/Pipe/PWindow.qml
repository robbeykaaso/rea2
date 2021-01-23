﻿import QtQuick 2.0
import "../Basic"
import Pipeline 1.0

PWindow0 {
    property string service_tag
    tr: Pipeline.tr

    onAccept: {
        Pipeline.run("_objectNew", outputModel(), service_tag, false)
        close()
    }

    onReject: {
        Pipeline.run("_objectNew", {}, service_tag, false)
        close()
    }

    Component.onCompleted: {
        Pipeline.add(function(aInput){
            aInput.out()
        }, {name: "_objectNew", type: "Partial"})

        Pipeline.add(function(aInput){
            service_tag = aInput.tag()
            showModel(aInput.data())
        }, {name: "_newObject", type: "Delegate", delegate: "_objectNew"})
    }
}