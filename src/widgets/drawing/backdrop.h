// File:         backdrop.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the Backdrop class.


#ifndef BACKDROP_H
#define BACKDROP_H


#include <QGraphicsObject>
class Canvas;


class Backdrop : public QGraphicsObject {
    Q_OBJECT;

public:
    Backdrop(Canvas *canvas, QGraphicsItem *parent=NULL);
    ~Backdrop();

    // Overrides
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    // Info
    QColor color();


public slots:
    void setSize(QSize size);
    void setColor(QColor clr);


private slots:
    void _onCanvasZoomChanged(qreal zoom);


private:
    QColor _clr;            // Color of the backdrop
    QSize _size;            // Size of the backdrop (in pixels)
    qreal _zoom = 1;        // Zoom of the canvas
    qreal _boxSize = 16;    // Size of the boxes;

};


#endif // BACKDROP_H

