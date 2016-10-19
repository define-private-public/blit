// File:         celswindow.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  source implementation for the Cels Window


/*!
    \inmodule Widgets
    \class CelsWindow
    \brief CelsWindow displays a list of CelRef / StagedCelWidget for the currently loaded Frame.

    CelsWindows pretty much does that.  It also features buttons for adding, deleting, and copying
    Cels.
*/


#include "widgets/celswindow.h"
#include "ui_cels_window.h"
#include "blitapp.h"
#include "animation/pngcel.h"
#include "animation/celref.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include "widgets/stagedcelwidget.h"
#include <QToolButton>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QDebug>



/*!
    Constrcuts the CelsWindow.  \a parent must be BlitApp.
*/
CelsWindow::CelsWindow(BlitApp *blitApp, QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::CelsWindow)
{
    // Make a scene
    _scene = new QGraphicsScene(this);

    // first things first.
    _ui->setupUi(this);
    _ui->view->setScene(_scene);
    _enableWidgets(false);

    // signals & slots
    connect(blitApp, &BlitApp::curCelRefChanged, this, &CelsWindow::onCurCelRefChanged);
    connect(blitApp, &BlitApp::animationPlaybackStateChanged, this, &CelsWindow::_onAnimationPlaybackStateChanged);
    connect(_ui->copyCelButton, &QToolButton::clicked, this, &CelsWindow::_onCopyCelButtonClicked);
    connect(_ui->addCelButton, &QToolButton::clicked, this, &CelsWindow::_onAddCelButtonClicked);
    connect(_ui->deleteCelButton, &QToolButton::clicked, this, &CelsWindow::_onDeleteCelButtonClicked);

    // Window Frame data
    setWindowTitle("Cels");
    setFixedSize(STAGED_CEL_WIDGET_WIDTH + 20, 400);            // TODO change

    qDebug() << "[CelsWindow created]";
}


/*!
    Destroys the Cels Window, not much to it.
*/
CelsWindow::~CelsWindow() {
    delete _ui;
    qDebug() << "[CelsWindow destroyed]";
}


/*!
    Overloaded function.

    Re-implemented to make sure the window doesn't close on the "X," button
    clicked
*/
void CelsWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}


/*!
    Call this either to enable or disable all of the widgets.  default for \a
    enable is true.  Should be called if there are Cels in the Frame or not.
*/
void CelsWindow::_enableWidgets(bool enable) {
    _ui->copyCelButton->setEnabled(enable);
    _ui->addCelButton->setEnabled(enable);
    _ui->deleteCelButton->setEnabled(enable);
}


/*!
    Internal funciton used to add more Cels to the Cel widget.  This method will be called
    when the user either wants to add a new blank Cel or copy the current one.  Cels will
    always be added to the top of the current one.  This is called via the "addCell" button's
    clicked() signal.

*/
void CelsWindow::_addCel(CelRef *ref) {
    // Put it ontop of the current cel, or just ontop if there isn't one
    int to = FRAME_TOP_LAYER;
    if (_curRef)
        to = _curFrame->cels().indexOf(_curRef);

    _curFrame->addCel(ref, to);        // Signal will call _onFrameCelAdded()
}


/*!
    Adds a separator to the scene after index \a i.
*/
void CelsWindow::_addSeparator(int i) {
    int y = (STAGED_CEL_WIDGET_HEIGHT * i) + ((i - 1) * STAGED_CEL_WIDGET_SEPARATOR_HEIGHT);
    QGraphicsRectItem *rect = new QGraphicsRectItem(0, y, STAGED_CEL_WIDGET_WIDTH, STAGED_CEL_WIDGET_SEPARATOR_HEIGHT);
    rect->setPen(Qt::NoPen);
    rect->setBrush(Qt::gray);
    rect->setZValue(10);        // Put it above the CelWidgets

    // Add to list and scene
    _scene->addItem(rect);
    _separators.append(rect);
}


/*!
     Computes a Y value for the given index \i.  This is only for the StagedCelWidgets.
     I don't recommend doing this for the separator.
*/
qreal CelsWindow::_yValueForIndex(int i) {
    return (STAGED_CEL_WIDGET_HEIGHT + STAGED_CEL_WIDGET_SEPARATOR_HEIGHT) * i;
}


/*!
    Used to set the Frame that is currently be held by the window.  If the supplied pointer to
    the Frame is already set, this will do nothing.  If a NULL pointer is supplied, this will
    clear out the widget and deactivate the signals.  By default, the widget has no Frame set.
    Setting a new Frame will delete all of the other widgets and clear out the _scene.
*/
void CelsWindow::setFrame(TimedFrame *tf) {
    Frame *frame = tf->frame();

    if (_curFrame != frame) {
        // Clear it all
        _scWidgets.clear();
        _scwMap.clear();
        _proxies.clear();
        _proxyMap.clear();
        _separators.clear();
        _scene->clear();
        _curRef = NULL;
        _curSCW = NULL;

        // Disconnect the old frame
        if (_curFrame)
            disconnect(_curFrame, 0, this, 0);

        // Set Frame & connect
        _curFrame = frame;
        connect(_curFrame, &Frame::destroyed, this, &CelsWindow::_onFrameDestroyed);
        connect(_curFrame, &Frame::celAdded, this, &CelsWindow::_onFrameCelAdded);
        connect(_curFrame, &Frame::celRemoved, this, &CelsWindow::_onFrameCelRemoved);
        connect(_curFrame, &Frame::celMoved, this, &CelsWindow::_onFrameCelMoved);

        // Create the StagedCelWidgets (top to bottom)
        QList<CelRef *> refs = _curFrame->cels();
        int i = 0, scwWidth = 0, scwHeight = 0;
        for (auto iter = refs.begin(); iter != refs.end(); iter++) {
            // Create the widget w/ proxy
            CelRef *cr = *iter;
            StagedCelWidget *scw = new StagedCelWidget(cr);
            scw->index = i;
            scw->setEnabled(!_isPlayingBack);
            scwWidth = scw->width(), scwHeight = scw->height();
            QGraphicsProxyWidget *proxy = _scene->addWidget(scw);
            proxy->setPos(0, _yValueForIndex(i));

            // Add it to the cache
            _scWidgets.append(scw);
            _scwMap.insert(cr, scw);
            _proxies.append(proxy);
            _proxyMap.insert(cr, proxy);

            // Signals & Slots
            connect(scw, &StagedCelWidget::mousePressed, this, &CelsWindow::_onCelWidgetPressed);
            connect(scw, &StagedCelWidget::raiseButtonClicked, this, &CelsWindow::_onCelWidgetRaiseButtonClicked);
            connect(scw, &StagedCelWidget::lowerButtonClicked, this, &CelsWindow::_onCelWidgetLowerButtonClicked);

            // Inc counter
            i++;
        }

        // Add the separators
        int numCels = _curFrame->numCels();
        if (numCels >= 2) {
            for (int i = 1; i < numCels; i++)
                _addSeparator(i);
        }

        // Adjust the scene size
        _scene->setSceneRect(0, 0, scwWidth, refs.size() * (scwHeight + STAGED_CEL_WIDGET_SEPARATOR_HEIGHT));
    }

    // Enable or disable some UI elements
    if (_curFrame) {
        bool showCopyAndDelete = (_curFrame->numCels() != 0) && !_isPlayingBack;

        _enableWidgets(true && !_isPlayingBack);
        _ui->copyCelButton->setEnabled(showCopyAndDelete);
        _ui->deleteCelButton->setEnabled(showCopyAndDelete);
    } else
        _enableWidgets(false);
}


/*!
    Triggered via BlitApp::animationPlaybackStateChanged() signal, this will turn
    off a few UI elements if \a isPlaying is true, but if it's false, it will then
    re-enable them.

    Will emit the animationPlaybackStateChanged() signal
*/
void CelsWindow::_onAnimationPlaybackStateChanged(bool isPlaying) {
    // Set a state variable
    _isPlayingBack = isPlaying;

    // Turn on/off the buttons
    _enableWidgets(!_isPlayingBack);
    for (auto iter = _scWidgets.begin(); iter != _scWidgets.end(); iter++)
        (*iter)->setEnabled(!_isPlayingBack);

    // Send out a signal
    emit animationPlaybackStateChanged(!_isPlayingBack);
}


/*!
    In the event that the _curFrame variable is deleted by something else (e.g. loading another
    animation, this will reset the variable, as well as the widget.  this is called by the 
    Frame::destroyed() signal.
*/
void CelsWindow::_onFrameDestroyed(QObject *obj) {
    _curFrame = NULL;
    _curRef = NULL;
    _curSCW = NULL;
    _scWidgets.clear();
    _scwMap.clear();
    _proxies.clear();
    _proxyMap.clear();
    _separators.clear();
    _scene->clear();
    
    // Empty scene rect
    _scene->setSceneRect(0, 0, 0, 0);

    // Disable the widgets
    _enableWidgets(false);
}


/*!
    Called via BlitApp::curCelRefChanged(), this will deslect the current widget and select
    the precently pressed one.
*/
void CelsWindow::onCurCelRefChanged(CelRef *ref) {
    // first unselect the previous, then select the next
    if (_curRef)
        _curSCW->deselect();

    // Select the new (or not if NULL)
    if (ref) {
        _curSCW = _scwMap[ref];
        _curSCW->select();
    } else
        _curSCW = NULL;

    // Swap over
    _curRef = ref;
}


/*!
    Signal is tripped via StagedCelWidget::mousePressed().  This will notify BlitApp
    to set a new CelRef
*/
void CelsWindow::_onCelWidgetPressed() {
    // Select the one sending the event
    StagedCelWidget *scw = qobject_cast<StagedCelWidget *>(sender());
    if (scw)
        BlitApp::app()->setCurCelRef(scw->ref());
}


/*!
    Will raise the currently select CelRef a layer.  This is called
    by the StagedCelWidget's QToolButton::clicked() signal.
*/
void CelsWindow::_onCelWidgetRaiseButtonClicked() {
    // Get the index of the current Cel,  CelWidget order matches w/ Frame Cel order
    StagedCelWidget *scw = qobject_cast<StagedCelWidget *>(sender());
    int index = scw->index;
    if (scw->index > 0)
        _curFrame->moveCel(index, index - 1);            // This trips a signal
}


/*!
    Will lower the currently select CelRef a layer.  This is called
    by the StagedCelWidget's QToolButton::clicked() signal.
*/
void CelsWindow::_onCelWidgetLowerButtonClicked() {
    // Get the index of the current Cel,  CelWidget order matches w/ Frame Cel order
    StagedCelWidget *scw = qobject_cast<StagedCelWidget *>(sender());
    int index = scw->index;
    if ((index + 1) < _curFrame->numCels())
        _curFrame->moveCel(index, index + 1);        // Trips a signal
}


/*!
    Triggered via Frame::celAdded() signal.  This will create a new widget for the newly created
    Cel.  Will also make the newly added Cel current
*/
void CelsWindow::_onFrameCelAdded(CelRef *ref) {
    // Get the index
    int at = -1;
    if (!_curSCW) {
        // No other cels in the Frame
        at = 0;
    } else {
        // Must be the index of the (previous) current one
        at = _curSCW->index;
    }

    // Create the new objects
    StagedCelWidget *scw = new StagedCelWidget(ref);
    scw->index = at;
    QGraphicsProxyWidget *proxy = _scene->addWidget(scw);

    // Update the containsers
    _scWidgets.insert(at, scw);
    _proxies.insert(at, proxy);
    _scwMap.insert(ref, scw);
    _proxyMap.insert(ref, proxy);

    // update everything below the index
    int numCels = _scWidgets.size();
    for (int i = at; i < numCels; i++) {
        _scWidgets[i]->index = i;
        _proxies[i]->setPos(0, _yValueForIndex(i));
    }

    // Add a separator
    if (numCels >= 2)
        _addSeparator(numCels - 1);

    _scene->setSceneRect(0, 0, scw->width(), numCels * (scw->height() + STAGED_CEL_WIDGET_SEPARATOR_HEIGHT));

    // Connect signals and slots
    connect(scw, &StagedCelWidget::mousePressed, this, &CelsWindow::_onCelWidgetPressed);
    connect(scw, &StagedCelWidget::raiseButtonClicked, this, &CelsWindow::_onCelWidgetRaiseButtonClicked);
    connect(scw, &StagedCelWidget::lowerButtonClicked, this, &CelsWindow::_onCelWidgetLowerButtonClicked);

    // Tell the master module, (will also select it)
    BlitApp::app()->setCurCelRef(ref);
    BlitApp::app()->saveAnim();                // Save

    // Turn some buttons back on
    _ui->deleteCelButton->setEnabled(true);
    _ui->copyCelButton->setEnabled(true);
}


/*!
    Triggered via the Frame::celRemoved() signal.  This will remove that CelRef's widget from
    the Cels Window and set a new current CelRef (if applicable)
*/
void CelsWindow::_onFrameCelRemoved(CelRef *ref) {
    // Get the index
    StagedCelWidget *scw = _scwMap[ref];
    int at = scw->index;

    // Remove the widget & proxy from the containers
    _scWidgets.removeAt(at);
    _proxies.removeAt(at);
    _scwMap.remove(ref);
    QGraphicsProxyWidget *proxy = _proxyMap.take(ref);
    _scene->removeItem(proxy);

    // Update the widgets at/below the index
    int numCels = _scWidgets.size();
    for (int i = at; i < numCels; i++) {
        _scWidgets[i]->index = i;
        _proxies[i]->setPos(0, _yValueForIndex(i));
    }

    // Remove a separator
    if (!_separators.isEmpty()) {
        QGraphicsRectItem *rect = _separators.takeLast();
        _scene->removeItem(rect);
        delete rect;
    }

    // Set the scene rect
    _scene->setSceneRect(0, 0, scw->width(), numCels * (scw->height() + STAGED_CEL_WIDGET_SEPARATOR_HEIGHT));

    // If the ref was the current one, find a new one
    if (_curRef == ref) {
        // No cels?  no new ref
        if (numCels == 0) {
            // NULL out
            _curRef = NULL;
            _curSCW = NULL;
        } else {
            // Set it to the same index as the removed ref
            at = (at < numCels) ? at : (numCels - 1);
            _curRef = _curFrame->cels()[at];
            _curSCW = _scwMap[_curRef];
        }
    }

    // Save
    BlitApp::app()->setCurCelRef(_curRef);
    BlitApp::app()->saveAnim();

    // Delete
    delete proxy;

    // Disable buttons?
    _ui->deleteCelButton->setEnabled(_curFrame->numCels() != 0);
    _ui->copyCelButton->setEnabled(_curFrame->numCels() != 0);
}


/*!
    When a Cel is moved, this will tell the widget to redraw and reposition cel
    widgets.  Will save the current Animation files.  Supplying NULL to this
    function will just cause the widget to refresh which it totally fine.

    Called via Frame::celMoved() signal.
*/
void CelsWindow::_onFrameCelMoved(CelRef *ref) {
    // Clear the lists, (but not the maps or objects)
    QList<CelRef *> refs = _curFrame->cels();
    _scWidgets.clear();
    _proxies.clear();

    // Set it all again
    int i = 0;
    for (auto iter = refs.begin(); iter != refs.end(); iter++) {
        // Set containters
        CelRef *ref = *iter;
        QGraphicsProxyWidget *proxy = _proxyMap[ref];
        StagedCelWidget *scw = _scwMap[ref];
        scw->index = i;
        _scWidgets.append(scw);
        _proxies.append(proxy);

        // Move proxy
        proxy->setPos(0, _yValueForIndex(i));

        // Inc counter
        i++;
    }

    BlitApp::app()->saveAnim();
}


/*!
    Slot that will get triggered when the Copy Cel Button is clicked.  Will
    enable the delete cel button.
*/
void CelsWindow::_onCopyCelButtonClicked(bool checked) {
    // TODO should not just copy PNGCels
    if (_curRef) {
        // Copy over only PNGCels
        Cel *cel = _curRef->cel();
        if (cel->type() == PNG_CEL_TYPE) {
            CelRef *cr = new CelRef(((PNGCel *)cel)->copy());
            cr->setPos(_curRef->pos());
            _addCel(cr);
        }
    }
}


/*!
    Slot that will be triggered when the Add Cel Button is clicked.  Creates a new Cel of the
    current frame size.  Will enable the delete Cel Button
*/
void CelsWindow::_onAddCelButtonClicked(bool checked) {
    _addCel(new CelRef(new PNGCel(BlitApp::app()->anim(), "", BlitApp::app()->frameSize()), 0, 0));        // New PNGCel w/ new CelRef
}


/*!
    Slot that will be triggered when the Delete Cel Button is clicked.  Will delete the currntly
*/
void CelsWindow::_onDeleteCelButtonClicked(bool checked) {
    // selected Frame
    if (_curRef) {
        int at = _curFrame->cels().indexOf(_curRef);
        CelRef *ref = _curFrame->removeCel(at);

        // Only remove PNG if no other Refs point to it.
        if (ref->cel()->celRefs().size() == 0)
            ref->cel()->remove();
        delete ref;
    }
}

