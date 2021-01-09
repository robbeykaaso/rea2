﻿import QtQuick 2.0
import "../Basic"
import Pipeline 1.0

List0 {
    tr: Pipeline.tr
    onSelected: {
        Pipeline.run(name + "_listViewSelected", [], selectSuffix + "manual")
    }

    Component.onCompleted: {
        Pipeline.add(function(aInput){
            aInput.setData(selects).out()
        }, {name: name + "_listViewSelected", type: "Partial", vtype: "array"})

        Pipeline.add(function(aInput){
            updateList(aInput.data())
            aInput.out()
        }, {name: name + "_updateListView"})
    }
}
