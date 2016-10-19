// File:         movetool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the MoveTool.



#include "tools/movetool.h"
#include "blitapp.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPixmap>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_brush, MoveTool);


MoveTool::MoveTool(QObject *parent) :
    Tool(parent)
{

}

MoveTool::~MoveTool() {

}


QString MoveTool::name() {
    // returns the name of the Tool
    return tr("Move");
}


QString MoveTool::desc() {
    // A small tooltip
    return tr("Move Cels around.");
}


QIcon MoveTool::icon() {
    return QIcon(":/move_tool_icon");
}


void MoveTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // TODO should resize the Cel Window, or maybe bring up a dialog
    //      for absolute positioning
    qDebug() << "[MoveTool] double-click";
}


void MoveTool::onMouseMoved(QGraphicsSceneMouseEvent *event){
    // Will move the Cel if its held down
    if (_movingCel) {
        // Normalize point
        QPointF pos = event->scenePos() - _startingMousePos;
        pos.setX(qFloor(pos.x()));
        pos.setY(qFloor(pos.y()));

        // Take the staring Cel pos and add it to the moved position
        _ref->setPos(_startingCelPos + pos);
    }
}


void MoveTool::onMousePressed(QGraphicsSceneMouseEvent *event){
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Will start a move action
    _movingCel = true;
    QPointF pos = event->scenePos();
    _ref = ref;
    _startingMousePos = QPointF(qFloor(pos.x()), qFloor(pos.y()));
    _startingCelPos = _ref->pos();
}


void MoveTool::onMouseReleased(QGraphicsSceneMouseEvent *event){
    // Will stop moving the Cel (if it's moveing),  Make sure to
    // save The sequence then.
    if (_movingCel) {
        _movingCel = false;
        _startingCelPos = QPointF();
        _startingMousePos = QPointF();        // Null out
        _ref = NULL;
    }
}



