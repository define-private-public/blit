// File:         blitapp.h
// Author:       Ben Summerton (define-private-public)
// Description:  Main "Runner class," of the Blit application.  Is a QMainWindow, contains a canvas as its
//               main widget.  Also manages other things about the state of the appliation.  May be
//               accessed globally by other modules


#ifndef BLIT_APP_H
#define BLIT_APP_H


#include <QMainWindow>
class Cel;
class CelRef;
class Frame;
class TimedFrame;
class XSheet;
class Animation;
class Canvas;
class MenuBar;
class StatusBar;
class Tool;
class EditorContainer;
class TimelineWindow;
class ToolsWindow;
class CelsWindow;
class LightTableWindow;
class QSize;
class QColor;
class QImage;
class QGraphicsSceneMouseEvent;
#include <QDir>


class BlitApp: public QMainWindow {
    Q_OBJECT;

public:
    BlitApp();
    ~BlitApp();

    // State
    static BlitApp *app() { return _app; };
    QSize frameSize();
    double zoom();
    QColor curColor();
    quint32 curSeqNum();

    // Animation module access
    Animation *anim();
    XSheet *xsheet();
    TimedFrame *curTimedFrame();
    Frame *curFrame();
    CelRef *curCelRef();
    Cel *curCel();

    // File saving/loading
    bool load(QString path);
    bool saveAnim(QString path="");
    bool savePalette(QString path="");
    bool saveAll(QString path="");
    bool saveAs(QString path);        

    // Canvas
    Canvas *canvas();
    void setCanvas(Canvas *canvas);

    // For drawing
    QImage getPaintableImage();
    void drawOntoCel(QImage &buffer);
    void copyOntoCel(QImage &buffer);
    QImage celImage();


signals:
    /*! Emitted when the zoom has changed. */
    void zoomChanged(double zoom);

    /*! Emitted when the frameSize has changed. */
    void frameSizeChanged(QSize size);

    /*! Emitted when the current TimeFramed has changed. */
    void curTimedFrameChanged(TimedFrame *tf);

    /*! Emitted when the current CelRef has changed. */
    void curCelRefChanged(CelRef *cr);

    /*! Emitted when the current drawing QColor has changed. */
    void curColorChanged(QColor clr);

    /*! Emitted when the current sequence number has changed. */
    void curSeqNumChanged(quint32 seqNum);

    /*! Emitted when an Animation has just been loaded. */
    void animLoaded(Animation *anim);

    /*! Emitted when the the state of playback has changed. */
    void animationPlaybackStateChanged(bool isPlaying);

    /*! Emitted when the mouse has moved on the Canvas. */
    void canvasMouseMoved(QPointF pos);


public slots:
    void setFrameSize(QSize frameSize);
    void setZoom(double zoom);
    void setCurColor(QColor clr);
    void setCurTimedFrame(TimedFrame *tf);
    void setCurCelRef(CelRef *cr);
    void setCurSeqNum(quint32 seqNum);
    void onNewAnim();
    void onOpenAnim();
    void onSaveAs();
    void shutdown();

    void onCurToolChanged(Tool *tool);

    void showAnimationProperties(Animation *anim=NULL);
    void showImportStillImage();
    void showExportSpritesheet();
    void showExportStillImage();
    void onSetBackdrop();
    void showAboutBlit();

    void playAnimation(bool play=true);

    // Animation changes
    void onAnimationNameChanged(QString str);
    void onFrameSizeChanged(QSize size);

    // Accessors
    bool isAnimationPlaying();


private slots:
    void _onAnimationPlaybackStateChanged(bool isPlaying);

    void _onCanvasPressed(QGraphicsSceneMouseEvent *event);
    void _onCanvasMouseMoved(QGraphicsSceneMouseEvent *event);


protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);


private:
    static BlitApp *_app;            // Singleton isntance of the application

    // Private utility functions
    void _freeAnim();

    // Widgets
    TimelineWindow *_timelineWnd;
    ToolsWindow *_toolsWnd;
    CelsWindow *_celsWnd;
    LightTableWindow *_ltWnd;
    EditorContainer *_ec;
    StatusBar *_statusBar;
    MenuBar *_menuBar;

    // Drawing widgets
    Canvas *_canvas;

    // Current animation varibles
    Animation *_anim = NULL;                // Pointer to current Animation
    CelRef *_curCelRef = NULL;                // Current CelRef
    Frame *_frame = NULL;                    // Current Frame
    TimedFrame *_curTimedFrame = NULL;        // Current TimedFrame
    quint32 _curSeqNum = 0;                    // Current sequence number (alwasy  a positive number)

    // State variables
    QString _lastImportStillDir;    // Directory of the last imported still image (to Cel)
    QString _lastStillFilename;        // Filename of the last exported Still
    QString _lastStillFilter;        // Filter used for above variable

    // Editor state
    double _zoom;
    QColor _curClr = Qt::black;

    // Tools state
    Tool *_curTool;

};


#endif // BLIT_APP_H

