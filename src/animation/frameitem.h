// File:         frameitem.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the FrameItem class.


#ifndef FRAME_ITEM_H
#define FRAME_ITEM_H


#include <QGraphicsObject>
#include <QPointer>
class CelRef;
class Frame;


class FrameItem : public QGraphicsObject {
    Q_OBJECT;

public:
    FrameItem(Frame *frame);
    ~FrameItem();

    // virtual functions to implement
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);


private slots:
    // Cel signals
    void _onCelAdded(CelRef *cel);
    void _onCelRemoved(CelRef *cel);
    void _onCelMoved(CelRef *cel);
    void _onCelRefPositionChanged(CelRef *ref);


private:
    QPointer<Frame> _frame;


};


#endif // FRAME_ITEM_H

