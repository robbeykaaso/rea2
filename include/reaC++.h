#ifndef REA_H_
#define REA_H_

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

class stream0;
class pipeline;

class DSTDLL routine{
public:
    routine(const QString& aName, const QString& aTag);
    ~routine();
    void log(const QString& aLog);
    void fail(){
        m_fail = true;
    }
    const QString print();
    QString getName(){return m_name;}
private:
    void executed(const QString& aPipe);
    void addTrig(const QString& aStart, const QString& aNext);
    std::mutex m_mutex;
    std::vector<QString> m_logs;
    QHash<QString, int> m_candidates;
    QString m_name;
    bool m_fail = false;
    friend stream0;
    friend pipeline;
};

class pipe0;
template <typename T>
class stream;
template <typename T>
using pipeFunc = std::function<void(stream<T>*)>;

template <typename T, typename F = pipeFunc<T>>
class pipe;

class DSTDLL stream0{
public:
    stream0(const QString& aTag = "") {m_tag = aTag;}
    stream0(const stream0&) = default;
    stream0(stream0&&) = default;
    stream0& operator=(const stream0&) = default;
    stream0& operator=(stream0&&) = default;
    virtual ~stream0(){}
    void fail(){
        getRoutine()->fail();
    }
    void log(const QString& aLog){
        getRoutine()->log(aLog);
    }
protected:
    void addTrig(const QString& aStart, const QString& aNext){
        if (m_routine)
            m_routine->addTrig(aStart, aNext);
    }
    void executed(const QString& aPipe);
    std::shared_ptr<routine> getRoutine(){
        if (!m_routine)
            qFatal("no this routine!");
        return m_routine;
    }
    QString m_tag;
    std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> m_cache;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<stream0>>>> m_outs = nullptr;
    std::shared_ptr<routine> m_routine = nullptr;
    friend class pipe0;
    template<typename T, typename F>
    friend class pipe;
};

class pipeFuture;
template<typename T, typename F>
class funcType;
template <typename T, typename F = pipeFunc<T>>
class pipeDelegate;

template <typename T>
class stream : public stream0{
public:
    stream() : stream0(){}
    stream(T aInput, const QString& aTag = "", std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> aCache = nullptr, std::shared_ptr<routine> aRoutine = nullptr) : stream0(aTag){
        m_data = aInput;
        if (aCache)
            m_cache = aCache;
        else
            m_cache = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
        m_routine = aRoutine;
    }
    stream<T>* setData(T aData) {
        m_data = aData;
        return this;
    }
    T data() {return m_data;}

    stream<T>* out(const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        m_tag = aTag;
        return this;
    }

    template<typename S>
    stream<S>* outs(S aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        auto ot = std::make_shared<stream<S>>(aOut, aTag, aShareCache ? m_cache : nullptr, m_routine);
        m_outs->push_back(std::pair<QString, std::shared_ptr<stream0>>(aNext, ot));
        return ot.get();
    }

    template<typename S>
    stream<T>* outsB(S aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true){
        outs<S>(aOut, aNext, aTag, aShareCache);
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
    template<typename T, typename F>
    friend class funcType;
    template <typename T, typename F>
    friend class pipeDelegate;
};

class DSTDLL pipe0 : public QObject{
public:
    virtual ~pipe0(){}
    virtual QString actName() {return m_name;}

    template <typename T>
    pipe0* nextF(pipeFunc<T> aNextFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject()){
        return nextF0(this, aNextFunc, aTag, aParam);
    }
    virtual pipe0* next(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* next(const QString& aName, const QString& aTag = "");
    virtual void removeNext(const QString& aName);
    virtual pipe0* nextB(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* nextB(const QString& aName, const QString& aTag = "");

    void execute(std::shared_ptr<stream0> aStream, const QString& aTag = "");

    virtual pipe0* createLocal(const QString& aName, const QJsonObject& aParam);
    bool isBusy() {return m_busy;}
private:
    friend pipeline;
protected:
    class streamEvent : public QEvent{
    public:
        static const Type type = static_cast<Type>(QEvent::User + 1);
    public:
        streamEvent(const QString& aName, std::shared_ptr<stream0> aStream, const QString& aTag = "") : QEvent(type) {
            m_name = aName;
            m_tag = aTag;
            m_stream = aStream;
        }
        QString getName() {return m_name;}
        QString getTag(){
            if (m_stream->m_tag == "")
                return m_tag;
            else
                return m_stream->m_tag;
        }
        std::shared_ptr<stream0> getStream() {return m_stream;}
    private:
        QString m_name;
        std::shared_ptr<stream0> m_stream;
        QString m_tag;
    };
    pipe0(const QString& aName = "", int aThreadNo = 0, bool aReplace = false);
    virtual QString workName() {return actName();}
    virtual QString localName() {return "";}
    virtual void insertNext(const QString& aName, const QString& aTag) {
        m_next.insert(aName, aTag);
    }
protected:
    void doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream);
protected:
    QString m_name;
    bool m_anonymous;
    QMap<QString, QString> m_next;
    bool m_busy = false;
    std::shared_ptr<stream0> m_stream_cache = nullptr;
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
    void insertNext(const QString& aName, const QString& aTag) override;
private:
    QString m_act_name;
    QHash<QString, QJsonObject> m_locals;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipeLocal;
template <typename T, typename F>
class pipeParallel;

class DSTDLL pipeline{
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
            pipeline::call<int>(ad, 0);
            pipeline::remove(ad);
        }
        tmp->initialize(aFunc, aParam);
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
    static void run(const QString& aName, T aInput, const QString& aTag = "", bool aRoutine = true){
        auto pip = instance()->m_pipes.value(aName);
        if (pip){
            auto rt = aRoutine ? std::make_shared<routine>(aName, aTag) : nullptr;
            if (rt){
                auto st = instance()->m_pipes.value("routineStart");
                if (st)
                    st->execute(std::make_shared<stream<routine*>>(rt.get()));
            }
            pip->execute(std::make_shared<stream<T>>(aInput, "", nullptr, rt), aTag);
        }
    }

    template<typename T, typename F = pipeFunc<T>>
    static void call(const QString& aName, T aInput){
        auto pip = instance()->m_pipes.value(aName);
        if (pip){
            auto pip2 = dynamic_cast<pipe<T, F>*>(pip);
            auto stm = std::make_shared<stream<T>>(aInput);
            pip2->m_func(stm.get());
        }
    }
private:
    QThread* findThread(int aNo);
    QHash<QString, pipe0*> m_pipes;
    QHash<int, std::shared_ptr<QThread>> m_threads;
    friend pipe0;
    friend pipeFuture;
    friend routine;
};

template <typename T>
pipe0* nextF0(pipe0* aPipe, pipeFunc<T> aNextFunc, const QString& aTag, const QJsonObject& aParam){
    return aPipe->next(pipeline::add<T>(aNextFunc, aParam), aTag);
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
    std::shared_ptr<stream0> createStreamList(std::vector<T>& aDataList, std::shared_ptr<stream<T>> aStream){
        auto stms = std::make_shared<stream<std::vector<T>>>(aDataList, "", aStream->m_cache, aStream->m_routine);
        stms->out();
        return stms;
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
    void executed(const std::shared_ptr<stream<T>> aStream){
        aStream->executed(workName());
    }
    void doEvent(const std::shared_ptr<stream<T>> aStream){
        m_busy = true;
        if (m_stream_cache && !aStream->m_routine){
            aStream->m_routine = m_stream_cache->m_routine;
            aStream->m_cache = m_stream_cache->m_cache;
            m_stream_cache = nullptr;
        }
        funcType<T, F>().doEvent(m_func, aStream);
        executed(aStream);
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
    QString workName() override {return localName();}
    QString localName() override {return m_act_name;}
private:
    bool m_init = false;
    QString m_act_name;
    friend pipeline;
};

template <typename T, typename F>
class pipeDelegate : public pipe<T, F>{
public:
    pipe0* next(pipe0* aNext, const QString& aTag = "") override{
        return pipeline::find(m_delegate)->next(aNext, aTag);
    }
    pipe0* next(const QString& aName, const QString& aTag = "") override{
        return pipeline::find(m_delegate)->next(aName, aTag);
    }
    void removeNext(const QString& aName) override{
        pipeline::find(m_delegate)->removeNext(aName);
    }
protected:
    pipeDelegate(const QString& aName = "", int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {}
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm0 = eve->getStream();
                auto stm = std::dynamic_pointer_cast<stream<T>>(stm0);
                doEvent(stm);
                stm->addTrig(workName(), m_delegate);
                pipeline::find(m_delegate)->m_stream_cache = stm0;
            }
        }
        return true;
    }
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override{
        m_delegate = aParam.value("delegate").toString();
        auto del = pipeline::find(m_delegate);
        for (auto i : m_next2)
            del->insertNext(i.first, i.second);
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    void insertNext(const QString& aName, const QString& aTag) override{
        m_next2.push_back(QPair<QString, QString>(aName, aTag));
    }
private:
    QString m_delegate;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

template <typename T, typename F = pipeFunc<T>>
class pipePartial : public pipe<T, F> {
public:
    pipe0* next(pipe0* aNext, const QString& aTag = "") override{
        insertNext(aNext->actName(), aTag);
        return aNext;
    }
    pipe0* next(const QString& aName, const QString& aTag = "") override{
        insertNext(aName, aTag);
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
    void insertNext(const QString& aName, const QString& aTag) override {
        tryFind(&m_next2, aTag)->insert(aName, aTag);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                auto tg = eve->getTag();
                if (tg != "")
                    doNextEvent(m_next2.value(tg), stm);
            }
        }
        return true;
    }
private:
    QHash<QString, QMap<QString, QString>> m_next2;
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
                auto stm0 = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                m_buffers.push_back(stm0->data());
                if (m_buffers.size() >= m_count){
                    auto stm = funcType<T, F>().createStreamList(m_buffers, stm0);
                    for (auto i = 0; i <= m_count; ++i)
                        executed(stm0);
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
                                QMap<QString, QString> nxts;
                                nxts.insert(i, pipe0::m_next.value(i));
                                doEvent(stm);
                                pipe0::doNextEvent(nxts, stm);
                            }
                        }
                    }else{
                        executed(stm);
                        m_cache.insert(i, stm);
                    }
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
                    QMap<QString, QString> nxts;
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

DSTDLL pipe0* local(const QString& aName, const QJsonObject& aParam = QJsonObject());
template <typename T>
pipe0* parallel(const QString& aName){
    return pipeline::add<T, pipeParallel>(nullptr, rea::Json("delegate", aName));
};
template <typename T>
pipe0* buffer(const int aCount = 1, const QString& aName = "", const int aThread = 0){
    return pipeline::add<T, pipeBuffer>(nullptr, Json("name", aName, "thread", aThread, "count", aCount));
}

}

#endif