#include "reaQML.h"
#include <QFile>
#include <QJsonDocument>

namespace rea {

QVariant qmlPipe::nextP(QVariant aNext, const QString& aTag){
    auto nxt = qobject_cast<qmlPipe*>(qvariant_cast<QObject*>(aNext));
    pipeline::find(m_pipe)->next(nxt->m_pipe, aTag);
    return aNext;
}

QVariant qmlPipe::next(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    auto ret = createPipe(aNext, aPipeParam);
    pipeline::find(m_pipe)->next(ret->m_pipe, aTag);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant qmlPipe::nextB(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    next(aNext, aTag, aPipeParam);
    return QVariant::fromValue<QObject*>(this);
}

QVariant qmlPipe::next(const QString& aName, const QString& aTag){
    qmlPipe* ret = new qmlPipe();
    ret->m_pipe = pipeline::find(m_pipe)->next(aName, aTag)->actName();
    return QVariant::fromValue<QObject*>(ret);
}

QVariant qmlPipe::nextB(const QString& aName, const QString& aTag){
    next(aName, aTag);
    return QVariant::fromValue<QObject*>(this);
}

QVariant qmlPipe::nextL(const QString& aName, const QString& aTag, const QJsonObject& aPipeParam){
    qmlPipe* ret = new qmlPipe();
    ret->m_pipe = pipeline::find(aName)->createLocal(aName, aPipeParam)->actName();
    pipeline::find(m_pipe)->next(ret->m_pipe, aTag);
    return QVariant::fromValue<QObject*>(ret);
}

void qmlPipe::removeNext(const QString& aName){
    pipeline::find(m_pipe)->removeNext(aName);
}

qmlPipe* qmlPipe::createPipe(QJSValue aFunc, const QJsonObject& aParam){
    qmlPipe* ret = new qmlPipe();
    auto tp = aParam.value("type").toString();

    auto prm = std::make_shared<ICreateJSPipe>(aParam, aFunc);
    pipeline::run<std::shared_ptr<ICreateJSPipe>>("createJSPipe_" + tp, prm);
    if (prm->param.contains("actname"))
        ret->m_pipe = prm->param.value("actname").toString();
    else
        assert(0);

    return ret;
}

static bool m_language_updated;
static QJsonObject translates;

pipelineQML::pipelineQML(){
    m_language_updated = false;
    QFile fl(".language");
    if (fl.open(QFile::ReadOnly)){
        translates = QJsonDocument::fromJson(fl.readAll()).object();
        fl.close();
    }
}

pipelineQML::~pipelineQML(){
    if (m_language_updated){
        QFile fl(".language");
        if (fl.open(QFile::WriteOnly)){
            fl.write(QJsonDocument(translates).toJson());
            fl.close();
        }
    }
}

void pipelineQML::run(const QString& aName, const QJSValue& aInput, const QString& aTag, bool aRoutine){
    if (aInput.isString())
        pipeline::run<QString>(aName, aInput.toString(), aTag, aRoutine);
    else if (aInput.isBool())
        pipeline::run<bool>(aName, aInput.toBool(), aTag, aRoutine);
    else if (aInput.isNumber())
        pipeline::run<double>(aName, aInput.toNumber(), aTag, aRoutine);
    else if (aInput.isArray())
        pipeline::run<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()), aTag, aRoutine);
    else
        pipeline::run<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()), aTag, aRoutine);
}

void pipelineQML::remove(const QString& aName){
    pipeline::remove(aName);
}

QVariant pipelineQML::add(QJSValue aFunc, const QJsonObject& aPipeParam){
    auto ret = qmlPipe::createPipe(aFunc, aPipeParam);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant pipelineQML::find(const QString& aName){
    qmlPipe* ret = new qmlPipe();
    ret->setPipe(aName);
    return QVariant::fromValue<QObject*>(ret);
}

QString tr0(const QString& aOrigin){
    auto key = aOrigin.trimmed();
    if (!translates.contains(key)){
        translates.insert(key, aOrigin);
        m_language_updated = true;
    }
    return translates.value(key).toString(aOrigin);
}

QVariant pipelineQML::tr(const QString& aOrigin){
    return tr0(aOrigin);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //https://stackoverflow.com/questions/54307407/why-am-i-getting-qwindowswindowsetgeometry-unable-to-set-geometry-warning-wit
    /*if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry")) {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stdout, localMsg.constData());
    }*/
    //https://blog.csdn.net/liang19890820/article/details/51838096
    QByteArray localMsg = msg.toLocal8Bit();
    auto ret = QString(localMsg) + " (" + context.file + ":" + context.line + ", " + context.function + ")";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "debug", "msg", "Debug: " + ret));
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "info", "msg", "Info: " + ret));
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "warning", "msg", "Warning: " + ret));
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "critical", "msg", "Critical: " + ret));
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "error", "msg", "Fatal: " + ret));
        abort();
    }
}


static regPip<QQmlApplicationEngine*> reg_recative2_qml([](stream<QQmlApplicationEngine*>* aInput){

    qInstallMessageHandler(myMessageOutput);

    //ref from: https://stackoverflow.com/questions/25403363/how-to-implement-a-singleton-provider-for-qmlregistersingletontype
    pipeline::instance()->engine = aInput->data();
    qmlRegisterSingletonType<pipelineQML>("Pipeline", 1, 0, "Pipeline", &pipelineQML::qmlInstance);
    qmlRegisterType<TextFieldDoubleValidator>("TextFieldDoubleValidator", 1, 0, "TextFieldDoubleValidator");
    aInput->out();
}, rea::Json("name", "regQML"));

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    aInput->data()->load(QUrl(QStringLiteral("qrc:/gui/main.qml")));
    aInput->out();
}, rea::Json("name", "loadGUIMain"));

}

