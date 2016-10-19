// File:         trianglemarker.h
// Author:       Ben Summerton (define-private-public)
// Description:  A small little triangle that appears belog the ticks as a sort of marker in the Timeline
//               widget.
//
//               An example use of this widget is when the user is trying to move a Frame/Tick in the
//               Timeline widget.  It's mainly meant to be just a temporary object that will be quickly
//               deleted after use.


#ifndef TRIANGLE_MARKER_H
#define TRIANGLE_MARKER_H


#define TRIANGLE_MARKER_TYPE 4


#include <QGraphicsPolygonItem>


class TriangleMarker : public QGraphicsPolygonItem {
public:
    enum { Type = UserType + TRIANGLE_MARKER_TYPE };

    TriangleMarker(QGraphicsItem *parent=NULL);
    int type() const;

    void moveToSeqNum(int num);

};


#endif // TRIANGLE_MARKER_H

