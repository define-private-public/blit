// File:         timelinewindow.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for TimelineWindow class



#include "widgets/timelinewindow.h"
#include "ui_timeline_window.h"
#include "widgets/timeline/timeline.h"
#include "widgets/timeline/tick.h"
#include "widgets/timeline/cursor.h"
#include "widgets/timeline/bracketmarker.h"
#include "animation/pngcel.h"
#include "animation/celref.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include "animation/animation.h"
#include "blitapp.h"
#include <QString>
#include <QDateTime>
#include <QTimeLine>
#include <QLabel>
#include <QIcon>
#include <QToolButton>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QKeyEvent>




TimelineWindow::TimelineWindow(BlitApp *blitapp, QWidget *parent) :
    QWidget(parent),        // BlitApp isn't the parent, need the other widget (dock widget) for signals & slots
    _ui(new Ui::TimelineWindow)
{
    // for Animaton playback
    _playbackTimeline = new QTimeLine(1000, this);        // (placeholder time)

    // GUI
    _ui->setupUi(this);
    _timeline = _ui->timeline;

    // Metrics
    setWindowTitle("Timeline");
//    setWindowFlags(Qt::Tool);
    setMinimumWidth(640);
    setFixedHeight(sizeHint().height());
//    setFixedSize(640, sizeHint().height());

    // Becuase of Qt5, we need to do some hacky-variable signal/slot loading
    void (QSpinBox:: *valueChangedSignal)(int) = &QSpinBox::valueChanged;

    // Signals & Slots
    connect(blitapp, &BlitApp::curTimedFrameChanged, _timeline, &Timeline::selectTickByTimedFrame);
//    connect(_timeline->timelineCursor(), &Cursor::overNewTick, this, &TimelineWindow::onTickSelected);        // Defunct?
//    connect(_ui->frameNameEdit, &QLineEdit::textEdited, this, &TimelineWindow::onFrameNameEditChanged);
    connect(blitapp, &BlitApp::curSeqNumChanged, this, &TimelineWindow::_onCurSeqNumChanged);
    connect(_ui->holdSpinner, valueChangedSignal, this, &TimelineWindow::_onHoldSpinnerChanged);
    connect(_ui->copyFrameButton, &QToolButton::clicked, this, &TimelineWindow::_onCopyFrameClicked);
    connect(_ui->addFrameButton, &QToolButton::clicked, this, &TimelineWindow::_onAddFrameClicked);
    connect(_ui->deleteFrameButton, &QToolButton::clicked, this, &TimelineWindow::_onDeleteFrameClicked);
    connect(_ui->fpsSpinner, valueChangedSignal, this, &TimelineWindow::_onFPSSpinnerChanged);

    // Playback stuff
    connect(_ui->playButton, &QToolButton::clicked, this, &TimelineWindow::_onPlayClicked);
    connect(_ui->beginningButton, &QToolButton::clicked, this, &TimelineWindow::_onBeginningClicked);
    connect(_ui->endButton, &QToolButton::clicked, this, &TimelineWindow::_onEndClicked);
    connect(_playbackTimeline, &QTimeLine::frameChanged, this, &TimelineWindow::_onPlaybackTimelineFrameChanged);
    connect(_playbackTimeline, &QTimeLine::finished, this, &TimelineWindow::_onPlaybackTimelineFinished);
    connect(_ui->selectivePlaybackButton, &QToolButton::clicked, this, &TimelineWindow::_onSelectivePlaybackButtonClicked);

    setAnimation(NULL);
}


TimelineWindow::~TimelineWindow() {
    delete _ui;
}


void TimelineWindow::closeEvent(QCloseEvent *event) {
    // Reimplementation of the QWidget::closeEvent() function, just makes sure to stop playing the 
    // the animation.  Will call the parent class
    event->accept();
}


void TimelineWindow::keyPressEvent(QKeyEvent *event) {
    // Overloaded protected virtual function.  Will process some hotkey events for us.  Some
    // Keys will stop playing the animation

    // Have to place these outside the swtich
    int seq;
    
    // do something
    switch (event->key()) {
        case Qt::Key_J:
            // Goto Beginngin
            _ui->beginningButton->click();
            break;

        case Qt::Key_K:
            // Play/Pause
            _ui->playButton->click();
            break;

        case Qt::Key_L:
            // Goto End
            _ui->endButton->click();
            break;

        case Qt::Key_Minus:
            // Decrement current hold
            _ui->holdSpinner->setValue(_ui->holdSpinner->value() - 1);
            event->accept();
            break;
            
        case Qt::Key_Equal:
            // Increment current hold
            _ui->holdSpinner->setValue(_ui->holdSpinner->value() + 1);
            event->accept();
            break;

        case Qt::Key_Comma:
            // Move Left one frame (not sequence number)
            goBy(-1);
            event->accept();
            break;

        case Qt::Key_Period:
            // Move Right one frame (not sequence number)
            goBy(1);
            event->accept();
            break;

        defaut:
            // Call the base class in any other case
            QWidget::keyPressEvent(event);
    }

}


void TimelineWindow::setAnimation(Animation *anim) {
    // Most likely will be called by BlitApp's animLoaded() signal.  It will tell the Timeline widget
    // there there is a new XSheet to be set to.  Will also modify the states of the widgets, as well
    // as connect/disconnect them
    bool animSet = anim != NULL;

    // Eanble or disable the child widgets
    _ui->frameNumLabel->setEnabled(animSet);
    _ui->holdLabel->setEnabled(animSet);
    _ui->holdSpinner->setEnabled(animSet);
    _ui->frameNameLabel->setEnabled(animSet);
    _ui->copyFrameButton->setEnabled(animSet);
    _ui->addFrameButton->setEnabled(animSet);
    _ui->deleteFrameButton->setEnabled(animSet);
    _ui->fpsLabel->setEnabled(animSet);
    _ui->fpsSpinner->setEnabled(animSet);

    // Playback widgets
    _ui->playButton->setEnabled(animSet);
    _ui->beginningButton->setEnabled(animSet);
    _ui->endButton->setEnabled(animSet);
    _ui->loopButton->setEnabled(animSet);

    // Change the widget values depending upon an Animation being set
    if (animSet) {
        // Set an XSheet, connect the tocks, and set some default values
        _timeline->setXSheet(anim->xsheet());

        // Connect all ticks to onTickSelected() slot
        QListIterator<Tick *> ticks(_timeline->ticks());
        while (ticks.hasNext())
            connect(ticks.next(), &Tick::selected, this, &TimelineWindow::onTickSelected);

        // Last few widget settings
        _ui->fpsSpinner->setValue(anim->xsheet()->FPS());        // Set FPS value
        _checkDisableDeleteFrame();                            // Check to disable to delete frame button

        connect(anim->xsheet(), &XSheet::frameMoved, this, &TimelineWindow::_onTimedFrameMoved);

        // connect the right bracket marker from the timeline
        connect(_timeline->rightBM(), &BracketMarker::seqNumOverChanged, this, &TimelineWindow::_onRightBracketMarkerSeqNumOverChanged);
        connect(_timeline->leftBM(), &BracketMarker::seqNumOverChanged, this, &TimelineWindow::_onLeftBracketMarkerSeqNumOverChanged);
    } else {
        disconnect(_timeline->xsheet(), 0, this, 0);        // TODO should not be doing this...

        // Set the Widgtets to the "Clear," mode
        _ui->frameNumLabel->setText(tr("No Animation set."));
        _ui->holdSpinner->clear();
        _ui->fpsSpinner->clear();
        _timeline->setXSheet(NULL);    // Change out the Timeline's XSheet last, would be a Null XSheet
    }    
}


void TimelineWindow::_onHoldSpinnerChanged(int hold) {
    // If the spin box is changed, then adjust the hold of the current frame
    BlitApp *bApp = BlitApp::app();

    // Do nothing if an animtion isn't set
    if (bApp->anim() == NULL)
        return;
    
    // Or if playing animation
    if (_playingAnim)
        return;

    // Only if we aren't moving the cursor, change the spinner value
    if (!_changingFrames) {
        // First compare the values of the current hold to the last
        TimedFrame *curTimedFrame = bApp->curTimedFrame();
        int oldHold = curTimedFrame->hold();
        Cursor *tCursor = _timeline->timelineCursor();

        // Set the hold
        curTimedFrame->setHold(hold);
        int endSeqLen = curTimedFrame->seqNum() + curTimedFrame->hold();

        // If the cursor isn't over the frame after the hold adjustment
        if (tCursor->seqNumOver() >= endSeqLen) {
            // Move the cursor to the new last spot
            tCursor->moveToSeqNum(endSeqLen - 1);
        }

        // Save the animation
        bApp->saveAnim();
    }
}


//void TimelineWindow::_onFrameNameEditChanged(QString name) {
//    // If if the line edit for the frame name is changed, set the name for the frame.
//    // Do nothing if an animtion isn't set
//    if (BlitApp::app()->anim() == NULL)
//        return;
//    
//    // Only if we aren't moving the cursor, change the spinner value
//    if (!_changingFrames) {
//        BlitApp::app()->curTimedFrame()->frame()->setName(name);
//        BlitApp::app()->saveAnim();
//    }
//}


void TimelineWindow::onTickSelected() {
    // When a Tick/Frame has been selected (or is set to the current). this function will be called.
    Tick *tick = qobject_cast<Tick *>(sender());

    // Make sure there is a XSheet first
    if (!BlitApp::app()->xsheet())
        return;

    // Make sure not to move the Frames
    _changingFrames = true;

    // Set the widget values
    TimedFrame *tf= tick->timedFrame();
    _ui->frameNameLabel->setText(tf->frame()->name());
    _ui->frameNumLabel->setText(QString::number(tf->seqNum()));
    _ui->holdSpinner->setValue(tf->hold());

    _changingFrames = false;
}


void TimelineWindow::_onCopyFrameClicked(bool checked) {
    // Will add a Frame to the Animation, but will copy the current Frame.  The hold value
    // of the Frame will be set to 1.
    playAnimation(false);
    Frame *curFrame= BlitApp::app()->curTimedFrame()->frame();
    _addTimedFrameToAnimation(new TimedFrame(curFrame->copy()));                        // will also save animation for us
    _checkDisableDeleteFrame();
}


void TimelineWindow::_onAddFrameClicked(bool checked) {
    // Will add a new Frame to the Animation;
    playAnimation(false);
    _addTimedFrameToAnimation(_mkNewFrame());    // will also save animation for us
    _checkDisableDeleteFrame();
}


void TimelineWindow::_onDeleteFrameClicked(bool checked) {
    // Will delete the Frame/Tick that the cursor is currently over

    // Check for no XSheet
    // TODO see funciton TimelineWindow::_addTimedFrametoAnimation() to see why this is a major problem
    if (BlitApp::app()->anim() == NULL)
        return;

    playAnimation(false);

    // Vars
    XSheet *xsheet = BlitApp::app()->xsheet();
    int overSeq = _timeline->timelineCursor()->seqNumOver();

    // Bounds check
    if (overSeq > xsheet->seqLength())
        overSeq = xsheet->seqLength();
    
    // Do the removal, slots/signals should be taken care of,  Cleanup Cels
    TimedFrame *tf = xsheet->removeFrame(overSeq);
    tf->frame()->removeCelFiles();
    delete tf->frame();
    delete tf;

    // Save the file
    BlitApp::app()->saveAnim();
    
    // Disblae widget possibly
    _checkDisableDeleteFrame();
}


void TimelineWindow::_onFPSSpinnerChanged(int fps) {
    // Changes the FPS of the animaion
    // Check for a set Animation
    if (BlitApp::app()->anim() == NULL)
        return;
    
    playAnimation(false);
    
    BlitApp::app()->xsheet()->setFPS(fps);
    _adjustTimingLabel(BlitApp::app()->curSeqNum());
    BlitApp::app()->saveAnim();
}


void TimelineWindow::playAnimation(bool play) {
    // Will either play or stop playing the Animation.  set play=true to Play it, set play=false to
    // stop playing (default is true).  Will do nothing if you try to play/stop, if its already playing
    // or stopped.

    // Prelim check
    // TODO see other functions about how the below statement is a problem
    if (BlitApp::app()->anim() == NULL)
        return;
    
    // If we are already playing or not, just ignore
    if (_playingAnim == play)
        return;

    // First check if we are at the end of the timeline, reset it
    if (play && (_timeline->timelineCursor()->seqNumOver() == _playbackTimeline->endFrame())) {
        _timeline->timelineCursor()->moveToSeqNum(1);
    }
    
    // Else, must be a new command
    _playingAnim = play;
    _loopDone = false;
    if (_playingAnim) {
        // Start from first frame and loop
        _setupPlayback(_timeline->timelineCursor()->seqNumOver());
        _playbackTimeline->start();
    } else {
        // // Stop playback
        _ui->playButton->setChecked(false);
        _playbackTimeline->stop();
    }

    // Set some other GUI stuff
    _ui->holdSpinner->setEnabled(!_playingAnim);
    _ui->fpsSpinner->setEnabled(!_playingAnim);
    _ui->copyFrameButton->setEnabled(!_playingAnim);
    _ui->addFrameButton->setEnabled(!_playingAnim);
    _ui->deleteFrameButton->setEnabled(!_playingAnim);
    _ui->selectivePlaybackButton->setEnabled(!_playingAnim);

    // Emit the signal
    emit animationPlaybackStateChanged(_playingAnim);
}


void TimelineWindow::_onPlayClicked(bool checked) {
    // Will play the animation sequence (on loop) if the button is checked.  If checked=false
    // then it will mean that you want to stop playing the animaiton;
    playAnimation(checked);
}


void TimelineWindow::_onBeginningClicked(bool checked) {
    // Whcn the _beginningButton widget is clicked, this will move the cursor to the beginning
    // of the animation and make the first Frame current.
    playAnimation(false);

    // change where the end is if selective playback is on or not
    if (_ui->selectivePlaybackButton->isChecked())
        _timeline->timelineCursor()->moveToSeqNum(_timeline->leftBM()->seqNumOver());
    else
        _timeline->selectTickByIndex(0);
}


void TimelineWindow::_onEndClicked(bool checked) {
    // When the _endButton widget is clicked, this will move the cursor to the end of the animation
    // and make the Frame current.
    playAnimation(false);

    // Change there the end is if selective playback is on or not
    if (_ui->selectivePlaybackButton->isChecked())
        _timeline->timelineCursor()->moveToSeqNum(_timeline->rightBM()->seqNumOver() + 1);
    else {
        _timeline->selectTickByIndex(_timeline->ticks().size() - 1);
        _timeline->timelineCursor()->moveToSeqNum(BlitApp::app()->xsheet()->seqLength() + 1);
    }
}


/*!
    Triggered via the Ui's selectivePlaybackButton member's QToolButton::clicked() signal, this will
    toggle on/off selective playback mode.  set \a checked to true or it on, or false for off.
*/
void TimelineWindow::_onSelectivePlaybackButtonClicked(bool checked) {
    _timeline->turnOnSelectivePlayback(checked);
}


/*!
    When the Right BracketMarker's BracketMarker::seqNumOver() signal is tripped, this will
    adjust the QTimeline that is being used for playback if selective playback is on (and
    playback is happening).
*/
void TimelineWindow::_onRightBracketMarkerSeqNumOverChanged(int seqNum) {
    if (_playingAnim && _ui->selectivePlaybackButton->isChecked()) {
        _setupPlayback(_playbackTimeline->currentFrame());
    }
}


/*!
    When the Left BracketMarker's BracketMarker::seqNumOver() signal is tripped, this will
    adjust the QTimeline that is being used for playback if selective playback is on (and
    playback is happening).

    A case for this if the marker is moved before the cursor while playback is happening.
    We would want the cursor to be adjusted.
*/
void TimelineWindow::_onLeftBracketMarkerSeqNumOverChanged(int seqNum) {
    if (_playingAnim && _ui->selectivePlaybackButton->isChecked()) {
        if (seqNum > _playbackTimeline->currentFrame())
            _setupPlayback(seqNum);
    }
}



/*!
    Will move the Cursor forward or backward dependant upon the number provided.
    To go forward, supply a positive integer, to go back give a negative one.
    Nothing will happen if 0 is provided.  Will change the current Frame and 
    stop playing the current Animation if a non-zero value is provided.  Will
    wrap around if the number provided excends the bounds.
*/
void TimelineWindow::goBy(int numFrames) {
    // First checks
    if (numFrames == 0)
        return;
    playAnimation(false);

    
    XSheet *xsheet = BlitApp::app()->xsheet();
    QList<QPointer<TimedFrame>> frames = xsheet->frames();
    int curTimedFrameIndex = frames.indexOf(BlitApp::app()->curTimedFrame());
    TimedFrame *tf = NULL;

    // Calculate where to go (with wrapping)
    int dest = (curTimedFrameIndex + numFrames) % frames.size();
    if (dest <= -1)
        dest = frames.size() + dest;

    // go there
    tf = frames[dest];
    _timeline->selectTickByTimedFrame(tf);
    _timeline->timelineCursor()->moveToTimedFrame(tf);

//    // All go, do the move
//    seq = BlitApp::app()->curTimedFrame()->seqNum() - 1;
//
//    // Pick a Frame
//    if (seq <= 0)
//        frame = BlitApp::app()->xsheet()->frames().last();
//    else
//        frame = BlitApp::app()->xsheet()->frameAtSeq(seq);
//
//    // Move there
//    _timeline->selectTickByFrame(frame);
//    _timeline->timelineCursor()->moveToFrame(frame);
}


bool TimelineWindow::isAnimationPlaying() {
    // Small accessors method to check of the Animation is playing or not
    return _playingAnim;
}


TimedFrame *TimelineWindow::_mkNewFrame(int hold) {
    // Internal function used to create a new new Frame object for the Timeline.  Will also add a Cel
    // for you of the same size as the Animation.  hold should be a positive integer.  Will return a
    // pointer to a Frame. 
    Animation *anim = BlitApp::app()->anim();
    Frame *frame = new Frame(anim);
    frame->addCel(new CelRef(new PNGCel(anim, "", BlitApp::app()->frameSize())));
    TimedFrame *tf = new TimedFrame(frame);
    tf->setHold(hold);

    return tf;
}


void TimelineWindow::_setupPlayback(int startSeq) {
    // Internal function to both create a new QTimeLine object for the _playbackTimeline, connect
    // it up, and start it up.  Will cleanup any other QTimeLines in existance.  Will start changing
    // Frames/Ticks.

    // Make code cleaner
    XSheet *xsheet = BlitApp::app()->xsheet();
    Cursor *cursor = _timeline->timelineCursor();
    int endSeq = xsheet->seqLength() + 1;

    // Use selective playback or not?
    if (_ui->selectivePlaybackButton->isChecked()) {
        startSeq = _timeline->leftBM()->seqNumOver();
        endSeq = _timeline->rightBM()->seqNumOver() + 1;
    }
    
    // Fixes a playback bug when the cursor is at the end
    if (startSeq >= xsheet->seqLength())
        startSeq = xsheet->seqLength() - 1;

    // Do Math for the time left
    qreal msPerSeq = 1000.0 / (qreal)xsheet->FPS();
    qreal duration = msPerSeq * (endSeq - startSeq);
    
    // Setup the duration and ranges for the timeline:w
    _playbackTimeline->setDuration(duration);
    _playbackTimeline->setFrameRange(startSeq, endSeq);
//    _playbackTimeline->setLoopCount(_ui->loopButton->isChecked() ? 0 : 1);
    _playbackTimeline->setUpdateInterval(10);
    _playbackTimeline->setCurveShape(QTimeLine::LinearCurve);
}


void TimelineWindow::_checkDisableDeleteFrame() {
    // Internal function to check if the _deleteFrameButton should be disabled or not.  This depends on
    // the number of frames currently set in the XSheet.
    if (!BlitApp::app()->xsheet())
        return;

    _ui->deleteFrameButton->setEnabled(BlitApp::app()->xsheet()->numFrames() > 1);
}


void TimelineWindow::_addTimedFrameToAnimation(TimedFrame *tf) {
    // Internal function.
    // This is the function that will actually add the Frame to the Animation/XSheet.
    // The frame will be added to the closest inbetween of two ticks/frames; this also includes
    // the start and end of the animation sequence.
    //
    // Say we have a sequence of three frames, with a length of 9:
    //   1    2    3    4    5    6    7    8   9
    // [             ][                  ][       ]
    //
    // If the cursor was over sequence numbers 4 or 5, the new frame would be inserted inbetween
    // frames 1 and 2.  If it was over 6 and 7, then it would go between frames 2 and 3.  In a case
    // Where a cursor is over a frame with an odd hold (e.g. frame #1 in this case), the middle sequence
    // number should go to the right: over seq. num. 1 means an add to the start of the animation, over
    // seq nums 2 and 3 should be a insert between frames 1 and 2.

    // Check for no XSheet
    // TODO this can cause issues because an Animation is considered Null if the XSheet is Null,
    //      and thus the XSheet is considered Null if it has no frames, this is a serious problem
    if (BlitApp::app()->anim() == NULL)
        return;

    // Variables
    XSheet *xsheet = BlitApp::app()->xsheet();
    int overSeq = _timeline->timelineCursor()->seqNumOver();
    int at = -1;

    // Check where the cursor is and figure out a position to add the Frame
    if (overSeq < xsheet->seqLength()) {
        // Within bounds of the current sequence, split of the seq nums for that frame
        TimedFrame *overFrame = xsheet->frameAtSeq(overSeq);
        QList<int> seqNums = overFrame->seqNums();
        QList<int> firstHalf = seqNums.mid(0, seqNums.size() / 2);

        // If we are in the first half, then insert it before, else, insert it after
        at = (firstHalf.contains(overSeq)) ? overFrame->seqNum() : (overFrame->seqNum() + overFrame->hold());
    } else
        at = xsheet->seqLength() + 1;        // Pop it onto the end

    // Make and add the Frame (takes care of the Tick too)
    xsheet->addFrame(tf, at);

    // Connect up the Tick, add a slot/signal and select it
    Tick *newTick = _timeline->tickByTimedFrame(tf);
    connect(newTick, &Tick::selected, this, &TimelineWindow::onTickSelected);
    newTick->select();

    // Save the file
    BlitApp::app()->saveAnim();
}


/*!
    Will set the Timing Label (HH:MM:SS:FF) to dispaly the correct values for \a seqNum.
*/
void TimelineWindow::_adjustTimingLabel(quint32 seqNum) {
    // Check for an XSheet first
    XSheet *xsheet = BlitApp::app()->xsheet();
    if (!xsheet)
        return;

    seqNum -= 1;
    quint32 fps = xsheet->FPS();
    quint32 fpm = fps * 60;
    quint32 fph = fpm * 60;

    _ui->timingLabel->setText(QString("%1:%2:%3:%4")
        .arg(seqNum / fph, 1, 10, QChar('0'))
        .arg(seqNum / fpm, 2, 10, QChar('0'))
        .arg(seqNum / fps, 2, 10, QChar('0'))
        .arg((seqNum % fps) + 1, 2, 10, QChar('0')));
}



void TimelineWindow::_onPlaybackTimelineFrameChanged(int frame) {
    // Internal slot for the _playbackTimeline when a frame is changed.  Should advance the cursor
    // through the timeline and possibly change to the next frame.  It is assumed that the animation
    // is currently playing.

    // Pull out the cursor object, and manipulate it
    Cursor *cursor = _timeline->timelineCursor();
    cursor->moveToSeqNum(frame);
    cursor->getTickOver()->select();                // Won't always do something
}


void TimelineWindow::_onPlaybackTimelineFinished() {
    // This is a slot to be called when the _playbackTimeline has reported a "finished," signal, but
    // we still need to play the animation.  I.e. the QTimeLine wasn't set to loop, but we want it
    // to.  This slot shouldn't be called if the user stops playing the animaiton.
    if (_ui->loopButton->isChecked()) {
        _setupPlayback(1);
        _playbackTimeline->start();
    } else {
        // Un-check the play button
        _ui->playButton->setChecked(false);

        // Fix the GUI
        _ui->holdSpinner->setEnabled(true);
        _ui->fpsSpinner->setEnabled(true);
        _ui->copyFrameButton->setEnabled(true);
        _ui->addFrameButton->setEnabled(true);
        _ui->deleteFrameButton->setEnabled(true);
        _ui->selectivePlaybackButton->setEnabled(true);

        _playingAnim = false;
        emit animationPlaybackStateChanged(_playingAnim);
    }
}


/*!
    Called by the BlitApp::onCurSeqNumChanged() signal.  Will adjust the timing label to represent
    the current sequence number in a H:MM:SS:FF format
*/
void TimelineWindow::_onCurSeqNumChanged(quint32 seqNum) {
    _adjustTimingLabel(seqNum);
}


/*!
    Called by XSheet::frameMoved() signal that the TimeineWindow is attached to, this will adjust
    the timingLabel widget.
*/
void TimelineWindow::_onTimedFrameMoved() {
    _adjustTimingLabel(_timeline->timelineCursor()->seqNumOver());
}

