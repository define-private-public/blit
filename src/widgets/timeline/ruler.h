// File:         ruler.h
// Author:       Ben Summerton (define-private-public)
// Description:  Ruler is just a simple box that is used to "meausre," the lenght of the XSheet inside
//               The Timeline widget.


#ifndef RULER_H
#define RULER_H

#define RULER_HEIGHT 20
#define RULER_DEFAULT_STEP 5
#define RULER_TYPE 2


#include <QGraphicsObject>


class Ruler : public QGraphicsObject {
    Q_OBJECT;

public:
    enum { Type = UserType + RULER_TYPE };

    Ruler(int length, int step=RULER_DEFAULT_STEP, QGraphicsItem *parent=NULL);

    // Overriden functions
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);
    QRectF boundingRect() const;
    int type() const;

    // Accessors
    int step();
    int length();


public slots:
    void setStep(int step);
    void setLength(int seqLen);


private:
    int _step;            // How many sequence numbers to make a marking
    int _length;        // Maximum size

};


#endif // RULER_H

