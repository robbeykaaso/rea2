#ifndef REAL_FRAMEWORK_RECATIVE2_H_
#define REAL_FRAMEWORK_RECATIVE2_H_

#include "util.h"
#include <vector>
#include <functional>
#include <QQmlApplicationEngine>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QEvent>

namespace rea {

//class stream;

class pipe0;

class stream0{
public:
    stream0(const QJsonObject& aParam = QJsonObject()) {m_param = aParam;}
    stream0(const stream0&) = default;
    stream0(stream0&&) = default;
    stream0& operator=(const stream0&) = default;
    stream0& operator=(stream0&&) = default;
    virtual ~stream0(){}
protected:
    QJsonObject m_param;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<stream0>>>> m_outs = nullptr;
    friend pipe0;
};

class pipeFuture;

template <typename T>
class stream : public stream0{
public:
    stream() : stream0(){}
    stream(T aInput, const QJsonObject& aParam = QJsonObject(), std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> aCache = nullptr) : stream0(aParam){
        m_data = aInput;
        if (aCache)
            m_cache = aCache;
        else
            m_cache = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
    }
    stream<T>* setData(T aData) {
        m_data = aData;
        return this;
    }
    T data() {return m_data;}

    stream<T>* out(const QJsonObject& aParam = QJsonObject()){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        m_param = aParam;
        return this;
    }

    template<typename S>
    stream<S>* out(S aOut, const QString& aNext = "", const QJsonObject& aParam = QJsonObject(), bool aShareCache = true){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        auto ot = std::make_shared<stream<S>>(aOut, aParam, aShareCache ? m_cache : nullptr);
        m_outs->push_back(std::pair<QString, std::shared_ptr<stream0>>(aNext, ot));
        return ot.get();
    }

    template<typename S>
    stream<T>* outB(S aOut, const QString& aNext = "", const QJsonObject& aParam = QJsonObject(), bool aShareCache = true){
        out<S>(aOut, aNext, aParam, aShareCache);
        return this;
    }

    template<typename S>
    stream<T>* var(const QString& aName, S aData){
        m_cache->insert(aName, std::make_shared<stream<S>>(aData));
        return this;
    }

    template<typename S>
    S varData(const QString& aName){
        auto ret = std::dynamic_pointer_cast<stream<S>>(m_cache->value(aName));
        if (ret)
            return ret->data();
        else{
            return S();
            //assert(0);
        }
    }
private:
    T m_data;
    std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> m_cache;
};

class qmlStream : public QObject
{
    Q_OBJECT
public:
    qmlStream(){}
    qmlStream(QJSValue aInput, const QJsonObject& aParam = QJsonObject(), std::shared_ptr<QHash<QString, std::shared_ptr<qmlStream>>> aCache = nullptr){
        m_data = aInput;
        m_param = aParam;
        if (aCache)
            m_cache = aCache;
        else
            m_cache = std::make_shared<QHash<QString, std::shared_ptr<qmlStream>>>();
    }
public:
    Q_INVOKABLE QVariant setData(QJSValue aData){
        m_data = aData;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QJSValue data(){
        return m_data;
    }
    Q_INVOKABLE QVariant out(const QJsonObject& aParam = QJsonObject()){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        m_param = aParam;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QVariant out(QJSValue aOut, const QString& aNext, const QJsonObject& aParam = QJsonObject(), bool aShareCache = true){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        auto ot = std::make_shared<qmlStream>(aOut, aParam, aShareCache ? m_cache : nullptr);
        m_outs->push_back(std::pair<QString, std::shared_ptr<qmlStream>>(aNext, ot));
        return QVariant::fromValue<QObject*>(ot.get());
    }
private:
    QJSValue m_data;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>> m_outs = nullptr;
    QJsonObject m_param;
    std::shared_ptr<QHash<QString, std::shared_ptr<qmlStream>>> m_cache;
};

template <typename T>
using pipeFunc = std::function<void(stream<T>*)>;

template <typename T, typename F = pipeFunc<T>>
class pipeDelegate;

template <typename T, typename F = pipeFunc<T>>
class pipe;

class pipeline;

class pipe0 : public QObject{
public:
    virtual ~pipe0(){}
    virtual QString actName() {return m_name;}

    template <typename T>
    pipe0* nextF(pipeFunc<T> aNextFunc, const QJsonObject& aParam = QJsonObject(), const QJsonObject& aPipeParam = QJsonObject()){
        return nextF0(this, aNextFunc, aParam, aPipeParam);
    }
    virtual pipe0* next(pipe0* aNext, const QJsonObject& aParam = QJsonObject());
    virtual pipe0* next(const QString& aName, const QJsonObject& aParam = QJsonObject());
    virtual void removeNext(const QString& aName);
    virtual pipe0* nextB(pipe0* aNext, const QJsonObject& aParam = QJsonObject());
    virtual pipe0* nextB(const QString& aName, const QJsonObject& aParam = QJsonObject());

    void execute(std::shared_ptr<stream0> aStream, const QJsonObject& aParam = QJsonObject());

    virtual pipe0* createLocal(const QString& aName, const QJsonObject& aParam);
    bool isBusy() {return m_busy;}
private:
    friend pipeline;
protected:
    class streamEvent : public QEvent{
    public:
        static const Type type = static_cast<Type>(QEvent::User + 1);
    public:
        streamEvent(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aParam = QJsonObject()) : QEvent(type) {
            m_name = aName;
            m_param = aParam;
            m_stream = aStream;
        }
        QString getName() {return m_name;}
        QJsonObject getParam(){
            if (m_stream->m_param.empty())
                return m_param;
            else
                return m_stream->m_param;
        }
        std::shared_ptr<stream0> getStream() {return m_stream;}
    private:
        QString m_name;
        std::shared_ptr<stream0> m_stream;
        QJsonObject m_param;
    };
    pipe0(const QString& aName = "", int aThreadNo = 0, bool aReplace = false);
    virtual QString localName() {return "";}
    virtual void insertNext(const QString& aName, const QJsonObject& aParam) {
        m_next.insert(aName, aParam);
    }
protected:
    void doNextEvent(const QMap<QString, QJsonObject>& aNexts, std::shared_ptr<stream0> aStream);
protected:
    QString m_name;
    bool m_anonymous;
    bool m_busy = false;
    QMap<QString, QJsonObject> m_next;
    QThread* m_thread = QThread::currentThread();
private:
    friend pipeFuture;

    template<typename, typename>
    friend class pipeDelegate;
};

class pipeFuture : public pipe0 {
public:
    QString actName() override {return m_act_name;}
    pipe0* createLocal(const QString& aName, const QJsonObject& aParam) override;
protected:
    pipeFuture(const QString& aName);
    void insertNext(const QString& aName, const QJsonObject& aParam) override;
private:
    QString m_act_name;
    QHash<QString, QJsonObject> m_locals;
    QVector<QPair<QString, QJsonObject>> m_next2;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipeLocal;
template <typename T, typename F>
class pipeParallel;

class crashDump {
public:
    crashDump();
};

class pipeline{
public:
    QQmlApplicationEngine *engine = nullptr;
    static pipeline* instance();
public:
    pipeline();
    pipeline(pipeline&&) = delete;
    ~pipeline();

    static void remove(const QString& aName);

    template<typename T, template<class, typename> class P = pipe, typename F = pipeFunc<T>, typename S = pipeFunc<T>>
    static pipe0* add(F aFunc, const QJsonObject& aParam = QJsonObject()){
        auto nm = aParam.value("name").toString();
        auto tmp = new P<T, S>(nm, aParam.value("thread").toInt(), aParam.value("replace").toBool());  //https://stackoverflow.com/questions/213761/what-are-some-uses-of-template-template-parameters
        if (nm != ""){
            auto ad = tmp->actName() + "_pipe_add";
            pipeline::run<int>(ad, 0);
            pipeline::remove(ad);
        }
        tmp->initialize(aFunc, aParam.value("param").toObject());
        pipe0* ret = tmp;
        return ret;
    }

    static pipe0* find(const QString& aName, bool needFuture = true) {
        auto ret = instance()->m_pipes.value(aName);
        if (!ret && needFuture){
            ret = new pipeFuture(aName);
        }
        return ret;
    }

    template<typename T>
    static void run(const QString& aName, T aInput, const QJsonObject& aParam = QJsonObject()){
        auto pip = instance()->m_pipes.value(aName);
        if (pip)
            pip->execute(std::make_shared<stream<T>>(aInput), aParam);
    }
private:
    void addOneLog(const QString& aLog);
    QThread* findThread(int aNo);
    QHash<QString, pipe0*> m_pipes;
    QHash<int, std::shared_ptr<QThread>> m_threads;
    std::vector<QString> m_logs;
    size_t m_log_index, m_log_count;
    friend pipe0;
    friend pipeFuture;
    friend crashDump;
};

template <typename T>
pipe0* nextF0(pipe0* aPipe, pipeFunc<T> aNextFunc, const QJsonObject& aParam, const QJsonObject& aPipeParam){
    return aPipe->next(pipeline::add<T>(aNextFunc, aPipeParam), aParam);
}

template<typename T, typename F>
class funcType{
public:
    pipe0* createLocal(const QString& aName, const QJsonObject& aParam){
        auto prm = aParam;
        return pipeline::add<T, pipeLocal>(nullptr, Json(prm, "name", aName));
    }
    void doEvent(F aFunc, std::shared_ptr<stream<T>> aStream){
        aFunc(aStream.get());
    }
    std::shared_ptr<stream0> createStreamList(std::vector<T>& aDataList){
        auto stms = std::make_shared<stream<std::vector<T>>>(aDataList);
        stms->out();
        return stms;
    }
};

template <typename T>
class valType{
public:
    void setData(std::shared_ptr<stream<T>>, const QJsonValue&){

    }
};

template <>
class valType<QJsonObject>{
public:
    void setData(std::shared_ptr<stream<QJsonObject>> aStream, const QJsonValue& aValue){
        aStream->setData(aValue.toObject());
    }
};

template <>
class valType<QString>{
public:
    void setData(std::shared_ptr<stream<QString>> aStream, const QJsonValue& aValue){
        aStream->setData(aValue.toString());
    }
};

template <>
class valType<double>{
public:
    void setData(std::shared_ptr<stream<double>> aStream, const QJsonValue& aValue){
        aStream->setData(aValue.toDouble());
    }
};

template <>
class valType<bool>{
public:
    void setData(std::shared_ptr<stream<bool>> aStream, const QJsonValue& aValue){
        aStream->setData(aValue.toBool());
    }
};

template <>
class valType<QJsonArray>{
public:
    void setData(std::shared_ptr<stream<QJsonArray>> aStream, const QJsonValue& aValue){
        aStream->setData(aValue.toArray());
    }
};

template<typename T>
class funcType<T, QJSValue>{
public:
    pipe0* createLocal(const QString& aName, const QJsonObject& aParam){
        auto prm = aParam;
        return pipeline::add<T, pipeLocal, QJSValue, QJSValue>(QJSValue(), Json(prm, "name", aName));
    }
    void doEvent(QJSValue aFunc, std::shared_ptr<stream<T>> aStream){
        if (pipeline::instance()->engine != nullptr && !aFunc.equals(QJsonValue::Null)){
            QJSValueList paramList;
            qmlStream stm(pipeline::instance()->engine->toScriptValue(aStream->data()));
            paramList.append(pipeline::instance()->engine->toScriptValue(QVariant::fromValue<QObject*>(&stm)));
            aFunc.call(paramList);
            auto dt = stm.data();
            if (!dt.isNull()){
                if (dt.isObject())
                    valType<T>().setData(aStream, QJsonObject::fromVariantMap(dt.toVariant().toMap()));
                else if (dt.isArray())
                    valType<T>().setData(aStream, QJsonArray::fromVariantList(dt.toVariant().toList()));
                else if (dt.isNumber())
                    valType<T>().setData(aStream, dt.toNumber());
                else if (dt.isBool())
                    valType<T>().setData(aStream, dt.toBool());
                else if (dt.isString())
                    valType<T>().setData(aStream, dt.toString());
            }
        }
    }
    std::shared_ptr<stream0> createStreamList(std::vector<T>& aDataList){
        QJsonObject lst;
        QString ky = "0";
        for (int i = 0; i < aDataList.size(); ++i){
            lst.insert(ky, QJsonValue(aDataList[i]));
            ky += "0";
        }
        auto stms = std::make_shared<stream<QJsonObject>>(lst);
        stms->out();
        return std::move(stms);
    }
};

template <typename T, typename F>
class pipe : public pipe0{
public:
    pipe0* createLocal(const QString& aName, const QJsonObject& aParam) override{
        return funcType<T, F>().createLocal(aName, aParam);
    }
protected:
    pipe(const QString& aName = "", int aThreadNo = 0, bool aReplace = false) : pipe0(aName, aThreadNo, aReplace) {}
    virtual pipe0* initialize(F aFunc, const QJsonObject&){
        m_func = aFunc;
        return this;
    }
    bool event( QEvent* e) override{
        if(e->type()== streamEvent::type){
            auto eve = reinterpret_cast<streamEvent*>(e);
            if (eve->getName() == m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                doNextEvent(m_next, stm);
            }
        }
        return true;
    }
    void doEvent(const std::shared_ptr<stream<T>> aStream){
        m_busy = true;
        funcType<T, F>().doEvent(m_func, aStream);
        m_busy = false;
    }
protected:
    F m_func;
    friend pipeLocal<T, F>;
    friend pipeParallel<T, F>;
    friend pipeline;
};

template <typename T, typename F>
class pipeLocal : public pipe<T, F> {
protected:
    pipeLocal(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>("", aThreadNo, aReplace) {
        m_act_name = aName;
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe<T, F>::m_name){
                if (!m_init){
                    auto pip = pipeline::find(m_act_name, false);
                    if (pip)
                        pipe<T, F>::m_func = dynamic_cast<pipe<T, F>*>(pip)->m_func;
                    m_init = true;
                }
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                doNextEvent(pipe0::m_next, stm);
            }
        }
        return true;
    }
    QString localName() override {return m_act_name;}
private:
    bool m_init = false;
    QString m_act_name;
    friend pipeline;
};

template <typename T, typename F>
class pipeDelegate : public pipe<T, F>{
public:
    pipe0* next(pipe0* aNext, const QJsonObject& aParam = QJsonObject()) override{
        return pipeline::find(m_delegate)->next(aNext, aParam);
    }
    pipe0* next(const QString& aName, const QJsonObject& aParam = QJsonObject()) override{
        return pipeline::find(m_delegate)->next(aName, aParam);
    }
    void removeNext(const QString& aName) override{
        pipeline::find(m_delegate)->removeNext(aName);
    }
protected:
    pipeDelegate(const QString& aName = "", int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {}
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override{
        m_delegate = aParam.value("delegate").toString();
        auto del = pipeline::find(m_delegate);
        for (auto i : m_next2)
            del->insertNext(i.first, i.second);
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    void insertNext(const QString& aName, const QJsonObject& aParam) override{
        m_next2.push_back(QPair<QString, QJsonObject>(aName, aParam));
    }
private:
    QString m_delegate;
    QVector<QPair<QString, QJsonObject>> m_next2;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipePartial : public pipe<T, F> {
public:
    pipe0* next(pipe0* aNext, const QJsonObject& aParam = QJsonObject()) override{
        insertNext(aNext->actName(), aParam);
        return aNext;
    }
    pipe0* next(const QString& aName, const QJsonObject& aParam = QJsonObject()) override{
        insertNext(aName, aParam);
        auto nxt = pipeline::find(aName);
        return nxt;
    }
    void removeNext(const QString& aName) override {
        for (auto i : m_next2)
            i.remove(aName);
    }
protected:
    pipePartial(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {

    }
    void insertNext(const QString& aName, const QJsonObject& aParam) override {
        auto pre = aParam.value("tag").toString();
        tryFind(&m_next2, pre)->insert(aName, aParam);
    }
    /*
     * @nextParam
     * nextParam["tag"]: the last pipe name or last param["tag"]
     * streamin: stream
     * streamout: streamList
    */
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                auto tg = eve->getParam().value("tag").toString();
                if (tg != "")
                    doNextEvent(m_next2.value(tg), stm);
            }
        }
        return true;
    }
private:
    QHash<QString, QMap<QString, QJsonObject>> m_next2;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipeBuffer : public pipe<T, F> {
protected:
    pipeBuffer(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {

    }
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override {
        m_count = aParam.value("count").toInt(1);
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                m_buffers.push_back(stm->data());
                if (m_buffers.size() >= m_count){
                    auto stm = funcType<T, F>().createStreamList(m_buffers);
                    pipe0::doNextEvent(pipe0::m_next, stm);
                    m_buffers.clear();
                }
            }
        }
        return true;
    }
private:
    std::vector<T> m_buffers;
    int m_count;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipeThrottle : public pipe<T, F> {
protected:
    pipeThrottle(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {

    }
    ~pipeThrottle() override{
        for (auto i : m_timer.keys())
            killTimer(i);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                for (auto i : pipe0::m_next.keys()){
                    if (!m_cache.contains(i)){
                        auto nxt = rea::pipeline::find(i, false);
                        if (nxt){
                            if (nxt->isBusy()){
                                m_cache.insert(i, stm);
                                m_timer.insert(startTimer(5), i);
                            }
                            else{
                                QMap<QString, QJsonObject> nxts;
                                nxts.insert(i, pipe0::m_next.value(i));
                                doEvent(stm);
                                pipe0::doNextEvent(nxts, stm);
                            }
                        }
                    }else
                        m_cache.insert(i, stm);
                }
            }
        }else if (e->type() == QEvent::Timer){
            auto tm_e = reinterpret_cast<QTimerEvent*>(e);
            auto id = tm_e->timerId();
            if (m_timer.contains(id)){
                auto key = m_timer.value(id);
                auto nxt = rea::pipeline::find(key, false);
                if (nxt && !nxt->isBusy()){
                    killTimer(id);
                    auto stm = m_cache.value(key);
                    m_cache.remove(key);
                    m_timer.remove(id);
                    QMap<QString, QJsonObject> nxts;
                    nxts.insert(key, pipe0::m_next.value(key));
                    doEvent(stm);
                    pipe0::doNextEvent(nxts, stm);
                }
            }
        }
        return true;
    }
private:
    QHash<QString, std::shared_ptr<stream<T>>> m_cache;
    QHash<int, QString> m_timer;
    friend pipeline;
};

template <typename T, typename F>
class parallelTask : public QRunnable{
public:
    parallelTask(pipeParallel<T, F>* aPipe, std::shared_ptr<stream<T>> aStream) : QRunnable(){
        m_pipe = aPipe;
        m_source = aStream;
    }
    void run() override{
        m_pipe->doEvent(m_source);
        m_pipe->doNextEvent(m_pipe->m_next, m_source);
    }
private:
    std::shared_ptr<stream<T>> m_source;
    pipeParallel<T, F>* m_pipe;
};

template <typename T, typename F = pipeFunc<T>>
class pipeParallel : public pipe<T, F> {
protected:
    pipeParallel(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {

    }
    ~pipeParallel() override{

    }
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override {
        m_act_name = aParam.value("delegate").toString();
        m_init = aFunc != nullptr;
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                if (!m_init){
                    auto pip = pipeline::find(m_act_name, false);
                    if (pip)
                        pipe<T, F>::m_func = dynamic_cast<pipe<T, F>*>(pip)->m_func;
                    m_init = true;
                }
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                QThreadPool::globalInstance()->start(new parallelTask<T, F>(this, stm));
            }
        }
        return true;
    }
private:
    bool m_init = false;
    QString m_act_name;
    friend parallelTask<T, F>;
    friend pipeline;
};

template <typename T, template<class, typename> class P = pipe>
class regPip
{
public:
    regPip(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject(), const QString& aPrevious = ""){
        auto pip = pipeline::add<T, P>(aFunc, aParam);
        actName = pip->actName();
        if (aPrevious != "")
            rea::pipeline::find(aPrevious)->next(pip);
    }
    QString actName;
};

struct ICreateJSPipe{
public:
    ICreateJSPipe(const QJsonObject& aParam, QJSValue aFunc) {
        param = aParam;
        func = aFunc;
    }
    QJsonObject param;
    QJSValue func;
};

pipe0* local(const QString& aName, const QJsonObject& aParam = QJsonObject());
template <typename T>
pipe0* parallel(const QString& aName){
    return pipeline::add<T, pipeParallel>(nullptr, rea::Json("param", rea::Json("delegate", aName)));
};

template <typename T>
pipe0* buffer(const int aCount = 1, const QString& aName = "", const int aThread = 0){
    return pipeline::add<T, pipeBuffer>(nullptr, Json("name", aName, "thread", aThread, "param", Json("count", aCount)));
}

#define FUNC(aType, aFunc) \
    rea::pipeline::add<aType>([](rea::stream<aType>* aInput){aFunc})

#define FUNCT(aType, aRoot, aFunc) \
    rea::pipeline::add<aType>([aRoot](rea::stream<aType>* aInput){aFunc})

#define STR(S) #S

}

#endif
