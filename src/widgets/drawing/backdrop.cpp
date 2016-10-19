// File:         backdrop.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the Backdrop class


/*!
    \inmodule Drawing
    \class Backdrop
    \brief Backdrop is a QGraphicObject that appears behind Frame in the Canvas widget.

    It is simply nothing more than presentation.  It allows you to change the color of
    itself.  It does not (currently) affect how things will be rendered;  it is currently
    a visual aid.  Backdrops should only be used inside Canvas Widgets.
*/


//#include <QDebug>
#include "widgets/drawing/backdrop.h"
#include "widgets/drawing/canvas.h"
#include <QtMath>
#include <QPainter>
#include <QDebug>



/*!
    Instatiates the Backdrop graphics object for the Canvas
*/
Backdrop::Backdrop(Canvas *canvas, QGraphicsItem *parent) {
    _clr = Qt::white;

    // Signals & slots
    connect(canvas, &Canvas::zoomChanged, this, &Backdrop::_onCanvasZoomChanged);

    // Debugging info
    qDebug() << "[" << this << " created]";
}


/*!
    Deconstructor.  Nothing but cleanup
*/
Backdrop::~Backdrop() {
    // Debugging info
    qDebug() << "[" << this << " deleted]";
}


/*!
    Returns the area the backdrop occupies, should be the same as the Frame Size
*/
QRectF Backdrop::boundingRect() const {
    return QRectF(0, 0, _size.width(), _size.height());
}


void Backdrop::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Vars
    qreal invZoom = 1.0 / _zoom;        // Zoom inverse
    if (invZoom > 1.0)
        invZoom = 1.0;

    // Main area
    if (_clr.alpha() != 0xFF) {
        // Got some alpha, make the boxes

        qreal boxSize = _boxSize / _zoom;
        qreal diagLength = qSqrt(qPow(boxSize, 2) + qPow(boxSize, 2));
        qreal boxesPerColumn = _size.height() / diagLength;

        // Create the (rotated) boxes
        QVector<QRectF> boxes;
        qreal x = 0, y = 0;
        qreal travWidth = 0, travHeight = 0;
        while (travHeight < (_size.height() + diagLength)) {
            x = 0;
            travWidth = 0;
            while (travWidth < (_size.width() + diagLength)) {
                 QRectF rect(x + y, y - x, boxSize, boxSize);
                 boxes.append(rect);

                 x += boxSize;
                 travWidth += diagLength;
            }

            y += boxSize;
            travHeight += diagLength;
        }

        // Draw background
        painter->setPen(Qt::NoPen);
        painter->fillRect(0, 0, _size.width(), _size.height(), Qt::white);

        // Draw little grey boxes
        painter->save();
        painter->setClipping(true);
        painter->setClipRegion(QRegion(0, 0, _size.width(), _size.height()));
        painter->setBrush(QColor(0xE0, 0xE0, 0xE0));
        painter->translate(-(travWidth - _size.width()) / 2, -(travHeight - _size.height()) / 2);        // Center the boxes
        painter->rotate(45);
        painter->drawRects(boxes);
        painter->restore();

    }
    painter->fillRect(0, 0, _size.width(), _size.height(), _clr);        // Always give it the overlay

    // Outline
    QPointF tl(-invZoom, -invZoom);
    QPointF tr(_size.width(), -invZoom);
    QPointF br(_size.width(), _size.height() + invZoom);
    QPointF bl(-invZoom, _size.height());
    QLineF outline[4] = {
        QLineF(tl, tr),        // Top
        QLineF(bl, br),        // Bottom
        QLineF(tl, bl),        // Left
        QLineF(tr, br)        // right
    };
    painter->setPen(QPen(Qt::black, invZoom, Qt::SolidLine, Qt::FlatCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawLines(outline, 4);
}


/*!
    Returns the color that this backdrop is given.
*/
QColor Backdrop::color() {
    return _clr;
}


/*!
    Sets the size of the backdrop to \a size, only if it isn't the exact same
    already.

    Will cause the item to be redraw.
*/
void Backdrop::setSize(QSize size) {
    // Check for diff
    if (_size != size) {
        _size = size;
        update();
    }
}


/*!
    Used to set the color of the backdrop.  Will do nothing if \a clr is already
    set to that Color.  Also will do nothing of the color is invalid.

    If a color is supplied with some alpha, then a checkboard pattern will appear.

    Will cause the item to update.
*/
void Backdrop::setColor(QColor clr) {
    if ((_clr != clr) && clr.isValid()) {
        _clr = clr;
        update();
    }
}


/*!
    Triggered via the Canvas::zoomChanged() signal, this will tell the Backdrop
    to redraw if the zoom is not the same
*/
void Backdrop::_onCanvasZoomChanged(qreal zoom) {
    if (_zoom != zoom) {
        _zoom = zoom;
        update();
    }
}

