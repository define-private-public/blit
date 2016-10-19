// File:         erasertool.h
// Author:       Ben Summerton (define-private-public)
// Description:  Eraser is an implementation of the Tool interface.  It comes standard in Blit.  
//               The Eraser erases (drawn) pixels from Cels.
//
//               Most of the code is just a copy n'paste job from the PenTool tool

// TODO
//   - Eventually add double spin box to set opacity between 0-100%
//     - Don't offer an in between 0-255 to the user, give them a more human readable number


#ifndef ERASER_TOOL_H
#define ERASER_TOOL_H


#include "tools/tool.h"
#include <QPointF>
#include <QPen>
#include <QPainter>
#include <QPointer>
class QLineF;
class QLabel;
class QSpinBox;


class EraserTool: public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    EraserTool(QObject *parent=NULL);
    ~EraserTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();

    QWidget *options();

private slots:
    void _onSizeSpinnerValueChanged(int value);
    void _onHardnessSpinnerValueChanged(int value);

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
//    int _size = 1;
    int _hardness = 0xFF;
    QPainter _painter;
    QPen _pen;
    QPointer<QLabel> hardnessLabel;

    // State variables for when painting
    bool _eraserDown = false;
    QPointF _celPos;
    QPointF _lastPoint;
    QImage *_celImage = NULL;
    QImage *_drawBuffer = NULL;
};


#endif // ERASER_TOOL_H

