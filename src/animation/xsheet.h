// File:         xsheet.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the XSheet class.

// TODO List:
//  - Framework is there to support layers, but now it's just using one
//  - Make the code cleaner for the sequence length thing
//  - bring back _seqMap?



#ifndef XSHEET_H
#define XSHEET_H


// Macros
#define XSHEET_DEFAULT_FPS 24
#define XSHEET_BEGINNING 1
#define XSHEET_END -1


#include <QObject>
#include <QPointer>
#include <QList>
class TimedFrame;
class Animation;


class XSheet : public QObject {
    Q_OBJECT;

public:
    explicit XSheet(Animation *anim, int fps=XSHEET_DEFAULT_FPS);
    ~XSheet();

    // Meta-data
    bool isEmpty();
    int numFrames();
    int seqLength();
    int FPS();
    void setFPS(int fps);

    // Frame operators
    QList<QPointer<TimedFrame>> frames();
    QPointer<TimedFrame> frameAtSeq(int num);
    void addFrame(TimedFrame *frame, int at=XSHEET_END);
    QPointer<TimedFrame> removeFrame(int at=XSHEET_END);
    void moveFrame(int at, int to);


private slots:
    void _onHoldChanged(int hold);


signals:
    void FPSChanged(int fps);
    void seqLegnthChanged(int seqLength);
    void seqNumsChanged();
    void frameAdded(QPointer<TimedFrame> frame);
    void frameRemoved(QPointer<TimedFrame> frame);
    void frameMoved(QPointer<TimedFrame> frame);


private:
    // Member functions
    void _updateSeqNums(int at=0);

    // Members vars
    QPointer<Animation> _anim;                        // Animathion that this XSheet is a part of
    QList<QList<QPointer<TimedFrame>>> _frames;        // List of layers, of lists of TimedFrames, in order by their sequence number
    int _seqLength = 0;                                // Total count of how many frames long the sequence is (inclues holds)
    int _fps = 1;                                    // Framerate of the animation sequence, should a positive integer


};
 

#endif // XSHEET_H

