// File:         movetool.h
// Author:       Ben Summerton (define-private-public)
// Description:  MoveTool is an implementation of the Tool interface.  It comes standard in Blit.
//               It will let the user move around Cels in the Frame.
//
//               Right now it only allows the user to move the currently selected Cel


#ifndef MOVE_TOOL_H
#define MOVE_TOOL_H


#include "tools/tool.h"
#include <QPointF>
class CelRef;


class MoveTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    MoveTool(QObject *parent=NULL);
    ~MoveTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();


public slots:
    // For drawing
    void onMouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    void onMouseMoved(QGraphicsSceneMouseEvent *event);
    void onMousePressed(QGraphicsSceneMouseEvent *event);
    void onMouseReleased(QGraphicsSceneMouseEvent *event);


private:
    // Member vars
    bool _movingCel = false;
    QPointF _startingMousePos;
    QPointF _startingCelPos;
    CelRef *_ref = NULL;

};


#endif // MOVE_TOOL_H

