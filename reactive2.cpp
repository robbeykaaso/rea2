#include "reactive2.h"
#include <mutex>
#include <sstream>
#include <QFile>
#include <QCoreApplication>

namespace rea {

void routine::executed(const QString& aCandidate){
    auto cnt = m_candidates.value(aCandidate) - 1;
    if (!cnt){
        m_candidates.remove(aCandidate);
        if (!m_candidates.size()){
            tryFind(&rea::pipeline::instance()->m_routines, m_name)->remove(this);
            rea::pipeline::call<routine*>("routineFinished", this);
        }
    }
}

void routine::addCandidate(const QString& aStart, const QString& aCandidate){
    log(aStart + ">" + aCandidate);
    if (!m_candidates.contains(aCandidate))
        m_candidates.insert(aCandidate, 0);
    m_candidates.insert(aCandidate, m_candidates.value(aCandidate) + 1);
}

routine::routine(const QString& aName){
    m_name = aName;
    tryFind(&rea::pipeline::instance()->m_routines, aName)->insert(this);
}

pipe0::pipe0(const QString& aName, int aThreadNo, bool aReplace){
    m_anonymous = aName == "";
    if (m_anonymous)
        m_name = generateUUID();
    else
        m_name = aName;
    if (aThreadNo != 0){
        m_thread = pipeline::instance()->findThread(aThreadNo);
        moveToThread(m_thread);
    }
    auto old = pipeline::find(m_name, false);
    if (old){
        if (aReplace)
            m_next = old->m_next;
        pipeline::remove(m_name);
    }
    pipeline::instance()->m_pipes.insert(m_name, this);
}

pipe0* pipe0::next(pipe0* aNext, const QString& aTag){
    insertNext(aNext->actName(), aTag);
    return aNext;
}

pipe0* pipe0::next(const QString& aName, const QString& aTag){
    insertNext(aName, aTag);
    auto nxt = pipeline::find(aName);
    return nxt;
}

void pipe0::removeNext(const QString &aName){
    m_next.remove(aName);
}

pipe0* pipe0::nextB(pipe0* aNext, const QString& aTag){
    next(aNext, aTag);
    return this;
}

pipe0* pipe0::nextB(const QString& aName, const QString& aTag){
    next(aName, aTag);
    return this;
}

void pipe0::doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream){
    auto outs = aStream->m_outs;
    aStream->m_outs = nullptr;
    if (outs){
        if (outs->size() > 0){
            for (auto i : *outs)
                if (i.first == ""){
                    for (auto j : aNexts.keys()){
                        auto pip = pipeline::find(j, false);
                        if (pip && pip->m_anonymous){
                            pip->execute(i.second, i.second->m_tag == "" ? aNexts.value(j) : i.second->m_tag);
                        }
                    }
                }else{
                    if (aNexts.contains(i.first)){
                        auto pip = pipeline::find(i.first, false);
                        if (pip){
                            pip->execute(i.second, i.second->m_tag == "" ? aNexts.value(i.first) : i.second->m_tag);
                        }
                    }else{
                        bool exed = false;
                        for (auto j : aNexts.keys()){
                            auto pip = pipeline::find(j, false);
                            if (pip && pip->localName() == i.first){
                                pip->execute(i.second, i.second->m_tag == "" ? aNexts.value(i.first) : i.second->m_tag);
                                exed = true;
                            }
                        }
                        if (!exed){
                            auto pip = pipeline::find(i.first, false);
                            if (pip)
                                pip->execute(i.second, i.second->m_tag);
                        }
                    }
                }
        }else
            for (auto i : aNexts.keys()){
                auto pip = pipeline::find(i, false);
                if (pip)
                    pip->execute(aStream, aNexts.value(i));
            }
    }
}

void pipe0::execute(std::shared_ptr<stream0> aStream, const QString& aTag){
    if (QThread::currentThread() == m_thread){
        streamEvent nxt_eve(m_name, aStream, aTag);
        QCoreApplication::sendEvent(this, &nxt_eve);
    }else{
        auto nxt_eve = std::make_unique<streamEvent>(m_name, aStream, aTag);
        QCoreApplication::postEvent(this, nxt_eve.release());  //https://stackoverflow.com/questions/32583078/does-postevent-free-the-event-after-posting-double-free-or-corruption // still memory leak, reason is unknown
    }
}

pipe0* pipe0::createLocal(const QString& aName, const QJsonObject& aParam){
    return nullptr;
}

class pipeFuture0 : public pipe0 {  //the next of pipePartial may be the same name but not the same previous
public:
    pipeFuture0(const QString& aName) : pipe0(aName){
    }
protected:
    void insertNext(const QString& aName, const QString& aTag) override{
        m_next2.push_back(QPair<QString, QString>(aName, aTag));
    }
private:
    QVector<QPair<QString, QString>> m_next2;
    QHash<QString, QJsonObject> m_locals;
    friend pipeFuture;
};

void pipeFuture::insertNext(const QString& aName, const QString& aTag){
    m_next2.push_back(QPair<QString, QString>(aName, aTag));
}

pipe0* pipeFuture::createLocal(const QString& aName, const QJsonObject& aParam){
    auto ret = new pipeFuture(generateUUID());
    m_locals.insert(ret->actName(), aParam);
    return ret;
}

pipeFuture::pipeFuture(const QString& aName) : pipe0 (){
    m_act_name = aName;

    if (pipeline::find(aName + "_pipe_add", false)){  //there will be another pipeFuture before, this future should inherit its records before it is removed
        auto pip = new pipeFuture0(aName);
        pipeline::call<int>(aName + "_pipe_add", 0);
        for (auto i : pip->m_next2)
            insertNext(i.first, i.second);
        for (auto i : pip->m_locals.keys())
            m_locals.insert(i, pip->m_locals.value(i));
        pipeline::remove(aName);
    }
    pipeline::add<int>([this, aName](const stream<int>* aInput){
        auto tmp = aName;
        auto this_event = pipeline::find(aName, false);
        for (auto i : m_next2)
            this_event->insertNext(i.first, i.second);

        for (auto i : m_locals.keys()){
            auto loc = this_event->createLocal(aName, m_locals.value(i));
            if (loc){
                auto pip = new pipeFuture0(i);
                pipeline::call<int>(i + "_pipe_add", 0);
                for (auto i : pip->m_next2)
                    loc->insertNext(i.first, i.second);
                pipeline::remove(i);

                pipeline::instance()->m_pipes.remove(loc->m_name);
                loc->m_name = i;
                pipeline::instance()->m_pipes.insert(i, loc);
            }else{  //pipeFuture0
                reinterpret_cast<pipeFuture0*>(this_event)->m_locals.insert(i, m_locals.value(i));
            }
        }

        pipeline::remove(m_name);
    }, Json("name", aName + "_pipe_add"));
}

void pipeline::remove(const QString& aName){
    auto pipe = instance()->m_pipes.value(aName);
    if (pipe){
        //std::cout << "pipe: " + aName.toStdString() + " is removed!" << std::endl;
        //run("re_log", STMJSON(dst::Json("msg", "pipe " + aName + " is removed")));
        instance()->m_pipes.remove(aName);
        delete pipe; //if aName is from pipe, this must be write in the end
    }
}

QThread* pipeline::findThread(int aNo){
    auto thread = m_threads.find(aNo);
    if (thread == m_threads.end()){
        auto tmp = std::make_shared<QThread>();
        tmp->start();
        m_threads.insert(aNo, tmp);
        thread = m_threads.find(aNo);
    }
    return thread->get();
}

pipeline* pipeline::instance(){
    static std::mutex apipeline_mutex;
    std::lock_guard<std::mutex> lg(apipeline_mutex);
    static pipeline ret;  //if realized in .h, there will be multi objects in different dlls
    return &ret;
}

pipeline::pipeline(){
    QThreadPool::globalInstance()->setMaxThreadCount(8);
}

pipeline::~pipeline(){
    for (auto i : m_threads)
        if (i.get()->isRunning()){
            i.get()->terminate();
            i->wait();
        }
    for (auto i : m_pipes.values())
        delete i;
}

QVariant qmlStream::var(const QString& aName, QJSValue aData){
    if (aData.isObject())
        m_cache->insert(aName, std::make_shared<stream<QJsonObject>>(QJsonObject::fromVariantMap(aData.toVariant().toMap())));
    else if (aData.isArray())
        m_cache->insert(aName, std::make_shared<stream<QJsonArray>>(QJsonArray::fromVariantList(aData.toVariant().toList())));
    else if (aData.isNumber())
        m_cache->insert(aName, std::make_shared<stream<double>>(aData.toNumber()));
    else if (aData.isBool())
        m_cache->insert(aName, std::make_shared<stream<bool>>(aData.toBool()));
    else if (aData.isString())
        m_cache->insert(aName, std::make_shared<stream<QString>>(aData.toString()));
    else
        qFatal("Invalid data type in qmlStream!");
    return QVariant::fromValue<QObject*>(this);
}

QJSValue qmlStream::varData(const QString& aName, const QString& aType){
#define getVarData(TP) \
{ \
    auto ret = std::dynamic_pointer_cast<stream<TP>>(m_cache->value(aName)); \
    if (ret) \
        return pipeline::instance()->engine->toScriptValue(ret->data()); \
    else  \
        return pipeline::instance()->engine->toScriptValue(TP()); \
}

    if (aType == "object")
        getVarData(QJsonObject)
    else if (aType == "array")
        getVarData(QJsonArray)
    else if (aType == "string")
        getVarData(QString)
    else if (aType == "bool")
        getVarData(bool)
    else if (aType == "number")
        getVarData(double)
    else
        qFatal("Invalid data type in qmlStream!");
    return QJSValue();
}

pipe0* local(const QString& aName, const QJsonObject& aParam){
    return pipeline::find(aName)->createLocal(aName, aParam);
}

crashDump::crashDump(){
    pipeline::add<double>([this](rea::stream<double>* aInput){
        auto pip = pipeline::instance();
        QFile fl(".crash");
        if (fl.open(QFile::WriteOnly)){
            /*QString cnt = "";
            auto up = size_t(std::min(50, int(pip->m_log_count)));
            for (size_t i = pip->m_log_index; i < up; ++i)
                cnt += pip->m_logs[i] + "\n";
            if (pip->m_log_count > 50){
                for (size_t i = 0; i < pip->m_log_index; ++i)
                    cnt += pip->m_logs[i] + "\n";
            }
            fl.write(cnt.toUtf8());*/
            fl.close();
        }
    }, rea::Json("name", "crashDump"));
}

static crashDump dump;

#define regCreateJSPipe(Name) \
static regPip<std::shared_ptr<ICreateJSPipe>> reg_createJSPipe_##Name([](stream<std::shared_ptr<ICreateJSPipe>>* aInput){ \
    auto dt = aInput->data(); \
    auto prm = dt->param; \
    if (!prm.contains("vtype")){ \
        dt->param.insert("actname", pipeline::add<QJsonObject, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    }else{ \
        auto tp = prm.value("vtype"); \
        if (tp.isString()) \
            dt->param.insert("actname", pipeline::add<QString, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
        else if (tp.isDouble()) \
            dt->param.insert("actname", pipeline::add<double, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
        else if (tp.isBool()) \
            dt->param.insert("actname", pipeline::add<bool, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
        else if (tp.isArray()) \
             dt->param.insert("actname", pipeline::add<QJsonArray, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
        else \
            assert(0); \
    } \
}, Json("name", STR(createJSPipe_##Name)));

regCreateJSPipe(Partial)
regCreateJSPipe(Delegate)
regCreateJSPipe(Buffer)
regCreateJSPipe(Local)
regCreateJSPipe(Throttle)
regCreateJSPipe()

void test1(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 3);
        aInput->out();
    }, Json("name", "test1"))
        ->next(pipeline::add<int>([](stream<int>* aInput){
            assert(aInput->data() == 3);
            aInput->outs<QString>("Pass: test1", "testSuccess");
        }))
        ->next("testSuccess");

    pipeline::run("test1", 3);
}

void test2(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 4);
        std::stringstream ss;
        ss << std::this_thread::get_id();
        aInput->outs<std::string>(ss.str(), "test2_0");
    }, Json("name", "test2"))
        ->next(pipeline::add<std::string>([](stream<std::string>* aInput){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            assert(ss.str() != aInput->data());
            aInput->outs<QString>("Pass: test2", "testSuccess");
        }, Json("name", "test2_0", "thread", 2)))
        ->next("testSuccess");

    pipeline::run("test2", 4);
}

static rea::regPip<int> reg_test3([](rea::stream<int>* aInput){
    aInput->outs<QString>("Pass: test3___", "testSuccess");
}, QJsonObject(), "unitTest");

void test3(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->outs<QString>("test3", "test3_0");
    }, Json("name", "test3"))
        ->next("test3_0")
        ->next("testSuccess");

    pipeline::add<QString>([](rea::stream<QString>* aInput){
        aInput->out();
    }, rea::Json("name", "test3_1"))
        ->next(rea::local("test3__"))
        ->next("testSuccess");

    rea::pipeline::find("test3_0")
        ->nextF<QString>([](rea::stream<QString>* aInput){
            aInput->out();
        }, "", Json("name", "test3__"))
        ->next("testSuccess");

    pipeline::add<QString>([](stream<QString>* aInput){
        assert(aInput->data() == "test3");
        aInput->outs<QString>("Pass: test3", "testSuccess");
        aInput->outs<QString>("Pass: test3_", "test3__");
    }, Json("name", "test3_0"));

    pipeline::run<int>("test3", 66);
    pipeline::run<QString>("test3_1", "Pass: test3__");
}

void test4(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 56);
        aInput->outs<QString>("8", "");
    }, Json("name", "test4"))
        ->nextB(local("test4_0")
                    ->nextB(rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
                                assert(aInput->data() == "Pass: test4");
                                aInput->outs<QString>("Pass: test4__");
                            })->nextB(local("testSuccess"))))
        //->next("test4_0")
        ->next(local("test4_0"))
        ->next(local("testSuccess"));

    pipeline::add<QString>([](stream<QString>* aInput){
        assert(aInput->data() == "8");
        //aInput->setData("Pass: test4");
        aInput->outs<QString>("Pass: test4");
    }, Json("name", "test4_0"))
        ->next("testFail");

    pipeline::run("test4", 56);

    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        assert(aInput->data().value("56") == "56");
        aInput->out();
    }, Json("name", "test4_"))
        //->next("test4_1");
        ->next(local("test4_1"))
        ->next(local("testSuccess"));
    pipeline::run("test4_", Json("56", "56"));
}

void test5(){
    pipeline::add<int, pipeDelegate>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->out();
    }, Json("name", "test5_0", "delegate", "test5"))
        ->next("testSuccess");

    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 56);
        aInput->outs<QString>("Pass: test5", "testSuccess");
    }, Json("name", "test5"));

    pipeline::run("test5_0", 66);
    pipeline::run("test5", 56);
}

void test6(){
    pipeline::add<int, pipePartial>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->setData(77);
        aInput->out();
        //aInput->out(dst::Json("tag", "service2"));
    }, Json("name", "test6"));

    pipeline::find("test6")
        ->nextB(pipeline::add<int>([](stream<int>* aInput){
                    assert(aInput->data() == 77);
                    aInput->outs<QString>("Pass: test6", "testSuccess");
                })->nextB("testSuccess"), "service1")
        ->next(pipeline::add<int>([](stream<int>* aInput){
            assert(aInput->data() == 77);
            aInput->outs<QString>("Fail: test6", "testFail");
        }), "service2")
        ->next("testFail");

    pipeline::run("test6", 66, "service1");
}

void test7(){
    pipeline::add<int, pipeBuffer>(nullptr, Json("name", "test7", "count", 2))
        ->next(pipeline::add<std::vector<int>>([](stream<std::vector<int>>* aInput){
            auto dt = aInput->data();
            assert(dt.size() == 2);
            assert(dt.at(0) == 66);
            assert(dt.at(1) == 68);
            aInput->outs<QString>("Pass: test7", "testSuccess");
        }))
        ->next("testSuccess");
    pipeline::run<int>("test7", 66);
    pipeline::run<int>("test7", 68);

    pipeline::run<QJsonObject>("test7_", Json("hello", "world"));
    pipeline::run<QJsonObject>("test7_", Json("hello", "world2"));
}

void test8(){
    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        aInput->var<QString>("test8_var", "test8_var");
        aInput->out();
    }, Json("name", "test8"))->next("test8_");

    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        assert(aInput->data().value("test8") == "test8");
        assert(aInput->varData<QString>("test8_var") == "test8_var_");
        aInput->outs<QString>("Pass: test8", "testSuccess");
    }, Json("name", "test8_0"))
        ->next("testSuccess");

    pipeline::run("test8", Json("test8", "test8"));
}

void test9(){
    static std::mutex mtx;
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        aInput->var<std::shared_ptr<QSet<QThread*>>>("threads", std::make_shared<QSet<QThread*>>())
            ->var<int>("count", 0);
        for (int i = 0; i < 200; ++i)
            aInput->outs<double>(i);
    }, rea::Json("name", "test9"))
        ->next(rea::pipeline::add<double, rea::pipeParallel>([](rea::stream<double>* aInput){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto trds = aInput->varData<std::shared_ptr<QSet<QThread*>>>("threads");
            trds->insert(QThread::currentThread());
            {
                std::lock_guard<std::mutex> gd(mtx);
                aInput->var<int>("count", aInput->varData<int>("count") + 1);
            }
            aInput->out();
        }))->nextF<double>([](rea::stream<double>* aInput){
            std::lock_guard<std::mutex> gd(mtx);
            auto cnt = aInput->varData<int>("count");
            if (cnt == 200){
                aInput->var<int>("count", cnt + 1);
                assert(aInput->varData<std::shared_ptr<QSet<QThread*>>>("threads")->size() == 8);
                aInput->outs<QString>("Pass: test9", "testSuccess");
            }
        });
    rea::pipeline::run<double>("test9", 0);
}

void test10(){
    static int cnt = 0;
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        for (int i = 0; i < 100; ++i)
            aInput->setData(i)->outs<double>(0, "", "", false)->var<int>("count", i);
    }, rea::Json("name", "test10"))
        ->next(rea::pipeline::add<double, rea::pipeThrottle>([](rea::stream<double>* aInput) {
            aInput->out();
        }))
        ->nextF<double>([](rea::stream<double>* aInput){
            ++cnt;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (aInput->varData<int>("count") == 99){
                assert(cnt < 100);
                aInput->outs<QString>("Pass: test10", "testSuccess");
            }
        }, "", rea::Json("thread", 6));
    rea::pipeline::run<double>("test10", 0);
}

void testReactive2(){

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Success:" << aInput->data().toStdString() << std::endl;
    }, Json("name", "testSuccess"));

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Fail:" << aInput->data().toStdString() << std::endl;
    }, Json("name", "testFail"));

    test1(); // test anonymous next
    test2(); // test specific next and multithread
    test3(); // test pipe future
    test4(); // test pipe local
    test5(); // test pipe delegate and pipe param
    test6(); // test pipe partial and next/stream param and nextB
    test7(); // test pipe Buffer
    test8(); // test pipe QML
    test9(); // test pipe parallel
    test10(); // test pipe throttle
}

static regPip<int> unit_test([](stream<int>* aInput){
    pipeline::instance()->engine->load(QUrl(QStringLiteral("qrc:/qml/test.qml")));
    testReactive2();
    aInput->out();
}, rea::Json("name", "unitTest"));

}
