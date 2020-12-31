#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QApplication>
#include <QResource>
#include <QTranslator>
#include <QIcon>
#include <iostream>
#include <Windows.h>
#include <QWindow>
#include <DbgHelp.h>
#include "reactive2.h"
#include "reactive2QML.h"

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    //创建 Dump 文件
    HANDLE hDumpFile = CreateFile("crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        //Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;

        //写入Dump文件内容
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    }
    //rea::pipeline::run<double>("crashDump", 0);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    SetUnhandledExceptionFilter(ApplicationCrashHandler);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setOrganizationName("somename");
    app.setOrganizationDomain("somename");

    qmlRegisterSingletonType<rea::pipelineQML>("Pipeline2", 1, 0, "Pipeline2", &rea::pipelineQML::qmlInstance);
    qmlRegisterType<rea::qmlPipe>("Pipe2", 1, 0, "Pipe2");
    qmlRegisterType<rea::qmlStream>("Stream2", 1, 0, "Stream2");

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->setData(rea::Json("hello", "world"))->out();
    }, rea::Json("name", "test"));
    QQmlApplicationEngine engine;
    rea::pipeline::instance()->engine = &engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    rea::pipeline::run<QJsonObject>("test", QJsonObject());
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
