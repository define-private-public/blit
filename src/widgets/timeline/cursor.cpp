// File:         cursor.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for Cursor.



#include "widgets/timeline/cursor.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/ruler.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include "blitapp.h"
#include <QPointF>
#include <QRectF>
#include <QPolygonF>
#include <QList>
#include <QPainter>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>



Cursor::Cursor(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    // Staring position is assmed to be the first frame
    setPos(0, 0);

    _shape << QPointF(0, 0)
           << QPointF(12, 0)
           << QPointF(12, 12)
           << QPointF(8, 16)
           << QPointF(0, 16);

    // child graphics object is the REDLINE
    _redLine = new QGraphicsLineItem(this);
    _redLine->setLine(0, 1, 0, TICK_HEIGHT + RULER_HEIGHT);
    _redLine->setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
}


QRectF Cursor::boundingRect() const {
    // Overrided pure virtual function
    return _shape.boundingRect();
}


void Cursor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Just draw the thing, should also take care of the REDLINE when rendering
    painter->translate(0.5, 0.5);    // For better (tiny pixel) drawing

    // Draw the cursor
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter->setBrush(Qt::white);
    painter->drawPolygon(_shape);
}


int Cursor::type() const {
    // enable qgraphicsitem_cast
    return Type;
}


void Cursor::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // Actions:
    //   - If the left button clicks on the tick, star the scrubbing process
    if (event->button() == Qt::LeftButton)
        _scrubbing = true;
}


void Cursor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // Actions:
    //   - If the left button is released, and we're scrubbing, then stop scrubbing
    if ((event->button() == Qt::LeftButton) && _scrubbing)
        _scrubbing = false;
}


void Cursor::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // Actions:
    //   - If the _scrubbing variable is set, then well... scrub
    if (_scrubbing) {
        // Move everything to the postion of the mouse cursor.

        Tick *t = getTickOver();

        // Move the cursor
        QPointF newPos(event->scenePos());
        newPos.setY(0);

        // Bounds checking
        int maxX = BlitApp::app()->xsheet()->seqLength() * TICK_WIDTH;
        if (newPos.x() < 0)
            newPos.setX(0);
        else if (newPos.x() > maxX)
            newPos.setX(maxX);

        // I like to move it move it
        setPos(newPos);

        // Failsafe
        if (!t)
            return;

        // Have the tic, move onto it
        if (t != _curTick) {
            _curTick = t;
            t->select();
            emit overNewTick(t);
        }

        // And update the postion
        update();
        BlitApp::app()->setCurSeqNum(seqNumOver());
    }
}


int Cursor::seqNumOver() {
    // Looks at the current position of the cursor and then returns a sequence number for that position
    return ((int)(x() / TICK_WIDTH)) + 1;
}


Tick *Cursor::getTickOver() {
    // Returns a pointer to the tick that the cursor (the _redLine part) is currently over, if any.
    // May return a NULL pointer.
    //
    // This function does not alter the _curTick variable

    // The cursor should only collide with two items (max)
    // Get the Tick object.  (Unfortunately, the Ruler collides, but we can forget about that easily)
    QList<QGraphicsItem *> collides = _redLine->collidingItems();
    Tick *tick = NULL;
    for (int c = 0; c < collides.size(); c++) {
        // Find the Tick
        QGraphicsItem *gi = collides[c];
        tick = qgraphicsitem_cast<Tick *>(gi);        // Will be Tick poiner on success, NULL on failure
        if (tick)
            break;
    }

    return tick;
}


void Cursor::onTickSelected() {
    // Called when a Tick has been selected, it will move the cursor to that Tick/Frame
    Tick *t = qobject_cast<Tick *>(sender());

    // Check to be apathetic
    if (t == getTickOver())
        return;

    if (t != _curTick) {
        moveToTick(t);
        _curTick = t;
    }
}


void Cursor::moveToTick(Tick *tick) {
    // Calls the funciton moveToFrame.  Is used when a Tick has been selected.  It doesn't select
    // the tick setself
    moveToTimedFrame(tick->timedFrame());
}


void Cursor::moveToTimedFrame(TimedFrame *tf) {
    // Moves the cursor to the Tick representing the supplied frame.  Only will work when not already
    // scrubbing through the Timeline.  Doesn't not select the Tick/Frame itself
    if (!_scrubbing)
        setX((tf->seqNum() - 1) * TICK_WIDTH);
}


void Cursor::moveToSeqNum(int seqNum) {
    // Will move the cursor to a supplied sequence number.  seqNum must be a non-negative integer, if
    // isn't nothing will happen.  Will not select tick/frame
    seqNum -= 1;
    if (seqNum >= 0)
        setX(seqNum * TICK_WIDTH);
    
    BlitApp::app()->setCurSeqNum(seqNumOver());
} 
