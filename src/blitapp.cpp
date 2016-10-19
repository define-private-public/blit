
// File:         blitapp.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the BlitApp class


/*!
    \inmodule Core
    \class BlitApp
    \brief BlitApp is the "all seeing eye," god module of the application.

    Fear it
*/



#include "blitapp.h"
#include "animation/cellibrary.h"
#include "animation/cel.h"
#include "animation/celref.h"
#include "animation/pngcel.h"
#include "animation/framelibrary.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include "animation/animation.h"
#include "util.h"
#include "fileops.h"
#include "spritesheet.h"
#include "widgets/timelinewindow.h"
#include "widgets/toolswindow.h"
#include "widgets/celswindow.h"
#include "widgets/lighttablewindow.h"
#include "widgets/menubar.h"
#include "widgets/statusbar.h"
#include "widgets/animationproperties.h"
#include "widgets/editorcontainer.h"
#include "widgets/toolbox.h"
#include "widgets/colorpalette.h"
#include "widgets/drawing/canvas.h"
#include "tools/tool.h"
#include "tools/toolparameters.h"
#include <QSize>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QColorDialog>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>



/*!
    Singleton varaible used to ensure that BlitApp is well, a singleton.
*/
BlitApp *BlitApp::_app = NULL;


/*!
    The constructor for BlitApp.  It will setup everything for you, all you need to do is call
    the show() method.
*/
BlitApp::BlitApp() :
    QMainWindow()
{
    // Setup the Null Cel after BlitApp's been initialized

    // Singleton check
    if (_app) {
        qDebug() << "Error, BlitApp is already instantiated";
        return;
    }
    _app = this;

    // Setups a few tings
    Tools::setupHashDefaults();

    // Some dfeault variables
    _lastStillFilename = "";
    _lastStillFilter = FileOps::validFilters()[0];
    _zoom = 1.0;

    // For the Menu bar to add actions
    QList<QDockWidget *> docks;

    // Make the timeline widget a dock
    QDockWidget *dock = new QDockWidget("Timeline", this);
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    _timelineWnd = new TimelineWindow(this, dock);
    dock->setWidget(_timelineWnd);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    docks.append(dock);

    // Tools Window
    dock = new QDockWidget("Tools", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _toolsWnd = new ToolsWindow(this, dock);
    dock->setWidget(_toolsWnd);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    docks.append(dock);

    // Cels Window
    dock = new QDockWidget("Cels", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _celsWnd = new CelsWindow(this, dock);
    dock->setWidget(_celsWnd);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    docks.append(dock);

    // Other widgets
    dock = new QDockWidget("Light Table", this);
    dock->setFloating(true);
    dock->setAllowedAreas(Qt::NoDockWidgetArea);
    _ltWnd = new LightTableWindow(this, dock);
    dock->setWidget(_ltWnd);
    addDockWidget(Qt::NoDockWidgetArea, dock);
    docks.append(dock);
    dock->hide();            // Hidden by default

    _ec = new EditorContainer(this);
    setCentralWidget(_ec);
    _statusBar = new StatusBar(this);
    setStatusBar(_statusBar);
    _menuBar = new MenuBar(this, docks);
    setMenuBar(_menuBar);

    // Window stuff
    setWindowTitle("Blit");
    resize(1024, 640);
    move(40, 80);

    // Slots and signals
    connect(this, &BlitApp::zoomChanged, _canvas, &Canvas::onZoomChanged);
    connect(this, &BlitApp::frameSizeChanged, _canvas, &Canvas::onFrameSizeChanged);
    connect(this, &BlitApp::curTimedFrameChanged, _canvas, &Canvas::setFrame);
    connect(this, &BlitApp::curCelRefChanged, _canvas, &Canvas::onCurCelRefChanged);
    connect(this, &BlitApp::animLoaded, _timelineWnd, &TimelineWindow::setAnimation);
    connect(this, &BlitApp::curTimedFrameChanged, _celsWnd, &CelsWindow::setFrame);
    connect(_canvas, &Canvas::mousePressed, this, &BlitApp::_onCanvasPressed);
    connect(_canvas, &Canvas::mouseMoved, this, &BlitApp::_onCanvasMouseMoved);
    connect(_toolsWnd->toolbox(), &Toolbox::curToolChanged, this, &BlitApp::onCurToolChanged);
    connect(_timelineWnd, &TimelineWindow::animationPlaybackStateChanged, this, &BlitApp::_onAnimationPlaybackStateChanged);

    // Last things
    onCurToolChanged(_toolsWnd->toolbox()->curTool());
    _ltWnd->setCanvas(_canvas);
}


/*!
    BlitApp Deconstructor.  Will ensure that the currently loaded Animation will be cleaned up.
*/
BlitApp::~BlitApp() {
    _freeAnim();
}


/*!
    Returns the set frame size of the currently set Animaiton.  If an Animation ins't set, then 
    a Null QSize object will be returned.

    \sa setFrameSize() 
    \sa frameSizeChanged()
*/
QSize BlitApp::frameSize() {
    // Returns the size each frame should be,  will return a blank size if _anim is not set
    // TODO reevaluate if it should return a blank size or not (possibly 1x1 instead)
    if (!_anim)
        return QSize();
    else
        return _anim->frameSize();
}


/*!
    If no animation is currently loaded, this will return zero, otherwise it should
    return a postive integer that is within the bounds of [1, xsheet()->seqLength()]
*/
quint32 BlitApp::curSeqNum() {
    return _curSeqNum;
}


/*!
    Sets the size that each Frame object should be.  Convienece function for blit->anim()->setFrameSize()
    If no Aimation is loaded, then it won't do anything.

    \sa frameSize()
    \sa frameSizeChanged()
*/
void BlitApp::setFrameSize(QSize frameSize) {
    if (_anim)
        _anim->setFrameSize(frameSize);
}


/*!
    Does the actual loading of an animation.  \a path is the location of the Animation.  If the
    animation at path is currently loaded, this function will do nothing.  Please supply a valid
    path.  If the path isn't valid, nothing good will come to you.  Will first save the current
    Animaton.

    Will clear out the CelLibrary & FrameLlibrary upon successful calling.

    \sa animLoaded()
    \sa saveAnim()
    \sa savePalette()
    \sa saveAll()
    \sa SaveAs()
*/
bool BlitApp::load(QString path) {
    saveAll();
    
    bool okayToLoad = false;
    if (!_anim)
        okayToLoad = true;
    else 
        okayToLoad = (_anim->resourceDir() != path);
        

    if (okayToLoad) {
        // Need to set the project locaiton before anything else, (Mainly for Cel)
//        if (_anim) {
//            _anim->cl()->_clear();
//            _anim->fl()->_clear();
//        }

        // Check for a Null animation
        Animation *tmp = FileOps::loadAnimation(path);
        if (!tmp)
            return false;

        // Load the palette file
        QList<QColor> colors = FileOps::loadPalette(path);
        _toolsWnd->colorPalette()->clear();
        _toolsWnd->colorPalette()->addList(colors);

        // Oh hey, it's good, print a happy message
        qDebug() << "Loaded an Animation:" << tmp->name();
        qDebug() << "  " << tmp->xsheet()->numFrames() << "frames, with a sequence length of" << tmp->xsheet()->seqLength() << ".";

        // Set the window title
        onAnimationNameChanged(tmp->name());

        // Last bits of cleanup
        if (_anim) {
            disconnect(_anim, 0, this, 0);

            if (curFrame())
                curFrame()->saveCelFiles();
        }

        // Slots n' signals
        connect(tmp, &Animation::nameChanged, this, &BlitApp::onAnimationNameChanged);
        connect(tmp, &Animation::frameSizeChanged, this, &BlitApp::onFrameSizeChanged);

        // Last things
        Animation *oldAnim = _anim;                        // Out with the old
        _anim = tmp;                                    // In with the new
        emit animLoaded(tmp);                            // Emit
        setCurTimedFrame(xsheet()->frameAtSeq(1));            // Switch
        delete oldAnim;                                    // Delete

        // Even laster things
        _canvas->onFrameSizeChanged(_anim->frameSize());        // Adjust canvas size
        setCurSeqNum(1);

        return true;
    }
}


/*!
    Call this function to Save the Animation file.  Will do nothing unless the Animation is
    Set and isn't Null.

    \sa animLoaded()
    \sa load()
    \sa savePalette()
    \sa saveAll()
    \sa SaveAs()
*/
bool BlitApp::saveAnim(QString path) {
    if (_anim) {
        // Null Check
        if (_anim->isEmpty())
            return false;

        // Update
        _anim->update();

        // Different path or current working directory?
        if (path.isEmpty())
            path = _anim->resourceDir();

        // Save palette and animation
        return FileOps::saveAnimation(_anim, path);
    } else
        return false;
}


/*!
    Call this function to save the Palette file.  If no path is supplied, it is assumed
    to save it in the current working directory.
    
    Animation must be set (and not be Null) for the palette file to be saved

    \sa animLoaded()
    \sa load()
    \sa saveAnim()
    \sa saveAll()
    \sa SaveAs()
*/
bool BlitApp::savePalette(QString path) {
    if (_anim) {
        if (_anim->isEmpty())
            return false;

        // Different path or current working directory?
        if (path.isEmpty())
            path = _anim->resourceDir();
        
        // Save Palette
        QList<QColor> clrs(_toolsWnd->colorPalette()->colors());
        return FileOps::savePalette(clrs, path);
    } else
        return false;
}


/*!
    Call this method to perform a save to the supplied path.  If no path is provied, it will
    Assume to shave it to the current working directory.  Will save both palette an animation info.

    \sa animLoaded()
    \sa load()
    \sa saveAnim()
    \sa savePalette()
    \sa SaveAs()
*/
bool BlitApp::saveAll(QString path) {
    bool res = savePalette(path);
    res &= saveAnim(path);
    return res;
}


/*!
    Similar to saveAll(), but it will also change the project location to path.  It is assumed
    that path is a valid Blit Animation.  If Save to the new location fails, then the project 
    location won't change.  Will also save the current animation first.

    \sa animLoaded()
    \sa load()
    \sa saveAnim()
    \sa savePalette()
    \sa saveAll()
*/
bool BlitApp::saveAs(QString path) {
    // First save to current directory
    saveAll();

    // Then try a save to new directory
    if (_anim) {
        // First try to save it, then if its successful, load it up
        bool res = saveAll(path);
        if (res)
            res = load(path);

        return res;
    } else
        return false;
}


/*!
    Return a pointer to the current animation set.  Should not be NULL, but there is a change.

    \sa load()
*/
Animation *BlitApp::anim() {
    return _anim;
}


/*!
    Convience function for blit->anim()->xsheet().  Will return NULL if there isn't an Animation set
*/
XSheet *BlitApp::xsheet() {
    if (_anim)
        return _anim->xsheet();
    else
        return NULL;
}


/*!
    Sets the zoom value for the editor.  Will emit a signal on sucessful change.  It's recommended to 
    keep the zoom an integer.  Must be a positive real number.  Recommended to be between [1, 32],
    but can be outside that range if needed.

    \sa zoom()
    \sa zoomChanged()
*/
void BlitApp::setZoom(double zoom) {
    if (_zoom != zoom) {
        _zoom = zoom;
        emit zoomChanged(_zoom);
    }    
}


/*!
    Will set the curernt color to be the one provided, if it is valid.

    \sa curColor()
    \sa curColorChanged()
*/
void BlitApp::setCurColor(QColor clr) {
    if ((clr.isValid()) && (_curClr != clr)) {
        _curClr = clr;
        emit curColorChanged(_curClr);
    }
}


/*!
    Returns the current set zoom.

    \sa setZoom()
    \sa zoomChanged()
*/
double BlitApp::zoom() {
    return _zoom;
}


/*!
    Returns what the currently set color is.

    \sa setCurColor()
    \sa curColorChanged()
*/
QColor BlitApp::curColor() {
    return _curClr;
}


/*!
    Slot that gets tripped when the "New," button under the "File" menu is pressed.  Will stop
    playing the current animation and show the user a dialog.  May possilby load a new animation

    \sa onNewAnim()
    \sa onSaveAs()
    \sa load()
    \sa animLoaded()
*/
void BlitApp::onNewAnim() {
    // TODO move some of the code into FileOps
    //
    // Stop playing the animation if it currently is
    playAnimation(false);

    // Creates a new project/sequence.  Will launch a dialog window and prompt the user for other questions.
    QString path = QFileDialog::getSaveFileName(this, tr("New Animaton"), (_anim ? _anim->resourceDir() : ""));
    QFileInfo fi(path);
    
    // Check that a path was supplied, and it doesn't exist
    if (!path.isEmpty() && !fi.exists()) {
        // First make the directory
        QDir dir(path);
        bool res = dir.mkpath(path);

        // If it was a success, then make a new blank Animation, and save it there
        if (res) {
            // TODO refactor this into its own private function
            // Need to make some dummy object, adding a Frame to make sure that the XSheet/Animation aren't considered Null
            Animation *anim = new Animation();
            XSheet *xsheet = anim->xsheet();
            Frame *frame = new Frame(anim);
            PNGCel *cel = new PNGCel(anim, "", CEL_MIN_SIZE);                    // Blank/Emtpy Cel
//            frame->setHold(1);
            frame->addCel(new CelRef(cel));
            xsheet->addFrame(new TimedFrame(frame));

            // Set some new colors to the palette and save it
            QList<QColor> clrs;
            clrs.append(Qt::black);
            clrs.append(Qt::white);
            res = FileOps::savePalette(clrs, path);

            // Set properties, adjust the single cel (and delete it), and save the animation
            showAnimationProperties(anim);
            cel->resize(0, 0, anim->frameWidth(), anim->frameHeight());
            cel->remove();                                    // Removes PNG from current directory
            res &= FileOps::saveAnimation(anim, path);

            if (!res)
                qDebug() << "Error, couldn't save the new Animaiton to path=" << path;

            // Cleanup
            delete anim;    // Will take care of XSheet

            if (res) {
                if (!load(path))
                    qDebug() << "Error, couldn't load newly created Animation at path=" << path;
            }
        } else
            qDebug() << "Error, wasn't able to make a new Animation at path=" << path;
    }
}


/*!
    Will open up an existing Blit Animation file.  Will do nothing if the directory that is selecteed
    is the current one open.  Will stop playing the animaiton.  slot is activated when the
    "File > Open," button is pressed

    \sa onNewAnim()
    \sa onSaveAs()
    \sa load()
    \sa animLoaded()
*/
void BlitApp::onOpenAnim() {
    // Stop playing
    playAnimation(false);

    // Get path
    QDir animDir(_anim ? _anim->resourceDir() : "");
    QString path = QFileDialog::getExistingDirectory(this, tr("Open Animation"), animDir.absolutePath());

    // Check for a valid blit directory
    if (!path.isEmpty() && (path != animDir.absolutePath()) && FileOps::isBlitDir(path))
        load(path);
    else if (path == animDir.absolutePath())
        qDebug() << "Already open.";
    else if (path.isEmpty())
        return;                // Catch this so we don't print out extra debgging info
    else
        qDebug() << "Error, couldn't open Animation at path=" << path;
}


/*!
    will opena a dialog window and upon success of a valid directory selected, will save a
    copy of the current Animation there, and set it as the current project locaiton.  It is
    tripped by "File > Save As..."  Will stop playing the animation.

    \sa onNewAnim()
    \sa onOpenAs()
    \sa load()
    \sa animLoaded()
*/
void BlitApp::onSaveAs() {
    // Stop playing
    playAnimation(false);

    // Get path and save it under a new name
    QDir animDir(_anim ? _anim->resourceDir() : "");
    QString path = QFileDialog::getExistingDirectory(this, tr("Save Animation As"), animDir.absolutePath());

    if (!path.isEmpty()) {
        bool res = saveAs(path);

        if (!res)
            qDebug() << "Error, wasn't able to save Animation to:" << path;
    }
}


/*!
    Call this method when you want to quit.  Will close all the other widgets and save the sheet
*/
void BlitApp::shutdown() {
    saveAll();
    _timelineWnd->close();
    _toolsWnd->close();
    _celsWnd->close();
}


/*!
    When the current tool is changed in the Toolbox widget, this slot will be called.  It's
     job is to disconnect the signals/slots of the previous tool and hookup the new one.
*/
void BlitApp::onCurToolChanged(Tool *tool) {
    qDebug() << "[BlitApp tool changed]:" << tool->name();

    // Disconnct the old one
    if (_curTool)
        disconnect(_canvas, 0, _curTool, 0);

    // Connect the new one
    connect(_canvas, &Canvas::mouseDoubleClicked, tool, &Tool::onMouseDoubleClicked);
    connect(_canvas, &Canvas::mouseMoved, tool, &Tool::onMouseMoved);
    connect(_canvas, &Canvas::mousePressed, tool, &Tool::onMousePressed);
    connect(_canvas, &Canvas::mouseReleased, tool, &Tool::onMouseReleased);

    // And set a state variable
    _curTool = tool;
}


/*!
    Displays a Dialog window to edit the properties of the supplied animation object.  Will
    stop playing the animation.  Will block all input until the dialog is closed.  It will
    only edit the Animation if the Dialog returns success.  If a NULL pointer is provided
    it will assume that the programmer wasnts to use the currently loaded Animation.  If
    there is no currently loaded animation, this function will do nothing.  It is tripped by
    "Animation > Properties"
*/
void BlitApp::showAnimationProperties(Animation *anim) {
    // Safe afe afe afe afe...  Ty ty ty ty ty...  Check eck eck eck eck...
    if (!anim)                // Use loaded Animation
        anim = _anim;
    if (!anim)                // There is no loaded animation
        return;

    // Stop playing and make the dialog
    playAnimation(false);
    AnimationPropertiesDialog *apd = new AnimationPropertiesDialog(anim, this);

    // Run the dialog as a Modal one
    int res = apd->exec();
    if (res == QDialog::Accepted) {
        // User wants the change
        anim->setName(apd->name());
        anim->setFrameSize(apd->size());
    }

    // Delete the dialog
    delete apd;
}


/*!
    Brings up a dialog to import a still image file to a Cel in the current frame.
    On press, will cause the animation to stop playing.
*/
void BlitApp::showImportStillImage() {
    // Safety checks
    playAnimation(false);
    if (!_curTimedFrame)
        return;

    // Get a filename then add if it isn't empy
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Still Image as Cel"), _lastImportStillDir, FileOps::validFilters().join(";;"), NULL);
    if (!filename.isEmpty()) {
        // Make the new Cel
        PNGCel *cel = FileOps::loadStillImage(filename, _anim);
        if (cel)
            _curTimedFrame->frame()->addCel(new CelRef(cel));
        else
            qDebug() << "Error, couldn't load still image into Cel; filename=" << filename;
    }
}


/*!
    Create a show the "Export as Spritesheet," dialog.  Stops playing the animation.  The
    dialog will also act as a Modal dialog, so all other input will be stopped.  Slot is tripped
    by "File > Export > Spritesheet"
*/
void BlitApp::showExportSpritesheet() {
    playAnimation(false);
    SpritesheetDialog *sd = new SpritesheetDialog(this);
    sd->setAnim(_anim);
    sd->exec();

    delete sd;
}


/*!
    Will show a dialog to export a still image.  Also will stop playing the Animation. "File >
    Export > Still Image"
*/
void BlitApp::showExportStillImage() {
    playAnimation(false);

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Frame as Still Image"), curFrame()->name() + ".png", FileOps::validFilters().join(";;"), &_lastStillFilter);

    // Check that the dialog was a success
    if (!filename.isEmpty()) {
        // Make sure we've got a good extension
        if (!filename.endsWith(FileOps::extensionMap()[_lastStillFilter]))
            filename.append(FileOps::extensionMap()[_lastStillFilter]);

        // Make the save
        bool success = FileOps::saveStillFrame(_curTimedFrame->frame(), filename);
        
        // Set a variable
        _lastStillFilename = filename;
    }
}


/*!
    Activated by Menu -> Canvas -> Set Background Color option, this will bring up a modal
    dialog to change the color of the backdrop
*/
void BlitApp::onSetBackdrop() {    
    QColor clr = QColorDialog::getColor(_canvas->backdropColor(), this, tr("Set Backdrop Color"), QColorDialog::ShowAlphaChannel);
    _canvas->setBackdropColor(clr);
}


/*!
    Brings up a dailog when the "Help > About Blit" action is triggered.
*/
void BlitApp::showAboutBlit() {
    // Create it, execute it, then delete it
    QMessageBox *aboutBlit = new QMessageBox(this);
    aboutBlit->setTextFormat(Qt::RichText);
    aboutBlit->setWindowTitle(tr("About Blit"));
    aboutBlit->setText(
        "<strong>Blit (Source Release)</strong><br/>"
        "<br/>"
        "Blit is an 2D Animation tool made using C++ and Qt5.<br/>"
        "It is currently not under active development.<br/>"
        "<br/>"
        "<a href=\"https://gitlab.com/define-private-public/blit\">Blit Source Code</a> (<a href=\"https://www.gnu.org/licenses/gpl-3.0.en.html\">GNU GPL v3</a>)<br/>"
        "<br/>"
        "Blit uses some of Google's <a href=\"https://design.google.com/icons/\">Material Design icons</a><br />"
        "under the <a href=\"http://creativecommons.org/licenses/by/4.0/\">Apache License v2.0</a>."
    );
    aboutBlit->exec();
    delete aboutBlit;
}


/*!
    Will call the TimelineWindow::playAnimation().  if play=true, then it will play if false, then
    it will stop it.

    \sa isAnimationPlaying()
*/
void BlitApp::playAnimation(bool play) {
    _timelineWnd->playAnimation(false);
}


/*!
    When the Animation's name is changed, this slot will set the window title.    Will output differnt
    stuff based upon if the string is empty or not.
*/
void BlitApp::onAnimationNameChanged(QString str) {
    if (!str.isEmpty())
        setWindowTitle("Blit    [ " + str + " ]");
    else
        setWindowTitle("Blit    [ " + _anim->resourceDir() + " ]");

}


/*!
    Will return of the Aniation is playing or not.

    \sa playAnimation()
*/
bool BlitApp::isAnimationPlaying() {
    return _timelineWnd->isAnimationPlaying();
}


/*!
    Called when the Canvas widget is just been pressed.  the primary funciton of this slot
    is to stop any Animaitons that might be currently playing
*/
void BlitApp::_onCanvasPressed(QGraphicsSceneMouseEvent *event) {
    playAnimation(false);
}


/*!
    Called via the Canvas::mouseMoved() signal, this will emit the BlitApp::canvasMouseMoved()
    signal with it's location.
*/
void BlitApp::_onCanvasMouseMoved(QGraphicsSceneMouseEvent *event) {
    emit canvasMouseMoved(event->scenePos());
}


/*!
    Called by the TimelineWindow::animationPlaybackStateChanged() signal, this will
    pass around the signal to other modules that want/need it.  Emits the
    animationPlaybackStateChanged() signal.
*/
void BlitApp::_onAnimationPlaybackStateChanged(bool isPlaying) {
    emit animationPlaybackStateChanged(isPlaying);
}


/*!
    When the Animation's framesize is changed, it will trip this slot, which will then pass it
    around to the rest of the application that needs to know about the frame size (e.g. the
    Canvas).  Will also save the currently loaded Animation.

    \sa frameSize()
    \sa setFrameSize()
*/
void BlitApp::onFrameSizeChanged(QSize size) {
    emit frameSizeChanged(size);
    saveAnim();
}


/*!
    This is an overriden method.

    when a close event is received, this method is called.  It will shutdown the application
*/
void BlitApp::closeEvent(QCloseEvent *event) {
    playAnimation(false);
    shutdown();
    event->accept();
}


/*!
    This is an overriden method.

    Overloaded protected virtual function.  Is currently used to pass up QKeyEvents (for
    shortcuts) to other widgets.  Since the child windows are Qt::Tool windows and not dock
    widgets, they won't recieve keypress events unless they are focused.  This overloaded
    function will set the focus to that window temporarly then dispatch the keypress event.

    It's kind of a hacky solution
*/
void BlitApp::keyPressEvent(QKeyEvent *event) {
    // Copy the key event
    QKeyEvent *copy = new QKeyEvent(event->type(), event->key(), event->modifiers(), event->text(), event->isAutoRepeat(), event->count());

    // Dispatch the event
    switch (event->key()) {
        case Qt::Key_J:
        case Qt::Key_K:
        case Qt::Key_L:
        case Qt::Key_Comma:
        case Qt::Key_Period:
        case Qt::Key_Minus:
        case Qt::Key_Equal:
            // Dispatch a copy event to the timeline window
            QCoreApplication::postEvent(_timelineWnd, copy);
            event->accept();
            break;

        default:
            // Must be for us, delete the copy and process the event
            delete copy;
            QMainWindow::keyPressEvent(event);
    }
}


/*!
    Returns the current frame that the animation has been set to.  It's a bit of an "editor state,"
    thing.  It is possible that this funciton may return a Null Frame if there isn't a current
    Frame set.

    \sa setCurTimedFrame()
    \sa curTimedFrameChanged()
*/
TimedFrame *BlitApp::curTimedFrame() {
    return _curTimedFrame;
}


/*!
    Returns a pointer to the crrent Frame object.  If there isn't a frame at all in the animaiton,
    or none is selected to be the current TimedFrame, then this will return a NULL pointer.
*/
Frame *BlitApp::curFrame() {
    if (_curTimedFrame)
        return _curTimedFrame->frame();
    else
        return NULL;
}

/*!
    Returns a pointer to the currently Set Cel.  There is a chance that this could be a NULL pointer.

    \sa setCurCelRef()
    \sa curCelRefChanged()
*/
CelRef *BlitApp::curCelRef() {
    return _curCelRef;
}


/*!
    Returns a pointer to the crrent Cel object.  If there isn't a frame at all in the animaiton,
    or none is selected to be the current Cel, then this will return a NULL pointer.
*/
Cel *BlitApp::curCel() {
    if (_curCelRef)
        return _curCelRef->cel();
    else
        return NULL;
}


/*!
    Sets the current Frame object.  Should not be NULL.  Will emit the frameChanged singal if the
    supplied Frame object is different than the current one.

    \sa curTimedFrame()
    \sa curTimedFrameChanged()
*/
void BlitApp::setCurTimedFrame(TimedFrame *tf) {
    if (_curTimedFrame != tf) {
        // Activate/Deactivate (and do some NULL checks)
        if (_curTimedFrame) {
            if (_curTimedFrame->frame())
                _curTimedFrame->frame()->deactivate();
        }
        if (tf) {
            if (tf->frame())
                tf->frame()->activate();
        }

        _curTimedFrame = tf;
        emit curTimedFrameChanged(_curTimedFrame);

        // Set the current Cel
        // TODO something other than the topmost one?
        if (_curTimedFrame->frame()->cels().size() > 0)
            setCurCelRef(_curTimedFrame->frame()->cel(0));
        else
            setCurCelRef(NULL);            // NULL pointer
    }
}


/*!
    Sets the current Cel object. Will emit the curCelRefChanged
    signal if the supplied Cel object is different than the current one.

    \sa curCle()
    \sa curCelRefChanged()
*/
void BlitApp::setCurCelRef(CelRef *cr) {
    if (_curCelRef != cr) {
        _curCelRef = cr;
        emit curCelRefChanged(_curCelRef);
    }
}


/*!
    Sets the current sequence number.  \a seqNum must be a positive integer,
    that isn't greater that the current XSheet's sequence length.  Unless no
    animation is currently loaded, it's okay for it to be zero.

    This will do nothing if \a seqNum is the same as the current one, or if
    \a seqNum is out of bounds.

    Will emit the Blitapp::curSeqNumChanged() signal
*/
void BlitApp::setCurSeqNum(quint32 seqNum) {
    // Check for an XSheet (or Animation)
    if (!_anim)
        return;
    
    if ((_curSeqNum != seqNum) && (seqNum <= xsheet()->seqLength())) {
        _curSeqNum = seqNum;
        emit curSeqNumChanged(_curSeqNum);
    }
}


/*!
    Returns a pointer to the currently set Canvas.  It is guarenteeded that this function
    Will never return a NULL pointer because the Canvas object is created when BlitApp is
    instantiated.

    \sa setCanvas()
*/
Canvas *BlitApp::canvas() {
    return _canvas;
}


/*!
    Sets the _cavnas varible in BlitApp.  Will do nothing if canvas=NULL

    NOTE:
       This function should only be called by the "EditorContainer," widget, which houses the
       Canvas object.
    
    \sa canvas()
*/
void BlitApp::setCanvas(Canvas *canvas) {
    if (canvas)
        _canvas = canvas;
}


/*!
    Will return a blank Image of the same size of the currently selected Cel.  If there is
    no currently selected Cel, then the currently set frame size will be used instead.
*/
QImage BlitApp::getPaintableImage() {
    Cel *cel = curCel();
    if (cel)
        return util::mkBlankImage(cel->size());
    else
        return util::mkBlankImage(frameSize());
}

/*!
    Will draw the supplied image onto the current Cel.  If there is no current Cel, this 
    function will do nothing.
*/
void BlitApp::drawOntoCel(QImage &buffer) {
    Cel *cel = curCel();
    if (!cel)
        return;

    // Only draw on a PNGCel
    if (cel->type() == PNG_CEL_TYPE) {
        PNGCel *pc = (PNGCel *)cel;
    
        // Make a copy
        QImage tmp(pc->image());
        QPainter p(&tmp);
        p.drawImage(0, 0, buffer);

        // copy
        pc->setImage(tmp);
    }
}


/*!
    This function will take a QImage and replace that with the one in the currently selected Cel
    If there is no curCelRef, then it will do nothing.
*/
void BlitApp::copyOntoCel(QImage &buffer) {
    Cel*cel = BlitApp::app()->curCel();
    if (!cel)
        return;

    // Only copy on a PNGCel
    if (cel->type() == PNG_CEL_TYPE) {
        PNGCel *pc = (PNGCel *)cel;
    
        // Transfer
        pc->setImage(buffer);
    }

}


/*!
    Returns a copy of the current Cel's image.  If there is no currently selected Cel, then it
    will return a blank image at the currently set frame size
*/
QImage BlitApp::celImage() {
    Cel *cel = curCel();
    if (cel)
        return cel->image();
    else
        return util::mkBlankImage(frameSize());
}


/*!
    If the Animation variable is set, this will free it from memory and set the pointer back to
    NULL.
*/
void BlitApp::_freeAnim() {
    if (_anim) {
        delete _anim;
        _anim = NULL;
        qDebug() << "[BlitApp _freeAnim]";
    }
}


