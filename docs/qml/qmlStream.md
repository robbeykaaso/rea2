# Abstract  
* the stream class in qml  

# API
* **QJSValue data()**  
    - return self data  
</br>

* **QVariant setData(QJSValue aData)**  
    - set self data  
    - return self  
</br>

* **QVariant out(const QString& aTag = "")**  
    - pass the stream to all its next pipes by default  
    - `aTag` decides which pipe of nexts will be executed if this is a pipePartial  
    - return self  
</br>

* **QVariant outs(QJSValue aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true)**  
    - pass the stream to its next specific pipe by `aNext`  
    - if `aNext` equals to "", it will only run all the nexts  
    - if `aNext` doesn't equal to "", it will try to run the `localPipe` with the same name. if there still not exists the same name `localPipe`, it will try to run the specific pipe directly on whole range  
    - this function will format the stragety of `out`  
    - `aShareCache` denotes whether the out stream to share the scope cache with self  
    - return this out stream  
</br>

* **QVariant outsB(QJSValue aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true)**  
    - work like `outs`  
    - return this stream  
</br>

* **noOut()**  
    - clear the out streams flowing the next pipes  
</br>

* **QVariant var(const QString& aName, QJSValue aData)**  
    - cache some data by name  
    - if `aName` has been existed in the scope cache, it will replace the old cache  
    - return this stream  
</br>

* **QJSValue varData(const QString& aName, const QString& aType = "object")**  
    - `aType` is the type of the variable
    - return the specific cache data by its name  
</br>

* **QString tag()**  
    - return the tag of this stream  
</br>

* **void fail()**  
    - tag fail status for this transaction if exists  
</br>

* **void log(const QString& aLog)**  
    - add a log to this transaction if exists
</br>