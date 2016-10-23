// File:         util.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Utilily modle function implementation


#include "util.h"
#include <QUuid>
#include <QPoint>
#include <QPointF>
#include <QColor>
#include <QImage>
#include <QDebug>

/*!
    Looks at a QUuid, strings out the brackets, and puts it out as a QString.
*/
QString util::stripUUID(QUuid uuid) {
    QString str(uuid.toString());
    str.remove("{");
    str.remove("}");
    return str;
}


/*!
    Makes a UUID String.  \a numChars is the number of characters to return; dashes are not counted
    but are included in the resulting string.  Supplying zero or less will just give you an empty
    string.  If numChars is greater than 32, nothing extra will happen.

    e.g.:
    \code
     numChars=8   ->  43e4ba02
     numChars=12  ->  9b03ddf6-88c5
    \endcode
*/
QString util::mkUUIDStr(int numChars) {
    // First get a uuid as a stripped string
    QString uuid = util::stripUUID(QUuid::createUuid());

    // Figure out how many to trunicate by
    if (numChars < 0)
        numChars = 0;
    else if ((8 < numChars) && (numChars <= 12))
        numChars += 1;
    else if ((12 < numChars) && (numChars <= 16))
        numChars += 2;
    else if ((16 < numChars) && (numChars <= 20))
        numChars += 3;
    else if ((20 < numChars) && (numChars <= 32))
        numChars += 4;
    else if (numChars > 32)
        numChars = 32 + 4;

    // Trunicate and return
    uuid.truncate(numChars);
    return uuid;
}


/*!
    Makes a blank image (filled with transparency), of the size supplied.  Image return will be in the
    format of ARGB_32_Premulitiplied
*/
QImage util::mkBlankImage(QSize size) {
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    
    if (img.isNull()) {
        qDebug() << "Wasn't able to make an blank image of size: " << size;
        qDebug() << "Returning a null image";
    } else 
        img.fill(QColor(0x00, 0x00, 0x00, 0x00));        // Fill with transparency

    return img;
}


/*!
    Produces the inverse of another color based on the sRGB colorpsace.
    Leaves alpha value alone.
*/
QColor util::invert(QColor clr) {
    clr.setRed(0xFF - clr.red());
    clr.setGreen(0xFF - clr.green());
    clr.setBlue(0xFF - clr.blue());
    return clr;
}


/*!
    Turns a QSize into a QString, with the format of "<width> x <height".  e.g. "3 x 2"
*/
QString util::sizeToStr(QSize size) {
    return QString::number(size.width()) + " x " + QString::number(size.height());
}


/*!
    Uses Bresenham's line algorithm, this will return a list of (integer) points
    that are used to construct the line between the two points.  Implementation based
    of of the one on Rosetta Code's wiki page.
*/
QList<QPoint> math::bresenhamLinePoints(qreal ax, qreal ay, qreal bx, qreal by) {
    // Variables
    QList<QPoint> points;

    // is the line steep?
    bool steep = (qAbs(by - ay) > qAbs(bx - ax));
    if (steep) {
        // Swap the x and y values
        qSwap(ax, ay);
        qSwap(bx, by);
    }

    // Is the first point more right than the second
    if (ax > bx) {    
        // If so, swap the points
        qSwap(ax, bx);
        qSwap(ay, by);
    }

    // Deltas
    qreal dx = bx - ax;
    qreal dy = qAbs(by - ay);

    // other
    qreal error = dx / 2.0;
    int yStep = (ay < by) ? 1 : -1;
    int y = (int)ay;
    int maxX = (int)bx;

    // Figure out all of the points
    for (int x = (int)ax; x < maxX; x++) {
        if (steep)
            points.append(QPoint(y, x));
        else
            points.append(QPoint(x, y));

        error -= dy;
        if (error < 0) {
            y += yStep;
            error += dx;
        }
    }

    return points;
}


/*!
    Overloaded convienience funciton for the QPointF math::bresenhamLinePoints() function.
*/
QList<QPoint> math::bresenhamLinePoints(QPointF a, QPointF b) {
    return math::bresenhamLinePoints(a.x(), a.y(), b.x(), b.y());
}


