#ifndef REAL_FRAMEWORK_IMAGEBOARD_H_
#define REAL_FRAMEWORK_IMAGEBOARD_H_

#include "qsgModel.h"
#include "util.h"
#include <QQuickItem>
#include <QQueue>

namespace rea {

class qsgBoardPlugin;

class qsgBoard : public QQuickItem{
    Q_OBJECT
    Q_PROPERTY(QString name WRITE setName READ getName)
    Q_PROPERTY(QJsonArray plugins WRITE installPlugins)
public:
    Q_INVOKABLE void beforeDestroy();
public:
    explicit qsgBoard(QQuickItem *parent = nullptr);
    ~qsgBoard() override;
protected:
    QSGNode *updatePaintNode(QSGNode* aOldNode, UpdatePaintNodeData* nodedata) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //QSGClipNode* m_clip_node = nullptr;
    QSGTransformNode* m_trans_node = nullptr;
    QQueue<std::shared_ptr<qsgModel>> m_models;
private:
    void setName(const QString& aName);
    void addUpdate(const IUpdateQSGAttr& aUpdate);
    QString getName() {return m_name;}
    void installPlugins(const QJsonArray& aPlugins);
    QString m_name;
    QMap<QString, std::shared_ptr<qsgBoardPlugin>> m_plugins;
    QQueue<IUpdateQSGAttr> m_updates;
    QQueue<int> m_updates_model_index;
    QJsonArray m_updates_modification;
    pipe0* m_add_qsg_plugin = nullptr;
    friend qsgBoardPlugin;
};

class DSTDLL qsgBoardPlugin{
public:
    qsgBoardPlugin(const QJsonObject& aConfig) {
        m_name = aConfig.value("name").toString();
        if (m_name == "")
            m_name = rea::generateUUID();
    }
    virtual ~qsgBoardPlugin() = default;

private:
    friend qsgBoard;
protected:
    virtual QString getName(qsgBoard* aParent = nullptr) {
        if (aParent)
            m_parent = aParent;
        return m_name;
    }
    virtual void beforeDestroy(){}
    virtual void keyPressEvent(QKeyEvent *event){}
    virtual void mousePressEvent(QMouseEvent *event){}
    virtual void mouseReleaseEvent(QMouseEvent *event){}
    virtual void mouseMoveEvent(QMouseEvent *event){}
    virtual void wheelEvent(QWheelEvent *event){}
    virtual void hoverMoveEvent(QHoverEvent *event){}
    QString getParentName() {return m_parent ? m_parent->getName() : "";}
    std::shared_ptr<qsgModel> getQSGModel() {return m_parent->m_models.size() > 0 ? m_parent->m_models.back() : nullptr;}
    void updateParent(IUpdateQSGAttr aUpdate){
        m_parent->addUpdate(aUpdate);
        m_parent->update();
    }
    void updateParent(std::vector<IUpdateQSGAttr> aUpdates){
        for (auto i : aUpdates)
            m_parent->addUpdate(i);
        m_parent->update();
    }
    QSGTransformNode* getTransNode(){
        return m_parent->m_trans_node;
    }
    qsgBoard* m_parent;
    QString m_name;
};

class DSTDLL qsgPluginTransform : public qsgBoardPlugin{
public:
    qsgPluginTransform(const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){}
    ~qsgPluginTransform() override;
protected:
    QString getName(qsgBoard* aParent = nullptr) override;
    void beforeDestroy() override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    virtual QJsonObject getMenu() {return QJsonObject();}
    bool tryMoveWCS(QMouseEvent * event, Qt::MouseButton aFlag);
    QPoint m_lastpos;
    QPointF m_wcspos;
protected:
    QString newShapeID();
    std::function<void(void)> removeShape(const QString& aShape, bool aCommand = true);
    std::function<void(void)> addPoly(const QString& aShape, const QJsonArray& aPoints, bool aCommand = true, int aFace = 125);
    std::function<void(void)> addEllipse(const QString& aShape, const QJsonArray& aCenter, const QJsonArray& aRadius, bool aCommand = true);
    std::shared_ptr<shapeObject> createEllipseHandle(QSGNode* aTransformNode, int aRadius, int aFace = - 1,
                                                     const QJsonArray& aCenter = rea::JArray(0, 0), const QString& aColor = "red");
    void updateHandlePos(int aIndex, const QPoint& aPos);
    void updateHandleRadius(int aIndex, int aRadius);
    void updatePos(const QPoint& aPos);
    std::vector<std::shared_ptr<shapeObject>> m_handles;
private:
    rea::qsgModel m_mdl;
};

}

#endif