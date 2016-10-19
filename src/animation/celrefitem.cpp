// File:         celrefitem.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for the CelRefItem class.


#include "animation/celrefitem.h"
#include "animation/cel.h"
#include "animation/celref.h"
#include "widgets/drawing/canvas.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QDebug>


/*!
    \inmodule Animation
    \class CelRefItem
    \brief CelRefItem is a class for containing a CelRef to be used in a QGraphicsScene.

    This object should not be instatiated directly, but rather use a factory method from
    CelRef to get the CelRefItem (CelRef::mkItem()).

    It has an underlying QPointer to the CelRef object (as long as the CelRef is still in
    existance).  If there is no underlying CelRef, then it won't do anything.
*/


/*!
    Constructs the RefItem for the Cel.  \a ref should be a non-NULL pointer to 
    a CelRef object.  If you do give it a NULL pointer, the CelRefItem is kinda
    useless then.
*/
CelRefItem::CelRefItem(CelRef *ref) :
    QGraphicsObject(NULL),
    _ref(ref)
{
    // If we have a reference, hookup the position changed signal
    if (_ref) {
        Cel *cel = _ref->cel();

        // Initial Postion & layering data
        setPos(_ref->pos());
        setZValue(_ref->zValue());

        // Signals & slots for changing data
        connect(_ref, &CelRef::positionChanged, this, &CelRefItem::_onRefPositionChanged);
        connect(_ref, &CelRef::zValueChanged, this, &CelRefItem::_onRefZValueChanged);
        connect(_ref, &CelRef::showCelInfoChanged, this, &CelRefItem::_onRefShowCelInfoChanged);
        connect(cel, &Cel::nameChanged, this, &CelRefItem::_onCelNameChanged);
        connect(cel, &Cel::resized, this, &CelRefItem::_onCelResized);

        // The child items
        _nameItem = new QGraphicsTextItem(cel->name(), this);
        _outlineItem = new QGraphicsRectItem(0, 0, cel->width(), cel->height(), this);
        _outlineItem->setBrush(Qt::NoBrush);
        _outlineItem->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

        // visibility
        _nameItem->setVisible(_ref->_showCelInfo);
        _outlineItem->setVisible(_ref->_showCelInfo);
    }

    // Info
    qDebug() << "[CelRefItem created] ref=" << _ref;
}


/*!
    Cealns up the CelRefItem.  Nothing special really happens.
*/
CelRefItem::~CelRefItem() {
    qDebug() << "[CelRefItem deleted] ref=" << _ref;
}


/*!
    Returns a pointer to the CelRerf this item is representing.  May be NULL.
*/
CelRef *CelRefItem::ref() {
    return _ref;
}


/*!
    Required to be implemented by QGraphicsObject.
    
    Returns the visible area.
*/
QRectF CelRefItem::boundingRect() const {
    if (_ref && _ref->_cel)
        return QRectF(QPointF(0, 0), _ref->_cel->size());
    else
        return QRectF();
}


/*!
    Required to be implemented by QGraphicsObject.  Will draw the Cel onto the painter.
*/
void CelRefItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (_ref && _ref->_cel)
        _ref->_cel->paint(painter);
    else
        return;
}


/*!
    Sets the \a canvas that this CelRefItem lives in.  Will hook up some
    signals and slots.
*/
void CelRefItem::setCanvas(Canvas *canvas) {
    // Disconnect any existing canvas
    if (_canvas)
        disconnect(_canvas, 0, this, 0);

    // Reuse to set the zoom for the child items
    _onCanvasZoomChanged(canvas->zoom());

    // Signals & slots
    connect(canvas, &Canvas::zoomChanged, this, &CelRefItem::_onCanvasZoomChanged);

    // Cache it
    _canvas = canvas;
}


/*!
    Called via the CelRef::positionChanged() signal, this will adjust the postion
    of the CelRefItem.
*/
void CelRefItem::_onRefPositionChanged(QPointF pos) {
    // Set the postion, and print some info
    setPos(pos);
    qDebug() << "[CelRefItem _onRefPositionChanged]" << pos;
}


/*!
    Called via CelRef::zValueChanged() signal, this will modify the Z value
    of the CelRefItem.
*/
void CelRefItem::_onRefZValueChanged(qreal z) {
    setZValue(z);
    qDebug() << "[CelRefItem _onRefZValueChanged]" << z;
}


/*!
    Triggered via CelRef::showCelInfoChanged(), this will mark the child text
    and pictures items to be either hidden or visible.
*/
void CelRefItem::_onRefShowCelInfoChanged(bool showIt) {
    _nameItem->setVisible(showIt);
    _outlineItem->setVisible(showIt);
}


/*!
    Called by the Canvas::zoomChanged, this will adjust some of the child
    GraphicsItems.
*/
void CelRefItem::_onCanvasZoomChanged(qreal zoom) {
    if (_ref) {
        // Get the inverse of the zoom
        qreal invZoom = 1 / zoom;
        if (invZoom > 1)
            invZoom = 1;

        // Name
        _nameItem->setScale(invZoom);

        // Outline
        QPen pen(_outlineItem->pen());
        pen.setWidth(invZoom);
        pen.setJoinStyle(Qt::MiterJoin);
        _outlineItem->setPen(pen);

        // We have to do this because of the outline, reset to hard integer boundaries
        int cw = _ref->cel()->width();
        int ch = _ref->cel()->height();
        if (_zoom != 0)
            _outlineItem->setRect(0, 0, cw, ch);

        // Adjust postion & size just to be 1px outside the actual size
        _outlineItem->setRect(-invZoom, -invZoom, cw + invZoom, ch + invZoom);
    }

    // Save the zoom
    _zoom = zoom;
}


/*!
    When The Cels' name is changed, the Cel::nameChanged() signal will notify
    The CelRefItem to change it's text.
*/
void CelRefItem::_onCelNameChanged(QString name) {
    _nameItem->setPlainText(name);
}


/*!
    When the Cel has been resized via the Cel::resized() signal, this will tell
    The CelRefItem to adjust its outline
*/
void CelRefItem::_onCelResized(QSize size) {
    _outlineItem->setRect(0, 0, size.width(), size.height());
}


