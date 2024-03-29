#include "imagePool.h"

namespace rea {

imagePool* imagePool::instance(){
    static std::mutex imagePool_mutex;
    std::lock_guard<std::mutex> lg(imagePool_mutex);
    static imagePool ret;  //if realized in .h, there will be multi objects in different dlls
    return &ret;
}

void imagePool::cacheImage(const QString& aPath, const QImage& aImage){
    //imagePool::instance()->m_images.insert(aPath, aImage);
    std::lock_guard<std::mutex> lg(imagePool::instance()->m_mutex);
    //std::cout << "imageObject cache: " << aPath.toStdString() << std::endl;
    tryFind(&imagePool::instance()->m_images, aPath)->push_back(aImage);
    qDebug() << "cache Image:" << aPath << ":" << tryFind(&imagePool::instance()->m_images, aPath)->size();
}

QImage imagePool::readCache(const QString& aPath){
    std::lock_guard<std::mutex> lg(imagePool::instance()->m_mutex);
    auto imgs = tryFind(&imagePool::instance()->m_images, aPath);
    if (imgs->size() > 0){
        auto ret = imgs->front();
        imgs->pop_front();
        qDebug() << "read cache Image: " << aPath << ":" << imgs->size();
        return ret;
    }
    else
        return QImage(aPath);
}

}

