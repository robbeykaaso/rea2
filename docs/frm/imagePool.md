# Abstract
a singleton for caching images by their paths for different threads  

# API
* **void cacheImage(const QString& aPath, const QImage& aImage)**  
cache a image by its path  
</br>

* **QImage readCache(const QString& aPath)**  
read the cached image. if it doesn't exisit, it will try to read it from the file system and cache it. the cache will be cleared after reading  
</br>