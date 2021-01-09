# Abstract
a message dialog component for qml  

# API Pipe
* **popMessage**  
    - popup the message dialog and show message  
    - input is json  
    - it is pipeDelegate for "messagePoped"  
_sample_:  
```
Pipeline2.run("popMessage", {title: "hello4", text: "world", tag: "nextPipe"}) //the tag of "messagedPoped"
```  
</br>

* **messagePoped**  
    - if the result is accepted, this pipe will be activate  
    - output is the input of `popMessage`  
    - it is pipePartial  
</br>

# Test and Demo
main.qml: qsTr("MsgDialog")  
</br>