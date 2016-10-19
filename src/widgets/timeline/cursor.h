// File:         cursor.h
// Author:       Ben Summerton (define-private-public)
//               the ability to scrub through all of the frames in the currently loaded sequence


#ifndef CURSOR_H
#define CURSOR_H


#define CURSOR_WIDTH 14
#define CURSOR_HEIGHT 28
#define CURSOR_TYPE 3


#include <QGraphicsObject>
class TimedFrame;
class Tick;
class QPainter;
class QGraphicsLineItem;


class Cursor : public QGraphicsObject {
    Q_OBJECT;

public:
    enum { Type = UserType + CURSOR_TYPE };

    Cursor(QGraphicsItem *parent=NULL);

    // Virtuals that need to be implemented
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);
    int type() const;

    // Actions
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    // Accessors
    int seqNumOver();
    Tick *getTickOver();


public slots:
    void onTickSelected();
    void moveToTick(Tick *tick);                // Will not select Tick
    void moveToTimedFrame(TimedFrame *tf);        // Will not select Tick
    void moveToSeqNum(int seqNum);                // Will not select Tick


signals:
    void overNewTick(Tick *tick);        // When the cursor is moved over a differnt/new Tick, it will emit its frame
     

private:
    // Member vars
    bool _scrubbing = false;            // Are we currently moving the cursor?
    QPolygonF _shape;                    // Shape of the cursor
    QGraphicsLineItem *_redLine;        // Pointer to the a line that appears below the cursor
    Tick *_curTick = NULL;                // Pointer to the Tick that is currently under the cursor (can be NULL)


};


#endif // CURSOR_H

