// File:         cel.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Cel class


/*!
    \inmodule Animation
    \class Cel
    \brief Cel is the main object for representing image data in Blit.

    The Cel object is a container which represents one image.  It is a subclass of QObject.
    The    image the Cel contains is not always active, it will usually just stay on disc until
    "activated."  A Bunch of these are packaged in a Frame object.  All Cels by default have a size
    of 1x1.  Images are currently represented as PNGs (which is instantited by PNGCel), but there are
    plans in include Cels like "Live Camera"

    Each Cel has a uniuqe name to it.  If one is not provided, a UUID will be supplied.  This name can
    be changed.

    By default all Cels of the Base class are considered to be empty (as they contain no data)
*/


#include "animation/cel.h"
#include "animation/animation.h"
#include "animation/cellibrary.h"
#include "animation/celref.h"
#include "util.h"
#include <QStringList>
#include <QImage>
#include <QPainter>
#include <QDebug>




/*!
    Creates the Cel object.  \a name must be unique, if it is not, a randomly
    selected UUID (of only 8 characters) will be given to it.  If \a name is 
    supplied an empty string, then it will also give it a random UUID.  \a size
    is the size of the cel, it will always be at least 1x1, if the supplied
    value is less than this, the width and/or height will be adjusted.
    \a parent is the Cel's parent object; this is usually NULL.
*/
Cel::Cel(Animation *anim, QString name, QSize size) :
    QObject(anim->cl()),
    _anim(anim)
{
    // First Try using the Cel's given name, then give is a random one if that doesn't work
    if (!setName(name))
        setName("");
    
    // Supply a size
    resize(size);

    // Add the Cel to the CelLibrary
    _anim->cl()->addCel(this);

    // Debug info
    qDebug() << "[Cel created] name=" << _name << ", size=" << _size;
}


/*!
    Will create a deep copy of the Cel object.  The copied Cel object will have
    The same parent as this Cel, same type, same size.  It should not have any
    CelRefs pointing to it.  It will be set to have the same parent object as
    this Cel.  It will be in an inactive state.

    Just like any other Cel, the name needs to be unique.

    \sa setName()
*/
Cel *Cel::copy(QString name) {
    qDebug() << "[Cel copy]";
    
    // Check for copied name, strip the UUID postfix
    if ((name == _name) && _usingUUIDPostfix) {
        name = _name;
        name.chop(CEL_UUID_POSTFIX_SIZE + 1);
    }

    return new Cel(_anim, name, _size);
}


/*!
    Will deactivate the Cel, and remove it's name from the set of used names.
*/
Cel::~Cel() {
    // Must deactivate first (if active)
    deactivate();

    qDebug() << "[Cel deleted] name=" << _name << ", size=" << _size;
}


/*!
    Cels that are of the Cel base class are alwasys considered to be empty. This
    means that they contain no image information.
*/
bool Cel::isEmpty() {
    return true;
}


/*!
    Used to tell if the Cel is using File resources or not.  By default, Cel does
    not, but things like PNGCel are.  Be sure to reimplement this in sublcasses.

    Returns false

    \sa fileResources()
*/
bool Cel::hasFileResources() {
    return false;
}


/*!
    If the Cel has file resources, this is a list of strings of what those files
    are.  The files should all be in the root of the currently set resource
    directory in the CelLibrary.  The Cel base class returns an emtpy String
    List.  Subclasses like PNGCel have one file resource.

    \sa hasFileResources()
*/
QStringList Cel::fileResources() {
    return QStringList();
}


/*!
    Reimplement this function to save the file resources to the disk.  Where
    \a basename is the name of the file w/out an extension or folder.  That
    should be handled by the child class.  By default \a basename is empty.
    Passing in empty means to save the file resources under the Cel's name()
    value.

    This base class doesn't do anything
*/
void Cel::save(QString basename) {
    // Absolutly nothing...
}


/*!
    Returns the type of the Cel, e.g. if it is a BASE type, or PNG type.
*/
int Cel::type() {
    return Type;
}


/*!
    Returns the unique name given to the Cel.  Should not be an empty string.
*/
QString Cel::name() {
    return _name;
}


/*!
    Returns the previous name this Cel has used.  May be an empty string.
*/
QString Cel::oldName() {
    return _oldName;
}


/*!
    Used to set the unique \a name for the Cel.  If \a name is given an emtpy
    string, the name will randomly be chosen with a UUID.  If \a name is already
    taken, it will append a random 8 character substring to the name.

    If \a name is already set to the Cel's name, this will just ignore the
    operation and return false.

    Returns true on success, false on failure.
*/
bool Cel::setName(QString name) {
    CelLibrary *cl = _anim->cl();

    // Treat "cel" and "cel-" and empty ones
    if ((name == "cel") || (name == "cel-"))
        name = "";

    if ((_name == name) && !name.isEmpty()) {
        // Same name, ignore 
        return false;
    } else if (name.isEmpty()) {
        // Choose a random UUID, stripped down to 8 characters

        // Keep looping until we have a uniq name
        while (name.isEmpty()) {
            name = util::mkUUIDStr(CEL_UUID_POSTFIX_SIZE);
            name.prepend("cel-");

            if (cl->nameTaken(name))
                name = "";
        }

        // Set some flags
        _usingRandomName = true;
        _usingUUIDPostfix = true;

    } else if (cl->nameTaken(name)) {
        // Add on a random 8 character postfix because the name already exists

        // Keep looping until we have a unique name
        QString postfix = "";
        while (postfix.isEmpty()) {
            postfix = "-" + util::mkUUIDStr(CEL_UUID_POSTFIX_SIZE);

            if (cl->nameTaken(name + postfix))
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

    // Go ahead, Set the name, and via the signal, notify the CelLibrary to update
    _oldName = _name;
    _name = name;
    emit nameChanged(_name);
    return true;
}


/*!
    This function is used to create a render of the Cel.  For this base class,
    it will just return a blank image of the Cel's size.
*/
QImage Cel::image() {
    return util::mkBlankImage(_size);
}


/*!
    Used for marking file resources for deletion.  Sublcasses (like PNGCel) that
    use file resources should implement this function.  File resources should
    only be deleted when the object is destroyed.

    In this case it does nothing.

    \sa toBeRemoved()
*/
void Cel::remove(bool deleteFiles) {
    // do nothing
}


/*!
    Used for cleaning up file resources, this base class function will always
    return false.

    \sa remove()
*/
bool Cel::toBeRemoved() {
    return false;
}

/*!
    Returns the size of the current Cel. Will be 1x1 or greater.

    \sa width()
    \sa height()
    \sa reisze()
*/
QSize Cel::size() {
    return _size;
}


/*!
    Returns the width of the current Cel. Will be a positive integer.

    \sa size()
    \sa height()
    \sa reisze()
*/
int Cel::width() {
    return _size.width();
}


/*!
    Returns the height of the current Cel. Will be a positive integer.

    \sa size()
    \sa width()
    \sa reisze()
*/
int Cel::height() {
    return _size.height();
}


/*!
    Sets the size of the Cel to be the \a width and \a height supplied.  Both
    parameters must be positive integers; they will be set to 1 if they are
    not.  Will emit the resizied() signal if resized successfully.

    \sa size()
    \sa width()
    \sa height()
*/
void Cel::resize(int width, int height) {
    // Adjust sizes
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    QSize newSize(width, height);

    // Check for different size
    if (_size != newSize) {
        _size = newSize;
        emit resized(_size);
    }
}


/*!
    Overloaded method, for convinence.  Will resize the Cel if provided value
    is different from the current size.

    \sa resize()
*/
void Cel::resize(QSize size) {
    resize(size.width(), size.height());
}


/*!
    If the name given to this Cel was generated by random, this will return true

    \sa setName()
    \sa usingUUIDPostfix()
*/
bool Cel::usingRandomName() {
    return _usingRandomName;
}


/*!
    Cel names must be unique.  They can be randomly generated.  Or if the name
    is already taken, it will append a postfix mini-uuid to the name.  This will
    return true for those two cases.

    \sa setName()
    \sa usingRandomName()
*/
bool Cel::usingUUIDPostfix() {
    return _usingUUIDPostfix;
}


/*!
    Used during a paint method to draw presentation data about the Cel.  Sublcasses
    Should always call this method in their paint() overload.
*/
void Cel::paint(QPainter *painter) {
    // Much ado about nothing...
}


/*!
    Registers a CelRef into this Cel's list of Refs. \a ref must be non NULL.

    \sa unregisterRef()
*/
void Cel::registerRef(CelRef *ref) {
    if (ref)
        _celRefs.append(QPointer<CelRef>(ref));
}


/*!
    Unregisters a CelRef from this Cel's list of Refs. \a ref must be non NULL.

    \sa registerRef()
*/
void Cel::unregisterRef(CelRef *ref) {
    if (ref)
        _celRefs.removeOne(QPointer<CelRef>(ref));
}


/*!
    Returns a list of QPointers to the CelRefs that use this Cel.  Under most
    cases this should only have one element, but chances there may be many.

    Please do not modify the data unless you know what you're doing.
*/
QList<QPointer<CelRef>> Cel::celRefs() {
    return _celRefs;
}


/*!
    Checks to see if the Cel is marked active or not.  returns true if so, false
    otherwise.

    /sa activate()
    /sa deactivate()
*/
bool Cel::active() {
    return _active;
}


/*!
    Sets the Cel to be active.  Will emit the activated signal if it already
    isn't active.
    
    \sa active()
    \sa deactivate()
*/
void Cel::activate() {
    if (!_active) {
        _active = true;
        emit activated();
    }
}


/*!
    Sets to Cel to be deactiavted,  Will emit the deactivated signal if the Cel
    is active.
    
    \sa active()
    \sa activate()
*/
void Cel::deactivate() {
    if (_active) {
        _active = false;
        emit deactivated();
    }
}


