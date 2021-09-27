#include "qsgBoard.h"
#include "reaC++.h"
#include "imagePool.h"
#include <QSGTransformNode>
#include <QTransform>

namespace rea{

QString qsgBoardPlugin::newShapeID(){
    return "shp_" + generateUUID();
}

std::function<bool(void)> qsgBoardPlugin::removeShape(const QString& aShape, bool aCommand){
    auto nm = getParentName();
    auto mdl = getQSGModel();
    if (mdl){
        auto id = getQSGModel()->value("id");
        return [nm, aShape, aCommand, id](){
            return rea::pipeline::run("updateQSGAttr_" + nm,
                               rea::Json("key", rea::JArray("objects"),
                                         "type", "del",
                                         "tar", aShape,
                                         "cmd", aCommand,
                                         "id", id), "delObject");
        };
    }else
        return nullptr;
}

std::function<bool(void)> qsgBoardPlugin::addPoly(const QString& aShape, const QJsonArray& aPoints, bool aCommand, int aFace){
    auto nm = getParentName();
    auto id = getQSGModel()->value("id");
    return [nm, aShape, aPoints, aCommand, id, aFace](){
        return rea::pipeline::run("updateQSGAttr_" + nm,
                           rea::Json("key", rea::JArray("objects"),
                                     "type", "add",
                                     "tar", aShape,
                                     "val", rea::Json(
                                                "type", "poly",
                                                "points", aPoints,
                                                "face", aFace),
                                     "cmd", aCommand,
                                     "id", id), "addPoly");
    };
}

std::function<bool(void)> qsgBoardPlugin::addEllipse(const QString& aShape, const QJsonArray& aCenter, const QJsonArray& aRadius, bool aCommand){
    auto nm = getParentName();
    auto id = getQSGModel()->value("id");
    return [nm, aShape, aCenter, aRadius, aCommand, id](){
        return rea::pipeline::run("updateQSGAttr_" + nm,
                           rea::Json("key", rea::JArray("objects"),
                                     "type", "add",
                                     "tar", aShape,
                                     "val", rea::Json(
                                                "type", "ellipse",
                                                "center", aCenter,
                                                "radius", aRadius,
                                                "face", 125),
                                     "cmd", aCommand,
                                     "id", id), "addEllipse");
    };
}

std::shared_ptr<shapeObject> qsgBoardPlugin::createEllipseHandle(QSGNode* aTransformNode, int aRadius, int aFace, const QJsonArray& aCenter, const QString& aColor){
    m_handles.push_back(std::make_shared<rea::ellipseObject>(rea::Json(
        "type", "ellipse",
        "center", aCenter,
        "radius", rea::JArray(aRadius, aRadius),
        "width", 0,
        "color", aColor,
        "face", aFace)));
    m_handles.back()->setParent(&m_mdl);
    m_handles.back()->getQSGNodes(m_parent, aTransformNode->parent());
    return m_handles.back();
}

void qsgBoardPlugin::updateHandlePos(int aIndex, const QPoint& aPos){
    if (aIndex < m_handles.size()){
        auto hdl = m_handles[aIndex];
        hdl->insert("center", rea::JArray(aPos.x(), aPos.y()));
        updateParent(hdl->updateQSGAttr("center_"));
    }
}

void qsgBoardPlugin::updateHandleRadius(int aIndex, int aRadius){
    if (aIndex < m_handles.size()){
        auto hdl = m_handles[aIndex];
        hdl->insert("radius", rea::JArray(aRadius, aRadius));
        updateParent(hdl->updateQSGAttr("radius_"));
    }
}

qsgPluginTransform::~qsgPluginTransform(){
//    pipeline::remove("updateQSGPos_" + getParentName());
//    pipeline::remove("updateQSGMenu_" + getParentName());
}

QString qsgPluginTransform::getName(qsgBoard* aParent) {
    auto ret = qsgBoardPlugin::getName(aParent);
    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "updateQSGPos_" + getParentName(), "replace", true));
    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "updateQSGMenu_" + getParentName(), "replace", true));
    rea::pipeline::find("QSGAttrUpdated_" + getParentName())
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            for (auto i : dt){
                if (i.toObject().value("type") == "zoom"){
                    auto mdl = getQSGModel();
                    if (mdl){
                        auto inv = getTransNode()->matrix().inverted();
                        aInput->outs<QJsonObject>(rea::Json("x", m_wcspos.x(),
                                                           "y", m_wcspos.y(),
                                                           "ratio", 100 / inv.data()[0],
                                                           "transform", mdl ? mdl->value("transform") : QJsonArray()), "updateQSGPos_" + getParentName());
                        break;
                    }
                }
            }
        aInput->out();
    }, rea::Json("name", "QSGTransformUpdated_" + getParentName())));  //the name is used to overwrite the same pipe of the old transform plugin
    return ret;
}

void qsgPluginTransform::wheelEvent(QWheelEvent *event){
    rea::pipeline::run<QJsonObject>("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("transform"),
                                                                         "type", "zoom",
                                                                         "dir", event->delta() < 0 ? - 1 : 1,
                                                                         "center", rea::JArray(m_lastpos.x(), m_lastpos.y())), "zoomWCS");
}

void qsgPluginTransform::mousePressEvent(QMouseEvent *event){
    rea::pipeline::run<QJsonObject>("updateQSGMenu_" + getParentName(), QJsonObject());
}

void qsgPluginTransform::mouseReleaseEvent(QMouseEvent *event){
    auto mn = event->button() == Qt::RightButton ? rea::Json(getMenu(), "x", event->x(), "y", event->y()) : QJsonObject();
    rea::pipeline::run<QJsonObject>("updateQSGMenu_" + getParentName(), mn);
}

void qsgPluginTransform::mouseMoveEvent(QMouseEvent *event){
    tryMoveWCS(event, Qt::MiddleButton);
    updatePos(event->pos());
}

bool qsgPluginTransform::tryMoveWCS(QMouseEvent *event, Qt::MouseButton aFlag){
    auto ret = event->buttons().testFlag(aFlag);
    if (ret){
        auto cur = event->pos();
        rea::pipeline::run<QJsonObject>("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("transform"),
                                                                                      "type", "move",
                                                                                      "del", rea::JArray(cur.x() - m_lastpos.x(), cur.y() - m_lastpos.y())), "moveWCS");
    }
    return ret;
}

void qsgPluginTransform::hoverMoveEvent(QHoverEvent *event){
    updatePos(event->pos());
}

void qsgPluginTransform::updatePos(const QPoint &aPos){
    auto inv = getTransNode()->matrix().inverted();
    m_lastpos = aPos;
    m_wcspos = inv.map(m_lastpos);
    auto mdl = getQSGModel();
    rea::pipeline::run<QJsonObject>("updateQSGPos_" + getParentName(), rea::Json("x", m_wcspos.x(),
                                                                                 "y", m_wcspos.y(),
                                                                                 "ratio", 100 / inv.data()[0],
                                                                                 "transform", mdl ? mdl->value("transform") : QJsonArray()));
}

static rea::regPip<QJsonObject, rea::pipePartial> create_qsgboardplugin_transform([](rea::stream<QJsonObject>* aInput){
    aInput->var<std::shared_ptr<qsgBoardPlugin>>("result", std::make_shared<qsgPluginTransform>(aInput->data()))->out();
}, rea::Json("name", "create_qsgboardplugin_transform"));

void qsgBoard::beforeDestroy(){
    if (m_models.size() > 0){
        //auto lst = m_models.front();
        //lst->clearQSGObjects();
        m_models.clear();
        m_updates.clear();
        m_updates_model_index.clear();
        m_updates_modification = QJsonArray();
    }
    for (auto i : m_plugins)
        i->beforeDestroy();
}

qsgBoard::~qsgBoard(){
    rea::pipeline::remove("updateQSGModel_" + m_name);
    rea::pipeline::remove("updateQSGAttr_" + m_name);
    rea::pipeline::remove("updateQSGAttrs_" + m_name);
    rea::pipeline::remove("updateQSGCtrl_" + m_name);
    rea::pipeline::remove("QSGAttrUpdated_" + m_name);
    if (m_add_qsg_plugin)
        rea::pipeline::remove(m_add_qsg_plugin->actName());
}

qsgBoard::qsgBoard(QQuickItem *parent) : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(true);
    setSmooth(true);
    setAntialiasing(true);
    setClip(true);
    //connect(, QQuickWindow::beforeSynchronizing(), this, SLOT(beforeRender()))
}

void qsgBoard::addUpdate(const IUpdateQSGAttr& aUpdate){
    m_updates_model_index.push_back(m_models.size() - 1);
    m_updates.push_back(aUpdate);
}

void tryFlushImageCache(const QJsonObject& aModification, QHash<QString, QImage>* aCache = nullptr){
    //qDebug() << "rea try flush image cache";
    if (aModification.contains("obj") && aModification.value("key") == QJsonArray({"path"})){
        auto pth = aModification.value("val").toString();
        auto img = rea::imagePool::readCache(pth);
        if (aCache){
            qDebug() << "update Image Path:" << pth;
            aCache->insert(pth, img);
        }
    }else{
        auto kys = aModification.value("key").toArray();
        if (kys.size() > 0){
            if (kys[0] == "objects" && (aModification.value("type") == "add")){
                auto val = aModification.value("val").toObject();
                if (val.contains("path")){
                    auto pth = val.value("path").toString();
                    auto img = rea::imagePool::readCache(pth);
                    if (aCache){
                        qDebug() << "add Image" << pth;
                        aCache->insert(pth, img);
                    }
                }
            }
        }
    }
}

void qsgBoard::setName(const QString& aName){
    m_name = aName;

    connect(this, &qsgBoard::heightChanged, [this](){
        if (m_models.size() > 0){
            addUpdate(m_models.back()->updateQSGAttr(rea::Json("key", rea::JArray("transform"), "type", "refresh")));
            update();
        }
    });

    connect(this, &qsgBoard::widthChanged, [this](){
        if (m_models.size() > 0){
            addUpdate(m_models.back()->updateQSGAttr(rea::Json("key", rea::JArray("transform"), "type", "refresh")));
            update();
        }
    });

    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto mdl = std::make_shared<qsgModel>(dt);
        auto che = mdl->getImageCache();
        auto objs = dt.value("objects").toObject();
        for (auto i : objs){
            auto obj = i.toObject();
            if (obj.contains("path")){
                auto pth = obj.value("path").toString();
                qDebug() << "updateQSGModel:" << pth;
                che->insert(pth, rea::imagePool::readCache(pth));
            }
        }
        m_models.push_back(mdl);
        update();
        aInput->out();
    }, rea::Json("name", "updateQSGModel_" + m_name));

    rea::pipeline::add<QJsonObject, pipeDelegate>([this](rea::stream<QJsonObject>* aInput){
        m_updates_modification.push_back(aInput->data());
        if (m_models.size() > 0){
            tryFlushImageCache(aInput->data(), m_models.back()->getImageCache());
            addUpdate(m_models.back()->updateQSGAttr(aInput->data()));
            update();
        }else{
            tryFlushImageCache(aInput->data());
            rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification, "", false);
            m_updates_modification = QJsonArray();
        }
        aInput->out();
    }, rea::Json("name", "updateQSGAttr_" + m_name, "delegate", "QSGAttrUpdated_" + m_name));

    rea::pipeline::add<QJsonArray, pipeDelegate>([this](rea::stream<QJsonArray>* aInput){
        auto dt = aInput->data();
        for (auto i : dt)
            m_updates_modification.push_back(i);
        if (m_models.size() > 0){
            for (auto i : dt){
                auto mdy = i.toObject();
                tryFlushImageCache(mdy, m_models.back()->getImageCache());
                addUpdate(m_models.back()->updateQSGAttr(mdy));
            }
            update();
        }else{
            for (auto i : dt)
                tryFlushImageCache(i.toObject());
            rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification, "", false);
            m_updates_modification = QJsonArray();
        }
        aInput->out();
    }, rea::Json("name", "updateQSGAttrs_" + m_name, "delegate", "QSGAttrUpdated_" + m_name));

    rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
        aInput->out();
    }, rea::Json("name", "QSGAttrUpdated_" + m_name));

    rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
        for (auto i : m_plugins)
            i->beforeDestroy();
        if (aInput->data().size() > 0){
            m_plugins.clear();
            installPlugins(aInput->data());
        }
        aInput->out();
    }, rea::Json("name", "updateQSGCtrl_" + m_name));
}

QJsonArray qsgBoard::getPlugins() {
    return m_plugins_config;
}

void qsgBoard::installPlugins(const QJsonArray& aPlugins){
    m_plugins_config = aPlugins;
    assert(!m_plugins.size());
    m_add_qsg_plugin = rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto plg = aInput->varData<std::shared_ptr<qsgBoardPlugin>>("result");
        m_plugins.insert(plg->getName(this), plg);
    });

    for (auto i : aPlugins){
        auto plg = i.toObject();
        auto cmd = "create_qsgboardplugin_" + plg.value("type").toString();
        auto tag = m_add_qsg_plugin->actName();
        rea::pipeline::find(cmd)->next(m_add_qsg_plugin, tag);
        rea::pipeline::run<QJsonObject>(cmd, plg, tag);
    }
}

QSGNode* qsgBoard::updatePaintNode(QSGNode* aOldNode, UpdatePaintNodeData* nodedata){
    auto ret = aOldNode;
    if (ret == nullptr){
        auto root_node = new QSGNode();
        root_node->setFlag(QSGNode::OwnedByParent);
        //clip_node->setClipRect(boundingRect());
        //clip_node->setIsRectangular(true);
        ret = root_node;

        m_trans_node = new QSGTransformNode();
        m_trans_node->setMatrix(QMatrix4x4(QTransform()));
        m_trans_node->setFlag(QSGNode::OwnedByParent);
        ret->appendChildNode(m_trans_node);
        //ret = m_trans_node;
    }
    if (m_models.size() > 1){
        while (m_updates.size() > 0 && m_updates_model_index.front() < m_models.size() - 1){
            if (m_updates.front())
                m_updates.front()(m_trans_node);
            m_updates.pop_front();
            m_updates_model_index.pop_front();
        }
        auto lst = m_models.front();
        lst->clearQSGObjects();
        while (m_models.size() > 1)
            m_models.pop_front();
    }
    if (m_models.size() == 1){
        m_models.front()->show(m_trans_node, this);
    }

    for (auto i : m_updates)
        if (i)
            i(m_trans_node);
    m_updates.clear();
    m_updates_model_index.clear();
    if (m_updates_modification.size() > 0){
        rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification, "", false);
        m_updates_modification = QJsonArray();
    }
    return ret;
}

void qsgBoard::keyPressEvent(QKeyEvent *event){
    for (auto i : m_plugins)
        i->keyPressEvent(event);
}

void qsgBoard::mousePressEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mousePressEvent(event);
}

void qsgBoard::mouseMoveEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mouseMoveEvent(event);
}

void qsgBoard::mouseReleaseEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mouseReleaseEvent(event);
}

void qsgBoard::hoverMoveEvent(QHoverEvent *event){
    for (auto i : m_plugins)
        i->hoverMoveEvent(event);
}

void qsgBoard::wheelEvent(QWheelEvent *event){
    for (auto i : m_plugins)
        i->wheelEvent(event);
}

static rea::regPip<QQmlApplicationEngine*> reg_imageboard([](rea::stream<QQmlApplicationEngine*>* aInput){
    qmlRegisterType<qsgBoard>("QSGBoard", 1, 0, "QSGBoard");
    aInput->out();
}, rea::Json("name", "install0_QSGBoard"), "regQML");

}
