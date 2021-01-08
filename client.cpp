#include "client.h"
#include "reaC++.h"
#include <QJsonDocument>
#include <QNetworkProxy>

namespace rea {

normalClient::normalClient(const QJsonObject& aConfig) : QObject()
{
    connect(&ssdp_, SIGNAL(FoundServer(QString, QString, QString)), this, SLOT(ServerFound(QString, QString, QString)));
    connect(&search_timer_, &QTimer::timeout, [this](){
        ssdp_.StartDiscovery();
        rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", "finding server..."));
    });

    connect(&m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(ReceiveState(QAbstractSocket::SocketState)));

    rea::pipeline::add<QJsonObject, rea::pipeDelegate>([this](rea::stream<QJsonObject>* aInput){
        if (!m_valid)
            return;
        auto dt = aInput->data();
        //rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", dt.value("type")));

        m_socket.write(QJsonDocument(dt).toJson(QJsonDocument::Compact));
        m_socket.flush();  //waitForBytesWritten
        while (m_socket.bytesToWrite() > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }, rea::Json("name", "callServer", "delegate", "receiveFromServer"));

    rea::pipeline::add<QJsonObject, rea::pipePartial>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "receiveFromServer"));

    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        m_socket.abort();
        auto dt = aInput->data();
        if (dt.contains("ip") && dt.contains("port") && dt.contains("id"))
            ServerFound(dt.value("ip").toString(), dt.value("port").toString(), dt.value("id").toString());
        else
            tryConnectServer();
        aInput->out();
    }, rea::Json("name", "tryLinkServer"));

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "clientBoardcast"));
}

normalClient::~normalClient(){
    m_socket.disconnectFromHost();
    //m_socket.abort();
    search_timer_.stop();
}

void normalClient::RegistOnStateChanged(std::function<void(QAbstractSocket::SocketState)> aEvent){
    connect(&m_socket, &QAbstractSocket::stateChanged, aEvent);
}

void normalClient::tryConnectServer()
{
    if (!search_timer_.isActive())
        search_timer_.start(1000);
}

#include <QNetworkProxy>
#include <iostream>
void normalClient::ServerFound(QString aIP, QString aPort, QString aID)
{
    m_detail = aIP + ":" + aPort + ":" + aID;
    rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", "connect socket", "detail", m_detail));
    m_socket.abort();
    m_socket.setProxy(QNetworkProxy::NoProxy);
    //m_socket.setSocketOption()
    m_socket.connectToHost(aIP, aPort.toInt());
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(ReceiveMessage()));
}

void normalClient::ReceiveState(QAbstractSocket::SocketState aState){
    if (aState == QAbstractSocket::SocketState::ConnectedState){
        m_valid = true;
        rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", "socket is connected", "detail", m_detail));
        connected();
        search_timer_.stop();
    }else if (aState == QAbstractSocket::SocketState::UnconnectedState){
        m_valid = false;
        rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", "socket is unconnected"));
        disconnect(&m_socket, SIGNAL(readyRead()), this, SLOT(ReceiveMessage()));
        rea::pipeline::run<QJsonObject>("tryLinkServer", QJsonObject());
    }
}

void normalClient::ReceiveMessage()
{
    QByteArray qba = m_socket.readAll();
    QString ss = QVariant(qba).toString();
    auto strs = rea::parseJsons(ss);
    for (auto msg : strs){
        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
        auto res = doc.object();
        rea::pipeline::run<QJsonObject>("clientBoardcast", rea::Json("value", "receive from server: " + res.value("type").toString()));
        rea::pipeline::run<QJsonObject>("receiveFromServer", res, res.value("type").toString(), false);
    }
}

}
