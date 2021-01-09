# Abstract
* a singleton class which provide the same APIs like pipeline for QML  
* the stream in QML only support QJsonObject

# API
* **QVariant add(QJSValue aFunc, const QJsonObject& aPipeParam = QJsonObject())**  
    - add a pipe into the pipeManager  
    - return the uiPipe of the specific type  
_sample_:  
```
import Pipeline2 1.0
Pipeline.add(function(aInput){
    return {data: aInput, out: {}}
})
```  
</br>

* **QVariant find(const QString& aName)**  
    - find the specific qmlPipe by name  
</br>

* **void run(const QString& aName, const QJSValue& aInput, const QJsonObject& aParam = QJsonObject())**  
    - execute the specific pipe by name  
_sample_:
```
Pipeline.run("pathSelected", 
             {path: pth}, //ainput could be object, array, string, number and bool
             {tag: aInput["tag"]})
```  
</br>

* **void remove(const QString& aName)**  
    - remove the specific pipe by name  
</br>

# Test and Demo
src/reactive2.cpp: test8()