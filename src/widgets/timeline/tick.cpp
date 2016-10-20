// File:         tick.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for Tick Class


#include "blitapp.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/timeline.h"
#include "widgets/timeline/ruler.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include <QSizeF>
#include <QRectF>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>


Tick::Tick(Timeline *timeline, TimedFrame *tf, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    _tf(tf),
    _timeline(timeline)
{
    // Main constructor, it's always assumed that the pointer to the Frame isn't NULL
    Q_ASSERT(tf != NULL);
    Q_ASSERT(timeline != NULL);

    // signals
    connect(_tf, &TimedFrame::seqNumChanged, this, &Tick::_onTimedFrameNumChanged);
    connect(_tf, &TimedFrame::holdChanged, this, &Tick::_onTimedFrameHoldChanged);

    // Last few things
    _onTimedFrameNumChanged(tf->seqNum());
    _onTimedFrameHoldChanged(tf->hold());
}


QRectF Tick::boundingRect() const {
    // Size of the rectangle is mainly determinted by the hold of the Frame
    return QRectF(0, 0, TICK_WIDTH * _tf->hold(), TICK_HEIGHT);
}


void Tick::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Will draw the widget to be red if the Frame contained is Null.  Blue if its selcted
    QColor lineClr = Qt::darkGray;
    QColor fillClr = (_timeline->curTick == this) ? QColor(0xB2, 0xE4, 0xF1) : Qt::white;

    // The real painting code
    painter->setPen(QPen(lineClr, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter->fillRect(boundingRect(), fillClr);
    painter->drawRect(boundingRect());

    // Draw the Dot
    double padding = (TICK_WIDTH / 2.0) - (TICK_KEY_SIZE / 2.0);
    QRectF keyRect;
    keyRect.setX(boundingRect().x() + padding);
    keyRect.setY(boundingRect().y() + TICK_HEIGHT - TICK_KEY_SIZE - padding);
    keyRect.setSize(QSizeF(TICK_KEY_SIZE, TICK_KEY_SIZE));

    painter->setPen(Qt::NoPen);
    painter->fillRect(keyRect, Qt::black);
}


int Tick::type() const {
    // enable qgraphicsitem_cast
    return Type;
}


void Tick::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // For when the frame is clicked, will stop playing the animation
        BlitApp::app()->playAnimation(false);
        select();
        _firstSP = event->scenePos();
        event->accept();
    } else
        event->ignore();
}


void Tick::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // Null out the first scene pos 
    _firstSP = QPointF();

    if (_moving) {
        // For when the frame moves
        emit doneMoving(event);
        _moving = false;
        event->accept();
    }
}


void Tick::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!_moving && (qAbs(event->scenePos().x() - _firstSP.x()) >= TICK_WIDTH)) {
        // If the Tick isn't currently moving and the mouse have moved at least one Tick's width, start 
        // the moving process
        _moving = true;
    }

    if (_moving)
        emit moving(event);
}


void Tick::select() {
    // Selcet the Tick (it will emit its signals).  Will do nothing if the Tick is the currently selcted
    // one.  Will change the current frame and set the Canvas widget
    if (_timeline->curTick != this) {
        // Swap for ourselves
        Tick *old = _timeline->curTick;
        _timeline->curTick = this;

        // Set the current Frame as well as the canvas
        BlitApp::app()->setCurTimedFrame(_tf);
        BlitApp::app()->setCurSeqNum(_tf->seqNum());
        
        // Other signals
        emit selected();
        update();
        if (old)
            old->update();
    }
}


TimedFrame *Tick::timedFrame() {
    // Returns a pointer to the Frame contained by this tick;
    return _tf;
}


/*!
    Called by TimedFrame::seqNumChanged() that this Tick is attached to.
*/
void Tick::_onTimedFrameNumChanged(int num) {
    QPointF pos;
    pos.setX((num - 1) * TICK_WIDTH);
    pos.setY(RULER_HEIGHT);

    // Tell the Scene to update
    setPos(pos);

    // Save the new state
    _lastSeqNum = num;
}


/*!
    Called by TimedFrame::holdChanged() that this Tick is attached to.
*/
void Tick::_onTimedFrameHoldChanged(int hold) {
    // If the hold is smaller, then we need to tell the scene to clear up some extra area
    int delta = _lastHold - hold;
    if (delta > 0) {
        // Compute the invalid area
        QRectF invalid = boundingRect();
        invalid.moveTo(pos());
        invalid.setWidth(_lastHold * TICK_WIDTH);
        scene()->update(invalid);
    } else
        update();            // Normal update

    // Save the new state
    _lastHold = hold;
}

