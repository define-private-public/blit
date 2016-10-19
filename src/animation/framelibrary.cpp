// File:         framelibrary.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the FrameLibrary.


/*!
    \inmodule Animation
    \class FrameLibrary
    \brief FrameLibrary is a module that is used to manage the reusable Frames and its resources.

    FrameLibrary is a module that manages the lifetime of Frame objects it might be best to think of
    this as a "god" module that oversees a few things.  You'll notice that this is very similar
    to the CelLibrary.
*/



#include "animation/framelibrary.h"
#include "animation/frame.h"
#include "animation/animation.h"
#include <QDebug>


/*!
    Instatiates the FrameLibrary.  Be sure to pass in
    BlitApp::app() as the \a parent.
*/
FrameLibrary::FrameLibrary(Animation *anim) :
    QObject(anim)
{
    qDebug() << "[FrameLibrary created]" << this;
}


/*!
    Destroys the FrameLibrary instance
*/
FrameLibrary::~FrameLibrary() {
    // Clear the library, say something if it was not good.
    if (!_clear())
        qDebug() << "[FrameLibrary error; issue removing some Frames upon FrameLibrary deletion]";

    qDebug() << "[FrameLibrary deleted]" << this;
}


/*!
    Adds a Frame to the library.  Should automatically be called upon Frame
    object creations.  Will Return true if sucessful, or false otherwise.
*/
bool FrameLibrary::addFrame(Frame *frame) {
    // -----------------------------------------------------
    // NOTE Most of this code was copied from Cel::addFrame()
    // -----------------------------------------------------
    // Add the Frame if the name isn't taken, but it's assumed via Frame::setName() that the name is unique
    if (nameTaken(frame->name()))
        return false;
    else {
        // Hookup Slots and signals
        connect(frame, &Frame::nameChanged, this, &FrameLibrary::_onFrameNameChanged);
        connect(frame, &Frame::destroyed, this, &FrameLibrary::_onFrameDestroyed);

        // Good to add
        qDebug() << "[FrameLibrary Frame added] " << frame->name();
        _frames.insert(frame->name(), frame);

        return true;
    }
}


/*!
    Removes a Frame from the library.  Returns true if the frame was
    successfully removed, false otherwise.
*/
bool FrameLibrary::removeFrame(Frame *frame) {
    // -----------------------------------------------------
    // NOTE Most of this code was copied from Cel::removeFrame()
    // -----------------------------------------------------
    if (_frames.contains(frame->name())) {
        // contains cel, disconnect signals/slots
        disconnect(frame, NULL, this, NULL);

        // Remove it
        qDebug() << "[FrameLibrary Frame removed] " << frame->name();
        return (_frames.remove(frame->name()) > 0);
    } else
        return false;
}


/*!
    Returns a count of the number of frames
*/
int FrameLibrary::numFrames() {
    return _frames.size();
}


/*!
    Checks to see if a Frame name is taken or not.
*/
bool FrameLibrary::nameTaken(QString name) {
    return _frames.contains(name);
}


/*!
    Returns a pointer to a Frame by \a name.  If \a name is not in the library,
    then this will return a NULL pointer.
*/
Frame *FrameLibrary::getFrame(QString name) {
    if (nameTaken(name))
        return _frames[name];
    else
        return NULL;
}


/*!
    Returns a list of all the Frames in the Library.
*/
QList<Frame *> FrameLibrary::frames() {
    return _frames.values();
}


/*!
    Called when a Frame emits is nameChanged() signal.  This wil update the
    internal data structure
*/
void FrameLibrary::_onFrameNameChanged(QString name) {
    // -----------------------------------------------------
    // NOTE Most of this code was copied from Cel::_onCelNameChanged()
    // -----------------------------------------------------

    // Simply remove the key then re-add it, Frame::setName() guarenttes that the
    // new name is unique.  Don't use add/remove
    Frame *frame = (Frame *)sender();
    _frames.remove(frame->oldName());
    _frames.insert(name, frame);

    // Info
    qDebug() << "[FrameLibrary Frame renamed]" << frame->oldName() << "->" << name;
}


/*!
    Called right before Frame is about to be deleted, this will just perform
    all of the remove stuff for us.
*/
void FrameLibrary::_onFrameDestroyed(QObject *obj) {
    // -----------------------------------------------------
    // NOTE Most of this code was copied from Cel::_onCelDestroyed()
    // -----------------------------------------------------

//    Frame *f = qobject_cast<Frame *>(sender());
    Frame *f = (Frame *)obj;
    if (f) {
        qDebug() << "[FrameLibrary Frame destroyed] " << f->name();
        removeFrame(f);
    }
}


/*!
    This function will nuke the entire library.  It won't deleted the Frames,
    just unhook them from the structure.

    It should only be called internally upon destruction, or by BlitApp
*/
bool FrameLibrary::_clear() {
    bool okay = true;
    
    // Have to do each Frame individually
    for (auto frame : _frames.values())
        okay &= removeFrame(frame);

    return okay;
}



