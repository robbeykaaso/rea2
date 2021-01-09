# Abstract
only execute the next specific pipe which satisfies some condition  
```
             -> pipe3  //we will only need execute pipe3 after pipePartial sometimes
pipePartial
             x-> pipe4
```
</br>

# Sample
**add and connect pipe:**  
```
pipeline::add<QJsonObject, pipePartial>([](stream<QJsonObject>* aInput){  //c++
    aInput.out();
}, Json("name", "WCS2SCS"))
->next("doSomething", Json("tag", "service1"))  //the previous pipe of the pipePartial whose connect parameter of "tag" is "service1" will activate "doSomething"

Pipeline2.add(function(aInput){  //qml
    return {out: {}}
}, {name: "WCS2SCS_", type: "Partial"})
```  

**make it work:**  
```
pipeline::run<QJsonObject>("WCS2SCS", QJsonObject(), Json("tag", "service1"))  //case 1

pipeline::find("beforeWCS2SCS")->next("WCS2SCS", Json("tag", "service1"))  //case 2
```  
</br>

# Test and Demo
src/reactive2.cpp: test6()  
</br>

# Reference
[pipe](pipe.md)