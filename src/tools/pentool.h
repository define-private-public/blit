// File:         pentool.h
// Author:       Ben Summerton (define-private-public)
// Description:  PenTool is an implementation of the Tool interface.  It comes standard in Blit.  
//               the PenTool doesn't do much other than draw solid lines.


#ifndef PEN_TOOL_H
#define PEN_TOOL_H


#include "tools/tool.h"
#include <QPointF>
#include <QPen>
#include <QPainter>
class QLineF;
class QSpinBox;


class PenTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    PenTool(QObject *parent=NULL);
    ~PenTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();

    QWidget *options();

private slots:
    void _onSizeSpinnerValueChanged(int value);

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
    QPainter _painter;
    QPen _pen;

    // State variables for when painting
    bool _penDown = false;
    QPointF _lastPoint;
    QPointF _celPos;
    QImage *_celImage = NULL;
    QImage *_drawBuffer = NULL;
};


#endif // PEN_TOOL_H

