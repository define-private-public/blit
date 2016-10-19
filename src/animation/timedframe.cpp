// File:         timedframe.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the TimedFrame class.


/*!
    \inmodule Animation
    \class TimedFrame
    \brief TimedFrame is the "Ref," for the Frame class that contains timing information.
    
    Similar to the Cel/CelRef relationship, this is the "FrameRef," for Frame.  It has information
    like the frame number [1, n] and the hold value [1, n].

    Each Timed Frame owsn a set of "sequcene numbers,"  these are the frame numbers that refer to
    this frame.  The definiation is [frame_number, frame_number + hold - 1].  E.g. a TimedFrame with 
    numer=5 and hold=5 has the sequence numbers [5, 6, 7, 8, 9].

    The default number and hold value are both 1.
*/


//#include <QDebug>
#include "animation/timedframe.h"
#include "animation/frame.h"
#include "animation/xsheet.h"
#include <QDebug>


/*!
    Created a TimedFrame object.  \a seqNum must be a postive integer, as the same with \a hold.
    The numbers from [num, num + hold - 1] are considerd the sequence numbers that this Frame "owns."
    It's higly recommended that two TimedFrame on the same layer don't share any sequence
    numbers.

    \sa setSeqNum()
    \sa setHold()
*/
TimedFrame::TimedFrame(Frame *frame, int seqNum, int hold) :
    QObject(NULL),
    _frame(frame)
{
    if (_frame) {
        _frame->registerTimedFrame(this);
    }

    // Set the number and hold
    setSeqNum(seqNum);
    setHold(hold);
    
    qDebug() << "[TimedFrame created] frame=" << _frame;
}

/*!
    Tears down a TimedFrame.  will unregister the TimedFrame from the normal frame
*/
TimedFrame::~TimedFrame() {
    if (_frame) {
        _frame->unregisterTimedFrame(this);
    }
    
    qDebug() << "[TimedFrame deleted] frame=" << _frame;
}


/*!
    Returns the starting Sequence number (Frame number) for this TimedFrame.
    Should always be a positive integer.
*/
int TimedFrame::seqNum() {
    return _seqNum;
}


/*!
    Used to check if sequence number \a num is "owned," by this TimedFrame.
    The range of sequence numbers for a TimedFrame are
    [frame_number, frame_number + hold - 1].

    Returns true if so, false otherwise.
*/
bool TimedFrame::hasSeqNum(int num) {
    int upper = _seqNum + (_hold - 1);
    return ((_seqNum <= num) && (num <= upper));
}


/*!
    Returns a list of sequence numbers that this TimedFrame "owns."
    Will be a list of positive integers in the range of
    [frame_number, frame_number + hold - 1].
*/
QList<int> TimedFrame::seqNums() {
    // Tak on all of the numbers
    QList<int> nums;
    int upper = _seqNum + (_hold - 1);
    for (int i = _seqNum; i <= upper; i++)
        nums.append(i);

    return nums;
}


/*!
    Returns the hold value that this TimedFrame is set to.  Should be a 
    positive integer.
*/
int TimedFrame::hold() {
    return _hold;
}


/*!
    Returns a pointer to the Frame object this class is Timing.
*/
QPointer<Frame> TimedFrame::frame() {
    return _frame;
}


/*!
    Returns true if this TimedFrame holds onto a Frame.
*/
bool TimedFrame::hasFrame() {
    return !_frame.isNull();
}


/*!
    Sets the XSheet for the TimedFrame.  The XSheet will also set to be
    the parent of the TimedFrame.  Normally is only called by XSheet::addFrame()
*/
void TimedFrame::setXSheet(XSheet *xsheet) {
    _xsheet = xsheet;
    setParent(_xsheet);
}


/*!
    Tries to get the TimedFrame that is in the XSheet before this one.
    Will return NULL if no XSheet is set.  If there is no TimeFrame before this
    one then NULL will be returned.
    
    Alternativly, if you consider the TimeFrame before this one to be at the end
    of the XSheet you can set \a loop to true.  This should ensure you always
    get a TimedFrame.  Though if there are no other TimedFrames in the XSheet,
    then this will return NULL (the frame before this one cannot be itself).

    \sa after()
    \sa setXSheet()
*/
TimedFrame *TimedFrame::before(bool loop) {
    // We need an XSheet
    if (!_xsheet)
        return NULL;

    // Seq num before
    int numBefore = _seqNum - 1;
    TimedFrame *tf = _xsheet->frameAtSeq(numBefore);

    // Could either be NULL or something
    if (!loop || tf)
        return tf;

    // Loop around
    // Only frame in the XSheet?  Just return NULL
    if (_xsheet->numFrames() == 1)
        return NULL;

    // Else there must be something there at the end, grab it
    return _xsheet->frames().last();
}


/*!
    Tries to get the TimedFrame that is in the XSheet after this one.
    Will return NULL if no XSheet is set.  If there is no TimeFrame after this
    one then NULL will be returned.
    
    Alternativly, if you consider the TimeFrame after this one to be at the end
    of the XSheet you can set \a loop to true.  This should ensure you always
    get a TimedFrame.  Though if there are no other TimedFrames in the XSheet,
    then this will return NULL (the frame after this one cannot be itself).

    \sa before()
    \sa setXSheet()
*/
TimedFrame *TimedFrame::after(bool loop) {
    // We need an XSheet
    if (!_xsheet)
        return NULL;

    // Seq num after
    int numAfter = _seqNum + _hold;
    TimedFrame *tf = _xsheet->frameAtSeq(numAfter);

    // Could either be NULL or something
    if (!loop || tf)
        return tf;

    // Loop around
    // Only frame in the XSheet?  Just return NULL
    if (_xsheet->numFrames() == 1)
        return NULL;

    // Else there must be something there at the end, grab it
    return _xsheet->frames().first();
}


/*!
    Set the staring sequence number (or frame number) for this TimedFrame.
    \a num must be a positive integer.  If it is not, then nothing will happen.

    Will emit the 'seqNumChanged()' signal upon success.
*/
void TimedFrame::setSeqNum(int num) {
    if (num < 1)
        qDebug() << "[TimedFrame setNum] frame=" << _frame << ", someone timed to set the number below 1";
    else {
        if (_seqNum != num) {
            _seqNum = num;
            emit seqNumChanged(_seqNum);
        }
    }
}


/*!
    Sets the hold vlaue for this TimedFrame. \a hold must be a positive integer
    or nothing will happen.  This function will most likely be used by the
    XSheet when a Frame has been added to it.

    Will emit the 'holdChanged()' signal upon success.
*/
void TimedFrame::setHold(int hold) {
    if (hold < 1)
        qDebug() << "[TimedFrame setHold] frame=" << _frame << ", someone tried to set the hold value below 1";
    else {
        if (_hold != hold) {
            _hold = hold;
            emit holdChanged(_hold);
        }
    }
}

