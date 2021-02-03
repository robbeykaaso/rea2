#include "storage0.h"
#include "reaC++.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QBuffer>
#include <QVector>

namespace rea {

void fsStorage0::checkPath(const QString &aPath){
    auto dirs = aPath.split("/");
    QDir dir;
    QString origin = "";
    for (int i = 0; i < dirs.size() - 1; ++i){
        if (i > 0)
            origin += "/";
        origin += dirs[i];
        if (!dir.exists(origin))
            dir.mkdir(origin);
    }
}

void fsStorage0::writeJson(const QString& aPath, const QJsonObject& aData){
    writeByteArray(aPath, QJsonDocument(aData).toJson());
}

QString fsStorage0::stgRoot(const QString& aPath){
    if (m_root == "")
        return aPath;
    else
        return m_root + "/" + aPath;
}

void fsStorage0::writeQImage(const QString& aPath, const QImage& aData){
    auto pth = stgRoot(aPath);
    checkPath(pth);
    aData.save(pth);
}

void fsStorage0::writeByteArray(const QString& aPath, const QByteArray& aData){
    auto pth = stgRoot(aPath);
    checkPath(pth);
    QFile fl(pth);
    if (fl.open(QFile::WriteOnly)){
        fl.write(aData);
        fl.close();
    }else{
//        qDebug() << "write bytearray: " + aPath + " failed!\n";
    }
}

QJsonObject fsStorage0::readJson(const QString& aPath){
    QJsonObject ret;
    QFile fl(stgRoot(aPath));
    if (fl.open(QFile::ReadOnly)){
        QJsonDocument doc = QJsonDocument::fromJson(fl.readAll());
        ret = doc.object();
        fl.close();
    }else{
//        qDebug() << "read json: " + aPath + " failed!\n";
    }
    return ret;
}

QImage fsStorage0::readQImage(const QString& aPath){
    return QImage(stgRoot(aPath));
}

QByteArray fsStorage0::readByteArray(const QString& aPath){
    QByteArray ret;
    QFile fl(stgRoot(aPath));
    if (fl.open(QFile::ReadOnly)){
        ret = fl.readAll();
        fl.close();
    }else{
//        qDebug() << "read bytearray: " + aPath + " failed!\n";
    }
    return ret;
}

void fsStorage0::deletePath(const QString& aPath){
    if (aPath.indexOf(".") >= 0)
        QDir().remove(stgRoot(aPath));
    else
        QDir(stgRoot(aPath)).removeRecursively();
}

std::vector<QString> fsStorage0::getFileList(const QString& aPath){
    std::vector<QString> ret;
    auto pth = stgRoot(aPath);
    QDir dir(pth);
    auto lst = dir.entryList();
    for (auto i : lst)
        if (i != "." && i != ".."){
            if (i.indexOf(".") >= 0)
                ret.push_back(aPath + "/" + i);
            else{
                auto clst = getFileList(aPath + "/" + i);
                ret.insert(ret.end(), clst.begin(), clst.end());
            }
        }
    return ret;
}

fsStorage0::fsStorage0(const QString& aRoot){
    m_root = aRoot;

    REGREADSTORAGE(Json);
    REGREADSTORAGE(ByteArray);
    REGREADSTORAGE(QImage);
    REGWRITESTORAGE(Json);
    REGWRITESTORAGE(ByteArray);
    REGWRITESTORAGE(QImage);

    rea::pipeline::add<stgVector<stgByteArray>, rea::pipePartial>([this](rea::stream<stgVector<stgByteArray>>* aInput){
        auto dt = aInput->data();
        auto lst0 = getFileList(dt);
        for (auto i : lst0)
            dt.getData().push_back(stgByteArray(readByteArray(i), i));
        aInput->setData(dt)->out();
    }, rea::Json("name", m_root + "readDir", "thread", 10));

    rea::pipeline::add<stgVector<QString>, rea::pipePartial>([this](rea::stream<stgVector<QString>>* aInput){
        auto dt = aInput->data();
        aInput->setData(stgVector<QString>(listFiles(dt), dt))->out();
        aInput->out();
    }, rea::Json("name", m_root + "listFiles", "thread", 10));

    rea::pipeline::add<stgVector<stgByteArray>, rea::pipePartial>([this](rea::stream<stgVector<stgByteArray>>* aInput){
        auto dt = aInput->data().getData();
        for (auto i : dt)
            writeByteArray(aInput->data() + "/" + i, i.getData());
        aInput->out();
    }, rea::Json("name", m_root + "writeDir", "thread", 11));

    rea::pipeline::add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        deletePath(aInput->data());
        aInput->out();
    }, rea::Json("name", m_root + "deletePath", "thread", 11));

    rea::pipeline::add<QString, rea::pipePartial>(
        [this](rea::stream<QString>* aInput) {
            auto dt = aInput->data();
            aInput->var<QJsonObject>(dt, readJson(dt))->out();
        },
        rea::Json("name", m_root + "readJson2", "thread", 10));

    rea::pipeline::add<QString, rea::pipePartial>(
        [this](rea::stream<QString>* aInput) {
            auto dt = aInput->data();
            writeJson(dt, aInput->varData<QJsonObject>(dt));
            aInput->out();
        },
        rea::Json("name", m_root + "writeJson2", "thread", 11));

    rea::pipeline::add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        auto dt = aInput->data();
        QJsonArray ret;
        auto fls = listFiles(dt);
        for (auto i : fls)
            ret.push_back(i);
        aInput->var<QJsonArray>(dt, ret)->out();
    }, rea::Json("name", m_root + "listFiles2", "thread", 10));
}

/*bool safetyWrite(const QString& aPath, const QByteArray& aData){
    QDir().mkdir("Temp");
    auto tmp = "Temp/" + aPath.mid(aPath.lastIndexOf("/") + 1, aPath.length());
    QFile fl(tmp);
    if (fl.open(QFile::WriteOnly)){
        fl.write(aData);
        fl.close();
        if (!MoveFileExA(tmp.toLocal8Bit().toStdString().data(), aPath.toLocal8Bit().toStdString().data(), MOVEFILE_REPLACE_EXISTING)){
            std::cout << "write file error: " << GetLastError() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}*/

fsStorage0::~fsStorage0(){

}

std::vector<QString> fsStorage0::listFiles(const QString& aDirectory){
    QDir dir(stgRoot(aDirectory));
    std::vector<QString> ret;
    auto lst = dir.entryList();
    for (auto i : lst)
        ret.push_back(i);
    return ret;
}

}
