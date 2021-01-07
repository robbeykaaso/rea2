#include "Python.h"
#include <iostream>
#include <string>

//https://www.cnblogs.com/findumars/p/6142330.html
void HelloPy(){
    Py_Initialize();
    PyRun_SimpleString("print ('hello')");
    Py_Finalize();
}

void simple_cnn(){
    Py_Initialize();

    std::string chdir_cmd = "sys.path.append('./plugin/')";
    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);

    PyObject* moduleName = PyUnicode_FromString("simple_cnn"); //模块名，不是文件名
    PyObject* pModule = PyImport_Import(moduleName);
    if (!pModule) // 加载模块失败
    {
        std::cout << "[ERROR] Python get module failed." << std::endl;
        return;
    }
    std::cout << "[INFO] Python get module succeed." << std::endl;

    // 加载函数
    PyObject* pv = PyObject_GetAttrString(pModule, "test");
    if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
    {
        std::cout << "[ERROR] Can't find function (test)" << std::endl;
        return;
    }
    std::cout << "[INFO] Get function (test) succeed." << std::endl;

    // 设置参数
    PyObject* args = PyTuple_New(2);   // 2个参数
    // 调用函数
    PyObject* pRet = PyObject_CallObject(pv, args);

    Py_Finalize();
}
