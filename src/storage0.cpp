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
        qDebug() << "write bytearray: " + aPath + " failed!\n";
    }
}

QJsonObject fsStorage0::readJson(const QString& aPath){
    QJsonObject ret;
    QFile fl(stgRoot(aPath));
    if (fl.open(QFile::ReadOnly)){
        QJsonDocument doc = QJsonDocument::fromJson(fl.readAll());
        ret = doc.object();
        fl.close();
    }else
        qDebug() << "read json: " + aPath + " failed!\n";
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
    }else
        qDebug() << "read bytearray: " + aPath + " failed!\n";
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

void testStorage(const QString& aRoot = ""){ //for fs: aRoot == ""; for minIO: aRoot != ""
    using namespace rea;
    auto tag = "testStorage";
    pipeline::find(aRoot + "writeJson")
        ->next(local(aRoot + "readJson"), tag)
        ->nextF<stgJson>([aRoot](rea::stream<stgJson>* aInput){
             auto js = aInput->data().getData();
             assert(js.value("hello") == "world2");
             aInput->outs<stgByteArray>(stgByteArray(QJsonDocument(Json("hello", "world")).toJson(), "testFS.json"), aRoot + "writeByteArray");
        })
        ->next(local(aRoot + "writeByteArray"))
        ->next(local(aRoot + "readByteArray"))
        ->nextF<stgByteArray>([aRoot](rea::stream<stgByteArray>* aInput){
             auto dt = aInput->data().getData();
             auto cfg = QJsonDocument::fromJson(dt).object();
             assert(cfg.value("hello") == "world");

             std::vector<stgByteArray> dts;
             dts.push_back(stgByteArray(dt, "testFS.json"));
             stgVector<stgByteArray> stm(dts, "testDir");
             aInput->outs<stgVector<stgByteArray>>(stm, aRoot + "writeDir");
        })
        ->next(local(aRoot + "writeDir"))
        ->next(local(aRoot + "readDir"))
        ->nextF<stgVector<stgByteArray>>([aRoot](rea::stream<stgVector<stgByteArray>>* aInput){
             auto dt = aInput->data().getData();
             if (aRoot == "")
                 assert(QDir().exists(aInput->data() + "/" + dt.at(0)));
             aInput->outs<stgVector<QString>>(stgVector<QString>(std::vector<QString>(), aInput->data()), aRoot + "listFiles");
        })
        ->next(local(aRoot + "listFiles"))
        ->nextF<stgVector<QString>>([aRoot](rea::stream<stgVector<QString>>* aInput){
             auto dt = aInput->data().getData();
             if (aRoot == "")
                 assert(dt.size() == 3);
             else
                assert(dt.size() == 1);
             aInput->outs<QString>("testDir", aRoot + "deletePath");
             aInput->outs<QString>("testFS.json", aRoot + "deletePath");
        })
        ->next(local(aRoot + "deletePath"))
        ->next(buffer<QString>(2))
        ->nextF<std::vector<QString>>([aRoot](rea::stream<std::vector<QString>>* aInput){
            aInput->outs<stgVector<QString>>(stgVector<QString>(std::vector<QString>(), aRoot + "/testDir"), aRoot + "listFiles");
        })
        ->next(local(aRoot + "listFiles"))
        ->nextF<stgVector<QString>>([aRoot](rea::stream<stgVector<QString>>* aInput){
             auto dt = aInput->data().getData();
             assert(dt.size() == 0);
             aInput->outs<QString>("Pass: testStorage " + aRoot, "testSuccess");
        })
        ->next("testSuccess");

    pipeline::run<stgJson>(aRoot + "writeJson", stgJson(Json("hello", "world2"), "testFS.json"), tag);
}

static rea::regPip<QJsonObject> unit_test([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("stg").toBool()){
        aInput->out();
        return;
    }
    static fsStorage0 local_storage;
    testStorage();
    aInput->out();
}, QJsonObject(), "unitTest");

}
