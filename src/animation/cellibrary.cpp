// File:         cellibrary.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the CelLibrary.


/*!
    \inmodule Animation
    \class CelLibrary
    \brief CelLibrary is a module that is used to manage the reusable Cels and its resources.

    CelLibrary is a module that manages the lifetime of Cel objects it might be best to think of
    this as a "god" module that oversees a few things.  Much of the older Cel functionality was
    moved over into here.  It seems a little rediculous, but it adds a lot more flexibility for
    the whole resuable Cel system.
*/



#include "animation/cellibrary.h"
#include "animation/cel.h"
#include "animation/animation.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>


/*!
    Instaties the CelLbirary.  It really doesn't do much right now...

    An Animation object must be passed in for \anim as it is the parent.
*/
CelLibrary::CelLibrary(Animation *anim) :
    QObject(anim)
{
    qDebug() << "[CelLibrary created]" << this;

}


/*!
    Will remove any leftover resources upon deletion
*/
CelLibrary::~CelLibrary() {
    // Clear the library, say something if it was not good.
    if (!_clear())
        qDebug() << "[CelLibrary error; issue removing some Cels upon CelLibrary deletion]";

    qDebug() << "[CelLibrary deleted]" << this;
}


/*!
    Will try to add the \a cel pointer to the CelLibrary.  The name set in the
    Cel must not share a name with any other Cels in the CelLibrary.  If there
    is a shared name, then this will not add the Cel.

    Returns true if Cel added successfully, false if not.

    \sa removeCel()
    \sa nameTaken()
*/
bool CelLibrary::addCel(Cel *cel) {
    // Add the Cel if the name isn't taken, but it's assumed via Cel::setName() that the name is unique
    if (nameTaken(cel->name()))
        return false;
    else {
        // Hookup Slots and signals
        connect(cel, &Cel::nameChanged, this, &CelLibrary::_onCelNameChanged);
        connect(cel, &Cel::destroyed, this, &CelLibrary::_onCelDestroyed);

        // Good to add
        qDebug() << "[CelLibrary Cel added] " << cel->name();
        _cels.insert(cel->name(), cel);

        return true;
    }
}


/*!
    Will remove A Cel From the library.  This is normally called when a Cel is
    destroyed.  But you can rmeove a Cel from the library before its life is
    over.  If the CelLibrary doesn't contain the Cel, it will do nothing and
    return false.

    Returns true on success, false otherwise

    \sa addCel()
*/
bool CelLibrary::removeCel(Cel *cel) {
    if (_cels.contains(cel->name())) {
        // contains cel, disconnect signals/slots
        disconnect(cel, NULL, this, NULL);

        // Remove it
        qDebug() << "[CelLibrary Cel removed] " << cel->name();
        return (_cels.remove(cel->name()) > 0);
    } else
        return false;
}


/*!
    Returns the number of Cels contained in the library.
*/
int CelLibrary::numCels() {
    return _cels.size();
}


/*!
    Checks to see if the current \a name is already taken by another Cel.

    Returns true if so, false otherwise
*/
bool CelLibrary::nameTaken(QString name) {
    // Check the hash table's keys
    return _cels.contains(name);    
}


/*!
    Returns a pointer to a Cel by \a name.  If \a name is not in the library,
    then this will return a NULL pointer.
*/
Cel *CelLibrary::getCel(QString name) {
    if (nameTaken(name))
        return _cels[name];
    else
        return NULL;
}


/*!
    Returns a list of all the Cels in the Library.
*/
QList<Cel *> CelLibrary::cels() {
    return _cels.values();
}


/*!
    If a Cel Changes its name, this will update the internal hash.  Picked up
    by Cel::nameChanged().
*/
void CelLibrary::_onCelNameChanged(QString name) {
    // Simply remove the key then readd it, Cel::setName() guarenttes that the
    // new name is unique.  Don't use add/remove
    Cel *cel = (Cel *)sender();
    _cels.remove(cel->oldName());
    _cels.insert(name, cel);

    // Info
    qDebug() << "[CelLibrary Cel renamed]" << cel->oldName() << "->" << name;
}


/*!
    When the Cel is going to be destroyed, this will catch the signal and do things
*/
void CelLibrary::_onCelDestroyed(QObject *obj) {
//    Cel *c = qobject_cast<Cel *>(sender());
    Cel *c = (Cel *)obj;
    if (c) {
        qDebug() << "[CelLibrary Cel destroyed] " << c->name();
        removeCel(c);
    }
}


/*!
    Will clear out the entire CelLibrary.  This function should only be called
    internally or by BlitApp::load().  Will not delete the library object.

    Returns true if the clearing had no hicups.
*/
bool CelLibrary::_clear() {
    bool okay = true;
    
    // Have to do each Cel individually
    for (auto cel : _cels.values())
        okay &= removeCel(cel);

    return okay;
}

