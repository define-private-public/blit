// File:         animation.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the Animation class


/*!
    \inmodule Animation
    \class Animation
    \brief The Animation class is container for the XSheet, frame size, and a few other metrics.

    Currently being worked with.  XSheet is meant for things like managing/organizing the 
    individual frames.  This one is for data like the name of the sequuence, its creation
    date, last edited date, and probably a buncyh of other things as well (in the future).
    
    An Animation object is considered Null if its containing XSheet is Null.  An XSheet
    pointer will always be present in the Animation object, but there is a chance that it
    would be Null.  To change the XSheet, just use setXSheet().
    
    The Frame Size is guarenteed to be at least 1x1
*/


#include "animation/animation.h"
#include "animation/cel.h"
#include "animation/xsheet.h"
#include "animation/cellibrary.h"
#include "animation/framelibrary.h"
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


/*!
    By default the Animation will be considered Null if a XSheet object isn't supplied.
*/
Animation::Animation(QString name, QSize size, QDateTime created, QDateTime updated, QObject *parent) :
    QObject(parent),
    _created(created),
    _updated(updated)
{
    // Set two extra parameters
    _xsheet = new XSheet(this);
    setName(name);
    setFrameSize(size);

    // If the updated time is Null, then set it to the current time
    if (_updated.isNull())
        _updated = _created;
    
    // Create the libraries
    _cl = new CelLibrary(this);
    _fl = new FrameLibrary(this);
}


/*!
    Deconstructor.
*/
Animation::~Animation() {
    // Have to manually delete these so there isn't a segfualt within the Cels
    delete _fl;
    delete _cl;
}


/*!
    The Animation is considered Empty if the XSheet is empty or not.  will return true
    if there is no XSheet.
*/
bool Animation::isEmpty() {
    return _xsheet->isEmpty();
}


/*!
    Internal slot if the Attached XSheet is destroyed before the Animation is destoryed.  This 
    funciton prevents a segfault in some cases where an XSheet's parent is set to some other
    QObject, but also set to this Animation.  In any normal case, the widget will delete the 
    XSheet first, but it won't clean up the _xsheet pointer.  this slot fixes that.
*/
void Animation::_onXSheetDestroyed(QObject *obj) {
    disconnect(_xsheet, 0, this, 0);
    _xsheet = NULL;
}


/*!
    Return a pointer to the currenly set XSheet.  May possibly be a Null XSheet.

    \sa setXSheet()
*/
QPointer<XSheet> Animation::xsheet() {
    return _xsheet;
}


/*!
    Sets the XSheet.  This function will always delect the current supplied XSheet and replace it
    with the one that is given.  If a NULL pointer is provided, then it will create a new Null
    XSheet and set that as the current XSheet.
    
    If supplied a pointer that is the same as the current pointer to the XSheet, nothing will happen.

    \sa xsheet()
    \sa XSheetChanged()
*/
void Animation::setXSheet(XSheet *xsheet) {
    if (_xsheet != xsheet) {
        // Make a new NULL XSheet
        if (xsheet == NULL)
            xsheet = new XSheet(this);

        // Safety check
        if (_xsheet) {
            disconnect(_xsheet, 0, this, 0);
            delete _xsheet;
        }

        // Set stuff
        _xsheet = xsheet;
        connect(_xsheet, &XSheet::destroyed, this, &Animation::_onXSheetDestroyed);
        emit XSheetChanged(_xsheet);
    }
}


/*!
    Returns the name that was given to the Animation.

    \sa setName()
*/
QString Animation::name() {
    return _name;
}


/*!
    Sets the name for the animation.  Can be an empty string if you don't want a name

    \sa name()
    \sa nameChanged()
*/
void Animation::setName(QString name) {
    if (_name != name) {
        _name = name;
        emit nameChanged(name);
    }
}


/*!
    Returns the dimensions of each Frame object should be.  The Frame object itself doesn't have
    any    members that dictate the dimensions, but it's nesscary for this.

    \sa frameWidth()
    \sa frameHeight()
    \sa setFrameSize()
    \sa setFrameWidth()
    \sa setFrameHeight()
*/
QSize Animation::frameSize() {
    return _size;
}


/*!
    Returns the width that each Frame should be in the Animation.

    \sa frameSize()
    \sa frameHeight()
    \sa setFrameSize()
    \sa setFrameWidth()
    \sa setFrameHeight()
*/
int Animation::frameWidth() {
    return _size.width();
}


/*!
    Returns the height that each frame should be, in the Animation.

    \sa frameSize()
    \sa frameWidth()
    \sa setFrameSize()
    \sa setFrameWidth()
    \sa setFrameHeight()
*/
int Animation::frameHeight() {
    return _size.height();
}


/*!
    Set the dimensions for the Animation's frame size.

    \sa frameSize()
    \sa frameWidth()
    \sa frameHeight()
    \sa setFrameWidth()
    \sa setFrameHeight()
    \sa frameSizeChanged()
*/
void Animation::setFrameSize(QSize size) {
    if (_size != size) {
        if ((size.width() < 1) || (size.height() < 1)) {
            qDebug() << "Error, cannot set a non-positive size; " << size;
            return;
        }

        _size = size;
        emit frameSizeChanged(size);
    }
}


/*!
    Sets the frame width (convienence function).

    \sa frameSize()
    \sa frameWidth()
    \sa frameHeight()
    \sa setFrameSize()
    \sa setFrameHeight()
    \sa frameSizeChanged()
*/
void Animation::setFrameWidth(int width) {
    setFrameSize(QSize(width, _size.height()));
}


/*!
    Sets the frame height (convienence function).

    \sa frameSize()
    \sa frameWidth()
    \sa frameHeight()
    \sa setFrameSize()
    \sa setFrameWidth()
    \sa frameSizeChanged()
*/
void Animation::setFrameHeight(int height) {
    setFrameSize(QSize(_size.width(), height));
}


/*!
    Retruns the creation date of the Animation as a QDateTime object.

    \sa createdTimestamp()
    \sa createdString()
*/
QDateTime Animation::created() {
    return _created;
}


/*!
    Sets the created Date.  Should only be used by FileOps
*/
void Animation::setCreated(QDateTime created) {
    _created = created;
}


/*!
    Returns a UNIX timestamp of the time/date that the Animation was created.

    \sa created()
    \sa createdString()
*/
quint32 Animation::createdTimestamp() {
    return _created.toTime_t();
}


/*!
    Returns the creation date as a String.

    \sa created()
    \sa createdTimestamp()
*/
QString Animation::createdString() {
    return _created.toString(ANIMATION_DATETIME_STRING_FORMAT);
}


/*!
    Retruns the QDateTime of when the Animation object was last updated.
    
    \sa updatedTimestamp()
    \sa updatedString()
    \sa update()
*/
QDateTime Animation::updated() {
    return _updated;
}


/*!
    Sets the updated Date.  Should only be used by FileOps
*/
void Animation::setUpdated(QDateTime updated) {
    _updated = updated;
}


/*!
    Returns a UNIX timestamp of when the Animation was last updated.  This could include something
    like a name change, a Cel edit, a frame move, etc.
    
    \sa updated()
    \sa updatedString()
    \sa update()
*/
quint32 Animation::updatedTimestamp() {
    return _updated.toTime_t();
}


/*!
    Returns the update date as a String.
    
    \sa updated()
    \sa updatedTimestamp()
    \sa update()
*/
QString Animation::updatedString() {
    return _updated.toString(ANIMATION_DATETIME_STRING_FORMAT);
}


/*!
    Tells the Animation to update the "last updated," variable.  This method should be called say
    if a Cel is painted or a Frame is moved inside of the XSheet.

    \sa updated()
    \sa updatedTimestamp()
    \sa updatedString()
*/
void Animation::update() {
    _updated = QDateTime::currentDateTime();
}


/*!
    Retrives a pointer to the contained CelLibrary.  Should not return a NULL
    pointer.
*/
QPointer<CelLibrary> Animation::cl() {
    return _cl;
}


/*!
    Retrives a pointer to the contained FrameLibrary.  Should not return a NULL
    pointer.
*/
QPointer<FrameLibrary> Animation::fl() {
    return _fl;
}


/*!
    Gets the set resource directory for the Animation
*/
QString Animation::resourceDir() {
    return _resourceDir;
}


/*!
    Sets the resource direcotry for all Cels that use file resources.  Will not
    Work if \a path is not a directory, or if it is the same as the currently
    set one.  \a path must also be an existing direcotry. If you want to copy 
    over all of the current Cel's resources to The new directory, set \copyOver
    to true (default is false).

    Will ensure that there is always a trailing slash at the end
*/
void Animation::setResourceDir(QString path) {
    // First check for new
    if (path != _resourceDir) {
        // First check if it's a dir
        QFileInfo fi(path);
        if (fi.isDir() && fi.exists()) {

            // Set the resource dir, but append on a trailing slash
            _resourceDir = path;
            if (!_resourceDir.endsWith(QDir::separator()))
                _resourceDir.append(QDir::separator());
            emit resourceDirChanged(_resourceDir);
//            qDebug() << "CelLibrary[MODIFIED]  resourceDir=" << _resourceDir << ", copyOver=" << copyOver;
        }
    }
}


/*!
    Will copy over all of the currently used file resources to a give path.
    If \a path is the same as the currently set resource dir nothing will
    happen.  \a path must also exist and be a directory
*/
void Animation::copyResourcesTo(QString path) {
    // Append on the trailing slash
    if (!path.endsWith(QDir::separator()))
        path.append(QDir::separator());

    // Check for existsance and path is not the same
    QFileInfo fi(path);
    if (fi.isDir() && fi.exists() && (_resourceDir != path)) {
        // Go through each Cel and save its file resources
        for (Cel *cel : _cl->_cels) {
            if (cel->hasFileResources()) {
                for (QString fr : cel->fileResources()) {
                    if (!QFile::copy(_resourceDir + fr, path + fr))
                        qDebug() << "Error, copying " << fr << " from " << _resourceDir << " to " << path;
                }
            }
        }
    } else {
        qDebug() << "Not copying Cel resources to " << path;
        qDebug() << "  Check if the destination is a directory, it exists, and it's not the same as the current resource dir.";
    }
}



