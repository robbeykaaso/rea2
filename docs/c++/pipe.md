# Abstract
* a step in the pipeline  
* a stream is passed by many pipes and each pipe may do one thing on one thread, such as  
```
input -> pipe1(calculate on thread2) -> pipe2(store data on thread1) -> pipe3(render GUI on thread0) -> output
```  
</br>

# API
* **QString actName()**  
    - the actual name of pipe  
</br>

* **pipe0* next(const QString& aName / pipe0\* aNext, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe after self  
    - return the next pipe  
    - the next pipe will be executed and get the stream after self is executed  
    - `aParam` is the pipe connected parameter  
_sample_:
```
next("doSomething", dst::Json("tag", "id0"))  
```  
</br>

* **pipe0* nextF(pipeFunc<T> aNextFunc, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject())**  
    - `aNextFunc` is the lambda function of the pipe  
    - the function is like `next`  
</br>

* **pipe0* previous(const QString& aName / pipe0\* aPrevious, const QJsonObject& aParam = QJsonObject())**  
    - connect a pipe before self  
    - return the previous pipe  
    - self will be executed and get the stream after the previous pipe is executed  
    - `aParam` is the pipe connected parameter  
</br>

* **pipe0* nextBEx(int aReturn, const QString& aName / pipe0\* aNext, const QJsonObject& aParam, Args&&... rest)**  
    - connect a next branch pipe  
    - return the specified pipe by `aReturn`  
    - the branch pipeline will be executed and get the stream after self is executed
_sample_:
```
nextB(1, "step1", QJsonObject(), "step2", QJsonObject())  //this will return the "step1" pipe
```  
</br>

* **pipe0* nextB(const QString& aName / pipe0\* aNext, const QJsonObject& aParam = QJsonObject())**  
    - connect a next branch pipe  
    - return self  
_sample_:
```
nextB("doSomething", dst::Json("previous", "id0"))  
```  
</br>

* **void removeNext(const QString& aName)**  
    - remove the next pipe  
</br>

# Child classes
_pipeFuture, [pipeLocal](pipeLocal.md), [pipeDelegate](pipeDelegate.md), [pipePartial](pipePartial.md), [pipeBuffer](pipeBuffer.md)..._