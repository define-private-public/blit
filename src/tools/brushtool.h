// File:         brushtool.h
// Author:       Ben Summerton (define-private-public)
// Description:  Brush is an implementation of the Tool interface.  It comes standard in Blit.  
//               The Brush tool draws soft strokes.


#ifndef BRUSH_TOOL_H
#define BRUSH_TOOL_H


#include "tools/tool.h"
#include <QPen>
#include <QBrush>
class QImage;
class QPainterPath;


class BrushTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    BrushTool(QObject *parent=NULL);
    ~BrushTool();

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
    // Internal functions
    void _transferDrawing();

    // Member vars
    int _size = 5;
    QPen _pen;
    QBrush _brush;

    // State variables for when painting
    bool _brushDown = false;
    QPainterPath *_path = NULL;
    QPointF _celPos;
    QImage *_celImage = NULL;

};


#endif // BRUSH_TOOL_H

