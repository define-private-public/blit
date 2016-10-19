// File:         ruler.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implemenation for the Ruler class


#include "widgets/timeline/ruler.h"
#include "widgets/timeline/tick.h"
#include <QString>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QFont>
#include <QStaticText>
#include <QPainter>
#include <QLinearGradient>


Ruler::Ruler(int length, int step, QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    // Simple constructor, make sure to provide a non-negative length and a step value that is a positive
    // integer, else it will be set to 1

    // If the step provided is less th
    setStep(step);
    setLength(length);
}


void Ruler::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Paints the object, overriden

    // Make the gradient background
    QLinearGradient grad(QPointF(0, 0), QPointF(0, RULER_HEIGHT));
    grad.setColorAt(0, Qt::lightGray);
    grad.setColorAt(1, Qt::gray);

    // Fill the background
    painter->setPen(Qt::NoPen);
    painter->fillRect(boundingRect(), grad);

    // The ticks
    painter->setPen(Qt::darkGray);
    for (int x = 0; x < ((_length + 1) * TICK_WIDTH); x += TICK_WIDTH)
        painter->drawLine(x, RULER_HEIGHT - 10, x, RULER_HEIGHT - 1);

    // The numbers
    QFont numFont(painter->font());
    numFont.setPointSize(6);
    painter->setFont(numFont);
    painter->setPen(Qt::black);
    for (int n = _step; n < (_length + 1); n += _step) {
        QPointF topLeft((n - 1) * TICK_WIDTH, RULER_HEIGHT - 15);
        topLeft.rx() += 2;
        painter->drawStaticText(topLeft, QStaticText(QString::number(n)));
    }

    // And then draw the first number too
    QPointF topLeft(0, RULER_HEIGHT - 15);
    topLeft.rx() += 2;
    painter->drawStaticText(topLeft, QStaticText("1"));
}


QRectF Ruler::boundingRect() const {
    // Bouding rect, overriden method
    return QRectF(0, 0, (_length * TICK_WIDTH) + 1, RULER_HEIGHT);
//    w += 1;        // For good luck (no really, this makes it look a bit better)
}


int Ruler::type() const {
    // enable qgraphicsitem_cast
    return Type;
}


/*!
    Returns the step value currently set

    \sa setStep()
*/
int Ruler::step() {
    return _step;
}


int Ruler::length() {
    // Returns the length of the ruler set
    return _length;
}


/*!
    Used to set how often to make a "ruler marking."  (Show a number).  A positive
    integer should be supplied.  If one isn't then step=1.  If the given step is
    the same as the one already set, nothing will happen.  Will cause the widget
    to be redrawn on a step change.

    \sa step()
*/
void Ruler::setStep(int step) {
    if (step < 1)
        step = 1;

    // Only do something if there is a difference
    if (_step != step) {
        _step = step;
        update();
    }
}



void Ruler::setLength(int seqLen) {
    // Sets the length (bouding rectangle too) of the Ruler, also sends an update signal
    // Set the width.  Must be a non-negative integer
    if (seqLen < 0)
        return;
    
    _length = seqLen;

    // The rest of the rect is easy
    update();
}


