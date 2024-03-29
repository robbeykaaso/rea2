#include "qsgModel.h"
#include "imagePool.h"
#include "tess.h"
#include <array>
#include <queue>
#include <QDebug>
#include <QJsonArray>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include <QPainter>
#include <QQuickItem>
#include <QDateTime>
#include <sstream>

namespace rea {

void qsgObject::checkTextVisible(){
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg)){
        if (!m_text){
            m_text = new QSGSimpleTextureNode();// window()->createImageNode();
            m_text->setOwnsTexture(true);
            updateTextValue(txt_cfg);
            updateTextLocation(txt_cfg);
            getRootQSGNode()->parent()->parent()->appendChildNode(m_text);
        }
    }else
        if (m_text){
        m_text->parent()->removeChildNode(m_text);
        m_text = nullptr;
    }
}

void qsgObject::removeQSGNodes() {
    auto nds = getQSGNodes();
    for (auto i : nds)
        if (i->parent()){
            i->parent()->removeChildNode(i);
            delete i;
        }
    if (m_text){
        m_text->parent()->removeChildNode(m_text); //cannot delete here
        m_text = nullptr;
    }
}

void qsgObject::transformChanged() {
    if (m_text && getRootQSGNode()){
        updateTextLocation(getTextConfig());
    }
}

IUpdateQSGAttr qsgObject::updateQSGAttr(const QString& aModification){
    if (aModification == "text_visible_")
        return [this](QSGNode*){checkTextVisible();};
    else if (aModification == "color_")
        return [this](QSGNode*){checkColor();};
    else if (aModification == "caption_")
        return [this](QSGNode*){checkCaption();};
    else
        return nullptr;
}

QJsonObject qsgObject::getTextConfig(){
    if (contains("text"))
        return value("text").toObject();
    else
        return m_parent->getTextConfig();
}

void qsgObject::updateTextValue(const QJsonObject& aTextConfig){
    auto sz = m_parent->getTextSize(aTextConfig);
    auto img = QImage(sz.x(), sz.y(), QImage::Format_ARGB32);
    auto clr = getColor();
    img.fill(QColor(clr.red(), clr.green(), clr.blue(), 64));

    auto txt = getText();
    QPainter p(&img);
    auto bnd = p.fontMetrics().boundingRect(txt);
    auto factor = std::min(sz.x() * 0.8 / bnd.width(), sz.y() * 0.8 / bnd.height()); // https://stackoverflow.com/questions/2202717/for-qt-4-6-x-how-to-auto-size-text-to-fit-in-a-specified-width
    if (factor < 1 || factor > 1.25){
        auto font = p.font();
        font.setPointSizeF(font.pointSizeF() * factor);
        p.setFont(font);
    }
    p.setPen(QPen(clr));
    //p.setFont(QFont("TimesNewRoman", std::min(22, int(std::round(img2.width() * 1.0 / wdh * 8))), QFont::Normal));
    p.drawText(img.rect(), Qt::AlignCenter, txt);
    m_text->setTexture(m_window->window()->createTextureFromImage(img));
    m_text->markDirty(QSGNode::DirtyMaterial);
}

void qsgObject::updateTextLocation(const QJsonObject& aTextConfig){
    auto bnd = getBoundBox();
    auto top_lft = bnd.topLeft(), btm_rt = bnd.bottomRight();

    auto trans = reinterpret_cast<QSGTransformNode*>(getRootQSGNode()->parent())->matrix();
    top_lft = trans.map(top_lft);
    btm_rt = trans.map(btm_rt);

    auto sz = m_parent->getTextSize(aTextConfig);
    auto del = 0.5 * (btm_rt.x() - top_lft.x() - sz.x());
    auto loc = m_parent->getTextLocation(aTextConfig);
    if (loc == "bottom")
        m_text->setRect(top_lft.x() + del, btm_rt.y() + 5, sz.x(), sz.y());
    else if (loc == "middle"){
        auto del2 = 0.5 * (btm_rt.y() - top_lft.y() - sz.y());
        m_text->setRect(top_lft.x() + del, top_lft.y() + del2, sz.x(), sz.y());
    }else if (loc == "cornerLT"){
        m_text->setRect(top_lft.x(), top_lft.y(), sz.x(), sz.y());
    }else
        m_text->setRect(top_lft.x() + del, top_lft.y() - sz.y() - 5, sz.x(), sz.y());
    m_text->markDirty(QSGNode::DirtyGeometry);
}

QString qsgObject::getText(){
    return value("caption").toString();
}

QColor qsgObject::getColor(){
    if (contains("color"))
        return QColor(value("color").toString());
    else
        return QColor(m_parent->value("color").toString("red"));
}

void qsgObject::checkColor(){
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextValue(txt_cfg);
}

void qsgObject::checkCaption(){
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextValue(txt_cfg);
}

imageObject::imageObject(const QJsonObject& aConfig) : qsgObject(aConfig){

}

void imageObject::setParent(qsgModel* aParent){
    qsgObject::setParent(aParent);
    auto pth = getPath();
    //qDebug() << "imageObject setParent: " << pth;
    //m_parent->m_image_cache.insert(pth, imagePool::readCache(pth));
}

std::vector<QSGNode*> imageObject::getQSGNodes(QQuickItem* aWindow, QSGNode* aParent, QSGTransformNode* aTransform){
    auto ret = qsgObject::getQSGNodes(aWindow, aParent, aTransform);
    if (!m_node){
        m_node = new QSGSimpleTextureNode();
        m_node->setOwnsTexture(true);
        updateImagePath(true);
        appendToParent(aParent);
    }
    ret.push_back(m_node);
    checkTextVisible();
    return ret;
}

void imageObject::removeQSGNodes(){
    if (m_node){
        qsgObject::removeQSGNodes();
        m_node = nullptr;
    }
}

QImage imageObject::getImage(){
    return m_parent->m_image_cache.value(getPath());
}

QImage imageObject::updateImagePath(bool aForce){
    QImage img = getImage();
    if (img.width() == 0 || img.height() == 0 || img.isNull()){
        if (!aForce)
            return img;
        img = QImage(10, 10, QImage::Format_ARGB32);
        img.fill(QColor("transparent"));
    }

    /*static int display_cnt = 0;
    static uint display_org = QDateTime::currentDateTime().toTime_t();
    std::cout << "showed: " << display_cnt++ << std::endl;
    auto del = QDateTime::currentDateTime().toTime_t() - display_org;
    if (del)
        std::cout << "fps: " << std::round(display_cnt * 1.0 / int(del)) << std::endl;*/

    m_node->setTexture(m_window->window()->createTextureFromImage(img));
    auto rng = getRange(img);
    auto ofst = QPointF(0.5, 0.5);
    m_node->setRect(QRectF(rng.topLeft() - ofst, rng.bottomRight() - ofst));
    m_node->markDirty(QSGNode::DirtyMaterial);
    return img;
}

QRectF imageObject::getBoundBox(){
    return getRange(QImage());
}

IUpdateQSGAttr imageObject::updateQSGAttr(const QString& aModification){
    if (aModification == "path_" || aModification == "range_"){
        auto pth = getPath();
        //qDebug() << "imageObject updateQSGAttr: " << pth;
        if (aModification == "range_")
            m_parent->m_image_cache.insert(pth, imagePool::readCache(pth));
        return [this](QSGNode*){
            if (m_node){
                updateImagePath();
                auto txt_cfg = getTextConfig();
                if (m_parent->getTextVisible(txt_cfg) && m_text)
                    updateTextLocation(txt_cfg);
            }
        };
    }
    else
        return qsgObject::updateQSGAttr(aModification);
}

bool imageObject::bePointSelected(double aX, double aY){
    auto rg = getRange(QImage());
    return rg.contains(aX, aY);
}

QRectF imageObject::getRange(const QImage& aImage){
    auto rg = value("range").toArray();
    return rg.size() == 4 ? QRectF(QPointF(rg[0].toDouble(), rg[1].toDouble()),
                                   QPointF(rg[2].toDouble(), rg[3].toDouble())) : aImage.rect();
}

QString imageObject::getPath(){
    return value("path").toString();
}

bool shapeObject::bePointSelected(double aX, double aY){
    auto bnd = getBoundBox();
    return aX >= bnd.left() && aX <= bnd.right() && aY >= bnd.top() && aY <= bnd.bottom();
}

IUpdateQSGAttr shapeObject::updateQSGAttr(const QString& aModification){
    if (aModification == "face_")
        return [this](QSGNode*){checkFaceOpacity();};
    else if (aModification == "width_")
        return [this](QSGNode*){checkWidth();};
    else if (aModification == "angle_")
        return [this](QSGNode*){checkAngle();};
    else
        return qsgObject::updateQSGAttr(aModification);
}

std::vector<QSGNode*> shapeObject::getQSGNodes(QQuickItem* aWindow, QSGNode* aParent, QSGTransformNode* aTransform){
    auto ret = qsgObject::getQSGNodes(aWindow, aParent, aTransform);
    m_trans_node = aTransform;
    if (!m_outline){
        m_outline = new QSGGeometryNode();
        auto nw = updateGeometry();
        auto clr = getColor();
        setQSGColor(*m_outline, clr);
        aParent->appendChildNode(m_outline);
        for (auto i = 0; i < nw; ++i){
            auto nd = m_holes[m_holes.size() - 1 - i];
            setQSGColor(*nd, clr);
            aParent->appendChildNode(nd);
        }
    }
    ret.push_back(m_outline);
    for (auto i : m_holes)
        ret.push_back(i);
    checkFaceOpacity();
    checkTextVisible();
    return ret;
}

int shapeObject::getFaceOpacity(){
    if (contains("face"))
        return value("face").toInt();
    else
        return m_parent->getFaceOpacity();
}

QJsonObject shapeObject::getArrowConfig(){
    if (contains("arrow"))
        return value("arrow").toObject();
    else
        return m_parent->getArrowConfig();
}

bool shapeObject::getArrowVisible(const QJsonObject& aConfig){
    return m_parent->getArrowVisible(aConfig);
}

bool shapeObject::getPoleArrow(const QJsonObject& aConfig){
    return m_parent->getPoleArrow(aConfig);
}

void shapeObject::removeQSGNodes() {
    if (m_outline){
        qsgObject::removeQSGNodes();

        m_outline = nullptr;
        m_holes.clear();

        for (auto i : m_arrows){
            i->parent()->removeChildNode(i);
            delete i;
        }
        m_arrows.clear();
        m_face = nullptr;
    }
}

QRectF calcBoundBox(const pointList &aPoints){
    QRectF ret(aPoints[0], aPoints[0]);
    for (auto i : aPoints){
        auto x = i.x(), y = i.y();
        if (x < ret.left())
            ret.setLeft(x);
        if (x > ret.right())
            ret.setRight(x);
        if (y < ret.top())
            ret.setTop(y);
        if (y > ret.bottom())
            ret.setBottom(y);
    }
    return ret;
}

void shapeObject::setQSGGemoetry(const pointList& aPointList, QSGGeometryNode& aNode, unsigned int aMode,
                                 const QString& aStyle, std::vector<uint32_t>* aIndecies){
    auto sz = aIndecies ? aIndecies->size() : aPointList.size();
    auto wth = getWidth();
    QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), int(sz));
    auto vertices = geometry->vertexDataAsTexturedPoint2D();
    if (aIndecies)
        for (auto i = 0; i < aIndecies->size(); ++i){
            auto idx = aIndecies->at(i);
            vertices[i].set(aPointList[idx].x(), aPointList[idx].y(), 0, 0);
        }
    else if (wth){
        if (aStyle == "dash"){
            float dis = 0;
            auto rt = m_trans_node->matrix().data()[0];
            for (auto i = 0; i < aPointList.size(); ++i){
                if (i > 0){
                    auto del = aPointList[i] - aPointList[i - 1];
                    dis += sqrt(QPointF::dotProduct(del, del));
                }
                vertices[i].set(aPointList[i].x(), aPointList[i].y(), dis * rt / 10, 0);
            }
        }else
            for (auto i = 0; i < aPointList.size(); ++i)
                vertices[i].set(aPointList[i].x(), aPointList[i].y(), 0, 0);
    }else{
        delete geometry;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    }
    geometry->setLineWidth(wth);
    /*if (getConfig()->value("fill").toBool() && aPointList.size() > 2)
        geometry->setDrawingMode(GL_POLYGON);
    else*/
    geometry->setDrawingMode(aMode);
    aNode.setGeometry(geometry);
    aNode.setFlag(QSGNode::OwnsGeometry);
    aNode.markDirty(QSGNode::DirtyGeometry);
}

QString shapeObject::getLineStyle(){
    return value("style").toString();
}

void shapeObject::setQSGColor(QSGGeometryNode& aNode, const QColor& aColor){
    //QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
    QSGOpaqueTextureMaterial *material = new QSGOpaqueTextureMaterial();

    auto img = QImage(2, 1, QImage::Format_ARGB32);
    img.fill("transparent");
    {
        QPainter pnt(&img);
        pnt.fillRect(0, 0, 1, 1, aColor);
    }
    auto txt = m_window->window()->createTextureFromImage(img);
    material->setHorizontalWrapMode(QSGTexture::WrapMode::Repeat);
    material->setTexture(txt);
    //material->setColor(aColor);
    aNode.setMaterial(material);
    //auto md = material->texture()->horizontalWrapMode();
    aNode.setFlag(QSGNode::OwnsMaterial);
    aNode.markDirty(QSGNode::DirtyMaterial);
}

void shapeObject::updateArrowCount(int aCount){
    for (int i  = m_arrows.size() - 1; i >= aCount; --i){
        auto arr = m_arrows[i];
        arr->parent()->removeChildNode(arr);
        m_arrows.pop_back();
        delete arr;
    }
    auto prt = m_outline->parent()->parent();
    for (int i = m_arrows.size(); i < aCount; ++i){
        auto arrow = new QSGGeometryNode();
        pointList pts;
        setQSGGemoetry(pts, *arrow, QSGGeometry::DrawLineStrip, "");
        setQSGColor(*arrow, getColor());
        prt->appendChildNode(arrow);
        m_arrows.push_back(arrow);
    }
}

void shapeObject::checkArrowVisible(int aCount){
    if (m_outline){
        if (getArrowVisible(getArrowConfig())) {
            updateArrowCount(aCount);
            if (m_arrows.size() > 0)
                updateArrowLocation();
        }else
            updateArrowCount(0);
    }
}

void shapeObject::updateQSGFace(QSGGeometryNode& aNode, int aOpacity){
    std::vector<std::vector<std::array<qreal, 2>>> polygon;
    pointList pts;
    for (auto i : m_points){
        std::vector<std::array<qreal, 2>> poly;
        for (auto j : i){
            poly.push_back({j.x(), j.y()});
            pts.push_back(j);
        }
        polygon.push_back(poly);
    }
    std::vector<uint32_t> indices = mapbox::earcut(polygon);
    setQSGGemoetry(pts, aNode, QSGGeometry::DrawTriangles, "", &indices);
    updateQSGFaceColor(aNode, aOpacity);
}

void shapeObject::updateQSGFaceColor(QSGGeometryNode& aNode, int aOpacity){
    if (aOpacity < 0)
        return;
    auto clr0 = getColor();
    auto clr = QColor(clr0.red(), clr0.green(), clr0.blue(), aOpacity);
    setQSGColor(aNode, clr);
}

void shapeObject::checkFaceOpacity(){
    if (!m_outline)
        return;
    auto fc = getFaceOpacity();
    if (fc > 0){
        if (!m_face){
            m_face = new QSGGeometryNode();
            updateQSGFace(*m_face, fc);
            m_face->setFlag(QSGNode::OwnedByParent);
            m_outline->appendChildNode(m_face);
        }else
            updateQSGFace(*m_face, fc);
    }else
        if (m_face){
            m_face->parent()->removeChildNode(m_face);
            delete m_face;
            m_face = nullptr;
        }
}

void shapeObject::checkColor(){
    if (m_outline){
        auto clr = getColor();
        setQSGColor(*m_outline, clr);
        for (auto i : m_holes)
            setQSGColor(*i, clr);
    }
    for (auto i : m_arrows)
        setQSGColor(*i, getColor());
    if (m_face)
        updateQSGFaceColor(*m_face, getFaceOpacity());
    qsgObject::checkColor();
}

void shapeObject::checkWidth(){
    if (m_outline){
        auto stl = getLineStyle();
        if (m_outline)
            setQSGGemoetry(m_points[0], *m_outline, QSGGeometry::DrawLineStrip, stl);
        for (auto i = 0; i < m_holes.size(); ++i)
            setQSGGemoetry(m_points[i + 1], *m_holes[i], QSGGeometry::DrawLineStrip, stl);
    }
    if (m_arrows.size() > 0)
        updateArrowLocation();
}

void shapeObject::checkAngle(){
    if (m_outline)
        updateGeometry();
    if (m_arrows.size() > 0)
        updateArrowLocation();
    if (m_face)
        updateQSGFace(*m_face, getFaceOpacity());
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextLocation(txt_cfg);
}

int shapeObject::getWidth(){
    return value("width").toInt(5);
}

void shapeObject::calcArrow(const QPointF& aStart, const QPointF& aEnd, QSGGeometryNode& aNode){
    const float si = 0.5, co = 0.866;
    auto dir = aStart - aEnd;
    dir = dir / sqrt(QPointF::dotProduct(dir, dir)) * 15;
    pointList pts;
    pts.push_back(QPointF(dir.x() * co + dir.y() * si, - dir.x() * si + dir.y() * co) + aEnd);
    pts.push_back(aEnd);
    pts.push_back(QPointF(dir.x() * co - dir.y() * si, dir.x() * si + dir.y() * co) + aEnd);
    setQSGGemoetry(pts, aNode, QSGGeometry::DrawLineStrip, "");
}

double shapeObject::getAngle(){
    return value("angle").toDouble();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->outs<std::shared_ptr<qsgObject>>(std::make_shared<imageObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_image"));

polyObject::polyObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

QJsonArray polyObject::getPoints(){
    return value("points").toArray();
}

void polyObject::checkGeometry(){
    auto nw = updateGeometry();
    if (nw > 0){
        for (auto i = 0; i < nw; ++i){
            auto nd = m_holes[m_holes.size() - 1 - i];
            setQSGColor(*nd, getColor());
            m_outline->parent()->appendChildNode(nd);
        }
    }
    if (m_arrows.size() > 0)
        checkArrowPole();
    if (m_face)
        updateQSGFace(*m_face, getFaceOpacity());
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextLocation(txt_cfg);
}

std::vector<QSGNode*> polyObject::getQSGNodes(QQuickItem* aWindow, QSGNode* aParent, QSGTransformNode* aTransform) {
    auto ret = shapeObject::getQSGNodes(aWindow, aParent, aTransform);
    checkArrowPole();
    return ret;
}

std::vector<pointList> polyObject::toPoints(){
    std::vector<pointList> pts2;

    auto pts0 = getPoints();
    if (pts0.size() == 0){
        return pts2;
    }
    for (auto j : pts0){
        auto pts = j.toArray();
        pointList pts1;
        for (int i = 0; i < pts.size(); i += 2)
            pts1.push_back(QPointF(pts[i].toDouble(), pts[i + 1].toDouble()));
        pts2.push_back(pts1);
    }
    return pts2;
}

int polyObject::updateGeometry(){
    m_points = toPoints();
    if (m_points.size() == 0)
        return 0;
    m_bound = calcBoundBox(m_points[0]);
    auto stl = getLineStyle();
    setQSGGemoetry(m_points[0], *m_outline, QSGGeometry::DrawLineStrip, stl);

    int ret = 0;
    for (auto i = int(m_holes.size()) - 1; i >= int(m_points.size()) - 1; --i){
        m_holes[i]->parent()->removeChildNode(m_holes[i]);
        delete m_holes[i];
        m_holes.pop_back();
    }
    for (auto i = 1; i < m_points.size(); ++i){
        if (i > m_holes.size()){
            auto nd = new QSGGeometryNode();
            m_holes.push_back(nd);
            ++ret;
        }
        setQSGGemoetry(m_points[i], *m_holes[i - 1], QSGGeometry::DrawLineStrip, stl);
    }
    return ret;
}

void polyObject::updateArrowLocation(){
    auto trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    int idx = 0;
    for (auto j : m_points){
        auto st = trans.map(j[0]);
        bool pole = getPoleArrow(getArrowConfig());
        for (size_t i = 1; i < j.size(); ++i){
            auto ed = trans.map(j[i]);
            calcArrow(st, ed, *m_arrows[idx]);
            if (pole)
                calcArrow(ed, st, *m_arrows[m_arrows.size() / 2 + idx]);
            st = ed;
            idx++;
        }
    }
}

void polyObject::transformChanged(){
    shapeObject::transformChanged();
    auto stl = getLineStyle();
    if (stl == "dash"){
        if (m_points.size() > 0)
            setQSGGemoetry(m_points[0], *m_outline, QSGGeometry::DrawLineStrip, stl);
        for (auto i = 1; i < m_points.size(); ++i)
            setQSGGemoetry(m_points[i], *m_holes[i - 1], QSGGeometry::DrawLineStrip, stl);
    }
    if (m_outline && m_arrows.size() > 0)
        updateArrowLocation();
}

IUpdateQSGAttr polyObject::updateQSGAttr(const QString& aModification){
    if (aModification == "arrow_visible_")
        return [this](QSGNode*){
            checkArrowPole();
        };
    else if (aModification == "arrow_pole_")
        return [this](QSGNode*){
            checkArrowPole();
        };
    else if (aModification == "points_" || aModification == "style_")
        return [this](QSGNode*){checkGeometry();};
    else
        return shapeObject::updateQSGAttr(aModification);
}

static rea::regPip<QJsonObject, rea::pipePartial> create_poly([](rea::stream<QJsonObject>* aInput){
    aInput->outs<std::shared_ptr<qsgObject>>(std::make_shared<polyObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_poly"));

void polyObject::checkArrowPole(){
    int sz = 0;
    for (auto i : m_points)
        sz += i.size() - 1;
    if (getPoleArrow(getArrowConfig()))
        checkArrowVisible(sz * 2);
    else
        checkArrowVisible(sz);
}

void ellipseObject::updateArrowLocation(){
    auto w_trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    auto r = getRadius();
    auto ct = getCenter();
    auto del = getCCW() ?  - 1 : 1;
    pointList pts = {QPointF(ct.x() - r.x(), ct.y()), QPointF(ct.x(), ct.y() + r.y()),
                     QPointF(ct.x() + r.x(), ct.y()), QPointF(ct.x(), ct.y() - r.y()),
                     QPointF(ct.x() - r.x(), ct.y() + del), QPointF(ct.x() + del, ct.y() + r.y()),
                     QPointF(ct.x() + r.x(), ct.y() - del), QPointF(ct.x() - del, ct.y() - r.y())};
    QTransform trans;
    auto ang = getAngle();
    trans.rotate(ang);
    for (auto i = 0; i < 4; ++i)
        calcArrow(w_trans.map(trans.map(pts[i + 4] - ct) + ct), w_trans.map(trans.map(pts[i] - ct) + ct), *m_arrows[i]);
}

void ellipseObject::transformChanged(){
    shapeObject::transformChanged();
    if (m_trans_node){
        updateGeometry();
        if (m_face)
            updateQSGFace(*m_face, getFaceOpacity());
    }
    if (m_arrows.size() > 0 && m_outline)
        updateArrowLocation();
}

IUpdateQSGAttr ellipseObject::updateQSGAttr(const QString& aModification){
    if (aModification == "arrow_visible_")
        return [this](QSGNode*){
            checkArrowVisible(4);
        };
    else if (aModification == "center_" || aModification == "radius_")
        return [this](QSGNode*){
            checkAngle();
        };
    else if (aModification == "style_")
        return [this](QSGNode*){
            if (m_outline)
                setQSGGemoetry(m_points[0], *m_outline, QSGGeometry::DrawLineStrip, getLineStyle());
        };
    else if (aModification == "ccw_")
        return [this](QSGNode*){
            if (m_arrows.size() > 0 && m_outline)
                updateArrowLocation();
        };
    else
        return shapeObject::updateQSGAttr(aModification);
}

ellipseObject::ellipseObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

double squarePointProjectToLine2D(const QPointF& aPoint, const QPointF& aStart, const QPointF& aEnd){
    QPointF s = aPoint - aStart, e = aEnd - aStart;
    auto dot = QPointF::dotProduct(s, e);
    return dot * dot / QPointF::dotProduct(e, e);
}

double judge3Points(const QPointF& aFirst, const QPointF& aSecond, const QPointF& aThird){
    auto ret = aFirst.x() * (aSecond.y() - aThird.y()) +
        aSecond.x() * (aThird.y() - aFirst.y()) +
        aThird.x() * (aFirst.y() - aSecond.y());
    return std::abs(ret);
}

std::shared_ptr<ellipseObject::l_qsgPoint3D> ellipseObject::evalPoint(const QPointF& aCenter, const QPointF& aRadius, double aParam){
    return std::make_shared<l_qsgPoint3D>(aCenter.x() + (aRadius.x() * std::cos(aParam)),
                                          aCenter.y() + (aRadius.y() * std::sin(aParam)),
                      aParam);
}

std::vector<QSGNode*> ellipseObject::getQSGNodes(QQuickItem* aWindow, QSGNode* aParent, QSGTransformNode* aTransform){
    auto ret = shapeObject::getQSGNodes(aWindow, aParent, aTransform);
    checkArrowVisible(4);
    return ret;
}

std::vector<pointList> ellipseObject::toPoints(){
    static const double PI = 3.14159265;

    double del = 10;
    if (m_trans_node){
        auto dt = m_trans_node->matrix().data()[0];
        del = 0.5 / dt;
    }
    //auto del = reinterpret_cast<QSGTransformNode*>(aTransformNode)->matrix().toAffine().m11();
    //auto pt = mtx.map(QPoint(0, 0)) - mtx.map(QPoint(1, 0));
    //auto del = QPoint::dotProduct(pt, pt) * 8;

    auto ct = getCenter();
    auto r = getRadius();

    QTransform trans;
    trans.rotate(getAngle());
    auto i0 = evalPoint(ct, r, 0), i1 = evalPoint(ct, r, PI);
    i0->nxt = i1;
    i1->nxt = evalPoint(ct, r, 2 * PI);
    std::queue<std::shared_ptr<l_qsgPoint3D>> candidates;
    candidates.push(i0);
    candidates.push(i1);
    int cnt = 0;
    while (candidates.size() > 0 && cnt < 125){
        auto cur = candidates.front();
        auto mid = evalPoint(ct, r, 0.5 * (cur->z + cur->nxt->z));
        if (judge3Points(*cur, *mid, *cur->nxt) > del){
            ++cnt;
            mid->nxt = cur->nxt;
            cur->nxt = mid;
            candidates.push(cur);
            candidates.push(mid);
        }
        candidates.pop();
    }

    std::vector<pointList> ret;
    pointList pts;
    auto st = i0;
    while (st){
        auto pt = trans.map(QPointF(st->x() - ct.x(), st->y() - ct.y()));
        pts.push_back(pt + ct);
        st = st->nxt;
    }
    ret.push_back(pts);
    return ret;
}

int ellipseObject::updateGeometry(){

    auto pts = toPoints();
    m_bound = calcBoundBox(pts[0]);

    /* for( int i = 0; i <= 99999; i++) {
            auto pt0 = *evalPoint(ct, r, i * PI / 50);
            auto pt1 = QPointF(pt0.x - ct.x, pt0.y - ct.y);
            auto pt2 = QPointF(pt1.x * trans.m11() + pt1.y * trans.m12() + trans.m13(),
                                  pt1.x * trans.m21() + pt1.y * trans.m22() + trans.m23());
            m_points.push_back(QPointF(pt2.x + ct.x, pt2.y + ct.y));
        }
        m_points.push_back(m_points.at(0));*/

    setQSGGemoetry(pts[0], *m_outline, QSGGeometry::DrawLineStrip, getLineStyle());

    m_points = pts;

    return 0;
}

QPointF ellipseObject::getRadius(){
    auto r = value("radius").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

bool ellipseObject::getCCW(){
    return value("ccw").toBool();
}

QPointF ellipseObject::getCenter(){
    auto r = value("center").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
    aInput->outs<std::shared_ptr<qsgObject>>(std::make_shared<ellipseObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_ellipse"));

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->removeQSGNodes();
}

void qsgModel::show(QSGTransformNode* aTransform, QQuickItem* aWindow){
    if (m_trans_node != aTransform){
        m_window = aWindow;
        m_trans_node = aTransform;
        for (auto i : m_objects)
            i->getQSGNodes(aWindow, aTransform, aTransform);
        WCS2SCS();
    }
}

QHash<QString, QImage>* qsgModel::getImageCache(){
    return &m_image_cache;
}

/*{type: "add", obj: "shp_3", val: {
     type: "poly",
     points: [300, 300, 500, 300, 400, 400, 300, 300],
     color: "green",
     caption: "new_obj",
     face: 200
 }} : {type: "del", obj: "shp_3"}}*/

IUpdateQSGAttr qsgModel::updateQSGAttr(const QJsonObject& aModification){
    if (aModification.contains("id") && aModification.value("id") != value("id"))
        return nullptr;
    if (aModification.contains("obj")){
        auto obj = aModification.value("obj").toString();
        if (m_objects.contains(obj)){
            auto nd = m_objects.value(obj);
            auto mdy = overwriteAttr(*nd, aModification.value("key").toArray(), aModification.value("val"), aModification.value("force").toBool());
            if (mdy != "")
                return nd->updateQSGAttr(mdy);
        }
    }else{
        auto kys = aModification.value("key").toArray();
        if (kys.size() > 0){
            if (kys[0] == "transform"){
                if (aModification.value("type") == "zoom"){
                    auto ct = aModification.value("center").toArray();
                    if (ct.size() == 2)
                        zoom(aModification.value("dir").toInt(), QPointF(ct[0].toDouble(), ct[1].toDouble()));
                    else
                        zoom(aModification.value("dir").toInt(), QPointF(0, 0));
                }else if (aModification.value("type") == "move"){
                    auto del = aModification.value("del").toArray();
                    move(QPointF(del[0].toDouble(), del[1].toDouble()));
                }else if (aModification.value("type") == "refresh"){
                }else
                    return nullptr;
                setTransform();
                return [this](QSGNode*){
                    WCS2SCS();
                };
            }else if (kys[0] == "objects"){
                auto objs = getObjects();
                if (aModification.value("type") == "add"){
                    auto obj = aModification.value("tar").toString();
                    if (!objs.contains(obj)){
                        auto attr = aModification.value("val").toObject();
                        objs.insert(obj, attr);
                        setObjects(objs);
                        addObject(rea::Json(attr, "id", obj));
                        auto nd = m_objects.value(obj);
                        return [this, nd](QSGNode*){
                            if (m_window) //m_window is null until the model is showed at first
                                nd->getQSGNodes(m_window, m_trans_node, m_trans_node);
                        };
                    }
                }else if (aModification.value("type") == "del"){
                    auto obj = aModification.value("tar").toString();
                    if (objs.contains(obj)){
                        objs.remove(obj);
                        setObjects(objs);
                        auto nd = m_objects.value(obj);
                        m_objects.remove(obj);
                        return [nd](QSGNode*){
                            nd->removeQSGNodes();
                        };
                    }
                }
            }else{
                auto mdy = overwriteAttr(*this, aModification.value("key").toArray(), aModification.value("val"), aModification.value("force").toBool());
                if (mdy != "")
                    return [this, mdy](QSGNode* aClip){
                        for (auto i : m_objects){
                            auto up = i->updateQSGAttr(mdy);
                            if (up)
                                up(aClip);
                        }
                    };
            }
        }
    }
    return nullptr;
}

void qsgModel::zoom(int aStep, const QPointF& aCenter, double aRatio){
    if (aStep == 0){
        m_trans = QTransform();
        if (aRatio == 0.0){
            return;
        }
    }
    QPointF ct = m_trans.inverted().map(QPointF(aCenter.x(), aCenter.y()));
    double ratio = aRatio;
    if (ratio == 0.0){
        if (aStep > 0)
            ratio = 1.25;
        else
            ratio = 0.8;
    }

    if ((aStep < 0 && m_trans.m11() > m_min_ratio ) || (aStep > 0 && m_trans.m11() < m_max_ratio)){
        m_trans.translate(ct.x(), ct.y());
        m_trans.scale(ratio, ratio);
        m_trans.translate(- ct.x(), - ct.y());
    }
}

void qsgModel::WCS2SCS(){
    if (!m_window)
        return;
    auto width = getWidth(),
         height = getHeight();
    QPoint sz = QPoint(m_window->width(), m_window->height());
    if (width == 0)
        width = int(sz.x());
    if (height == 0)
        height = int(sz.y());
    QTransform trans0;
    trans0.scale(sz.x() * 1.0 / width, sz.y() * 1.0 / height);
    // return aTransform;
    auto ratio =  width * 1.0 / height;
    QTransform trans;
    if (ratio > sz.x() * 1.0 / sz.y()){
        auto ry = sz.x() / ratio / sz.y();
        trans = trans.scale(1, ry);
        trans = trans.translate(0, (sz.y() - sz.x() / ratio) * 0.5 / ry);
        //m_image->setRect(QRect(0, (m_ui_height - m_ui_width / ratio) * 0.5, m_ui_width, m_ui_width / ratio));
    }else{
        auto rx = (sz.y() * ratio) / sz.x();
        trans = trans.scale(rx, 1);
        trans = trans.translate((sz.x() - sz.y() * ratio) * 0.5 / rx, 0);
        //m_image->setRect(QRect((m_ui_width - m_ui_height * ratio) * 0.5, 0, m_ui_height * ratio, m_ui_height));
    }
    m_trans_node->setMatrix(QMatrix4x4(trans0 * trans * getTransform()));
    m_trans_node->markDirty(QSGNode::DirtyMatrix);
    for (auto i : m_objects)
        i->transformChanged();
}

QString qsgModel::overwriteAttr(QJsonObject& aObject, const QJsonArray& aKeys, const QJsonValue&& aValue, bool aForce){
    if (aKeys.size() == 0)
        return "";
    else if (aKeys.size() == 1){
        auto ret = aKeys[0].toString();
        if (aObject.value(ret) == aValue && !aForce)
            return "";
        else{
            aObject.insert(ret, aValue);
            return ret + "_";
        }
    }
    QString ret = "";
    std::vector<QJsonObject> objs;
    auto org = aObject;
    for (int i = 0; i < aKeys.size(); ++i){
        auto key = aKeys[i].toString();
        ret += key + "_";
        if (i == aKeys.size() - 1){
            if (org.value(key) == aValue && !aForce)
                return "";
            objs.back().insert(key, aValue);
        }
        else{
            if (i == 0)
                objs.push_back(aObject.value(key).toObject());
            else
                objs.push_back(objs.back().value(key).toObject());
            org = org.value(key).toObject();
        }
    }
    for (int i = objs.size() - 1; i > 0; --i){
        objs[i - 1].insert(aKeys[i].toString(), objs[i]);
    }
    aObject.insert(aKeys[0].toString(), objs[0]);
    return ret;
}

QJsonObject qsgModel::getArrowConfig(){
    return value("arrow").toObject();
}

bool qsgModel::getArrowVisible(const QJsonObject& aConfig){
    return aConfig.value("visible").toBool();
}

bool qsgModel::getPoleArrow(const QJsonObject& aConfig){
    return aConfig.value("pole").toBool();
}

int qsgModel::getFaceOpacity(){
    return value("face").toInt();
}

QJsonObject qsgModel::getTextConfig(){
    return value("text").toObject();
}

bool qsgModel::getTextVisible(const QJsonObject& aConfig){
    return aConfig.value("visible").toBool();
}

QPoint qsgModel::getTextSize(const QJsonObject& aConfig){
    auto sz = aConfig.value("size").toArray();
    if (sz.size() == 2)
        return QPoint(sz[0].toInt(), sz[1].toInt());
    else
        return QPoint(100, 50);
}

QString qsgModel::getTextLocation(const QJsonObject& aConfig){
    return aConfig.value("location").toString();
}

void qsgModel::setTransform(){
    insert("transform", rea::JArray(m_trans.m11(), m_trans.m12(), m_trans.m13(),
                                    m_trans.m21(), m_trans.m22(), m_trans.m23(),
                                    m_trans.m31(), m_trans.m32(), m_trans.m33()));
}

QTransform qsgModel::getTransform(bool aDeserialize){
    if (aDeserialize){
        auto trans = value("transform").toArray();
        if (trans.size() == 9)
            m_trans = QTransform(trans[0].toDouble(), trans[1].toDouble(), trans[2].toDouble(),
                                 trans[3].toDouble(), trans[4].toDouble(), trans[5].toDouble(),
                                 trans[6].toDouble(), trans[7].toDouble(), trans[8].toDouble());
        else
            m_trans = QTransform();
    }
    return m_trans;
}

QJsonObject qsgModel::getObjects(){
    return value("objects").toObject();
}

void qsgModel::setObjects(const QJsonObject& aObjects){
    insert("objects", aObjects);
}

void qsgModel::move(const QPointF& aDistance){
    QPointF ds_ = QPointF(aDistance.x(), aDistance.y()),
            ds0 = m_trans.inverted().map(QPointF(ds_.x(), ds_.y())),
            ds1 = m_trans.inverted().map(QPointF(0, 0)),
            ds = ds0 - ds1;
    m_trans.translate(ds.x(), ds.y());
}


int qsgModel::getWidth() {
    return value("width").toInt();
}

int qsgModel::getHeight() {
    return value("height").toInt();
}

rea::pipe0* qsgModel::objectCreator(const QString& aName){
    auto id = std::this_thread::get_id();
    std::stringstream ss;
    ss << id;
    auto nm = QString::fromStdString(ss.str()) + aName;
    if (!m_creators.contains(nm))
        m_creators.insert(nm, rea::local(aName));
    return m_creators.value(nm);
}

void qsgModel::addObject(const QJsonObject& aConfig){
    auto cmd = "create_qsgobject_" + aConfig.value("type").toString();
    objectCreator(cmd)->nextB(m_add_object)  //ensure the create_qsgobject_ is in the same thead of here
        ->execute(std::make_shared<rea::stream<QJsonObject>>(aConfig));
}

qsgModel::qsgModel(const QJsonObject& aConfig) : QJsonObject(aConfig){
    auto shps = value("objects").toObject();

    m_max_ratio = value("max_ratio").toDouble(100);
    m_min_ratio = value("min_ratio").toDouble(0.01);

    m_add_object = rea::pipeline::add<std::shared_ptr<qsgObject>>([this](rea::stream<std::shared_ptr<qsgObject>>* aInput){
        auto dt = aInput->data();
        dt->setParent(this);
        m_objects.insert(dt->value("id").toString(), dt);
        dt->remove("id");
    });
    for (auto i : shps.keys())
        addObject(rea::Json(shps.value(i).toObject(), "id", i));
    getTransform(true);
}

qsgModel::~qsgModel(){
    for (auto i : m_creators)
        rea::pipeline::remove(i->actName());
}

}
