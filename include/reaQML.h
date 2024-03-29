#ifndef REAQML_H_
#define REAQML_H_

#include "reaC++.h"
#include <QObject>
#include <QJSValue>
#include <QQmlEngine>
#include <QDoubleValidator>

namespace rea {

class DSTDLL qmlStream : public QObject
{
    Q_OBJECT
public:
    qmlStream(){}
    qmlStream(QJSValue aInput, const QString& aTag = "", std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> aCache = nullptr, std::shared_ptr<transaction> aTransaction = nullptr){
        m_data = aInput;
        m_tag = aTag;
        if (aCache)
            m_cache = aCache;
        else
            m_cache = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
        m_transaction = aTransaction;
    }
    Q_INVOKABLE QVariant setData(QJSValue aData){
        m_data = aData;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QJSValue data(){
        return m_data;
    }
    Q_INVOKABLE QVariant out(const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        m_tag = aTag;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QVariant outs(QJSValue aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        auto ot = std::make_shared<qmlStream>(aOut, aTag, aShareCache ? m_cache : nullptr, m_transaction);
        m_outs->push_back(std::pair<QString, std::shared_ptr<qmlStream>>(aNext, ot));
        return QVariant::fromValue<QObject*>(ot.get());
    }
    Q_INVOKABLE QVariant outsB(QJSValue aOut, const QString& aNext = "", const QString& aTag = "", bool aShareCache = true){
        outs(aOut, aNext, aTag, aShareCache);
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QVariant outs(QJSValue aOut, const QString& aNext, const QString& aTag, int aShareCache){
        auto cache = m_cache;
        if (m_outs && size_t(aShareCache) < m_outs->size())
            cache = m_outs->at(size_t(aShareCache)).second->m_cache;
        auto ot = std::make_shared<qmlStream>(aOut, aTag, cache, m_transaction);
        m_outs->push_back(std::pair<QString, std::shared_ptr<qmlStream>>(aNext, ot));
        return QVariant::fromValue<QObject*>(ot.get());
    }
    Q_INVOKABLE QVariant outsB(QJSValue aOut, const QString& aNext, const QString& aTag, int aShareCache){
        outs(aOut, aNext, aTag, aShareCache);
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QVariant map(QJSValue aInput);
    Q_INVOKABLE QVariant call(const QString& aName, const QString& aType = "");
    Q_INVOKABLE QVariant call(QJSValue aFunc, const QJsonObject& aParam = QJsonObject());
    Q_INVOKABLE void noOut(){m_outs = nullptr;}
    Q_INVOKABLE QVariant var(const QString& aName, QJSValue aData);
    Q_INVOKABLE QJSValue varData(const QString& aName, const QString& aType = "object");
    Q_INVOKABLE void fail(){
        if (m_transaction)
            m_transaction->fail();
    }
    Q_INVOKABLE bool failed(){
        return m_transaction ? m_transaction->failed() : false;
    }
    Q_INVOKABLE void log(const QString& aLog){
        if (m_transaction)
            m_transaction->log(aLog);
    }
    Q_INVOKABLE QString transactionName(){
        return m_transaction ? m_transaction->getName() : "";
    }
    Q_INVOKABLE QString tag(){
        return m_tag;
    }
    Q_INVOKABLE QString cache(){
        m_cache_id = generateUUID();
        return m_cache_id;
    }
private:
    template<typename T, typename S = T>
    void doCall(const QString& aName, const T& aData){
        auto pip = pipeline::find(aName, false);
        if (!pip)
            return;
        QEventLoop loop;
        bool timeout = false;
        auto monitor = pipeline::find(aName)->nextF<S>([&loop, &timeout, this](stream<S>* aInput){
            m_data = pipeline::instance()->engine->toScriptValue(aInput->data());
            if (loop.isRunning()){
                loop.quit();
            }else
                timeout = true;
        }, m_tag);
        pip->execute(std::make_shared<stream<T>>(aData, m_tag, m_cache, m_transaction));
        if (!timeout)
            loop.exec();
        pipeline::remove(monitor->actName());
    }

    QJSValue m_data;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>> m_outs = nullptr;
    QString m_tag;
    QString m_cache_id = "";
    std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> m_cache;
    std::shared_ptr<transaction> m_transaction;
    template<typename T, typename F>
    friend class funcType;
};

template <typename T>
class valType{
public:
    static T data(const QJSValue&){
        return T();
    }
};

template <>
class valType<QJsonObject>{
public:
    static QJsonObject data(const QJSValue& aValue){
        return QJsonObject::fromVariantMap(aValue.toVariant().toMap());
    }
};

template <>
class valType<QJsonArray>{
public:
    static QJsonArray data(const QJSValue& aValue){
        return QJsonArray::fromVariantList(aValue.toVariant().toList());
    }
};

template <>
class valType<QString>{
public:
    static QString data(const QJSValue& aValue){
        return aValue.toString();
    }
};

template <>
class valType<double>{
public:
    static double data(const QJSValue& aValue){
        return aValue.toNumber();
    }
};

template <>
class valType<bool>{
public:
    static bool data(const QJSValue& aValue){
        return aValue.toBool();
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
            qmlStream stm(pipeline::instance()->engine->toScriptValue(aStream->data()), aStream->tag(), aStream->m_cache, aStream->m_transaction);
            paramList.append(pipeline::instance()->engine->toScriptValue(QVariant::fromValue<QObject*>(&stm)));
            aFunc.call(paramList);
            aStream->setData(valType<T>::data(stm.data()));
            if (stm.m_outs){
                aStream->out(stm.m_tag);
                for (auto i : *stm.m_outs){
                    if (i.second->data().isArray()){  //isArray must isObject
                        auto ot = aStream->template outs<QJsonArray>(valType<QJsonArray>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_cache != aStream->m_cache)
                            ot->m_cache = i.second->m_cache;
                    }else if (i.second->data().isObject()){
                        auto ot = aStream->template outs<QJsonObject>(valType<QJsonObject>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_cache != aStream->m_cache)
                            ot->m_cache = i.second->m_cache;
                    }else if (i.second->data().isNumber()){
                        auto ot = aStream->template outs<double>(valType<double>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_cache != aStream->m_cache)
                            ot->m_cache = i.second->m_cache;
                    }else if (i.second->data().isBool()){
                        auto ot = aStream->template outs<bool>(valType<bool>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_cache != aStream->m_cache)
                            ot->m_cache = i.second->m_cache;
                    }else if (i.second->data().isString()){
                        auto ot = aStream->template outs<QString>(valType<QString>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_cache != aStream->m_cache)
                            ot->m_cache = i.second->m_cache;
                    }else if (i.second->data().isQObject()){

                    }
                    else
                        qFatal("Invalid data type in qmlStream!");
                }
            }
            if (stm.m_cache_id != "")
                aStream->cache(stm.m_cache_id);
        }
    }
    std::shared_ptr<stream0> createStreamList(std::vector<T>& aDataList, std::shared_ptr<stream<T>> aStream){
        QJsonArray lst;
        for (int i = 0; i < aDataList.size(); ++i)
            lst.push_back(QJsonValue(aDataList[i]));
        auto stms = std::make_shared<stream<QJsonArray>>(lst, "", aStream->m_cache, aStream->m_transaction);
        stms->out();
        return std::move(stms);
    }
};

class pipelineQML;

class qmlPipe : public QObject
{
    Q_OBJECT
public:
    qmlPipe(){}
public:
    Q_INVOKABLE QString actName() {return m_pipe;}
    Q_INVOKABLE QVariant nextP(QVariant aNext, const QString& aTag = "");
    Q_INVOKABLE QVariant next(QJSValue aNext, const QString& aTag = "", const QJsonObject& aPipeParam = QJsonObject());
    Q_INVOKABLE QVariant nextB(QJSValue aNext, const QString& aTag = "", const QJsonObject& aPipeParam = QJsonObject());
    Q_INVOKABLE QVariant next(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE QVariant nextB(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE QVariant nextL(const QString& aName, const QString& aTag = "", const QJsonObject& aPipeParam = QJsonObject());
    Q_INVOKABLE void removeNext(const QString& aName);
    static qmlPipe* createPipe(QJSValue aFunc, const QJsonObject& aParam);
private:
    QString m_pipe;
    QJsonObject m_param;
    friend pipelineQML;
};

class DSTDLL pipelineQML : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(pipelineQML)
public:
    pipelineQML();
    ~pipelineQML();
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        return new pipelineQML();
    }
    static Q_INVOKABLE bool run(const QString& aName, const QJSValue& aInput, const QString& aTag = "", bool aTransaction = true, const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void runC(const QString& aName, const QJSValue& aInput, const QString& aStreamID, const QString& aTag = "");
    static Q_INVOKABLE void syncCall(const QString& aName, const QJSValue& aInput);
    static Q_INVOKABLE QVariant call(const QString& aName, const QJSValue& aInput);
    static Q_INVOKABLE QVariant input(const QJSValue& aInput, const QString& aTag = "", bool aTransaction = true, const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void remove(const QString& aName);
    static Q_INVOKABLE void removeAspect(const QString& aPipe, pipe0::AspectType aType, const QString& aAspect = "");
    /*
     * @aParam
     * aParam["name"]: pipeName, string, if is empty, it will be set random
     * aParam["type"]: pipeType, string, if is empty, it will be used as default
     * aParam["param"]: pipeParam, json
    */
    static Q_INVOKABLE QVariant add(QJSValue aFunc, const QJsonObject& aPipeParam = QJsonObject());
    static Q_INVOKABLE QVariant find(const QString& aName);
    static Q_INVOKABLE QVariant tr(const QString& aOrigin);
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

DSTDLL QString tr0(const QString& aOrigin);

//https://stackoverflow.com/questions/35178569/doublevalidator-is-not-checking-the-ranges-properly
class TextFieldDoubleValidator : public QDoubleValidator {
public:
    TextFieldDoubleValidator (QObject * parent = 0) : QDoubleValidator(parent) {}
    TextFieldDoubleValidator (double bottom, double top, int decimals, QObject * parent) :
                                                                                         QDoubleValidator(bottom, top, decimals, parent) {}

    QValidator::State validate(QString & s, int & pos) const {
        if (s.isEmpty() || (s.startsWith("-") && s.length() == 1)) {
            // allow empty field or standalone minus sign
            return QValidator::Intermediate;
        }
        // check length of decimal places
        QChar point = locale().decimalPoint();
        if(s.indexOf(point) != -1) {
            int lengthDecimals = s.length() - s.indexOf(point) - 1;
            if (lengthDecimals > decimals()) {
                return QValidator::Invalid;
            }
        }
        // check range of value
        bool isNumber;
        double value = locale().toDouble(s, &isNumber);
        if (isNumber && bottom() <= value && value <= top()) {
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }

};

}

#endif
