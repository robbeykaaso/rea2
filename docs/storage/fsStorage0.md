# Abstract
the supper and default class for storages, it supports file system as default. its operations are respectively on the thread 10(for reading) and 11(for writing)  

# API Pipe
* **afileRoot + readJson/writeJson**  
    - read/write the QJsonObject  
    - input: stgData<QJsonObject\>  
    - output: stgData<QJsonObject\>  
    - type: pipePartial  
_sample_:
```
    using stgJson = stgData<QJsonObject>;
    pipeline::run<stgJson>("" + "writeJson", stgJson(Json("hello", "world"), "testFS.js"), "service1");
```  
</br>

* **afileRoot + readByteArray/writeByteArray**  
    - read/write the QByteArray  
    - input: stgData<QByteArray\>  
    - output: stgData<QByteArray\>  
    - type: pipePartial  
</br>

* **afileRoot + readDir/writeDir**  
    - read/write the std::vector<QByteArray\> to the directory  
    - input: stgData<std::vector<stgData<QByteArray\>\>\>  
    - output: stgData<std::vector<stgData<QByteArray\>\>\>  
    - type: pipePartial  
</br>

* **afileRoot + deletePath**  
    - delete the directory/file recursively  
    - input: QString  
    - output: QString  
    - type: pipePartial  
</br>

* **afileRoot + listFiles**  
    - list the files of the directory not recursively  
    - input: stgData<std::vector<QString\>\>  
    - output: stgData<std::vector<QString\>\>  
    - type: pipePartial  
</br>

# Test and Demo
storage0.cpp: testStorage(const QString& aRoot = "")

# Reference
[stgData](stgData.md)  
