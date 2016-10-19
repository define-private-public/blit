// File:         frameitem.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the FrameItem class.


/*!
    \inmodule Animation
    \class FrameItem
    \brief FrameItem is a visualization of a rendered Frame inside the Canvas widget.

    This is used mainly for onionskinning/lightable purposes.
*/


#include "animation/frameitem.h"
#include "animation/frame.h"
#include "animation/celref.h"
#include <QPainter>
#include <QDebug>


/*!
    Creates the FrameItem.  \a frame should point to a valid Frame object (it's not
    recommended to give this parameter NULL).
*/
FrameItem::FrameItem(Frame *frame) :
    QGraphicsObject(NULL),
    _frame(frame)
{
    // got a Frame? do things
    if (_frame) {
        // connect signals & slots
        connect(_frame, &Frame::celAdded, this, &FrameItem::_onCelAdded);
        connect(_frame, &Frame::celRemoved, this, &FrameItem::_onCelRemoved);
        connect(_frame, &Frame::celMoved, this, &FrameItem::_onCelMoved);
    }

    qDebug() << "[FrameItem created] frame=" << _frame;
}


/*!
    Deconstructor (does cleanup)
*/
FrameItem::~FrameItem() {
    qDebug() << "[FrameItem destroyed] frame=" << _frame;
}


/*!
    Overloaded funtion.  Returns the bounding box for the FrameItem.  Should be the same
    as the current frame size.
*/
QRectF FrameItem::boundingRect() const {
    if (_frame)
        return QRectF(QRect(QPoint(0, 0), _frame->frameSize()));
    else
        return QRectF();
}


/*!
    Overloaded function.  Renders the frame onto the painter.
*/
void FrameItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (_frame) {
        // Draw the Frame
        painter->drawImage(0, 0, _frame->render());
    }
}


/*!
    Triggered via Frame::celAdded(), this will schedule a redraw.
*/
void FrameItem::_onCelAdded(CelRef *cel) {
    update();
}


/*!
    Triggered via Frame::celRemoved(), this will schedule a redraw.
*/
void FrameItem::_onCelRemoved(CelRef *cel) {
    update();
}


/*!
    Triggered via Frame::celMoved(), this will schedule a redraw.
*/
void FrameItem::_onCelMoved(CelRef *cel) {
    update();
}


/*!
    Triggered via Frame::celRefPositionChanged(), this will schedule a redraw.
*/
void FrameItem::_onCelRefPositionChanged(CelRef *ref) {
    update();
}
