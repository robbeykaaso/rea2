#include "qsgBoard.h"
#include "reaC++.h"
#include <QSGTransformNode>
#include <QTransform>

namespace rea{

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
                    auto inv = getTransNode()->matrix().inverted();
                    auto mdl = getQSGModel();
                    aInput->outs<QJsonObject>(rea::Json("x", m_wcspos.x(),
                                                       "y", m_wcspos.y(),
                                                       "ratio", 100 / inv.data()[0],
                                                       "transform", mdl ? mdl->value("transform") : QJsonArray()), "updateQSGPos_" + getParentName());
                    break;
                }
            }
        aInput->out();
    }, rea::Json("name", "QSGTransformUpdated_" + getParentName())));
    return ret;
}

void qsgPluginTransform::beforeDestroy(){
    for (auto i : m_handles)
        i->removeQSGNodes();
    m_handles.clear();
}

void qsgPluginTransform::wheelEvent(QWheelEvent *event){
    rea::pipeline::run<QJsonObject>("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("transform"),
                                                                         "type", "zoom",
                                                                         "dir", event->delta() < 0 ? - 1 : 1,
                                                                         "center", rea::JArray(m_lastpos.x(), m_lastpos.y())));
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
                                                                                      "del", rea::JArray(cur.x() - m_lastpos.x(), cur.y() - m_lastpos.y())));
    }
    return ret;
}

void qsgPluginTransform::hoverMoveEvent(QHoverEvent *event){
    updatePos(event->pos());
}

QString qsgPluginTransform::newShapeID(){
    return "shp_" + generateUUID();
}

std::function<void(void)> qsgPluginTransform::removeShape(const QString& aShape, bool aCommand){
    auto nm = getParentName();
    auto mdl = getQSGModel();
    if (mdl){
        auto id = getQSGModel()->value("id");
        return [nm, aShape, aCommand, id](){
            rea::pipeline::run("updateQSGAttr_" + nm,
                               rea::Json("key", rea::JArray("objects"),
                                         "type", "del",
                                         "tar", aShape,
                                         "cmd", aCommand,
                                         "id", id));
        };
    }else
        return nullptr;
}

std::function<void(void)> qsgPluginTransform::addPoly(const QString& aShape, const QJsonArray& aPoints, bool aCommand, int aFace){
    auto nm = getParentName();
    auto id = getQSGModel()->value("id");
    return [nm, aShape, aPoints, aCommand, id, aFace](){
        rea::pipeline::run("updateQSGAttr_" + nm,
                           rea::Json("key", rea::JArray("objects"),
                                     "type", "add",
                                     "tar", aShape,
                                     "val", rea::Json(
                                                "type", "poly",
                                                "points", aPoints,
                                                "face", aFace),
                                     "cmd", aCommand,
                                     "id", id));
    };
}

std::function<void(void)> qsgPluginTransform::addEllipse(const QString& aShape, const QJsonArray& aCenter, const QJsonArray& aRadius, bool aCommand){
    auto nm = getParentName();
    auto id = getQSGModel()->value("id");
    return [nm, aShape, aCenter, aRadius, aCommand, id](){
        rea::pipeline::run("updateQSGAttr_" + nm,
                           rea::Json("key", rea::JArray("objects"),
                                     "type", "add",
                                     "tar", aShape,
                                     "val", rea::Json(
                                                "type", "ellipse",
                                                "center", aCenter,
                                                "radius", aRadius,
                                                "face", 125),
                                     "cmd", aCommand,
                                     "id", id));
    };
}

std::shared_ptr<shapeObject> qsgPluginTransform::createEllipseHandle(QSGNode* aTransformNode, int aRadius, int aFace,
                                                                     const QJsonArray& aCenter, const QString& aColor){
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

void qsgPluginTransform::updateHandlePos(int aIndex, const QPoint& aPos){
    if (aIndex < m_handles.size()){
        auto hdl = m_handles[aIndex];
        hdl->insert("center", rea::JArray(aPos.x(), aPos.y()));
        updateParent(hdl->updateQSGAttr("center_"));
    }
}

void qsgPluginTransform::updateHandleRadius(int aIndex, int aRadius){
    if (aIndex < m_handles.size()){
        auto hdl = m_handles[aIndex];
        hdl->insert("radius", rea::JArray(aRadius, aRadius));
        updateParent(hdl->updateQSGAttr("radius_"));
    }
}

void qsgPluginTransform::updatePos(const QPoint &aPos){
    m_lastpos = aPos;
    auto inv = getTransNode()->matrix().inverted();
    m_wcspos = inv.map(QPointF(aPos));
    auto mdl = getQSGModel();
    rea::pipeline::run<QJsonObject>("updateQSGPos_" + getParentName(), rea::Json("x", m_wcspos.x(),
                                                                                 "y", m_wcspos.y(),
                                                                                 "ratio", 100 / inv.data()[0],
                                                                                 "transform", mdl ? mdl->value("transform") : QJsonArray()));
}

static rea::regPip<QJsonObject, rea::pipePartial> create_qsgboardplugin_transform([](rea::stream<QJsonObject>* aInput){
   aInput->outs<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginTransform>(aInput->data()));
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
        m_models.push_back(std::make_shared<qsgModel>(aInput->data()));
        update();
        aInput->outs<std::shared_ptr<qsgModel>>(m_models.back());
    }, rea::Json("name", "updateQSGModel_" + m_name));

    rea::pipeline::add<QJsonObject, pipeDelegate>([this](rea::stream<QJsonObject>* aInput){
        m_updates_modification.push_back(aInput->data());
        if (m_models.size() > 0){
            addUpdate(m_models.back()->updateQSGAttr(aInput->data()));
            update();
        }else{
            rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification);
            m_updates_modification = QJsonArray();
        }
    }, rea::Json("name", "updateQSGAttr_" + m_name, "delegate", "QSGAttrUpdated_" + m_name));

    rea::pipeline::add<QJsonArray, pipeDelegate>([this](rea::stream<QJsonArray>* aInput){
        auto dt = aInput->data();
        for (auto i : dt)
            m_updates_modification.push_back(i);
        if (m_models.size() > 0){
            for (auto i : dt)
                addUpdate(m_models.back()->updateQSGAttr(i.toObject()));
            update();
        }else{
            rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification);
            m_updates_modification = QJsonArray();
        }
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

void qsgBoard::installPlugins(const QJsonArray& aPlugins){
    m_add_qsg_plugin = rea::pipeline::add<std::shared_ptr<qsgBoardPlugin>>([this](rea::stream<std::shared_ptr<qsgBoardPlugin>>* aInput){
        auto plg = aInput->data();
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
        /*m_clip_node = new QSGClipNode();
        m_clip_node->setFlag(QSGNode::OwnedByParent);
        m_clip_node->setClipRect(boundingRect());
        m_clip_node->setIsRectangular(true);
        ret = m_clip_node;*/

        m_trans_node = new QSGTransformNode();
        m_trans_node->setMatrix(QMatrix4x4(QTransform()));
        m_trans_node->setFlag(QSGNode::OwnedByParent);
        //ret->appendChildNode(m_trans_node);
        ret = m_trans_node;
    }
    if (m_models.size() > 1){
        while (m_updates.size() > 0 && m_updates_model_index.front() < m_models.size() - 1){
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
        rea::pipeline::run<QJsonArray>("QSGAttrUpdated_" + m_name, m_updates_modification);
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