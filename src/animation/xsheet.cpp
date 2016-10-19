// File:         xsheet.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Implementation of the XSheet class
//


/*!
    \inmodule Animation
    \class XSheet
    \brief XSheet is the main object for organzing Frame objects by timing and spacing.

    TODO this documentation seriously needs to be updated, please ignore the following stuff down there

    This is the frame manager for the application.  Other classes act as a graphical view, but
    this module is an interface to all of the frames.  The Frames are a doubly linked list, but
    this manages other things like the current frame, rearaging frames, adding frames, etc.  You
    can touch the Frames/Cels directrly if you want, but it's recommand that you let the XSheet
    manage the Frame numbers and holds.
    
    An XSheet can also be considered Null, this means that it doesn't have any frames in it.  By
    this definition, a newly created XSheet will be Null until some frames are added to it.
    
    The "_seqMap" variable doesn't exist anymore, but it may be reintroduced on a later date.
    I'm still leaving this documentation in here since it expalins a bit how frame numbers and holds
    work for ordering in the XSheet
    --------------------------------------
      Because a Frame object has a number that identifies it start frame, as well as a hold, A
      Frame object can be visible for a set amound of frames
      Take for Example these two Frame object named A and B:
      \code
        A.num = 1, A.hold = 8
        B.num = 9, B.hold = 2
      \endcode
    
      Our sequence here only consists of two Frame objects, but lasts for 10 frames, so:
    
      Yet, the _seqMap variable will map a frame sequence numberto the correct Frame object, so:
      \code
        _seqMap[1..8] = A
        _seqMap[9..10] = B
      \endcode
*/



#include "animation/xsheet.h"
#include "animation/animation.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include <QDebug>


/*!
    Constrcuts a new Null XSheet.  XSheet will be Null unless you add a Frame to it.

    \sa isNull()
*/
XSheet::XSheet(Animation *anim, int fps) :
    QObject(anim)
{
    setFPS(fps);

    // Append the initial layer
    _frames.append(QList<QPointer<TimedFrame>>());

    // Info
    qDebug() << "[XSheet created]" << this;
}


/*!
    Deconstructor.  Will clear out all of the Frames that are still in the list
*/
XSheet::~XSheet() {
    // Clear out all of the TimedFrames
    // TODO call _frames.clear() instead?
    auto lIter = _frames.begin();
    while (lIter != _frames.end()) {
        auto tfIter = lIter->begin();

        while (tfIter != lIter->end()) {
            delete *tfIter;
            tfIter++;
        }
        lIter->clear();
        lIter++;
    }
    _frames.clear();

    qDebug() << "[XSheet deleted]" << this;
}


/*!
    The XSheet is considered Empty if there are no frames in the list (which also means the sequence
    length is zero).

    \sa seqLength()
*/
bool XSheet::isEmpty() {
    return (_seqLength == 0);
}


/*!
    Returns the number of Frame objects contained in the XSheet.
*/
int XSheet::numFrames() {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];
    return firstPlane.size();
}


/*!
    Returns the sequence length of the XSheet.
*/
int XSheet::seqLength() {
    return _seqLength;
}


/*!
    Returns the framerate of the XSheet.

    \sa setFPS()
*/
int XSheet::FPS() {
    return _fps;
}


/*!
    Sets the framerate of the XSheet, must be a positive integer

    \sa FPS()
    \sa FPSChanged()
*/
void XSheet::setFPS(int fps) {
    if (_fps != fps) {
        // Check for postivity
        if (fps < 1) {
            qDebug() << "fps must be a positive integer, fps=" << fps;
            return;
        }

        // FPS is good, set it and emit a signal
        _fps = fps;
        emit FPSChanged(_fps);
    }
}


/*!
    Return the pointers to the Frames in a QList.

    \sa frameAtSeq()
*/
QList<QPointer<TimedFrame>> XSheet::frames() {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];

    return firstPlane;
}


/*!
    Returns a pointer to a frame by sequence number.  If the frame isn't found, it will return a
    NULL QPointer
    
    \sa frames()
*/
QPointer<TimedFrame> XSheet::frameAtSeq(int num) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];

    // Iterator throught the _frames list till we find that one special Frame
    auto flIter = firstPlane.begin();
    while (flIter != firstPlane.end()) {
        if ((*flIter)->hasSeqNum(num))
            return *flIter;

        flIter++;
    }

    // didn't find it, return an empty QPointer
    return NULL;
}


/*!
    Adds a frame to the XSheet by Seqence number
    We assume that the hold is set, but the it's sequence number will be modified.
    
    If `at` is not set to End, it will try to insert it at the frame, but it doesn't always
    guareteed it.  Say for instance, we ahve a seq like this:
    \code
    ... 1  2  3  4  5  6  7  8  9 ...
    ... A  A  B  B  C  C  D  D  _ ...     (A-D are frames, _ is No frame)
    \endcode
    
    If you want to insert it at seq num 3 or 4 , it will look like this:
    \code
    ... 1  2  3  4  5  6  7  8  9 ...
    ... A  A  N  N  B  B  C  C  D ...
    \endcode
    
    Think that it will add/insert it at the nearest frame to the left

    Emits the frameAdded() and seqLegnthChanged() signals

    \sa frames()
    \sa frameAtSeq()
    \sa removeFrame()
    \sa moveFrame()
    \sa frameAdded()
*/
void XSheet::addFrame(TimedFrame *frame, int at) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];
    int frameIndex = -1;

    if ((at == XSHEET_END) || (at > _seqLength)) {
        // Append, there'll be no collisions
        frame->setSeqNum(_seqLength + 1);
        frame->setXSheet(this);
        firstPlane.append(frame);
        frameIndex = firstPlane.size() - 1;
    } else if (at < 1) {
        // Not a good value for at supplied, print a debug message and don't change a thing
        qDebug() << "[XSheet addFrame " << this << "] at=" << at << ", must be a postitive integer or XSHEET_END";
        return;
    } else {
        // Insert the TimedFrame into the middle or the beginning
        auto flIter = firstPlane.begin();
        TimedFrame *cursor = NULL;
        frameIndex = 0;

        while (flIter != firstPlane.end()) {
            cursor = *flIter;

            // Check for a location
            if (cursor->hasSeqNum(at)) {
                // Found where to insert
                frame->setSeqNum(cursor->seqNum());            // Take ownership of cursor's number
                frame->setXSheet(this);
                firstPlane.insert(frameIndex, frame);
                break;
            }

            frameIndex++;
            flIter++;
        }

    }

    // last minute things: adjust seq length, add a slot, and emit a signal
    connect(frame, &TimedFrame::holdChanged, this, &XSheet::_onHoldChanged);
    _updateSeqNums(frameIndex);
    emit frameAdded(frame);
    emit seqLegnthChanged(seqLength());
}


/*!
    Removes a frame from the XSheet by sequence number, will return a pointer to a Frame.  It
    might be a Null frame, or a Frame that was in the XSheet.
    
    Emits the frameRemoved() and seqLegnthChanged() signals

    \sa frames()
    \sa frameAtSeq()
    \sa addFrame()
    \sa moveFrame()
    \sa frameRemoved()
*/
QPointer<TimedFrame> XSheet::removeFrame(int at) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];
    QPointer<TimedFrame> frame;
    int frameIndex = -1;

    // Remove it from the correct spot
    if (firstPlane.size() == 0) {
        qDebug() << "[XSheet removeFrame " << this << "] There are no fraems in the XSheet right now.";
        return frame;
    } else if ((at > _seqLength) || (at == 0) || (at <= -2)) {
        // out of bounds
        qDebug() << "[XSheet removeFrame " << this << "] at=" << at << ", must be between 1 or " << _seqLength << " (inclusive)";
        return frame;
    } else if ((at == XSHEET_END) || firstPlane.last()->hasSeqNum(at)) {
        // Pop it off of the end, don't need to update any frame numbers
        frame = firstPlane.takeLast();
        frameIndex = firstPlane.size() - 1;
    } else {
        // Removing the frame from the middle or the beginning
        auto flIter = firstPlane.begin();
        TimedFrame *cursor = NULL;
        frameIndex = 0;

        while (flIter != firstPlane.end()) {
            cursor = *flIter;

            // Check for a locaiton
            if (cursor->hasSeqNum(at)) {
                // Found where to insert
                frame = firstPlane.takeAt(frameIndex);            // Take the frame out
                firstPlane[frameIndex]->setSeqNum(frame->seqNum());    // Take ownership of frame number
                break;
            }

            frameIndex++;
            flIter++;
        }

//        while (iter.hasNext()) {
//            cursor = iter.next();
//
//            // Check for a locaiton
//            if (cursor->hasSeqNum(at)) {
//                // Found where to insert
//                delete frame;                                // Delete the temporary Null Frame
//                frame = firstPlane.takeAt(frameIndex);            // Take the frame out
//                firstPlane[frameIndex]->setSeqNum(frame->seqNum());    // Take ownership of frame number
//                break;
//            }
//
//            frameIndex++;
//        }
    }

    // do a disconnect
    disconnect(frame, 0, this, 0);

    // Update other properties
    _updateSeqNums(frameIndex);
    emit frameRemoved(frame);
    emit seqLegnthChanged(seqLength());
    return frame;
}


/*!
    Moves a frme from one location to another by sequence number.
    It will grab the frame that has the sequence number of at then place it infront of (or at) to.

    \sa frames()
    \sa frameAtSeq()
    \sa addFrame()
    \sa removeFrame()
    \sa frameMoved()
*/
void XSheet::moveFrame(int at, int to) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];

    // Fist check bounds
    if ((at < 1) or (at > _seqLength)) {
        qDebug() << "[XSheet moveFrame " << this << "] at=" << at << ", must be between 1 or the sequence's length (" << _seqLength << ")";
        return;
    } else if ((to < 1) or (to > _seqLength)) {
        qDebug() << "[XSheet moveFrame " << this << "] to=" << to << ", must be between 1 or the sequence's length (" << _seqLength << ")";
        return;
    }
    
    // Bounds are okay, do the swap
    int indexAt = -1, indexTo = -1;

    // Loop throught all of the frames until we get both indices
    TimedFrame *frame = NULL;
    for (int i = 0; i < firstPlane.size(); i++) {
        frame = firstPlane[i];

        // Check for sequence numbers
        if (frame->hasSeqNum(at))
            indexAt = i;
        if (frame->hasSeqNum(to))
            indexTo = i;
        
        // Break if both found
        if ((indexAt != -1) && (indexTo != -1))
            break;
    }

    // Samies?  Just return
    if (indexAt == indexTo)
        return;
    
    // Make the move
    TimedFrame *a = firstPlane[indexAt];
    TimedFrame *b = firstPlane[indexTo];
    firstPlane.move(indexAt, indexTo);

    // update the sequence numbers (look at the one in the lower position)
    int pivot = qMin(indexAt, indexTo);
    TimedFrame *dispFrame = (pivot == indexAt) ? a : b;
    firstPlane[pivot]->setSeqNum(dispFrame->seqNum());
    _updateSeqNums(pivot);

    // And lastly emit a signal
    emit frameMoved(a);
} 


/*!
    When a frame's hold value is changed, this slot will be called to update all of the sequence
    nums of all subsequent frames in the _frames list.  frame is guarenteed to be in _frames.

    Emits the seqLegnthChanged() signal.

    \sa seqNumsChanged()
*/
void XSheet::_onHoldChanged(int hold) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];

    TimedFrame *frame = (TimedFrame *)sender();
    _updateSeqNums(firstPlane.indexOf(frame));

    emit seqLegnthChanged(seqLength());
}


/*!
    Will go through each frame in the list and adjust its sequence number
    If you don't supply a value for at, by default, it will go through the whole list
    else.  at in this case is an index [0, numFrames], not a sequence number.
    
    The frame at Index at should also have it's number correctly set.  So for instance,
    if we have at=0, then the first frame's seq number should be 1, else, it can cause
    some problems.
    
    This function will not alter the sequence length, just the Frame object's sequence nums

    \sa seqNumsChanged()
*/
void XSheet::_updateSeqNums(int at) {
    QList<QPointer<TimedFrame>> &firstPlane = _frames[0];

    // Get the first
    TimedFrame *frame = firstPlane[at];
    int nextNum = frame->seqNum() + frame->hold();

    // Start assigning to the rest
    for (int i = (at + 1); i < firstPlane.size(); i++) {
        frame = firstPlane[i];
        frame->setSeqNum(nextNum);
        nextNum += frame->hold();
    }

    // Update the sequence length
    _seqLength = nextNum - 1;
    emit seqNumsChanged();
}

