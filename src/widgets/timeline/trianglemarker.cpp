// File:         trianglemarker.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the TriangleMarker class


#include "widgets/timeline/trianglemarker.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/ruler.h"
#include <QPointF>
#include <QPolygonF>
#include <QPen>


TriangleMarker::TriangleMarker(QGraphicsItem *parent) :
    QGraphicsPolygonItem(parent)
{
    // Setup some points that will draw an isoscles triangle.
    QPolygonF poly;
    poly << QPointF(5, 0)
         << QPointF(10, 10)
         << QPointF(0, 10);
    setPolygon(poly);

    // Move to the first spot
    moveToSeqNum(1);

    // Drawing reateld
    setPen(Qt::NoPen);
    setBrush(Qt::black);
}


int TriangleMarker::type() const {
    // enable qgraphicsitem_cast
    return Type;
}


void TriangleMarker::moveToSeqNum(int num) {
    // Mvoes the tip of the triangle maker to the start of "num."  Please provide a positive integer for
    // num.  If one isn't provided, then the marker will just move to 1.
    if (num < 1)
        num = 1;

    qreal mid = boundingRect().width() / 2.0;
    setPos(((num - 1) * TICK_WIDTH) - mid + 1, RULER_HEIGHT + TICK_HEIGHT);
}

