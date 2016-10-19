// File:         tool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Tool interface.



#include "tools/tool.h"
#include <QGraphicsSceneMouseEvent>


Tool::Tool(QObject *parent) :
    QObject(parent)
{
    // Tool constructor
//    qDebug() << "[created]" << objectName();
}


Tool::~Tool() {
    // Virtual Tool deconstructor
//    qDebug() << "[destroyed]" << objectName();
}


QWidget *Tool::options() {
    // A Widget of other widgets that are used to modify tool options.  Can return a NULL
    // pointer if no options are needed.  See the maco TOOL_OPTIONS_PANEL_MAX_WIDTH
    return NULL;
}


void Tool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Slot for when the Canvas is double clicked
//    qDebug() << "[D-Click]" << objectName() << event->scenePos();
}


void Tool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // Action for when the mouse is moved
//    qDebug() << "[Move]" << objectName() << event->scenePos();
}


void Tool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // Action for when the mouse is pressed
//    qDebug() << "[Press]" << objectName() << event->scenePos();
}


void Tool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    // Action for when the mouse is released
//    qDebug() << "[Release]" << objectName() << event->scenePos();;
}



