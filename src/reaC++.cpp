#include "reaC++.h"
#include <mutex>
#include <sstream>
#include <QFile>
#include <QCoreApplication>

namespace rea {

void transaction::executed(const QString& aPipe){
    auto cnt = m_candidates.value(aPipe) - 1;
    if (!cnt)
        m_candidates.remove(aPipe);
    else if (cnt > 0)
        m_candidates.insert(aPipe, cnt);
}

void transaction::addTrig(const QString& aStart, const QString& aNext){
    std::lock_guard<std::mutex> gd(m_mutex);
    m_logs.push_back(aStart + " > " + aNext);
    if (!m_candidates.contains(aNext))
        m_candidates.insert(aNext, 0);
    m_candidates.insert(aNext, m_candidates.value(aNext) + 1);
}

transaction::transaction(const QString& aName, const QString& aTag){
    m_name = aName + ";" + aTag;
    addTrig(aTag + ":", aName);
}

transaction::~transaction(){
    rea::pipeline::run<QJsonObject>("transactionEnd", rea::Json("name", m_name, "detail", print()), "", false);
}

void transaction::log(const QString& aLog){
    std::lock_guard<std::mutex> gd(m_mutex);
    m_logs.push_back(aLog);
}

const QString transaction::print(){
    std::lock_guard<std::mutex> gd(m_mutex);
    QString ret = "******************** " + m_name + "\n";
    for (auto i : m_logs)
        ret += " " + i + "\n";
    for (auto i : m_candidates.keys())
        ret += " alive: " + i + "*" + QString::number(m_candidates.size()) + "\n";
    return ret;
}

void stream0::executed(const QString& aPipe){
    if (m_transaction)
        m_transaction->executed(aPipe);
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
                            aStream->addTrig(workName(), pip->workName());
                            pip->execute(i.second);
                        }
                    }
                }else{
                    if (aNexts.contains(i.first)){
                        auto pip = pipeline::find(i.first, false);
                        if (pip){
                            aStream->addTrig(workName(), pip->workName());
                            pip->execute(i.second);
                        }
                    }else{
                        bool exed = false;
                        for (auto j : aNexts.keys()){
                            auto pip = pipeline::find(j, false);
                            if (pip && pip->localName() == i.first){
                                aStream->addTrig(workName(), pip->workName());
                                pip->execute(i.second);
                                exed = true;
                            }
                        }
                        if (!exed){
                            auto pip = pipeline::find(i.first, false);
                            if (pip){
                                aStream->addTrig(workName(), pip->workName());
                                pip->execute(i.second);
                            }
                        }
                    }
                }
        }else
            for (auto i : aNexts.keys()){
                auto pip = pipeline::find(i, false);
                if (pip){
                    aStream->addTrig(workName(), pip->workName());
                    pip->execute(aStream);
                }
            }
    }
}

void pipe0::execute(std::shared_ptr<stream0> aStream){
    if (QThread::currentThread() == m_thread){
        streamEvent nxt_eve(m_name, aStream);
        QCoreApplication::sendEvent(this, &nxt_eve);
    }else{
        auto nxt_eve = std::make_unique<streamEvent>(m_name, aStream);
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
        m_before = pip->m_before;
        m_after = pip->m_after;
        for (auto i : pip->m_locals.keys())
            m_locals.insert(i, pip->m_locals.value(i));
        pipeline::remove(aName);
    }
    pipeline::add<int>([this, aName](const stream<int>* aInput){
        auto tmp = aName;
        auto this_event = pipeline::find(aName, false);
        for (auto i : m_next2)
            this_event->insertNext(i.first, i.second);
        this_event->m_before = m_before;
        this_event->m_after = m_after;

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

pipe0* local(const QString& aName, const QJsonObject& aParam){
    return pipeline::find(aName)->createLocal(aName, aParam);
}

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

static rea::regPip<QJsonObject> reg_test3([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("rea").toBool()){
        aInput->out();
        return;
    }
    aInput->outs<QString>("Pass: test3___", "testSuccess");
    aInput->out();
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
    pipeline::run("test5", 56, "", false);
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
    pipeline::add<int>([](rea::stream<int>* aInput){
        aInput->outs<int>(66);
        aInput->outs<int>(68);
    }, rea::Json("name", "test7"))
    ->next(buffer<int>(2))
    ->next(pipeline::add<std::vector<int>>([](stream<std::vector<int>>* aInput){
        auto dt = aInput->data();
        assert(dt.size() == 2);
        assert(dt.at(0) == 66);
        assert(dt.at(1) == 68);
        aInput->outs<QString>("Pass: test7", "testSuccess");
    }))
    ->next("testSuccess");
    pipeline::run<int>("test7", 0);
    pipeline::run<QJsonObject>("test7_", QJsonObject());
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
            {
                std::lock_guard<std::mutex> gd(mtx);
                auto trds = aInput->varData<std::shared_ptr<QSet<QThread*>>>("threads");
                trds->insert(QThread::currentThread());
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

void test11(){
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 1.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test__11", "before", "test_11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 2.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test_11", "before", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 3.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 4.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test11_", "after", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 5.0);
        aInput->outs<QString>("Pass: test11", "testSuccess");
    }, rea::Json("name", "test11__", "after", "test11_"));

    rea::pipeline::run<double>("test11", 1);
}

void testReactive2(){
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
    test11(); //test pipe aop
}

static regPip<QJsonObject> unit_test([](stream<QJsonObject>* aInput){
    pipeline::instance()->engine->load(QUrl(QStringLiteral("gui/test.qml")));

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Success:" << aInput->data().toStdString() << std::endl;
    }, Json("name", "testSuccess"));

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Fail:" << aInput->data().toStdString() << std::endl;
    }, Json("name", "testFail"));

    if (!aInput->data().value("rea").toBool()){
        aInput->out();
        return;
    }

    static std::vector<QString> transactions;
    static QHash<QString, transaction*> alive_transactions;
    rea::pipeline::add<double>([](stream<double>* aInput){
        testReactive2();
    }, rea::Json("name", "doUnitTest"));

    rea::pipeline::add<transaction*>([](stream<transaction*>* aInput){
        //std::cout << "***transaction start***: " << aInput->data().toStdString() << std::endl;
        auto rt = aInput->data();
        alive_transactions.insert(rt->getName(), rt);
    }, rea::Json("name", "transactionStart"));

    rea::pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto nm = dt.value("name").toString();
        transactions.push_back(dt.value("detail").toString());
        alive_transactions.remove(nm);
        //std::cout << "***transaction end***: " << dt.value("name").toString().toStdString() << std::endl;
    }, rea::Json("name", "transactionEnd"));

    rea::pipeline::add<double>([](stream<double>* aInput){
        if (aInput->data()){
            QString ret = "";
            for (auto i : transactions)
                ret += i;
            for (auto i : alive_transactions)
                ret += (i->print() + "running!\n");
            QFile sv("transaction.txt");
            if (sv.open(QFile::WriteOnly)){
                sv.write(ret.toUtf8());
                sv.close();
            }
        }else{
            for (auto i : transactions)
                std::cout << i.toStdString();
            for (auto i : alive_transactions)
                std::cout << (i->print().toStdString() + "running!\n");
        }
    }, rea::Json("name", "logTransaction"));

    aInput->out();
}, rea::Json("name", "unitTest"));

}
