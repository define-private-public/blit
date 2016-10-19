// File:         timedframe.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the TimedFrame class


#ifndef TIMED_FRAME_H
#define TIMED_FRAME_H


#define TIMED_FRAME_DEFAULT_SEQ_NUM 1
#define TIMED_FRAME_DEFAULT_HOLD 1


#include <QObject>
#include <QPointer>
class Frame;
class XSheet;


class TimedFrame : public QObject {
    Q_OBJECT;

public:
    TimedFrame(Frame *frame, int seqNum=TIMED_FRAME_DEFAULT_SEQ_NUM, int hold=TIMED_FRAME_DEFAULT_HOLD);
    ~TimedFrame();

    // Timing information
    int seqNum();
    bool hasSeqNum(int num);
    QList<int> seqNums();
    int hold();

    // Accessors
    QPointer<Frame> frame();
    bool hasFrame();

    // XSheet
    void setXSheet(XSheet *xsheet);            // Should only be called by XSheet::addFrame()
    TimedFrame *before(bool loop=false);
    TimedFrame *after(bool loop=false);


public slots:
    void setSeqNum(int num);
    void setHold(int hold);


signals:
    void seqNumChanged(int num);
    void holdChanged(int hold);


private:
    // Member vars
    QPointer<XSheet> _xsheet;    // Pointer to the XSheet object this TimedFrame is a part of
    QPointer<Frame> _frame;        // Pointer to Frame object this TimedFrame uses
    int _seqNum = 1;            // sequence number; where in the XSheet this frame begins, is a positive integer
    int _hold = 1;                // How many frames to hold this one out for, is a positive integer

};


#endif // TIMED_FRAME_H

