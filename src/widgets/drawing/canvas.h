// File:         canvas.h
// Author:       Ben Summerton (define-private-public)
// Description:  Canvas is the widget that is responsible for drawing ontop of a Frame object, as well as
//               the Cels that it contains.  It should be a singleton, but it can have some other thigns


#ifndef CANVAS_H
#define CANVAS_H

#define CANVAS_BACKGROUND_Z_START -1000
#define CANVAS_LIGHT_TABLE_BEFORE_Z_START 300
#define CANVAS_FRAME_Z_START 600
#define CANVAS_LIGHT_TABLE_AFTER_Z_START 900
#define CANVAS_FOREGROUND_Z_START 1000


#include <QGraphicsView>
#include <QList>
#include <QHash>
class CanvasScene;
class CelRef;
class CelRefItem;
class Frame;
class FrameItem;
class TimedFrame;
class Backdrop;
class QSize;
class QImage;
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsLineItem;
class QGraphicsSceneMouseEvent;


class Canvas : public QGraphicsView {
    Q_OBJECT;

public:
    Canvas(QWidget *parent = NULL);
    ~Canvas();

    // Modifiers
    void showGrid(bool showIt);

    // Misc
    QRectF getVisibleRect();
    qreal zoom();
    QColor backdropColor();


public slots:
    // Canvas functions
    void onZoomChanged(double zoom);
    void onFrameSizeChanged(QSize size);
    void setFrame(TimedFrame *tf);
    void onCurCelRefChanged(CelRef *cel);
    void setBackdropColor(QColor clr);

    // Light Table
    void turnOnLightTable(bool enable);
    void setLightTableLooping(bool looping);
    void setLightTableNumBefore(quint8 num);
    void setLightTableNumAfter(quint8 num);
    void setLightTableFadeStep(qreal fadeStep);

    // For drawing from the Frame object
    void onMouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    void onMouseMoved(QGraphicsSceneMouseEvent *event);
    void onMousePressed(QGraphicsSceneMouseEvent *event);
    void onMouseReleased(QGraphicsSceneMouseEvent *event);


private slots:
    void _onFrameDestroyed();

    // For the currently loaded up Frame
    void _onCelAdded(CelRef *ref);
    void _onCelRemoved(CelRef *ref);
    void _onCelMoved(CelRef *ref);


signals:
    // Mouse signals
    void entered(QEvent *event);
    void left(QEvent *event);
    void mouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    void mouseMoved(QGraphicsSceneMouseEvent *event);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    // Other things
    void zoomChanged(double zoom);


protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *evnet);

    void drawBackground(QPainter *painter, const QRectF &rect);


private:
    // Internal functions
    void _addCelRef(CelRef *cr);
    void _removeCelRef(CelRef *cr);
    void _createLightTableItems();
    void _removeLightTableItems();

    // Member vars
    CanvasScene *_scene = NULL;                        // Where all of the presentation for the drawing stuff takes place
    Backdrop *_backdrop = NULL;                        // A color/image that appears behind all of the Cels in every scene.
    QHash<CelRef *, CelRefItem *> _frameItems;        // List of all of items, most typically will be CelRefs; TODO bad name since FrameItems is another class, maybe thing of something different here...
    QList<FrameItem *> _lightTableItems;            // Used for light-table/onion skinning
    QList<QGraphicsLineItem *> _gridItems;            // List of all of items that are used for the grid

//    QList<QGraphicsItem *> _backgroundItems;        // Items for the background

    // Behavior vars
    int _rulerSize = 20;                // Size of the rulers

    // Grid variables
    double _minZoomForGrid = 8.0;        // Minimum zoom for the grid

    // State vars
    Frame *_frame = NULL;                // Pointer to current Frame object that is being edited
    TimedFrame *_tf = NULL;                // Pointer to the current TimedFrame object
    qreal _zoom = 1;                    // Zoom as a floating point
    bool _showGrid = true;                // Boolean to show the grid or not
    bool _lightTableOn = false;            // Boolean to toggle the light-table on/off
    bool _lightTableLooping = false;    // Flag to use looping for the light table
    quint8 _lightTableNumBefore = 1;    // How many to get before the current frame
    quint8 _lightTableNumAfter = 1;        // How many to get after the current frame
    qreal _lightTableFadeStep = 3;        // How much to fade by each progressive frame before/after

};



// Private inner class (or what was supposed to be one...
class CanvasScene : public QGraphicsScene {
    Q_OBJECT

public:
    // Constructors
    CanvasScene(QObject *parent=NULL);

signals:
    void mouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    void mouseMoved(QGraphicsSceneMouseEvent *event);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);


protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

};

#endif // CANVAS_H

