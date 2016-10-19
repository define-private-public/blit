// File:         linetool.h
// Author:       Ben Summerton (define-private-public)
// Description:  LineTool is an implementation of the Tool interface.  It comes standard in Blit.  
//               the LineTool doesn't do much other than draw solid lines.


#ifndef LINE_TOOL_H
#define LINE_TOOL_H


#include "tools/tool.h"
#include <QPointF>
#include <QPen>
class QLineF;
class QSpinBox;


class LineTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    LineTool(QObject *parent=NULL);
    ~LineTool();

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
    QPen _pen;

    // State variables for when painting
    bool _penDown = false;
    QPointF _startPoint;
    QPointF _curPoint;
    QPointF _celPos;
    QImage *_celImage = NULL;
};


#endif // LINE_TOOL_H

