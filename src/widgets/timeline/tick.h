// File:         tick.h
// Author:       Ben Summerton (define-private-public)
// Description:  A Tick is a graphical representation of a Frame object inside the Timeline widget.  The
//               tick itself shouldn't modify the Frame it represents, but it does have a reference to it.
//               
//               The Tick itself cannot be Null, but the Frame is represents can be.


#ifndef TICK_H
#define TICK_H


#define TICK_WIDTH 14
#define TICK_HEIGHT 28
#define TICK_KEY_SIZE 6
#define TICK_TYPE 1


#include <QGraphicsObject>
#include <QPointer>
class TimedFrame;
class Timeline;
class QPointF;


class Tick : public QGraphicsObject {
    Q_OBJECT;

public:
    enum { Type = UserType + TICK_TYPE };
    
    // Constructors
    Tick(Timeline *timeline, TimedFrame *tf, QGraphicsItem *parent=NULL);

    // Overridden Functions
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);
    int type() const;

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void select();

    TimedFrame *timedFrame();


signals:
    void selected();                                    // Will notify slot it's been selcted
    void moving(QGraphicsSceneMouseEvent *event);        // Used for when the tick is being moved
    void doneMoving(QGraphicsSceneMouseEvent *event);    // Used for when the tick is done being moved


private slots:
    void _onTimedFrameNumChanged(int num);
    void _onTimedFrameHoldChanged(int hold);


private:
    // Member vars
    TimedFrame *_tf = NULL;            // Pointer to containing frame
    QPointer<Timeline> _timeline;    // Pointer to the parent Timeline widget
    bool _moving = false;            // Flag for if the tick is currently being moved
    QPointF _firstSP;                // Used for moving Tick
    int _lastSeqNum = 1;            // Last sequence number the TimedFrame had
    int _lastHold = 1;                // Last hold number the TimedFrame had

};


#endif // TICK_H

