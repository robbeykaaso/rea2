# Abstract
* a container and interface for passing data between pipes  
</br>

# API
* **T data()**  
    - return self data  
</br>

* **stream<T\>* setData(T aData)**  
    - set self data  
    - return self  
</br>

* **stream<T\>* out(const QJsonObject& aParam = QJsonObject())**  
    - pass the stream to all its next pipes by default  
    - `aParam` is the param of the stream which will format the pipe connected param  
    - return self  
_example_:  
```
    out(); // pass the stream to all its next pipes and run them by alphabet sequence
```  
</br>

* **stream<S\>* out(S aOutput, const QString& aNext = "", const QJsonObject& aParam = QJsonObject(), bool aShareCache = true)**  
    - pass the stream to its next specific pipe by `aNext`  
    - if `aNext` equals to "", it will only run all the anonymous nexts  
    - if `aNext` doesn't equal to "" and isn't connected, it will try to run the `localPipe` with the same name. if there still not exists the same name `localPipe`, it will try to run the specific pipe directly on whole range  
    - this function will format the stragety of the above API  
    - `aShareCache` denotes whether the out stream to share the cache with self  
    - return the specific out stream  
_example_:
```
    out<int>(5, "pipe1");  // pass the stream to pipe1 and run pipe1 on next
    out<int>(5, "");  // pass the stream to all next anonymous pipe whose name is "" and run them
```  
</br>

* **stream<T\>* cache(S aData, int aIndex = - 1)**  
    - cache some data  
    - if `aIndex` is in the cache range, it will replace the old cache  
    - return self    
</br>

* **S cacheData(int aIndex)**  
    - return the specific cache by index  
</br>