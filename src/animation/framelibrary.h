// File:         framelibrary.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the FrameLibrary


#ifndef FRAME_LIBRARY_H
#define FRAME_LIBRARY_H


#include <QObject>
#include <QHash>
#include <QSize>
class Frame;
class Animation;


class FrameLibrary : public QObject {
    Q_OBJECT;

public:
    // Constructor and deconstructor
    FrameLibrary(Animation *anim);
    ~FrameLibrary();

    // Frame operators
    bool addFrame(Frame *frame);
    bool removeFrame(Frame *frame);
    int numFrames();
    bool nameTaken(QString name);
    Frame *getFrame(QString name);
    QList<Frame *> frames();


private slots:
    void _onFrameNameChanged(QString name);
    void _onFrameDestroyed(QObject *obj);


private:
    friend Animation;
    bool _clear();                            // Should only be called by Animation

    // Member vars
    QHash<QString, Frame *> _frames;        // Hash of all the Frames in use.


};


#endif // FRAME_LIBRARY_H

