// File:         celrefitem.h
// Author:       Ben Summerton (define-private-public)
// Description:  The CelRefItem is the QGraphicsScene representation of a
//               CelRef object.

// TODO List
//  - Z ordering and lyering of the CelRefItems
//  - What happens if someone changes the postion of the CelRefItem?
//    - Should it change the postion in the CelRef?  Or should is just ignore it
//      - Probably is necessary for the QGraphicScene to be able to set the position via setPos()
//        so it probably isn't wise to disable the function


#ifndef CEL_RELF_ITEM_H
#define CEL_RELF_ITEM_H


#include <QGraphicsObject>
#include <QPointer>
class CelRef;
class Canvas;
class QPointF;
class QGraphicsRectItem;
class QGraphicsTextItem;


class CelRefItem : public QGraphicsObject {
    Q_OBJECT;

public:
    CelRefItem(CelRef *ref);
    ~CelRefItem();

    // Accessors
    CelRef *ref();

    // virtual functions to implement
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);

    // The other things
    void setCanvas(Canvas *canvas);


private slots:
    void _onRefPositionChanged(QPointF pos);
    void _onRefZValueChanged(qreal z);
    void _onRefShowCelInfoChanged(bool showIt);
    void _onCanvasZoomChanged(qreal zoom);
    void _onCelNameChanged(QString name);
    void _onCelResized(QSize size);


private:
    // Member vars
    QPointer<CelRef> _ref;                // Pointer to the CelRef this RefItem is respresenting
    QPointer<Canvas> _canvas;            // Pointer to the Canvas this RefItem is contained in
    QGraphicsTextItem *_nameItem;        // Name of the Cel
    QGraphicsRectItem *_outlineItem;    // Outline of the Cel's dimensions

    // From the Canvas
    qreal _zoom = 0;


};


#endif // CEL_RELF_ITEM_H

