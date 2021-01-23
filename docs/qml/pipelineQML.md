# Abstract
* a singleton class which provide the same APIs like pipeline for QML  
* the stream in QML only support types of `string`, `number`, `bool`, `array` and `object`  

# API
* **QVariant add(QJSValue aFunc, const QJsonObject& aPipeParam = QJsonObject())**  
    - add a pipe into the line  
    - return the qmlPipe of the specific type  
_sample_:  
```
import Pipeline 1.0
Pipeline.add(function(aInput){
    aInput.out()
})
```  
</br>

* **QVariant find(const QString& aName)**  
    - find the specific qmlPipe by name  
</br>

* **void run(const QString& aName, const QJSValue& aInput, const QString& aTag = "", bool aTransaction = true)**  
    - execute the specific pipe by name  
_sample_:
```
Pipeline.run("pathSelected", {path: ""}, "service1")
```  
</br>

* **void remove(const QString& aName)**  
    - remove the specific pipe by name  
</br>

* **QVariant tr(const QString& aOrigin)**  
    - translate the string to the target string in run time  
</br>

# Test and Demo
test_rea.cpp: test8()