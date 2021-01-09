# Abstract
a super class for customized qsgBoardPlugin. it provides all the manual mouse events for differet business

# Usage
the provided plugins includes: "transform", "select", "drawfree", "drawRect", "drawEllipse", "editNode"  
_sample_:  
```
//register
static rea::regPip<QJsonObject, rea::pipePartial> create_qsgboardplugin_transform([](rea::stream<QJsonObject>* aInput){
   aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginTransform>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_transform"));  //transform is the plugin type

//install
plugins: [{type: "transform"}]  //install it as a plugin of qsgBoard
```  
</br>

# API Pipe
* **name + _moveShapes**  
move the specific shapes  
_sample_:  
```
Pipeline2.run("testbrd_moveShapes", {shapes: ["shp_0", "shp_1"], del: [500, 500]})
```  
</br>

# Test and Demo
main.qml: qsTr("free"), qsTr("select"), qsTr("editNode")  
</br>