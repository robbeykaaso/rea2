# Abstract
* a class which wraps the pipe in c++ for QML
* the pipes are all managed in pipeline in c++

# API
* **QJSValue func WRITE setFunc**  
    - set the callback of the pipe  
_sample_:
```
    import Pipe2 1.0
    Pipe{
        func: function(aInput){
            return {data: aInput, out: {}}  // data will format the original stream data; if out is object, it is equal to "out(QJsonObject)", else if is array, each item's format as {out: aInput, next: "nextPipe", param: ""} is equal to "out(S, const QString, const QJsonObject)"
        }
    }
```  
</br>

* **QJsonObject param WRITE setParam**  
    - set the parameter of the pipe    
_sample_:
```
    import Pipe2 1.0
    Pipe{
        param: {
            "name": "switchLogType",  //notice: 1. the key must be written with ""; 2. thread not work, it could only be executed on the main thread
            "type": "Partial",  //Partial, Local, Buffer, Delegate
            "param": {

            },
            "vtype": {} //{}, [], 0, "", true; only support 5 types above
        }
    }
```  
</br>

* **QVariant next(const QString& aEventName, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe after this  
    - return the next uiPipe  
_sample_:
```
    import Pipeline2 1.0
    Pipeline.find("selectPath")
            .next("doSomething", {previous: "hello"})
```  
</br>

* **QVariant next(QJSValue aNext, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject())**  
    - connect a pipe after this  
    - return the next uiPipe  
_sample_:
```
    import Pipeline2 1.0
    Pipeline.find("selectPath")
            .next(function(aInput){
                return {data: aInput, out: {}}
            }, {previous: "hello"}, {name: "doSomething"})
```  
</br>

* **QVariant nextEx(QVariant aNext, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe after this  
    - return the next uiPipe  
_sample_:
```
    import Pipe2 1.0
    import Pipeline2 1.0
    Pipe{
        id: doSomething
        func: function(aInput){
            return {data: aInput, out: {}}
        }
    }
    Pipeline.find("selectPath")
            .nextEx(doSomething, {previous: "hello"})
```  
</br>

* **QVariant previous(const QString& aEventName, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe before this  
    - return the previous uiPipe  
</br>

* **QVariant previous(QJSValue aPrevious, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject())**  
    - connect a pipe before this  
    - return the previous uiPipe  
</br>

* **QVariant previousEx(QVariant aPrevious, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe before this  
    - return the previous uiPipe  
</br>

* **QVariant nextB(QJSValue aNext, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject())**  
    - connect a branch pipe  
    - return self  
</br>

* **QVariant nextB(const QString& aEventName, const QJsonObject& aParam = QJsonObject())**  
    - connect a branch pipe  
    - return self  
</br>

* **QVariant nextL(const QString& aName, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject());**  
    - connect a pipeLocal for the specific pipe  
</br>