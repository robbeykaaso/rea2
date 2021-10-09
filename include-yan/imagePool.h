#ifndef REAL_FRAMEWORK_IMAGEPOOL_H_
#define REAL_FRAMEWORK_IMAGEPOOL_H_

#include "util.h"
#include <mutex>
#include <QImage>
#include <QQueue>

namespace rea {

class DSTDLL imagePool{
public:
    static imagePool* instance();
    static void cacheImage(const QString& aPath, const QImage& aImage);
    static QImage readCache(const QString& aPath);
protected:
    imagePool(){}
    //~imagePool(){}
private:
    std::mutex m_mutex;
    QHash<QString, QQueue<QImage>> m_images;
};

}


#endif
