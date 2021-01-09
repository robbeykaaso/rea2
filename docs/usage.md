# Abstract
this page describe how to use the framework in your codes  

# Steps
**0**: initialize the reactive engine only once  
_sample_:
```
    QQmlApplicationEngine engine;
    pipeline::run<QQmlApplicationEngine*>("regQML", &engine);
```    
</br>

**1**: add pipes and build the pipeline graph by connecting pipes  
_sample_:
```
    pipeline::add<QJsonObject>(stream<QJsonObject>* aInput){
        aInput->setData(dst::Json("hello", "world"));  //set the data of the stream
        aInput->out(); //output the stream and run the nexts after this pipe
    }, Json("name", "pipe0", "thread", 2))  //add pipe0 in c++
    .next("pipe1")

    Pipeline.add(function(aInput){  //add a pipe1 and connect it after the pipe0
        return {data: aInput, out: {}} //output the stream and run the nexts after this pipe
    }, {name: "pipe1"})
```  
</br>

**2**: run the pipeline  
_sample_:  
```
    pipeline::run<QJsonObject>("pipe0", QJsonObject())  //run the pipeline: pipe0 -> pipe1 in c++

    Pipeline.run("pipe0") //run the pipeline: pipe0 -> pipe1 in qml
```  
</br>