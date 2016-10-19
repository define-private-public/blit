// File:         frame.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the Frame class.


#ifndef FRAME_H
#define FRAME_H


// Macros
#define FRAME_TOP_LAYER 0
#define FRAME_BOTTOM_LAYER -1
#define FRAME_UUID_POSTFIX_SIZE 8


#include <QObject>
#include <QPointer>
#include <QList>
#include <QGraphicsScene>
class CelRef;
class Animation;
class TimedFrame;
class FrameItem;
class QImage;



class Frame : public QObject {
    Q_OBJECT;

public:
    Frame(Animation *anim, QString name="");
    ~Frame();
    Frame *copy(QString name="");

    // Important info stuff
    QString name();
    QString oldName();
    bool setName(QString name);

    // Misc info
    bool usingRandomName();
    bool usingUUIDPostfix();

    // Cel operations
    void addCel(CelRef *ref, int at=FRAME_TOP_LAYER);
    CelRef *removeCel(int at=FRAME_TOP_LAYER);
    void moveCel(int at, int to);
    QList<CelRef *> cels();
    CelRef *cel(int at);
    int numCels();
    void saveCelFiles();
    void removeCelFiles();

    // TimedFrame Referecnes
    void registerTimedFrame(TimedFrame *tf);
    void unregisterTimedFrame(TimedFrame *tf);
    QList<QPointer<TimedFrame>> timedFrames();

    // Rendering
    QImage render();

    // Animation stuff
    QSize frameSize();

    // Canvas
    FrameItem *mkItem();
    

public slots:
    // State
    bool active();
    void activate();
    void deactivate();


signals:
    // Meta
    void activated();
    void deactivated();
    void nameChanged(QString name);

    // Cel signals
    void celAdded(CelRef *cel);
    void celRemoved(CelRef *cel);
    void celMoved(CelRef *cel);
    void celRefPositionChanged(CelRef *ref);


private:
    // Cel stuff
    void _onCelRefPositionChanged(QPointF pos);


protected:
    // Member functions
    void _updateCelRefOrder(int startingIndex=0);

    // Member vars
    QPointer<Animation> _anim;                    // Animathion that this Frame is a part of
    QString _name = "";                            // Name of the Frame, must be unique vs. all other instances of Frame
    QString _oldName = "";                        // Previous name of the Frame, is used in conjunction with the nameChanged() signal
    bool _usingRandomName = false;                // If a name was randomly generated for the Frame, then this flag is set, the postfix is it's given name
    bool _usingUUIDPostfix = false;                // When assigning a name, if the name is taken, then it will automatically assing a UUID postfix
    bool _active = false;                        // Flag to see if the Frame is currently marked as active or not

    // Scence & View stuff
    QList<CelRef *> _celRefs;                    // List of pointers to Cel objects; order is important and matters for layering
    QList<QPointer<TimedFrame>> _timedFrames;    // Frame Refs that are pointing to this Frame


};


#endif // FRAME_H

