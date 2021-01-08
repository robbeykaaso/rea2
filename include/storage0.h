#ifndef REAL_FRAMEWORK_STORAGE0_H_
#define REAL_FRAMEWORK_STORAGE0_H_

#include <QImage>
#include <QJsonObject>
#include <QString>
#include "util.h"

namespace rea {

template <typename T>
class stgData : public QString {
 public:
  stgData() : QString() {}
  stgData(T aData, const QString& aPath) : QString(aPath) { m_data = aData; }
  stgData(const QString& aPath) : QString(aPath) { m_data = T();}
  T getData() { return m_data; }

 private:
  T m_data;
};

using stgJson = stgData<QJsonObject>;
using stgByteArray = stgData<QByteArray>;
using stgQImage = stgData<QImage>;
template <typename T>
using stgVector = stgData<std::vector<T>>;

class DSTDLL fsStorage0 {
 public:
  fsStorage0(const QString& aRoot = "");
  virtual ~fsStorage0();
  virtual bool isValid() { return true; }

 protected:
  virtual std::vector<QString> listFiles(const QString& aDirectory);
  virtual void writeJson(const QString& aPath, const QJsonObject& aData);
  virtual void writeQImage(const QString& aPath, const QImage& aData);
  virtual void writeByteArray(const QString& aPath, const QByteArray& aData);
  virtual QJsonObject readJson(const QString& aPath);
  virtual QImage readQImage(const QString& aPath);
  virtual QByteArray readByteArray(const QString& aPath);
  virtual void deletePath(const QString& aPath);
  virtual std::vector<QString> getFileList(const QString& aPath);
  virtual QString stgRoot(const QString& aPath);
  void checkPath(const QString& aPath);

 protected:
  QString m_root;
};

#define REGREADSTORAGE(aType)                                    \
  rea::pipeline::add<stg##aType, rea::pipePartial>(              \
      [this](rea::stream<stg##aType>* aInput) {                  \
        auto dt = aInput->data();                                \
        aInput->setData(stg##aType(read##aType(dt), dt))->out(); \
      },                                                         \
      rea::Json("name", m_root + STR(read##aType), "thread", 10));

#define REGWRITESTORAGE(aType)                      \
  rea::pipeline::add<stg##aType, rea::pipePartial>( \
      [this](rea::stream<stg##aType>* aInput) {     \
        auto dt = aInput->data();                   \
        write##aType(dt, dt.getData());             \
        aInput->out();                              \
      },                                            \
      rea::Json("name", m_root + STR(write##aType), "thread", 11));

}  // namespace rea

#endif
