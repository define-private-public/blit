// File:         filltool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the FillTool Tool



#include "tools/filltool.h"
#include "tools/toolparameters.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QtMath>
#include <QStack>
#include <QPointF>
#include <QPixmap>
#include <QPen>
#include <QPainter>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <ctime>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_pen, FillTool);


FillTool::FillTool(QObject *parent) :
    Tool(parent)
{
    // Do nothing
}

FillTool::~FillTool() {
    // Do nothing
}


QString FillTool::name() {
    // returns the name of the Tool
    return tr("Fill");
}


QString FillTool::desc() {
    // A small tooltip
    return tr("Performs a flood fill on an area.");
}


QIcon FillTool::icon() {
    return QIcon(":/fill_tool_icon");
}


void FillTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Do nothing
}


void FillTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // Do nothing
}


void FillTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Put the pen in the down state
    _penDown = true;

    // Setup the Painter and state
    _celPos = ref->pos();
    _celImage = new QImage(BlitApp::app()->celImage());
}


void FillTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    // For for release
    if (!_penDown)
        return;

    // Move the fill source
    _fillSource = QPoint(qFloor(event->scenePos().x()), qFloor(event->scenePos().y()));

    // Do the fill
    _transferDrawing();

    // Cleanup state and Painter
    _celPos = QPointF();
    _fillSource = QPoint();
    delete _celImage;
    _celImage = NULL;
    
    // Bring the pen back up
    _penDown = false;

}


void FillTool::_transferDrawing() {
    // Internal function.  Used to transfer the current drawing to the Cel.  Requires that the 
    // FillTool is down.  Code is based off of the one from lodev.org
    if (!_penDown)
        return;

    // Points & bounds checking
    int x = _fillSource.x() - _celPos.x(), y = _fillSource.y() - _celPos.y();
    if ((x < 0) || (y < 0) || (x > (_celImage->width() - 1)) || (y > (_celImage->height() - 1)))
        return;
    
    
    // Get colors and check
//    clock_t start = clock();
    QRgb fillClr = BlitApp::app()->curColor().rgba();
    QRgb oldClr = _celImage->pixel(x, y);
    if (oldClr == fillClr)
        return;

    // Then put it onto the Cel
    QImage celBuff = _celImage->copy();

    // Vars for flood fill
    QStack<QPoint> stack;
    QVector<QPoint> toDraw;
    bool spanLeft, spanRight;
    int w = celBuff.width(), h = celBuff.height();
    int y1;

    // TODO cache pixel data (see if there is a speed improvement)

    // First push
    stack.push(QPoint(x, y));

    while (stack.size() != 0) {
        QPoint p = stack.pop();
        x = p.x();
        y = p.y();

        y1 = y;
        while ((y1 >= 0) && (celBuff.pixel(x, y1) == oldClr))
            y1--;
        y1++;

        spanLeft = spanRight = false;
        while ((y1 < h) && (celBuff.pixel(x, y1) == oldClr)) {
            QPoint draw(x, y1);
            celBuff.setPixel(draw, fillClr);        // Put the pixel
            toDraw << draw;


            if (!spanLeft && (x > 0) && (celBuff.pixel(x - 1, y1) == oldClr)) {
                stack.push(QPoint(x - 1, y1));
                spanLeft = true;
            } else if (spanLeft && (x > 0) && (celBuff.pixel(x - 1, y1) != oldClr)) {
                spanLeft = false;
            }

            if (!spanRight && (x < (w - 1)) && (celBuff.pixel(x + 1, y1) == oldClr)) {
                stack.push(QPoint(x + 1, y1));
                spanRight = true;
            } else if (spanRight && (x < (w - 1)) && (celBuff.pixel(x + 1, y1) != oldClr)) {
                spanRight = false;
            }

            y1++;
        }
    }

    // Apply the update
    QPen pen(BlitApp::app()->curColor());
    celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.setPen(pen);
    celBuffPainter.drawPoints(toDraw.data(), toDraw.size());
    BlitApp::app()->copyOntoCel(celBuff);
}

