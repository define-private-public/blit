// File:         util.h
// Author:       Ben Summerton (define-private-public)
// Description:  util.h contains a set of misc utility functions, as well as some math functions

#ifndef UTIL_H
#define UTIL_H


class QString;
class QSize;
class QImage;
class QColor;
class QPoint;
class QPointF;
class QUuid;
#include <QList>


namespace util {
    QString stripUUID(QUuid uuid);
    QString mkUUIDStr(int numChars=8);
    QImage mkBlankImage(QSize size);
    QColor invert(QColor clr);
    QString sizeToStr(QSize size);
};


// TODO maybe move the math functions to their own file
namespace math {
    QList<QPoint> bresenhamLinePoints(qreal aX, qreal aY, qreal bX, qreal bY);
    QList<QPoint> bresenhamLinePoints(QPointF a, QPointF b);
};

#endif // UTIL_H

