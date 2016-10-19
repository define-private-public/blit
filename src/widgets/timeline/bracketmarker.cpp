// File:         bracketmarker.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the BracketMarker class.


/*!
    \inmodule Timeline
    \class BracketMarker
    \brief A Simple type of marker used in Timeline widget, similar to TriangleMarker.

    Used to denote special information to the user.  One of the use cases is for specifying
    a selective playback section.

    Don't use setPos() to move this class, use moveToSeqNum() instead.
*/


//#include <QDebug>
#include "widgets/timeline/bracketmarker.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/ruler.h"
#include "widgets/timeline/timeline.h"
#include "blitapp.h"
#include "animation/xsheet.h"
#include <QPointF>
#include <QPolygonF>
#include <QPainter>
#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


/*!
    Creates a BracketMarker instance.  \a timeline must point to a valid Timeline instance (no
    NULL pointers or it will stop the application).  \a bmType must either be
    BracketMarkerType::LEFT or BracketMarkerType::RIGHT.  \a parent may be NULL.
*/
BracketMarker::BracketMarker(Timeline *timeline, BracketMarkerType bmType, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    _timeline(timeline),
    _type(bmType),
    _sideSize(TICK_WIDTH  / 2.0)
{
    Q_ASSERT(_timeline != NULL);

    // Setup the dimmer
    int seqLength = BlitApp::app()->xsheet()->seqLength();
    _dimmer = new QGraphicsRectItem(this);
    _dimmer->setPen(Qt::NoPen);
    _dimmer->setBrush(QColor(0xA0, 0xA0, 0xA0, 0xD0));        // Transparent

    // Change the rectangle based upon if it's the left or right bracket
    if (_type == BracketMarkerType::LEFT) {
        qreal width = TICK_WIDTH * 1;
        _dimmer->setPos(-width, 0);
        _dimmer->setRect(-width, RULER_HEIGHT, width, TICK_HEIGHT);
    } else if (_type == BracketMarkerType::RIGHT) {
        _dimmer->setPos(boundingRect().width() * 2, RULER_HEIGHT);
        _dimmer->setRect(0, 0, (seqLength - 1) * TICK_WIDTH, TICK_HEIGHT);
    }
//    _dimmer->hide();                                    // Hidden by default

    qDebug() << "[BracketMarker created] " << this << " type=" << typeStr(_type);
}


/*!
    Cleans up the BracketMarker, nothing much to see here.
*/
BracketMarker::~BracketMarker() {
    // Delete the dimemr
    delete _dimmer;

    qDebug() << "[BracketMarker destroyed] " << this << " type=" << typeStr(_type);
}


/*!
    Overloaded Function.  Returns the boudning rectangle for this GraphicsItem.

    Returns different bounding boxes based upon which type it is
*/
QRectF BracketMarker::boundingRect() const {
    qreal height = RULER_HEIGHT + TICK_HEIGHT;
    switch (_type) {
        case BracketMarkerType::LEFT: return QRectF(0, 0, _sideSize, height);
        case BracketMarkerType::RIGHT: return QRectF(_sideSize, 0, _sideSize, height);
    }
}


/*!
    Overloaded function.  Paints the bracket
*/
void BracketMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Draw two right triangles and a box
    painter->save();
    
    // Vars
    QPolygonF triTop, triBottom;
    QRectF rect;
    qreal width = TICK_WIDTH;
    qreal height = RULER_HEIGHT + TICK_HEIGHT;
    qreal barWidth = 2;

    // Change the geometry based upon which type we are
    switch (_type) {
        case BracketMarkerType::LEFT:
            triTop << QPointF(0, 0) << QPointF(_sideSize, 0) << QPointF(0, _sideSize);
            triBottom << QPointF(0, height) << QPointF(0, height - _sideSize) << QPointF(_sideSize, height);
            rect = QRectF(0, 0, barWidth, height);
            break;

        case BracketMarkerType::RIGHT:
            triTop << QPointF(_sideSize - 1, 0) << QPointF(width, 0) << QPointF(width, _sideSize);
            triBottom << QPointF(_sideSize - 1, height) << QPointF(width, height - _sideSize) << QPointF(width, height);
            rect = QRectF(width - barWidth, 0, barWidth, height);
            break;
    }

    // Paint it
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);
    painter->drawPolygon(triTop);
    painter->drawPolygon(triBottom);
    painter->fillRect(rect, Qt::black);

    painter->restore();
}


/*!
    Overloaded function.  Returns a custom type for this Graphics Item.
*/
int BracketMarker::type() const {
    return Type;
}


/*!
    Returns the sequence number the Bracket marker is currently over.  Should be
    a postive number.
*/
int BracketMarker::seqNumOver() {
    return ((int)(x() / TICK_WIDTH)) + 1;
}


/*!
    Will move the cursor to a supplied sequence number. \a  seqNum must be a
    non-negative integer. If it isn't nothing will happen. 

    Emits the seqNumOverChanged() signal on success.
*/
void BracketMarker::moveToSeqNum(int seqNum) {
    // move ourselves
    seqNum -= 1;
    if (seqNum >= 0) {
        setX(seqNum * TICK_WIDTH);
        seqNum += 1;
        emit seqNumOverChanged(seqNum);
    }

    // Move our brother
    BracketMarker *leftBM = _timeline->leftBM();
    BracketMarker *rightBM = _timeline->rightBM();
    if (_type == BracketMarkerType::LEFT) {
        //  Move the right one with it (if it's farther)
        if (!rightBM->moving() && (seqNum > _rightStartSeqNum))
            rightBM->moveToSeqNum(seqNum);

        // Dimmer
        qreal width = TICK_WIDTH * seqNum;
        _dimmer->setPos(0, 0);
        _dimmer->setRect(-width, RULER_HEIGHT, width, TICK_HEIGHT);
    } else if (_type == BracketMarkerType::RIGHT) {
        //  Move the left one with it (if it's farther)
        if (!leftBM->moving() && (seqNum < _leftStartSeqNum))
            leftBM->moveToSeqNum(seqNum);

        // Dimmer
        int seqLength = BlitApp::app()->xsheet()->seqLength();
        _dimmer->setPos(boundingRect().width() * 2, RULER_HEIGHT);
        _dimmer->setRect(0, 0, (seqLength - seqNum) * TICK_WIDTH, TICK_HEIGHT);
    }
} 


/*!
    Actions:
      - If the left button clicks on the marker, start the moving process
*/
void BracketMarker::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        _moving = true;
        
        // Record the start position
        _leftStartSeqNum = _timeline->leftBM()->seqNumOver();
        _rightStartSeqNum = _timeline->rightBM()->seqNumOver();

        
        qDebug() << "[BracketMarker mousePressEvent] type=" << typeStr(_type);
    }
}


/*!
    Actions:
      - If the left button is released, and we're moving , then stop moving
*/
void BracketMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if ((event->button() == Qt::LeftButton) && _moving) {
        _moving = false;

        // Reset the seq nums
        _leftStartSeqNum = 0;
        _rightStartSeqNum = 0;

        qDebug() << "[BracketMarker mouseReleaseEvent] type=" << typeStr(_type);
    }
}


/*!
    When the mouse if moved, and if the `_moving` variable is set to true, then
    this will move the marker
*/
void BracketMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // Actions:
    //   - If the _moving variable is set, then well... scrub
    if (_moving) {
        // Move everything to the postion of the mouse cursor.
        QPointF newPos(event->scenePos());
        if (_type == BracketMarkerType::LEFT)
            newPos.rx() += _sideSize;
        else if (_type == BracketMarkerType::RIGHT)
            newPos.rx() -= _sideSize;

        // Lock Y coordinate
        newPos.setY(pos().y());

        // Bounds Checking (For XSheet)
        int seqLength = BlitApp::app()->xsheet()->seqLength();
        int maxX = (seqLength - 1) * TICK_WIDTH;
        if (newPos.x() < 0)
            newPos.setX(0);
        else if (newPos.x() > maxX)
            newPos.setX(maxX);

        // I like to move it move it
        moveToSeqNum((newPos.x() / TICK_WIDTH) + 1);

        qDebug() << "[BracketMarker mouseMoveEvent] type=" << typeStr(_type);
    }
}


/*!
    Returns if the Bracket marker is currently being moved by another entity.
    (e.g. a mouse)
*/
bool BracketMarker::moving() const {
    return _moving;
}

/*!
    Returns the named string for BracketMarkerType \a type.  Returns
    an empty string if unkown.
*/
QString BracketMarker::typeStr(BracketMarkerType type) {
    switch (type) {
        case BracketMarkerType::LEFT: return "Left"; break;
        case BracketMarkerType::RIGHT: return "Right"; break;
        default: return "";
    }
}


/*!
    Called via the XSheet::seqLegnthChanged() signal, this will update the location.
    of the bracket marker if the \a seqNum reported is greather than the current one. 
*/
void BracketMarker::onXSheetSeqLengthChanged(int seqLength) {
    if (seqLength < seqNumOver())
        moveToSeqNum(seqLength);
    
    _dimmer->setRect(0, 0, (seqLength - seqNumOver()) * TICK_WIDTH, TICK_HEIGHT);
}

