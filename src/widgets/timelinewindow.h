// File:         timelinewindow.h
// Author:       Ben Summerton (define-private-public)
// Description:  TimelineWindow is mainly an organizational/container widget for the BlitApp Timeline
//               controls.
//
//               In Qt 5.3, the Dock Widgets were acting kind of funky under Xubuntu linux, so on that
//               platform, Dock Widgets are not enabled.  Though the code is still there if you want to
//               turn on Dock Widgets for example, MS Windows.


// TODO List:
//  - Styleize the widgets via QStyle


#ifndef TIMELINE_WINDOW_H
#define TIMELINE_WINDOW_H


#include <QWidget>
#include <QTimeLine>
class BlitApp;
class TimedFrame;
class Animation;
class Tick;
class Timeline;
class QString;
class QTimeLine;
class QLabel;
class QSpinBox;
class QToolButton;
class QHBoxLayout;


namespace Ui {
    class TimelineWindow;
}


class TimelineWindow : public QWidget {
    Q_OBJECT;

public:
    TimelineWindow(BlitApp *blitapp, QWidget *parent=NULL);
    ~TimelineWindow();


signals:
    void animationPlaybackStateChanged(bool isPlaying);


public slots:
    void setAnimation(Animation *anim);

    void onTickSelected();

    // Play state
    void playAnimation(bool play=true);            // false for stop

    // Timeline navigation
    void goBy(int numFrames);

    // Accessors
    bool isAnimationPlaying();


private slots:
    void _onPlaybackTimelineFrameChanged(int frame);
    void _onPlaybackTimelineFinished();
    void _onCurSeqNumChanged(quint32 seqNum);
    void _onTimedFrameMoved();

    // Playback controls
    void _onPlayClicked(bool checked);
    void _onBeginningClicked(bool checked);
    void _onEndClicked(bool checked);
    void _onSelectivePlaybackButtonClicked(bool checked);
    void _onRightBracketMarkerSeqNumOverChanged(int seqNum);
    void _onLeftBracketMarkerSeqNumOverChanged(int seqNum);

    // Frame OP (+ FPS) actions
    void _onHoldSpinnerChanged(int hold);
//    void _onFrameNameEditChanged(QString name);
    void _onCopyFrameClicked(bool checked);
    void _onAddFrameClicked(bool checked);
    void _onDeleteFrameClicked(bool checked);
    void _onFPSSpinnerChanged(int fps);


protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);


private:
    // Internal functions
    TimedFrame *_mkNewFrame(int hold=1);
    void _setupPlayback(int startSeq);
    void _checkDisableDeleteFrame();
    void _addTimedFrameToAnimation(TimedFrame *tf);
    void _adjustTimingLabel(quint32 seqNum);


    // Member vars
    bool _changingFrames = false;
    bool _playingAnim = false;
    QTimeLine  *_playbackTimeline;
    bool _loopDone = false;

    // GUI
    Ui::TimelineWindow *_ui;
    Timeline *_timeline;

};


#endif // TIMELINE_WINDOW_H

