// File:         timeline.h
// Author:       Ben Summerton (define-private-public)
// Description:  The Timeline class is a visualization of the XSheet that is currently Set.  It must alway
//               Have an XSheet set to it.  The XSheet could be a Null XSheet though in some cases.
//
//               A Bit of terminology:
//                 Tick -- A Marking that represents an entire frame & its hold.
//                 Ruler -- Meausres how "long," the sequence is in terms of frame count numbers.
//                 Cursor -- Cursor that is used to mark the current position in the timeline
//                 TriMaker -- A small black triangle that is used as a marker when there is user
//                             interaction


#ifndef TIMELINE_H
#define TIMELINE_H

#define TIMELINE_HEIGHT (RULER_HEIGHT + TICK_HEIGHT)


#include <QWidget>
#include <QPointer>
#include <QList>
#include <QHash>
class TimedFrame;
class XSheet;
class Tick;
class Ruler;
class Cursor;
class TriangleMarker;
class BracketMarker;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsSceneMouseEvent;


class Timeline : public QWidget {
    Q_OBJECT;

public:
    Timeline(QWidget *parent=NULL);
    ~Timeline();

    void setXSheet(XSheet *xsheet);
    XSheet *xsheet();

    // Tick operations
    QList<Tick *> ticks();
    Tick *tickByTimedFrame(TimedFrame *tf);
    
    // When we need to touch internals
    Cursor *timelineCursor();
    BracketMarker *leftBM();
    BracketMarker *rightBM();


public slots:
    void addTick(TimedFrame *tf);
    void removeTickByTimedFrame(TimedFrame *tf);
    void movingTick(QGraphicsSceneMouseEvent *event);        // Tells a tick that its moving
    void doneMovingTick(QGraphicsSceneMouseEvent *event);
    void updateRuler();

    void selectTickByIndex(int index);        // "Selects," a Tick
    void selectTickByTimedFrame(TimedFrame *tf);

    void turnOnSelectivePlayback(bool enabled);


public:
    // Member vars
    QPointer<Tick> curTick;                        // Currently selected Tick, may be NULL


private:
    int _seqNumAtEvent(QGraphicsSceneMouseEvent *event);
    
    // Member variables
    QPointer<XSheet> _xsheet;                    // Pointer to XSheet (could possibly be NULL)

    QGraphicsScene *_scene = NULL;                // Scene that cotains the Tickers, Ruler, Cursor, etc...
    QGraphicsView *_view = NULL;                // Just the view for the scene
    Ruler *_ruler = NULL;                        // Pointer to ruler contained in _scene
    Cursor *_cursor = NULL;                        // Pointer to cursor contained in _scene
    QList<Tick *> _ticks;                        // List of all the Ticks currently in the timeline (in order)
    QHash<TimedFrame *, Tick *> _tickMap;        // Use a Frame pointer to Map to a Tick pointer (unordered)

    // Markers
    TriangleMarker *_triMarker = NULL;            // Pointer to TriangleMarker
    BracketMarker *_leftBM = NULL;    // Pointer to the left Bracket Marker
    BracketMarker *_rightBM = NULL;    // Pointer to the right Bracket Marker

};


#endif // TIMELINE_H

