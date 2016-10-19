// File:         celref.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for the CelRef class.


/*!
    \inmodule Animation
    \class CelRef
    \brief CelRef is a class for containing a Cel to be used in a Frame.

    It has an underlying QPointer to the Cel object (as long as the Cel is still in
    existance).  If there is no underlying Cel, then it won't do anything.

    It also must have an underlying CelRefItem pointer.  If the pointer to the
    CelRelRefItem is already gone (via scene deltion) it should just create
    a new one.

    It's recommened that You don't have one CelRef instance in two Frames.
*/


#include "animation/celref.h"
#include "animation/cel.h"
#include "animation/celrefitem.h"
#include "animation/frame.h"
#include <QDebug>


/*!
    Constructor for the CelRef.  Requires a pointer to a Cel object.  If given
    a NULL pointer, nothing will happen; though you should never have to do this.
*/
CelRef::CelRef(Cel *cel) :
    QObject(NULL),
    _cel(cel)
{
    if (_cel) {
        // We've got something, tell the Cel That it's not refernce by this
        _cel->registerRef(this);
    }

    qDebug() << "[CelRef created] for" << _cel;
}


/*!
    Conveience constructor for the CelRef.  Let's whoever uses this function
    create a CelRef for \a cel along with a default \a x and \a y paramters.
*/
CelRef::CelRef(Cel *cel, qreal x, qreal y) :
    CelRef(cel)
{
    setPos(x, y);
}


/*!
    Deconstructor for the CelRef.

    If there is a set Cel, it will disconnect itself from the underlying
    Cel.
*/
CelRef::~CelRef() {
    if (_cel) {
        // Disconnect the Cel from knowing this is a reference to it
        _cel->unregisterRef(this);
    }

    // Clear out any CelRefItems now deleted 
    for (auto iter = _cris.begin(); iter != _cris.end(); iter++) {
        CelRefItem *cri = *iter;
        disconnect(cri, 0, this, 0);
        delete cri;
    }
    
    qDebug() << "[CelRef deleted] for" << _cel;
}

/*!
    Creats a copy of the CelRef.  The position data will be preserved,
    but the CelRef will be pointing to the same Cel.  This is not a deep
    copy.  It will not have a host Frame.
*/
CelRef *CelRef::copy() {
    CelRef *cr = new CelRef(_cel);
    cr->setPos(pos());
    cr->setFrame(_frame);
    return cr;
}


/*!
    Returns the QPointer to the Cel.  It is possible that it may be NULL.
*/
QPointer<Cel> CelRef::cel() {
    return _cel;
}


/*!
    Checks to see if there is a Cel set to the CelRef.

    Return true if so, false otherwise.
*/
bool CelRef::hasCel() {
    return !_cel.isNull();
}


/*!
    Returns true if the CelRef is marked to show the Info about the Cel on the Canvas
*/
bool CelRef::showCelInfo() {
    return _showCelInfo;
}


/*!
    This will set the Frame for the CelRef.  It si recommened that you (the programmer)
    do not touch this function unless you know what you are doing.  So far it is only
    called in Frame::addCel() and Frame::removeCel().  Wil also set the parent of
    the CelRef to the frame.

    You can also pass in NULL fo \a frame if you want to unlink it.
*/
void CelRef::setFrame(Frame *frame) {
    _frame = frame;
    setParent(frame);
}


/*!
    Returns a pointer to the Frame object that this CelRef is used in.
    May return a NULL pointer
*/
QPointer<Frame> CelRef::frame() {
    return _frame;
}


/*!
    Returns true of the CelRef is attached to a frame, false otherwise
*/
bool CelRef::hasFrame() {
    return !_frame.isNull();
}




/*!
    Acts as a factory method for the CelRefItem class.  This function should be
    used to obtain a CelRefItem for a CelRef object.  Don't instatiate RefItems
    yourself.  

    Returns a pointer to the CelRefItem for this CelRef
*/
QPointer<CelRefItem> CelRef::mkItem() {
    // first check to see there is a RefItem there or not
    CelRefItem *cri = new CelRefItem(this);
    connect(cri, &CelRefItem::destroyed, this, &CelRef::_onCelRefItemDestroyed);
    _cris.insert(cri);
    
    return cri;
}


/*!
    Informs the CelRefItem to update (possibly repaint).  \a rect is the area
    of the CelRef that will be updated.  You can pass in nothing to update the
    whole area.
*/
void CelRef::update(const QRectF &rect) {
    for (auto iter = _cris.begin(); iter != _cris.end(); iter++)
        (*iter)->update();
}


/*!
    Informs the CelRefItem to update (possibly repaint).

    This is an overloaded convienence function.
*/
void CelRef::update(qreal x, qreal y, qreal width, qreal height) {
    update(QRectF(x, y, width, height));
}


/*!
    Returns the current position of the CelRef.  Should be on an integer boundary

    \sa x()
    \sa y()
    \sa setPos()
*/
QPointF CelRef::pos() const {
    return _pos;
}


/*!
    Returns the X position of the CelRef.

    \sa pos()
    \sa y()
    \sa setX()
*/
qreal CelRef::x() const {
    return _pos.x();
}


/*!
    Returns the Y postion of the CelRef.

    \sa pos()
    \sa x()
    \sa setY()
*/
qreal CelRef::y() const {
    return _pos.y();
}


/*!
    Returns the Z-Value of the CelRef.  Is used as the layer value.

    \sa setZValue()
*/
qreal CelRef::zValue() const {
    return _zValue;
}


/*!
    Sets the position of the CelRef.  Will cause the related RefItem to update.
    Nothing will hapen if \a pos is the same as the current position.  Will 
    emit the positionChanged() signal upon success.

    \sa pos()
    \sa setX()
    \sa setY()
*/
void CelRef::setPos(const QPointF &pos) {
    // Do nothing if the position is the same.
    if (_pos == pos)
        return;
    
    _pos = pos;
    emit positionChanged(_pos);
}


/*!
    Sets the position of the CelRef.  Will cause the related RefItem to update.
    Emits positionChanged() signal upon success.

    This is an overloaded function.

    \sa pos()
    \sa setX()
    \sa setY()
*/
void CelRef::setPos(qreal x, qreal y) {
    setPos(QPointF(x, y));
}


/*!
    Sets the X position of the CelRef.  Will cause the related RefItem to update.
    Emits positionChanged() signal upon success.

    \sa x()
    \sa setY()
*/
void CelRef::setX(qreal x) {
    setPos(x, y());
}


/*!
    Sets the Y position of the CelRef.  Will cause the related RefItem to update.
    Emits positionChanged() signal upon success.

    \sa setX()
    \sa y()
*/
void CelRef::setY(qreal y) {
    setPos(x(), y);
}
    

/*!
    Moves the CelRef over by \a x pixels left and \y pixels down.  Emits the 
    positionChanged() signal upon success.

    \sa moveX()
    \sa moveY()
*/
void CelRef::move(qreal x, qreal y) {
    setPos(this->x() + x, this->y() + y);
}


/*!
    Moves the CelRef over by \a x pixels left.  Emits the positionChanged()
    signal upon success.

    \sa move()
    \sa moveY()
*/
void CelRef::moveX(qreal x) {
    move(x, 0);
}


/*!
    Moves the CelRef over by  \y pixels down.  Emits the positionChanged()
    signal upon success.

    \sa move()
    \sa moveX()
*/
void CelRef::moveY(qreal y) {
    move(0, y);
}


/*!
    Sets the Z-Value (layering flag) of the CelRef.  Lower is on the bottom, higher
    is on the top.  Will emit the zValueChanged() signal upon success.

    \sa z()
*/
void CelRef::setZValue(qreal z) {
    if (_zValue != z) {
        _zValue = z;
        emit zValueChanged(_zValue);
    }
}


/*!
    Toggle showing the Cel Info (name & outline) for this Cel).  \a showIt
    by default is true.  Will notify any CelRefItems of the change
*/
void CelRef::setShowCelInfo(bool showIt) {
    if (_showCelInfo != showIt) {
        _showCelInfo = showIt;
        emit showCelInfoChanged(_showCelInfo);
        qDebug() << "[CelRef setShowCelInfo] ref=" << this << ", showIt=" << showIt;
    }
}


/*!
    When a CelRefItem is deleted, this will remove it from the internal Set.

    Called by CelRefItem::destoryed()
*/
void CelRef::_onCelRefItemDestroyed(QObject *obj) {
    qDebug() << "[CelRef _onCelRefItemDestroyed]" << obj;
    _cris.remove((CelRefItem *)sender());
}
