#include "modbusMaster.h"
#include <QSerialPort>

namespace rea {

modBusMaster::modBusMaster(const QJsonObject& aConfig) : QObject(){

    connect(&m_modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus error: " + m_modbus.errorString()));
        // statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    connect(&m_modbus, &QModbusClient::stateChanged, [](QModbusDevice::State aState){
        if (aState == QModbusDevice::ConnectedState)
            rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus is connected"));
        else if (aState == QModbusDevice::UnconnectedState)
            rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus is unconnected"));
    });

    for (auto i : aConfig.keys())
        m_modbus.setConnectionParameter(i.toInt(), aConfig.value(i));

    m_modbus.connectDevice();

    rea::pipeline::add<QJsonObject, pipePartial>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto req = QModbusRequest(QModbusRequest::FunctionCode(dt.value("func").toInt()),
                                  QByteArray::fromHex(dt.value("payload").toString().toUtf8()));
        sendRequest(req, dt.value("server").toInt(1), aInput);
    }, rea::Json("name", "callSlave"));

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "modbusBoardcast"));
}

void modBusMaster::sendRequest(const QModbusRequest& aRequest, int aServer, rea::stream<QJsonObject>* aInput) {
    QByteArray ret;
    if (auto* reply = m_modbus.sendRawRequest(aRequest, aServer)) {
        if (!reply->isFinished()) {
            QEventLoop loop;
            auto connection = connect(
                reply, &QModbusReply::finished, this, [&ret, this, reply, &loop, aInput]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        aInput->log("modbus protocal error: " + m_modbus.errorString());
                    } else if (reply->error() != QModbusDevice::NoError) {
                        auto test = m_modbus.errorString();
                        aInput->log("modbus reply error: " + m_modbus.errorString());
                    } else {
                        ret = reply->rawResult().data().toHex();
                    }
                    reply->deleteLater();
                    loop.quit();
                });

            loop.exec();
            QObject::disconnect(connection);
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else
        aInput->log("modbus send error: " + m_modbus.errorString());
    aInput->outs<QByteArray>(ret);
}

void modBusMaster::close() {
    if (m_modbus.state() == QModbusDevice::ConnectedState)
        m_modbus.disconnectDevice();
}

}
