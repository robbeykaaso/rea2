#include "reaC++.h"
#include "server.h"
#include "client.h"

const auto protocal_test = "test";
const auto protocal = rea::Json(protocal_test,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "hello"
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "world"
                                                     ))
                                    );

void testSocket(){
    using namespace rea;

    pipeline::find("receiveFromClient")  //server end
        ->nextF<clientMessage>([](rea::stream<clientMessage>* aInput){
            auto dt = aInput->data();
            assert(dt.value("key") == "hello");
            aInput->setData(protocal.value(protocal_test).toObject().value("res").toObject())->out();
        }, protocal_test)
        ->next("callClient");

    pipeline::find("clientBoardcast")  //client end
        ->nextF<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            auto val = aInput->data().value("value");
            if (val == "socket is connected")
                aInput->outs<QJsonObject>(protocal.value(protocal_test).toObject().value("req").toObject(), "callServer");
            else
                aInput->log("cast value: " + val.toString());
        })
        ->next("callServer")
        ->nextF<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            assert(dt.value("key") == "world");
            aInput->outs<QString>("Pass: testSocket ", "testSuccess");
        }, protocal_test)
        ->next("testSuccess");

    pipeline::run<QJsonObject>("tryLinkServer", rea::Json("ip", "127.0.0.1",
                                                          "port", "8081",
                                                          "id", "hello"));
}

static rea::regPip<QJsonObject> unit_test([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("tcp").toBool()){
        aInput->out();
        return;
    }

    static rea::normalServer sev(rea::Json("protocal", protocal));
    static rea::normalClient clt;
    testSocket();
    aInput->out();
}, QJsonObject(), "unitTest");
