#ifndef REAL_FRAMEWORK_RECATIVE2QML_H_
#define REAL_FRAMEWORK_RECATIVE2QML_H_

#include "reactive2.h"
#include <QObject>
#include <QJSValue>
#include <QQmlEngine>
#include <QDoubleValidator>

namespace rea {

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
    void setPipe(const QString& aPipe) {m_pipe = aPipe;}
    static qmlPipe* createPipe(QJSValue aFunc, const QJsonObject& aParam);
private:
    QString m_pipe;
    QJsonObject m_param;
};

class pipelineQML : public QObject
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
    static Q_INVOKABLE void run(const QString& aName, const QJSValue& aInput, const QString& aTag = "", bool aRoutine = true);
    static Q_INVOKABLE void remove(const QString& aName);
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

QString tr0(const QString& aOrigin);

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
