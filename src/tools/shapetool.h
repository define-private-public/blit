// File:         shapetool.h
// Author:       Ben Summerton (define-private-public)
// Description:  ShapeTool is an implementation of the Tool interface.  It comes standard in Blit.  
//               the ShapeTool doesn't do much other than draw solid lines.


#ifndef SHAPE_TOOL_H
#define SHAPE_TOOL_H


#include "tools/tool.h"
#include <QPointer>
#include <QPointF>
#include <QPainterPath>
#include <QPen>
class QLineF;
class QSpinBox;
class QCheckBox;


class ShapeTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    // Enumeration, used to select what shape to draw
    enum Shape {Box, Ellipse, Polygon};

    ShapeTool(QObject *parent=NULL);
    ~ShapeTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();

    QWidget *options();


private slots:
    void _onSizeSpinnerValueChanged(int value);

    // For options panel
    void _onShapeBoxChanged(int index);
    void _onSameSizeBoxClicked(bool checked);


public slots:
    // For drawing
    void onMouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    void onMouseMoved(QGraphicsSceneMouseEvent *event);
    void onMousePressed(QGraphicsSceneMouseEvent *event);
    void onMouseReleased(QGraphicsSceneMouseEvent *event);


private:
    // Internal functions
    void _transferDrawing();
    void _reset();

    // Member vars
    QPen _pen;
    Shape _shape;
    bool _sameSize = false;
    QPointer<QCheckBox> _sameSizeBox;

    // State variables for when painting
    bool _penDown = false;
    QPainterPath _path;
    QList<QPointF> _points;
    QPointF _startPoint;
    QPointF _curPoint;
    QPointF _celPos;
    QImage *_celImage = NULL;

};


#endif // SHAPE_TOOL_H

