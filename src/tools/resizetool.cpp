// File:         resizetool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the ResizeTool.


#include "tools/resizetool.h"
#include "blitapp.h"
#include "animation/cel.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPair>
#include <QPointF>
#include <QLineF>
#include <QPixmap>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_brush, ResizeTool);


/*!
    Constructor, nothing much here.
*/
ResizeTool::ResizeTool(QObject *parent) :
    Tool(parent)
{

}


/*!
    Deconstructor, not much else here either.
*/
ResizeTool::~ResizeTool() {

}


/*!
    returns the name of the Tool
*/
QString ResizeTool::name() {
    return tr("Resize");
}


/*!
    Returns a toolip description of what the ResizeTool does.
*/
QString ResizeTool::desc() {
    return tr("Resize cels.");
}


/*!
    Returns an icon for QToolButton.
*/
QIcon ResizeTool::icon() {
    return QIcon(":/resize_tool_icon");
}


/*!
    TODO document
*/
void ResizeTool::onMouseMoved(QGraphicsSceneMouseEvent *event){
    if (_resizingCel) {
        // Normalize point
        QPointF pos = event->scenePos() - _startingMousePos;
        pos.setX(qFloor(pos.x()));
        pos.setY(qFloor(pos.y()));

        qDebug() << "[ResizeTool onMouseMoved] pos=" << pos;

        // TODO there are some issues with showing a preview of a resize.  It might be best
        //      instead to show a preview while dragging instead of doing the actual resize.
        //      It also might be better to fundementall change that the Cel has no size (or
        //      a hard coded sized, and the CelRef is just a viewport for that Cel.
        //
        //      That second part is kind of big and huge, it might be better for the
        //      milestone.
        //
        // TODO log this for (P-3) under [High Priority]

        Cel *cel = _ref->cel();

        // Based on which op, do a resize
        switch (_op) {
            case _ResizeOp::Left:
                cel->resize(_startWidth - pos.x(), _startHeight);
                _ref->setX(_startingCelPos.x() + pos.x());
                break;

            case _ResizeOp::Right:
                cel->resize(_startWidth + pos.x(), _startHeight);
                break;

            case _ResizeOp::Top:
                cel->resize(_startWidth, _startHeight - pos.y());
                _ref->setY(_startingCelPos.y() + pos.y());
                break;

            case _ResizeOp::Bottom:
                cel->resize(_startWidth, _startHeight + pos.y());
                break;

            case _ResizeOp::TopLeft:
                cel->resize(_startWidth - pos.x(), _startHeight - pos.y());
                _ref->setPos(_startingCelPos + pos);
                break;

            case _ResizeOp::TopRight:
                cel->resize(_startWidth + pos.x(), _startHeight - pos.y());
                _ref->setY(_startingCelPos.y() + pos.y());
                break;

            case _ResizeOp::BottomLeft:
                cel->resize(_startWidth - pos.x(), _startHeight + pos.y());
                _ref->setX(_startingCelPos.x() + pos.x());
                break;

            case _ResizeOp::BottomRight:
                cel->resize(_startWidth + pos.x(), _startHeight + pos.y());
                break;
        }
    }
}


/*!
    When the mouse is pressed, this will restart the resizing operaiton.
*/
void ResizeTool::onMousePressed(QGraphicsSceneMouseEvent *event){
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Get the cel
    Cel *cel = ref->cel();

    // Will start a resize action
    _resizingCel = true;
    QPointF pos = event->scenePos();
    _ref = ref;
    _startingMousePos = QPointF(qFloor(pos.x()), qFloor(pos.y()));
    _startingCelPos = _ref->pos();
    _startWidth = cel->width();
    _startHeight = cel->height();

    // Depending on where the offset is in relation to the edges of the CelRef, do the resize.
    QPointF offset = _startingMousePos - _startingCelPos;
    qreal cw = cel->width();
    qreal ch = cel->height();
    qreal ox = offset.x();
    qreal oy = offset.y();
    qreal xRadius = cw * _resizeCornerPercentage;
    qreal yRadius = ch * _resizeCornerPercentage; 

    // Find some distances from each point/line
    qreal left = qAbs(ox);
    qreal right = qAbs(ox - cw);
    qreal top = qAbs(oy);
    qreal bottom = qAbs(oy - ch);
    qreal topLeft = QLineF(offset, QPointF(0, 0)).length();
    qreal topRight = QLineF(offset, QPointF(cw, 0)).length();
    qreal bottomLeft = QLineF(offset, QPointF(0, ch)).length();
    qreal bottomRight = QLineF(offset, QPointF(cw, ch)).length();

    // Choose a op, give preference to the corners first if they are within the radius
    if ((topLeft <= xRadius) && (topLeft <= yRadius))
        _op = _ResizeOp::TopLeft;
    else if ((topRight <= xRadius) && (topRight <= yRadius))
        _op = _ResizeOp::TopRight;
    else if ((bottomLeft <= xRadius) && (bottomLeft <= yRadius))
        _op = _ResizeOp::BottomLeft;
    else if ((bottomRight <= xRadius) && (bottomRight <= yRadius))
        _op = _ResizeOp::BottomRight;

    // If no other op has been chosen, chose a side one
    // TODO, these is a bug here, this algorithm needs to be revised (try to do a left resize op, but when the mouse is far
    //       away from the left cel
    if (_op == _ResizeOp::None) {
        // Make a list of pairs of the distances and operations
        QPair<qreal, _ResizeOp> l(left, _ResizeOp::Left);
        QPair<qreal, _ResizeOp> r(right, _ResizeOp::Right);
        QPair<qreal, _ResizeOp> t(top, _ResizeOp::Top);
        QPair<qreal, _ResizeOp> b(bottom, _ResizeOp::Bottom);
        QList<QPair<qreal, _ResizeOp>> sides;
        sides << l << r << t << b;

        // Sort the list by first value of the pair
        qSort(sides.begin(), sides.end(),
            [](auto a, auto b) {
                return (a.first <= b.first);
            }
        );

        // Pop off the first one (which should be the least)
        _op = sides[0].second;
    }

    qDebug() << "[ResizeTool onMousePressed] Op: " << resizeOpStr(_op);
}


/*!
    TODO document
*/
void ResizeTool::onMouseReleased(QGraphicsSceneMouseEvent *event){
    if (_resizingCel) {
        // Reset
        _resizingCel = false;
        _startingCelPos = QPointF();
        _startingMousePos = QPointF();
        _startWidth = 0;
        _startHeight = 0;
        _ref = NULL;
        _op = _ResizeOp::None;
    }
}


/*!
    Returns a string for \a op.  Returns an empty string if \a op
    isn't valid.
*/
QString ResizeTool::resizeOpStr(_ResizeOp op) {
    switch (op) {
        case _ResizeOp::TopLeft: return "Top-Left";
        case _ResizeOp::TopRight: return "Top-Right";
        case _ResizeOp::BottomLeft: return "Bottom-Left";
        case _ResizeOp::BottomRight: return "Bottom-Right";
        case _ResizeOp::Left: return "Left";
        case _ResizeOp::Right: return "Right";
        case _ResizeOp::Top: return "Top";
        case _ResizeOp::Bottom: return "Bottom";
        default: return "";
    }
}
