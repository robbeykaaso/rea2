# Abstract
a model for qsgBoard, it will worked by the pipe apis of qsgBoard

# Model format
the same key in the object attributes will be prior to the whole attributes  
_sample_:
```
{
    width: 600,  //the width of the reference world coordinate system
    height: 600,  //the height of the reference world coordinate system
    arrow: {
        visible: true,  //whether show arrow on the shapeObject
        pole: true  //whether double arrows on the polyObject
    },
    face: 200,  //the fill opacity of the shapeObject
    text: {
        visible: true,  //whether show text on the shapeObject
        size: [100, 50],  //the text width and height
        location: "bottom"  //the text location: "bottom", "middle", "top"
    },
    transform: [1, 0, 0, 0, 1, 0, 0, 0, 1],  //the world transformation matrix
    color: "blue",  //the color of the shapeObject, include text, arrow, face and shape itself
    objects: {  //the object is saved by object name and object attributes
        img_2: {  //the fps will be affected by the scale of the data for the render feature of QSG. developers should control the scale of the input data to satisfy the specific performence
            type: "image",  //the type of imageObject
            path: "c:/xxx/xx.png",  //the path of the image in the imagePool or file system
            range: [0, 0, 400, 400],  //the view port of the image, the default is the boundbox of it
            caption: "Text",  //the text value
            color: "green"  //the text color
        },
        shp_0: {
            type: "poly",  //the type of polyObject which inherits from shapeObject
            points: [[50, 50, 200, 200, 200, 50, 50, 50], [80, 70, 120, 100, 120, 70, 80, 70]],  //the point coordinates: x, y, x, y; the first array is the outline, others are holes
            color: "red",  //as above in the whole attributes
            width: 3,  //the line width
            caption: "hello",  //the text value
            style: "dash", //the line style
            face: 50  //as above in the whole attributes
            text: {
                visible: true,  //as above in the whole attributes
                size: [100, 60]  //as above in the whole attributes
            }
        },
        shp_1: {
            type: "ellipse",  //the type of ellipse which inherits from shapeObject
            center: [400, 400],  //the center
            radius: [300, 200],  //the x, y radius
            width: 5,  //as above in the polyObject
            style: "dash", //as above in the polyObject
            ccw: true,  //whether is counterclockwise
            angle: 30,  //the rotate angle
            caption: "world"  //as above in the polyObject
        }
    }
}
```  
* Notice  
    - <font color="red">the objects will be rendered by Alphabetic order as a z-order</font><br />  
</br>

# Modification format
* use "updateQSGAttr" or "updateQSGAttrs" api to update specific attribute of the qsgModel  
_sample_:
```
{
    obj: "shp_0", //the object name, if there is no "obj", it will be for the whole attributes
    key: ["text", "visible"],  //the attribute key chain
    val: true,  //the new attribute value
    force: false,  //whether to force updating the attributes which have the same values
    id: "" //if it declares the id and which is not the same with that of the qsmModel, the update will not be executed
}
```  
</br>

* specific format  
_sample_:  
```
{
    key: ["transform"],
    type: "zoom",  //zoom WCS
    dir: 1,  //zoom direction
    center: [0, 0]  //zoom center
}

{
    key: ["transform"],
    type: "move", //move WCS
    del: [200, 200]  //move delta
}

{
    key: ["objects"],
    type: "add",  //add object
    tar: "shp_3",  //object name
    val: {  //object attribute
        type: "poly",
        points: [[500, 300, 700, 300, 700, 500, 500, 300]],
        color: "pink",
        caption: "new_obj"
    }
}

{
    key: ["objects"],
    type: "del",  //delete object
    tar: "shp_3"  //object name
}
```  
</br>

# Test and Demo
test.qml: qsTr("qsgShow"); test_qsg.cpp  
</br>

# Reference
[qsgBoard](qsgBoard.md)  