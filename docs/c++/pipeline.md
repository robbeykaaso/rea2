# Abstract
* a singleton class which manages all the pipe life  
* let the app developer add and use their own pipes  
</br>

# API
* **pipe0\* add<T, P, F, S\>(pipeFunc/QJSValue aFunc, const QJsonObject& aParam = QJsonObject())**  
    - add a pipe into the pipeline  
    - return the pipe of the specific type  
    - if there is a pipe with the same name, the original pipe will be removed  
_sample_:
```
add<int, pipe, pipeFunc<int>, pipeFunc<int>>([](stream<int>* aInput){  //the second parameter is pipe as default, other choices can be such as pipePartial, pipeLocal, pipeDelegate, pipeBuffer; the third and the forth parameter must be the same, and should be only in pipeFunc<T> and QJSValue, the default is pipeFunc<T>
    return aInput;
}, Json("name", "pipe0",  //the name of the pipe, if there is no name, it will be regarded as an anonymous pipe
             "thread", 2,  //which thread the pipe will be executed on. if it is 0, it will be executed on the current thread
             "replace", false, //the pipe will reserve the old pipe's next pipes if it exists
             "param", Json()  //the parameter of the pipe
             ))
```  
</br>

* **pipe0\* find(const QString& aName, bool needFuture = true)**  
    - find the specific pipe by name  
    - `needFuture` is used for the case that the pipe is not existed now, but will be existed in the future  
</br>

* **void run<T>(const QString& aName, T aInput, const QJsonObject& aParam = QJsonObject())**  
    - execute the specific pipe by name  
_sample_:  
```
run<int>("pipe0", 0, Json("tag", "pipe0"))
```  
</br>

* **void remove(const QString& aName)**  
    - remove the specific pipe by name  
</br>

# Test and Demo
src/reactive2.cpp: test1(), test2(), test3()  
</br>

