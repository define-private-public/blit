// File:         resizetool.h
// Author:       Ben Summerton (define-private-public)
// Description:  ResizeTool is an implementation of the Tool interface.  It comes standard in Blit.
//               It will let the user resize cels


#ifndef RESIZE_TOOL_H
#define RESIZE_TOOL_H


#include "tools/tool.h"
#include <QPointF>
class CelRef;


class ResizeTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    ResizeTool(QObject *parent=NULL);
    ~ResizeTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();


public slots:
    // For drawing
    void onMouseMoved(QGraphicsSceneMouseEvent *event);
    void onMousePressed(QGraphicsSceneMouseEvent *event);
    void onMouseReleased(QGraphicsSceneMouseEvent *event);


private:
    // Internal enumerator
    enum class _ResizeOp { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };

    // Member functions
    QString resizeOpStr(_ResizeOp op);

    // Member vars
    bool _resizingCel = false;
    QPointF _startingMousePos;
    QPointF _startingCelPos;
    qreal _startWidth;
    qreal _startHeight;
    CelRef *_ref = NULL;
    _ResizeOp _op = _ResizeOp::None;
    qreal _resizeCornerPercentage = 0.15;            // How far from a corner (as a percentagle) to register a resize op as one on the corners, this should be less than 0.5 or chaos will ensue)

};


#endif // RESIZE_TOOL_H

