// File:         filltool.h
// Author:       Ben Summerton (define-private-public)
// Description:  FillTool is an implementation of the Tool interface.  It comes standard in Blit.  
//               the FillTool doesn't do much other than draw solid lines.


#ifndef FILL_TOOL_H
#define FILL_TOOL_H


#include "tools/tool.h"
#include <QPointF>
class QLineF;


class FillTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    FillTool(QObject *parent=NULL);
    ~FillTool();

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

    // State variables for when painting
    bool _penDown = false;
    QPointF _celPos;
    QPoint _fillSource;
    QImage *_celImage = NULL;
};


#endif // FILL_TOOL_H

