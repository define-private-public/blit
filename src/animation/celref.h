// File:         celref.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the CelRef class.  It points to a Cel object


#ifndef CEL_REF_H
#define CEL_REF_H


#include <QObject>
#include <QPointer>
#include <QSet>
#include <QPointF>
#include <QRectF>
class Cel;
class CelRefItem;
class Frame;


class CelRef : public QObject {
    Q_OBJECT;

public:
    CelRef(Cel *cel);
    CelRef(Cel *cel, qreal x, qreal y);
    ~CelRef();
    CelRef *copy();

    // Stuff for the RefItem
    QPointer<CelRefItem> mkItem();
    void update(const QRectF &rect=QRectF());
    void update(qreal x, qreal y, qreal width, qreal height);

    // Position data
    QPointF pos() const;
    qreal x() const;
    qreal y() const;
    qreal zValue() const;
    void setPos(const QPointF &pos);
    void setPos(qreal x, qreal y);
    void setX(qreal x);
    void setY(qreal y);
    void move(qreal x, qreal y);
    void moveX(qreal x);
    void moveY(qreal y);
    void setZValue(qreal z);

    // Accessors
    QPointer<Cel> cel();
    bool hasCel();
    bool showCelInfo();

    // Frame info
    void setFrame(Frame *frame);            // Should only be called by Frame::addCel()
    QPointer<Frame> frame();
    bool hasFrame();


signals:
    void positionChanged(QPointF pos);            // Emiited when the position is changed
    void zValueChanged(qreal z);                // Emitted when the Z value is changed
    void showCelInfoChanged(bool showIt);        // Emitted when the "ShowCelInfo" is changed


public slots:
    void setShowCelInfo(bool showIt=true);


private slots:
    void _onCelRefItemDestroyed(QObject *obj=NULL);


private:
    // Friends
    friend CelRefItem;            // This is the friend of the class

    // For the CelRefItem
    bool _showCelInfo = true;    // Show's an outline and Info of a Cel in the Canvas

    // Member vars
    QPointer<Frame> _frame;        // Pointer to the Frame object this CelRef is encased in
    QSet<CelRefItem *> _cris;
//    QPointer<CelRefItem> _cri;    // Pointer to the CelRefItem this CelRef is being used by
    QPointer<Cel> _cel;            // The pointer to the Cel this reference represents
    QPointF _pos;                // Position data, (should normally be hard integers, but floats for flexibility & funsies!)
    qreal _zValue = 0;            // Layering information higher is drawn on top, lower on bottom, undefined for the same

};


#endif // CEL_REF_H

