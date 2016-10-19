// File:         frame.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for the Frame class.


/*!
    \inmodule Animation
    \class Frame
    \brief Frame is a colleciton of CelRef objects.

    It is basically the place where you want a certains set of Cel/CelRefs to
    staged for a scene.  This doesn't contain any Timeline info; see FrameRef.

    Frames don't have any set dimensions, this is determined by the FrameSize
    variable (that is either contained in BlitApp or FrameLibrary).

    TODO update the below documentation one FrameRef is made
    ------------

    frame should hold out for, its frame number in the XSheet.  At minimum, all frames should hold out
    for at least 1 frame.  By default, a Frame is marked to be inactive and considered to be Null.  A 
    Frame doesn't know how many other Frames are ahead of it in the XSheet, only what sequence number it
    is and its hold value
    
    A Frame is considered Null if either its _num or _hold variables aren't positive integers.
    
    Import notes
    ------------
     - Every Cel should be the same size, not just for the Frame, but for the whole animation.
     - A "frame sequence number," or "seq num" is the number in the sequence of the animation
       where the frame is located.  If we have a Frame with the number '5' assign to it,
       with a hold of '3', numbers (or frames) '5, 6, 7,' all correspond to it.  Be sure to 
       check the XSheet class and its variable _seqMap for more of this.
    
    How layering works
    ------------------
    The variable _celRefs is just a list that holds Cel objects.  In the order that they appear,
    s how they are layered in the frame.
    E.g. _celRefs[0] is the top layer item, where as _celRefs[n] is the bottom most layer.
       
    The function _updateCelOrder() is used to adjust the order of the cels as they should
    appear in the QGraphicsScene.  If you wish to adjust a cel's order (add/delete/move), this
    function will be called once you manipulate the cel.  Basically, this function gets called 
    whenever the _celRefs variable is modified.
    
    For add/remove/move *Cel functions (including the cel() function), consider it bad
    practice to use negative indicies.  The static member BottomLayer uses this to denote the
    something in well, the bottom layer.  Also, consider it really bad practice to try to
    access an index that doesn't exists.  You'll get standard python errors for that one.
*/



#include "animation/frame.h"
#include "animation/framelibrary.h"
#include "animation/cel.h"
#include "animation/celref.h"
#include "animation/timedframe.h"
#include "animation/frameitem.h"
#include "animation/animation.h"
#include "util.h"
#include <QPainter>
#include <QDebug>

    

/*!
    Creates the Frame object and will store it in the FrameLibrary.  \a name
    should be a unique value.  If it is not, a random mini-UUID will be assigned
    to it.  If you want a random name, feed it an empty string.
*/
Frame::Frame(Animation *anim, QString name) :
    QObject(anim->fl()),
    _anim(anim)
{
    // Try using the program given name, if not, then the self given name
    if (!setName(name))
        setName("");

    _anim->fl()->addFrame(this);

    qDebug() << "[Frame created] name=" << _name;
}


/*!
    Destroys and cleans up a Frame object
*/
Frame::~Frame() {
    // Deactivate first
    deactivate();

    qDebug() << "[Frame deleted] name=" << _name;
}


/*!
    Creates a deep copy of the Frame.  Resulting Frame will not be active
    Contained Cels will also be copied.

    \sa Frame()
*/
Frame *Frame::copy(QString name) {
    // Check for copied name, strip the UUID postfix
    if ((name == _name) && _usingUUIDPostfix) {
        name = _name;
        name.chop(FRAME_UUID_POSTFIX_SIZE + 1);
    }

    // Make the Frame
    Frame *f = new Frame(_anim, name);

    // Perform a deep copy
    for (auto iter = (_celRefs.end() - 1); iter != (_celRefs.begin() - 1); iter--) {
        CelRef *cr = new CelRef((*iter)->cel()->copy());
        cr->setPos((*iter)->pos());
        f->addCel(cr);
    }

    // Hand it out
    return f;
}


/*!
    Returns the unique name given to this Frame object
*/
QString Frame::name() {
    return _name;
}


/*!
    If the name of the Frame was changed, this funciton will return that name.
    Might possibly be an empty string if the name was never changed.
*/
QString Frame::oldName() {
    return _oldName;
}


/*!
    Used to set the unique \a name for the Frame.  If \a name is given an emtpy
    string, the name will randomly be chosen with a UUID.  If \a name is already
    taken, it will append a random 8 character substring to the name.

    If \a name is already set to the Frames's name, this will just ignore the
    operation and return false.

    Returns true on success, false on failure.
*/
bool Frame::setName(QString name) {
    // -----------------------------------------------------
    // NOTE Most of this code was copied from Cel::setName()
    // -----------------------------------------------------
    FrameLibrary *fl = _anim->fl();

    // Treat "frame" and "frame-" and empty ones
    if ((name == "frame") || (name == "frame-"))
        name = "";

    if ((_name == name) && !name.isEmpty()) {
        // Same name, ignore 
        return false;
    } else if (name.isEmpty()) {
        // Choose a random UUID, stripped down to 8 characters

        // Keep looping until we have a uniq name
        while (name.isEmpty()) {
            name = util::mkUUIDStr(FRAME_UUID_POSTFIX_SIZE);
            name.prepend("frame-");

            if (fl->nameTaken(name))
                name = "";
        }

        // Set some flags
        _usingRandomName = true;
        _usingUUIDPostfix = true;

    } else if (fl->nameTaken(name)) {
        // Add on a random 8 character postfix because the name already exists

        // Keep looping until we have a unique name
        QString postfix = "";
        while (postfix.isEmpty()) {
            postfix = "-" + util::mkUUIDStr(FRAME_UUID_POSTFIX_SIZE);

            if (fl->nameTaken(name + postfix))
                postfix = "";
        }

        // Got it, now use it
        name += postfix;
        _usingRandomName = false;
        _usingUUIDPostfix = true;
    } else {
        // Name must be unique, set some flags
        _usingRandomName = false;
        _usingUUIDPostfix = false;
    }

    // Go ahead, Set the name, and via the signal, notify the FrameLibrary to update
    _oldName = _name;
    _name = name;
    emit nameChanged(_name);
    return true;
}


/*!
    Returns true if this Frame is using a randomly given name.
*/
bool Frame::usingRandomName() {
    return _usingRandomName;
}


/*!
    Returns true if this Frame has a UUID postfix, but not necessarly randomly generated
*/
bool Frame::usingUUIDPostfix() {
    return _usingUUIDPostfix;
}


/*!
    Adds a Cel to the Frame.  By default, the Cel will be but on the top most layer.
    
    \sa removeCel()
*/
void Frame::addCel(CelRef *ref, int at) {
    // Add it to the list
    at = (at == FRAME_BOTTOM_LAYER) ? numCels() : at;    // Bottom layer or somewhere else?
    _celRefs.insert(at, ref);

    // Old QGraphicsScene based Frame code
//    // Then add it to the scene
    _updateCelRefOrder(at);
    emit celAdded(ref);
    ref->setFrame(this);                                // Lin with the CelRef

    // Add signals
    connect(ref, &CelRef::positionChanged, this, &Frame::_onCelRefPositionChanged);

    // If the frame is active, then activate the Cel as well
    if (_active)
        ref->cel()->activate();
}


/*!
    Removes the CelR from the Frame, will also return a pointer to it.  By
    default, this function will delete the Cel at the top.  Will Not delete it.

    \sa addCel()
*/
CelRef *Frame::removeCel(int at) {
    // Remove it from the list
    at = (at == FRAME_BOTTOM_LAYER) ? (numCels() - 1) : at;    // Bottom layer or somewhere else?
    CelRef *ref = _celRefs.takeAt(at);

    // Remove from the scene and update the order
    _updateCelRefOrder(at);
    emit celRemoved(ref);
    ref->setFrame(NULL);                                    // Unlink with the CelRef

    // Removed signals
    disconnect(ref, 0, this, 0);

    return ref;
}


/*!
    Will take the cel at "at," and move it to the index "to".
*/
void Frame::moveCel(int at, int to) {
    at = (at == FRAME_BOTTOM_LAYER) ? (numCels() - 1) : at;
    to = (to == FRAME_BOTTOM_LAYER) ? (numCels() - 1) : to;

    _celRefs.move(at, to);
    _updateCelRefOrder(qMin(at, to));
    emit celMoved(_celRefs[to]);
}


/*!
    Retruns the QList that contains all of the pointers to the CelRefs this Frame has.
    The Cels will be in the order where "[0] ... [n]" is "top to bottom."
*/
QList<CelRef *> Frame::cels() {
    return _celRefs;
}


/*!
    Retrives the pointer to a Cel at a certain index.  Expect an error if the index is out of bounds.

    \sa moveCel()
*/
CelRef *Frame::cel(int at) {
    return _celRefs[at];
}


/*!
    Returns the number of CelRefs contained in the Frame.
*/
int Frame::numCels() {
    return _celRefs.size();
}

/*!
    Trys to save all of the Frame's Cel's files.  This function will only do anything
    if the Frame is marked active.
*/
void Frame::saveCelFiles() {
    for (auto iter = _celRefs.begin(); iter != _celRefs.end(); iter++)
        (*iter)->cel()->save();
}


/*!
    For all of the Cels that contain file resources, this will mark all of those
    Cels to remove their file resources upon deletion.
*/
void Frame::removeCelFiles() {
    // Remove all of the Cels with file resources (The Cel implementation should handle that themselves)
    for (auto iter = _celRefs.begin(); iter != _celRefs.end(); iter++)
        (*iter)->cel()->remove();
}


/*!
    Registers a TimedFrame with this Frame.  \tf should not be NULL
*/
void Frame::registerTimedFrame(TimedFrame *tf) {
    if (tf)
        _timedFrames.append(QPointer<TimedFrame>(tf));
}


/*!
    De-registers a TimedFrame with this Frame.  \tf should not be NULL
*/
void Frame::unregisterTimedFrame(TimedFrame *tf) {
    if (tf)
        _timedFrames.removeOne(QPointer<TimedFrame>(tf));
}


/*!
    Returns a list of pointers to Timed Frames that this Frame instance is
    attached to.  May return an empty list.
*/
QList<QPointer<TimedFrame>> Frame::timedFrames() {
    return _timedFrames;
}


/*!
    Returns a render of the Frame.  If the frame has no Cels, just a transparent
    image will be returned of the current frame size.  If no _anim is set, then
    it will render a Null QImage.
*/
QImage Frame::render() {
    QImage img = util::mkBlankImage(_anim->frameSize());

    if (numCels() == 0)
        return img;
    else {
        // Paint all of the Cels
        QPainter p(&img);

        // Need to paint them in the reverse order of how they appear un the list
        for (auto iter = (_celRefs.end() - 1); iter != (_celRefs.begin() - 1); iter--) {
            CelRef *cr = *iter;
            p.drawImage(cr->pos(), cr->cel()->image());
        }
    }

    return img;
}


/*!
    Returns the current frameSize set in the Animation object this is connected to.

    \sa Animation::frameSize()
*/
QSize Frame::frameSize() {
    return _anim->frameSize();
}


/*!
    Small little utility function to return a FrameItem class with this Frame.
*/
FrameItem *Frame::mkItem() {
    return new FrameItem(this);
}


/*!
    Returns true of the Frame is marked to be active
*/
bool Frame::active() {
    return _active;
}


/*!
    Will turn the Frame to be active, as well as its contained Cels.
    Will do nothing if the Frame is already active.  Will emit the 
    activated() signal upon success.
*/
void Frame::activate() {
    if (!_active) {
        // Make all of the Cels active
        for (auto iter = _celRefs.begin(); iter != _celRefs.end(); iter++)
            (*iter)->cel()->activate();

        // mark and emit
        _active = true;
        emit activated();
    }
}


/*!
    Will turn the Frame to be inactive, as well as its contained Cels.
    Will do nothing if the Frame is already inactive.  Will emit the 
    deactivated() signal upon success.
*/
void Frame::deactivate() {
    if (_active) {
        // Make all of the cels inactive
        for (auto iter = _celRefs.begin(); iter != _celRefs.end(); iter++) {
            Cel *cel = (*iter)->cel();
            if (cel)
                cel->deactivate();
        }

        // Mark and emit
        _active = false;
        emit deactivated();
    }
}


/*!
    This slot is tripped when a contained CelRef emits CelRef::positionChanged()
    signal.  This will simply pass up that signal by emitting the celRefPositionChanged()
    signal
*/
void Frame::_onCelRefPositionChanged(QPointF pos) {
    emit celRefPositionChanged((CelRef *)sender());
}


/*!
    Internal function to adjust the Z Values of CelRefs when a Cel is added,
    removed, or moved withing the Frame.  \a startingIndex is the locatio to
    start updating, till it goes to the end of the list of CelRefs.  This 
    function will do nothing if \a startingIndex is greather than the
    current number of Cels.  Negative numbers will become 0.  Default value
    is 0.
*/
void Frame::_updateCelRefOrder(int startingIndex) {
    // TODO this function isn't working as I want it to, so the startingindex is alwasy ignored and set to
    //      0.  I want this optimization though, so on a later date, come in and fix it.
    
    // Sanity check #1
//    if (startingIndex < 0)
        startingIndex = 0;

    // Sanity check #2
    int z = numCels();
    if (startingIndex >= z)
        return;

    // Go through the refs, top-most should have higher Z value
    for (auto iter = (_celRefs.begin() + startingIndex); iter != _celRefs.end(); iter++) {
        (*iter)->setZValue(z--);
    }
}


