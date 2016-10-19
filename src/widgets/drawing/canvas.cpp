// File:         canvas.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Canvas widgtet



#include "widgets/drawing/canvas.h"
#include "blitapp.h"
#include "animation/cel.h"
#include "animation/celref.h"
#include "animation/celrefitem.h"
#include "animation/pngcel.h"
#include "animation/frame.h"
#include "animation/frameitem.h"
#include "animation/timedframe.h"
#include "widgets/drawing/backdrop.h"
#include <QtCore/qmath.h>
#include <QTransform>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QRect>
#include <QRectF>
#include <QPainter>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


Canvas::Canvas(QWidget *parent) :
    QGraphicsView(parent)
{
    // Set some display options
    setAlignment(Qt::AlignCenter);
    setFrameShape(QFrame::NoFrame);
    setMouseTracking(true);

    // Create the scene
    _scene = new CanvasScene(this);
    setScene(_scene);
    connect(_scene, &CanvasScene::mouseDoubleClicked, this, &Canvas::onMouseDoubleClicked);
    connect(_scene, &CanvasScene::mouseMoved, this, &Canvas::onMouseMoved);
    connect(_scene, &CanvasScene::mousePressed, this, &Canvas::onMousePressed);
    connect(_scene, &CanvasScene::mouseReleased, this, &Canvas::onMouseReleased);

    // Create the backdrop item
    _backdrop = new Backdrop(this);
    _backdrop->setColor(Qt::white);
    _scene->addItem(_backdrop);

//    _lightTableNumBefore = 2;        // How many to get before the current frame
//    _lightTableNumAfter = 2;        // How many to get after the current frame
//    _lightTableFadeStep = 3;
}


Canvas::~Canvas() {
    // Much todo about nothing...
}


void Canvas::_onFrameDestroyed() {
    // When the current _frame variable is destroyed (e.g. via opening a new Animation), we need
    // to reset some variables.  This prevents a segfault
    _frame = NULL;
}


/*!
    Called by Frame::celAdded() signal.  Will schedule a redraw.

    \sa _onCelRemoved()
    \sa _onCelMoved()
*/
void Canvas::_onCelAdded(CelRef *ref) {
    _addCelRef(ref);
}


/*!
    Called by Frame::celRemoved() signal.  Will schedule a redraw.

    \sa _onCelAdded()
    \sa _onCelMoved()
*/
void Canvas::_onCelRemoved(CelRef *ref) {
    _removeCelRef(ref);
}


/*!
    Called by Frame::celMoved() signal.  Will schedule a redraw.

    \sa _onCelAdded()
    \sa _onCelRemoved()
*/
void Canvas::_onCelMoved(CelRef *ref) {
    Cel *c = ref->cel();
    _scene->invalidate(ref->x(), ref->y(), c->width(), c->height());
}


/*!
    Returns the area (in scene coordinates) that is currently visible in the view.  It's usually
    larger or smaller than the actual scene rect.
*/
QRectF Canvas::getVisibleRect() {
    // TODO maybe add something so that when Cels go out of bounds (i.e. someone drags the Cels too
    //      far to the right), it will add scrollbars?
    //      - Could also make a separate function that includes this and the out Cel bounds
    //
    // Gets that is the currently visible rect in the viewport
    QRectF rect(mapToScene(0, 0), mapToScene(viewport()->width(), viewport()->height()));
    return rect;
}


void Canvas::showGrid(bool showIt) {
    // Toggles the grid overlay on and off.  true for On, false for Off.
    _showGrid = showIt;

    // Adjust visibily for the items
    for (auto iter = _gridItems.begin(); iter != _gridItems.end(); iter++)
        (*iter)->setVisible(_showGrid && (_zoom >= _minZoomForGrid));

    // For update the scene's rectangle
    _scene->invalidate();
}


/*!
    Reports a zoom value as a floating point (1 = 100% zoom, 2 = 200% zoom, 0.5 = 50% zoom, etc.)
*/
qreal Canvas::zoom() {
    return _zoom;
}


/*!
    Returns the Backdrop's current color
*/
QColor Canvas::backdropColor() {
    return _backdrop->color();
}


/*!
    Triggered via the BlitApp::frameSizeChanged() signal.  This will do things like
    adjust the scene rect and adjusting the grid.
*/
void Canvas::onFrameSizeChanged(QSize size) {
    // Adjust the Secene rect
    setSceneRect(QRectF(QPointF(0, 0), size));

    // Recrete the grid, regardless if it's being shown or not. (or min zoom)
    // First remove all of the lines
    for (auto iter = _gridItems.begin(); iter != _gridItems.end(); iter++) {
        _scene->removeItem(*iter);
        delete *iter;
    }
    _gridItems.clear();

    // Add new grid lines
    QRectF visible = getVisibleRect();
    int vx = qFloor(visible.x());
    int vy = qFloor(visible.y());
    int vw = qCeil(visible.width());
    int vh = qCeil(visible.height());
    qreal invZoom = 1.0 /  _zoom;        // mulplicative inverse
    if (invZoom > 1.0)
        invZoom = _zoom;
    Qt::PenStyle style = (_zoom < 12) ? Qt::SolidLine : Qt::DotLine;
    QPen pen(QPen(Qt::lightGray, invZoom, style, Qt::SquareCap, Qt::MiterJoin));

    // Vertical lines
    for (int x = vx; x < vw; x++) {
        // Make the line
        QGraphicsLineItem *line = new QGraphicsLineItem(x, vy, x, vh + invZoom);
        line->setPen(pen);
        line->setVisible(_showGrid && (_zoom >= _minZoomForGrid));
        line->setZValue(CANVAS_FOREGROUND_Z_START);

        // Add to things
        _scene->addItem(line);
        _gridItems.append(line);
    }

    // Horizontal lines
    for (int y = vy; y < vh; y++) {
        QGraphicsLineItem *line = new QGraphicsLineItem(vx, y, vw + invZoom, y);
        line->setPen(pen);
        line->setVisible(_showGrid && (_zoom >= _minZoomForGrid));
        line->setZValue(CANVAS_FOREGROUND_Z_START);

        // Add to things
        _scene->addItem(line);
        _gridItems.append(line);
    }

    // update the backdrop
    _backdrop->setSize(size);

    // Debug Info
    qDebug() << "[Canvas onFrameSizeChanged] size=" << size;
}


void Canvas::onZoomChanged(double zoom) {
    // Slots called by the BlitApps's signal zoomChanged.
    _zoom = zoom;
    emit zoomChanged(_zoom);

    // Reset the transform, then scale and update the view
    setTransform(QTransform());
    scale(zoom, zoom);

    // Adjust the grid lines
    qreal invZoom = 1.0 /  _zoom;        // mulplicative inverse
    if (invZoom > 1.0)
        invZoom = _zoom;
    Qt::PenStyle style = (_zoom < 12) ? Qt::SolidLine : Qt::DotLine;
    QPen pen(QPen(Qt::lightGray, invZoom, style, Qt::SquareCap, Qt::MiterJoin));
    for (auto iter = _gridItems.begin(); iter != _gridItems.end(); iter++) {
        (*iter)->setPen(pen);
        (*iter)->setVisible(_showGrid && (_zoom >= _minZoomForGrid));
    }

    // Request a redraw
    update();
}


void Canvas::setFrame(TimedFrame *tf) {
    // TODO what if NULL is set once the frame is set?  this should always clear out the
    //      lighttable items.
    //      Possibly it should just clear out the whole canvas instead (i.e. we want the Canvas at a "clear" state)
    // Set regard if NULL or not
    _tf = tf;

    // Should either be given a pointer to a Frame or a NULL pointer.  If given a Null Frame, not much of
    // anything will be displayed.
    if (tf) {
        Frame *frame = tf->frame();
        if ((_frame != frame) && (frame != NULL)) {
            // Clean the old one
            // Signals & Slots
            if (_frame)
                disconnect(_frame, 0, this, 0);        // Lazy    

            // Clear out the previous CelRefs manually
            for (auto iter = _frameItems.begin(); iter != _frameItems.end(); iter++) {
                _scene->removeItem(*iter);
                delete *iter;
            }
            _frameItems.clear();
            
            int z = 0;
//            // The background items
//            z = CANVAS_BACKGROUND_Z_START;
//            auto bgIter = (_backgroundItems.end() - 1);
//            while (bgIter != (_backgroundItems.begin() - 1)) {
//                (*bgIter)->setZValue(z);
//                z++;
//                bgIter--;
//            }

            // The CelRefItems (Don't need to worry about Z value since they are already sorted)
            QList<CelRef *> celRefs = frame->cels();
            auto crIter = celRefs.begin();
            while (crIter != celRefs.end()) {
                _addCelRef(*crIter);
                crIter++;
            }

            // Add in the light table
            // Out with the old, and in with the new
            _removeLightTableItems();
            _createLightTableItems();

            // Set the var (and signals & slots)
            _frame = frame;
            connect(_frame, &Frame::destroyed, this, &Canvas::_onFrameDestroyed);
            connect(_frame, &Frame::celAdded, this, &Canvas::_onCelAdded);
            connect(_frame, &Frame::celRemoved, this, &Canvas::_onCelRemoved);
            connect(_frame, &Frame::celMoved, this, &Canvas::_onCelMoved);
        }

        // Info
        qDebug() << "[Canvas setFrame] frame=" << _frame << " timedframe=" << tf;
    }
}


void Canvas::onCurCelRefChanged(CelRef *cel) {
    // Tripped when the current Cel is changed.  Will cause the widget to redraw the view & scene
    if (_frame)
        _scene->invalidate();
}


/*!
    Changes the color of the backdrop to \a clr
*/
void Canvas::setBackdropColor(QColor clr) {
    _backdrop->setColor(clr);
}


/*!
    If \a enable is set to true, this will turn on the light-table (onionskinning).
    if \a enable is set to false, it will turn it off.  Will schedule a redraw
    of the Canvas.
*/
void Canvas::turnOnLightTable(bool enable) {
    if (_lightTableOn != enable) {
        // State switch
        _lightTableOn = enable;

        // Clear & Create
        _removeLightTableItems();
        _createLightTableItems();

        // Debug info
        qDebug() << "[Canvas turnOnLightTable] turned " << (_lightTableOn ? "on" : "off");
    }
}


/*!
    If \a looping is set to true, the light table will loop frames if the current frame
    is at the beginning/end of the XSheet.  If set to false, this turns it off.

    Will schedule a redraw of the canvas
*/
void Canvas::setLightTableLooping(bool looping) {
    if (_lightTableLooping != looping) {
        // State switch
        _lightTableLooping = looping;

        // Clear & create
        _removeLightTableItems();
        _createLightTableItems();

        // Debug info
        qDebug() << "[Canvas setLightTableLooping] looping=" << (_lightTableLooping ? "on" : "off");
    }
}


/*!
    Sets the number of Frames to show before the current one. 
    
    Will schedule a redraw of the canvas if the light table is on.
*/
void Canvas::setLightTableNumBefore(quint8 num) {
    // Only do the redraw if the number is new and light table is on
    if (_lightTableOn && (_lightTableNumBefore != num)) {
        // Set this before calling remove/create
        _lightTableNumBefore = num;

        // Clear & create
        _removeLightTableItems();
        _createLightTableItems();
    } else
        _lightTableNumBefore = num;
}


/*!
    Sets the number of Frames to show after the current one. 
    
    Will schedule a redraw of the canvas if the light table is on.
*/
void Canvas::setLightTableNumAfter(quint8 num) {
    // Only do the redraw if the number is new and light table is on
    if (_lightTableOn && (_lightTableNumAfter != num)) {
        // Set this before calling remove/create
        _lightTableNumAfter = num;

        // Clear & create
        _removeLightTableItems();
        _createLightTableItems();
    } else
        _lightTableNumAfter = num;
}


/*!
    Set's the amount that each next before/after frame will fade by subsequent step.
    To illustrate this:

        fadeStep = 2
        frame.opacity = 1.0
        frame_1.opacity = 0.5
        frame_2.opacity = 0.25

        fadeStep = 3
        frame.opacity = 1.0
        frame_1.opacity = 0.33
        frame_2.opacity - 0.11
    
    \a fadeStep must be at least one.  This will schedule a redraw of the light table (if on).
    Nothing will have if the value \a fadeStep is already set in the Light Table.

*/
void Canvas::setLightTableFadeStep(qreal fadeStep) {
    // Make sure that fadeStep >= 1
    if (fadeStep < 1)
        fadeStep = 1;
    
    // Check for different
    if (_lightTableFadeStep != fadeStep) {
        _lightTableFadeStep = fadeStep;

        // Update Redraw the light table items (if there are any)
        _removeLightTableItems();
        _createLightTableItems();
    }
}


void Canvas::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Emits a double-click signals
    emit mouseDoubleClicked(event);
}


void Canvas::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // Just pass up the mouse move event to some signal
    emit mouseMoved(event);
}


void Canvas::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // Emit a signal and pass it up
    emit mousePressed(event);
}


void Canvas::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    // emit a signal about the mouse being released and pass it down
    emit mouseReleased(event);
}


void Canvas::enterEvent(QEvent *event) {
    // Emit a signal and pass down the mouse enter event
    emit entered(event);
//    qDebug() << "[Enter]:" << event;
    QWidget::enterEvent(event);
}


void Canvas::leaveEvent(QEvent *event) {
    // Emit a signal and pass down the mouse leave event
    emit left(event);
//    qDebug() << "[Leave]:" << event;
    QWidget::leaveEvent(event);
}


void Canvas::drawBackground(QPainter *painter, const QRectF &rect) {
    // Drawing of the background is divided into two parts
    //   1. the background behind the frame
    //   2. the background behind the rest of the widget
    //
    // The first should be a more "dynmaic," BG, while the second is something that is just a plain color

    // Vars
    QRect frameRect(QPoint(0, 0), BlitApp::app()->frameSize());
    int fw = frameRect.width();
    int fh = frameRect.height();
    double invZoom = 1.0 / _zoom;        // mulplicative inverse
    if (invZoom > 1.0)
        invZoom = _zoom;

    // Draw the background for the rest of the widget
    painter->setPen(Qt::NoPen);
    painter->fillRect(rect, Qt::lightGray);
}


/*!
    Adds \cr to the internal Canvas Scene.
*/
void Canvas::_addCelRef(CelRef *cr) {
    // Add to list and scene
    CelRefItem *cri = cr->mkItem();
    cri->setCanvas(this);
    cri->setZValue(CANVAS_FRAME_Z_START + cri->zValue());        // TODO the CRI already sets it own Z value?  Should it?
    _frameItems.insert(cr, cri);
    _scene->addItem(cri);
}

/*!
    Removes \a cr from the internal CanvasScene.
*/
void Canvas::_removeCelRef(CelRef *cr) {
    CelRefItem *cri = _frameItems[cr];
    _frameItems.remove(cr);
    _scene->removeItem(cri);
    delete cri;
}


/*!
    Internal utility function.  Will look at the current frame (if there is one)
    and add the light table objects.

    the light table flag must be set to `true` for this function to work, else 
    nothing will happen
*/
void Canvas::_createLightTableItems() {
    if (_lightTableOn && _tf) {
        // Vars
        int z = 0;
        qreal opacity = 1.0 / _lightTableFadeStep;
        TimedFrame *cursor = _tf;

        // Add frame(s) before the current one
        for (int i = 0; i < _lightTableNumBefore; i++) {
            // Try to grab the frame before the cursor
            TimedFrame *before = cursor->before(_lightTableLooping);
            if (before) {
                // Got it, create a Frame Item
                FrameItem *fi = before->frame()->mkItem();
                fi->setOpacity(opacity);
                fi->setZValue(CANVAS_LIGHT_TABLE_BEFORE_Z_START + z);
                _scene->addItem(fi);
                _lightTableItems.append(fi);

                // inc
                z++;
                opacity /= _lightTableFadeStep;
                cursor = before;
            } else {
                // There is nothing, just break the loop
                break;
            }
        }


        // TODO next
        // Add frames after the current one
        z = 0;
        opacity = 1.0 / _lightTableFadeStep;
        cursor = _tf;
        for (int i = 0; i < _lightTableNumAfter; i++) {
            // See if we can get a frame after the cursor
            TimedFrame *after = cursor->after(_lightTableLooping);
            if (after) {
                // got it, create a Frame Item
                FrameItem *fi = after->frame()->mkItem();
                fi->setOpacity(opacity);
                fi->setZValue(CANVAS_LIGHT_TABLE_AFTER_Z_START + z);
                _scene->addItem(fi);
                _lightTableItems.append(fi);
    
                // inc
                z++;
                opacity /= _lightTableFadeStep;
                cursor = after;
            } else {
                // There is nothing, just break the loop
                break;
            }
        }
    }
}


/*!
    Internal utility function.  Will always clear out (if any) light table items.
*/
void Canvas::_removeLightTableItems() {
    for (auto iter = _lightTableItems.begin(); iter != _lightTableItems.end(); iter++) {
        _scene->removeItem(*iter);
        delete *iter;
    }
    _lightTableItems.clear();
}


/*!
    Sets up the CanvasScene.  \a parent should be a Canvas.
*/
CanvasScene::CanvasScene(QObject *parent) :
    QGraphicsScene(parent)
{
    // Do nothing
}


/*!
    Emits the mouseDoubleClicked() signal.
*/
void CanvasScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseDoubleClicked(event);
}


/*!
    Emits the mouseMoved() signal.
*/
void CanvasScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseMoved(event);
}


/*!
    Emits the mousePressed() signal.
*/
void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    emit mousePressed(event);
}


/*!
    Emits the mouseReleased() signal.
*/
void CanvasScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseReleased(event);
}

