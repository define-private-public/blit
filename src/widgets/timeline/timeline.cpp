// File:         timeline.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Timeline.


#include "widgets/timeline/timeline.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/ruler.h"
#include "widgets/timeline/cursor.h"
#include "widgets/timeline/trianglemarker.h"
#include "widgets/timeline/bracketmarker.h"
#include "animation/xsheet.h"
#include "animation/timedframe.h"
#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QDebug>


Timeline::Timeline(QWidget *parent) :
    QWidget(parent)
{
    // Widgets and layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Set the scene and the views
    _scene = new QGraphicsScene(this);
    _view = new QGraphicsView(_scene, this);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _view->setFrameShape(QFrame::NoFrame);
    _view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _view->setSceneRect(0, 0, _scene->width(), TIMELINE_HEIGHT);
    layout->addWidget(_view);

    // Lastly, set the XSheet, which should take care of everything else
//    setXSheet(xsheet);
}


Timeline::~Timeline() {
    // Cleanup by NULLing out the XSheet
    setXSheet(NULL);
}


void Timeline::setXSheet(XSheet *xsheet) {
    // Will change/set the XSheet that is visualized by the Timeline.  Supplying a NULL pointer to an
    // XSheet will cause the Timeline to be cleared.  Providing a non-NULL pointer will do all those
    // magical things like adding Ticks and hooking up all of the signals/slots.  Supplying a new
    // XSheet will also reset the Timeline.

    // First clear out the old stuff
    _scene->clear();            // Will delete all of the items  (Including cursor, ruler, and trimarker)
    _ticks.clear();                // Clear out the list
    _tickMap.clear();            // Clear out the map
    curTick = NULL;                // Reset the pointer

    // Change the XSheet, and disconnect some old signals
    if (_xsheet) {
        // Only do this if we have an _xsheet set, that way we don't get any fun debugging logs
        disconnect(_xsheet, 0, this, 0);
        disconnect(_xsheet, 0, _cursor, 0);
        disconnect(_xsheet, 0, _leftBM, 0);
        disconnect(_xsheet, 0, _rightBM, 0);
    }
    _xsheet = xsheet;

//    // If the supplied XSheet is Null, reset some pointers, then exit
//    if (_xsheet->isNull()) {
//        _ruler = NULL;
//        _cursor = NULL;
//        _triMarker = NULL;
//        _xsheet = NULL;            // This too
//        return;
//    }

    // Check for set XSheet
    if (!_xsheet) {
        _ruler = NULL;
        _cursor = NULL;
        _triMarker = NULL;
        _leftBM = NULL;
        _rightBM = NULL;
        return;
    }

    // Make the ruler, cursor, and tri-marker
    _ruler = new Ruler(_xsheet->seqLength());
    _ruler->setZValue(1);
    _ruler->setVisible(!_xsheet.isNull());

    _cursor = new Cursor();
    _cursor->setZValue(2);
    _cursor->setVisible(!_xsheet.isNull());

    _triMarker = new TriangleMarker();
    _triMarker->setZValue(1);
    _triMarker->hide();                    // Hidden by default

    // Create the two bracket markers
    _leftBM = new BracketMarker(this, BracketMarker::BracketMarkerType::LEFT);
    _leftBM->setZValue(1);
//    _leftBM->setPos(TICK_WIDTH * 10, 0);
    _leftBM->hide();                    // Hidden by default
    _rightBM = new BracketMarker(this, BracketMarker::BracketMarkerType::RIGHT);
    _rightBM->setZValue(1);
//    _rightBM->setPos(TICK_WIDTH * 10, 0);
    _rightBM->hide();                // Hidden by default


    // Add in all of the ticks to the timeline
    QList<QPointer<TimedFrame>> frames = _xsheet->frames();
    for (auto fIter = frames.begin(); fIter != frames.end(); fIter++)
        addTick(*fIter);

    // Add in the cursor, ruler, and markers
    _scene->addItem(_ruler);
    _scene->addItem(_cursor);
    _scene->addItem(_triMarker);
    _scene->addItem(_leftBM);
    _scene->addItem(_rightBM);

    // Add some signals/sots
    connect(_xsheet, &XSheet::frameAdded, this, &Timeline::addTick);
    connect(_xsheet, &XSheet::frameRemoved, this, &Timeline::removeTickByTimedFrame);
    connect(_xsheet, &XSheet::seqNumsChanged, this, &Timeline::updateRuler);
    connect(_xsheet, &XSheet::frameMoved, _cursor, &Cursor::moveToTimedFrame);
    connect(_xsheet, &XSheet::seqLegnthChanged, _leftBM, &BracketMarker::onXSheetSeqLengthChanged);
    connect(_xsheet, &XSheet::seqLegnthChanged, _rightBM, &BracketMarker::onXSheetSeqLengthChanged);
}


XSheet *Timeline::xsheet() {
    // Return a pointer to the currently set XSheet
    return _xsheet;
}


void Timeline::selectTickByIndex(int index) {
    // Sets off a Tick's "select()," signal via an index number.  The Index should correspond with a Tick
    // in the _ticks list.  If supplied an invalid index, a debug message will print an nothing will happen

    // Check for set XSheet
    if (!_xsheet)
        return;

    if ((index < 0) || (index >= _ticks.size())) {
        qDebug() << "Invalid index supplied for selectTickByIndex() in Timeline; index=" << index;
        return;
    }

    // Else, all good!
    _ticks[index]->select();
}


void Timeline::selectTickByTimedFrame(TimedFrame *tf) {
    // Selects a tick By a pointer to a Frame.  Will do nothing if the pointer to the Frame doesn't exist.

    // Check for set XSheet
    if (!_xsheet)
        return;

    if (_tickMap.contains(tf))
        _tickMap[tf]->select();
}


/*!
    Will turn on/off Selective playback mode.  Will show/hide the bracket markers and 
    make them moveable.
*/
void Timeline::turnOnSelectivePlayback(bool enabled) {
    _leftBM->setVisible(enabled);
    _rightBM->setVisible(enabled);
}


/*!
    Returns a pointer to the Cursor widget set.  Will be NULL if there is no XSheet set.
*/
Cursor *Timeline::timelineCursor() {
    return _cursor;
}


/*!
    Returns a pointer to the Left Bracket Marker. Will be NULL if there is no XSheet set.
*/
BracketMarker *Timeline::leftBM() {
    return _leftBM;
}


/*!
    Returns a pointer to the Right Bracket Marker. Will be NULL if there is no XSheet set.
*/
BracketMarker *Timeline::rightBM() {
    return _rightBM;
}


QList<Tick *> Timeline::ticks() {
    // Returns a list of pointers to the Ticks currently in the Timeline.  Should all be in order
    return _ticks;
}


Tick *Timeline::tickByTimedFrame(TimedFrame *tf) {
    // Finds a Tick in the Timeline by looking for its Frame pointer.  If it's not found, then a NULL
    // pointer will be returned
    if (!_tickMap.contains(tf))
        return NULL;
    else
        return _tickMap[tf];
}


void Timeline::addTick(TimedFrame *tf) {
    // Will add a tick to the timeline.  This should be only called via the frameAdded() signal in the
    // XSheet class, but it's being called internall when setting an XSheet.

    // Check for set XSheet
    if (!_xsheet)
        return;


    // Make the Tick from the Frame
    Tick *tick = new Tick(this, tf);
    _ticks.append(tick);
    _scene->addItem(tick);
    _tickMap[tf] = tick;
    
    // Frame -> Tick

    // Connect up a crapton of signals
    // Tick -> Others
    connect(tick, &Tick::moving, this, &Timeline::movingTick);
    connect(tick, &Tick::doneMoving, this, &Timeline::doneMovingTick);
    connect(tick, &Tick::selected, _cursor, &Cursor::onTickSelected);
}
    
    
void Timeline::removeTickByTimedFrame(TimedFrame *tf) {
    // Delete a Tick from the GraphicsScene by its Frame object (or pointer to it).  It will not delete
    // the Frame object.  If the Frame/Tick isn't found, then nothing will happen.  This funciton will
    // usually be called after a Frame has been removed from the XSheet any everything else has updated.
    //
    // This should be called by the frameRemoved() signal in XSheet

    // Check for set XSheet
    if (!_xsheet)
        return;


    if (_tickMap.contains(tf)) {
        // Get rid of the Tick, don't delete yet
        Tick *tick = _tickMap[tf];
        _ticks.removeOne(tick);
        _scene->removeItem(tick);
        _tickMap.remove(tf);

        // Now select another tick, using the old sequence number
        TimedFrame *nextTF = _xsheet->frameAtSeq(tf->seqNum());
        if (nextTF) 
            _tickMap[nextTF]->select();                    // Some other Frame shares the number, select it
        else {
            // Okay, must have been the last frame, select the "next," last farme
            _tickMap[_xsheet->frameAtSeq(_xsheet->seqLength())]->select();
        }

        // Disconnect signals, where the Frame is the sender (don't need to worry about Tick as sender)
        disconnect(tf, 0, tick, 0);

        // Redraw
        tick->update();

        // Delete after
        delete tick;
    }
}


void Timeline::movingTick(QGraphicsSceneMouseEvent *event) {
    // This slot will ne called when a Tick emits its moving signal.  It will show the TriangleMarker when
    // a Tick is being moved.

    // Check for set XSheet
    if (!_xsheet)
        return;

    Q_ASSERT(_triMarker != NULL);
    _triMarker->moveToSeqNum(_seqNumAtEvent(event));
    _triMarker->show();
}


void Timeline::doneMovingTick(QGraphicsSceneMouseEvent *event) {
    // This slot is called when a Tick emits its doneMoving signal.  It will hide the TriangleMarker and 
    // possibly move a Frame/Tick in the XSheet/Timeline

    // Check for set XSheet
    if (!_xsheet)
        return;

    Q_ASSERT(_triMarker != NULL);
    _triMarker->hide();

    // Do the move
    Tick *tick = qobject_cast<Tick *>(sender());
    TimedFrame *tf = tick->timedFrame();
    int seqNum = _seqNumAtEvent(event);

    // Figure out which way the frame is omving
    if ((seqNum != 1) && (tf->seqNum() < seqNum))
        seqNum -= 1;                            // Moving a Frame to the right
    else if (seqNum >= _xsheet->seqLength())
        seqNum = _xsheet->seqLength() - 1;        // Tick/ Frame should be moved to the end

    // Check for a self move
    if (tf->hasSeqNum(seqNum))
        return;
    
    // Do the move
    _xsheet->moveFrame(tf->seqNum(), seqNum);
    tick->select();
}


void Timeline::updateRuler() {
    // Should be called by the XSheet's seqNumsChanged() signal.  Will change the size of the contained
    // Ruler object.  Also will dictate the size of the QGraphicsView Scene Rect policy

    // Check for set XSheet
    if (!_xsheet)
        return;

    if (!_ruler)
        return;
    
    // Set the size and figure out a draw bounds
    QRectF redrawRect(_ruler->boundingRect());
    _ruler->setLength(_xsheet->seqLength());
    
    if (_ruler->boundingRect().width() > redrawRect.width())
        redrawRect = _ruler->boundingRect();

    // Setup the view to only have a specific view size of the Scene
    _view->setSceneRect(0, 0, _ruler->boundingRect().width(), TIMELINE_HEIGHT);
    _scene->invalidate(_ruler->x(), _ruler->y(), redrawRect.width(), redrawRect.height());

}


int Timeline::_seqNumAtEvent(QGraphicsSceneMouseEvent *event) {
    // Takes in a mouse event, looks at its position, then determines an appropriate sequence number
    // based upon the X Position.

    // Check for set XSheet
    if (!_xsheet)
        return 1;        // TODO I don't think it should be returning this at all, investigate

    int x = event->scenePos().x();
    if (x < 0)
        return 1;
    else {
        TimedFrame *tf = _xsheet->frameAtSeq((x / TICK_WIDTH) + 1);
        if (tf)
            return tf->seqNum();        // Good Frame, return its sequence number
        else {
            // Couldn't find frame, must be at the end.
            return (_xsheet->seqLength() + 1);    // Put it at the end
        }
    }
}


