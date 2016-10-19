// File:         bracketmarker.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the BracketMarker class.


#ifndef BRACKET_MARKER_H
#define BRACKET_MARKER_H


#define BRACKET_MARKER_TYPE 5


#include <QGraphicsObject>
class Timeline;
class QGraphicsRecItem;


class BracketMarker : public QGraphicsObject {
    Q_OBJECT;

public:
    enum { Type = UserType + BRACKET_MARKER_TYPE };        // For QGraphicsScene
    enum class BracketMarkerType { LEFT, RIGHT };

    // Constructor & Deconstructor
    BracketMarker(Timeline *timeline, BracketMarkerType bmType, QGraphicsItem *parent=NULL);
    ~BracketMarker();

    // Virtuals that need to be implemented
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=NULL);
    int type() const;

    // Sequence number stuff
    int seqNumOver();
    void moveToSeqNum(int seqNum);

    // Actions
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    // Misc
    bool moving() const;
    static QString typeStr(BracketMarkerType type);


signals:
    void seqNumOverChanged(int seqNum);

public slots:
    void onXSheetSeqLengthChanged(int seqLength);


private slots:


private:
    // Member vars
    BracketMarkerType _type;            // Type of Bracket Marker to draw
    Timeline *_timeline;                // Timeline (QWidget) that owns this BracketMarker
    bool _moving = false;                // Is the marker being moved or not?
    qreal _sideSize;                    // Size of the triangles that are rednered
    qreal _leftStartSeqNum = 0;            // Starting seq num of the left bracket
    qreal _rightStartSeqNum = 0;        // Staring seq num of the right bracket
    QGraphicsRectItem *_dimmer;            // Dims the timeline before/after the bracket

};


#endif // BRACKET_MARKER_H

