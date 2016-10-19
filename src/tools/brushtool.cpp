// File:         brushtool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Brush Tool


/*!
    \inmodule Tools
    \class BrushTool
    \brief BrushTool is a Tool that is used to draw softer edge strokes.

    Unlike the PenTool, this tool draws much softer strokes/fills for its drawing.  It can draw
    pattersn and what not.  It is a tool that comes standard in Blit.
*/


#include "tools/brushtool.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QGraphicsSceneMouseEvent>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_brush, BrushTool);


/*!
    Sets up the Brush
*/
BrushTool::BrushTool(QObject *parent) :
    Tool(parent)
{
    // Some pen defaults
    _pen.setCapStyle(Qt::RoundCap);
    _pen.setJoinStyle(Qt::RoundJoin);

    // TODO remove
    _pen.setWidth(_size);
//    _brush.setStyle(Qt::RadialGradientPattern);
//    _brush.setColor(_pen.color());
}


/*!
    Deconstructor
*/
BrushTool::~BrushTool() {
    // Do nothing
}


/*!
    Name of the tool, returns Brush.
*/
QString BrushTool::name() {
    // returns the name of the Tool
    return tr("Brush");
}


/*!
    Description of the Tool.
*/
QString BrushTool::desc() {
    // A small tooltip
    return tr("Draws soft strokes.");
}


/*!
    Displays the Icon for this Tool.
*/
QIcon BrushTool::icon() {
    return QIcon(":/brush_tool_icon");
}


/*!
    Doesn't do anything right now.
*/
void BrushTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // No abosoluty nothing
}


/*!
    Will do tht drawing of the brush is down.
*/
void BrushTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // Make sure that the brush is down first
    if (!_brushDown)
        return;

    // First move to the point
    _path->lineTo(event->scenePos() - _celPos);

    // Transfer the drawing
    _transferDrawing();
}


/*!
    Will put the Brush down and setup everything for drawing.
*/
void BrushTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // Make sure only do function if the brush isn't already down
    if (_brushDown)
        return;

    // Next check for a current Cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Put the Brush down
    _brushDown = true;

    // Setup the painter and buffer objects
    // Make the path
    // TODO, isn't drawing a starting point
    _path = new QPainterPath();
    _path->moveTo(event->scenePos() - _celPos);
//    _path->lineTo(event->scenePos() - _celPos);

    // Setup buffer/state vars and start drawing
    _celPos = ref->pos();
    _celImage = new QImage(BlitApp::app()->celImage());

    // Draw the first point
    _transferDrawing();
}


/*!
    Will put the Brush in an "up" state
*/
void BrushTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    // Safety check
    if (!_brushDown)
        return;

    // release brush & stop painting
    _brushDown = false;
    
    // reset path
    delete _path;
    _path = NULL;

    // Cleanup state vars
    _celPos = QPointF();
    delete _celImage;
    _celImage = NULL;
}


/*!
    Internal function to transfer the drawing to the Current Cel
*/
void BrushTool::_transferDrawing() {
    // Check that the brush is down
    if (!_brushDown)
        return;

    // Make a gradient
    QRadialGradient radGrad = QRadialGradient(_path->currentPosition() - _celPos, _size);
    radGrad.setColorAt(0, BlitApp::app()->curColor());
    radGrad.setColorAt(1, Qt::transparent);
    QBrush radBrush(radGrad);
    _pen.setBrush(radBrush);
    //    _brush.setColor(_pen.color());


    // Then put it onto the Cel
    QImage celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.setPen(_pen);
    celBuffPainter.drawPath(*_path);
//    celBuffPainter.fillPath(*_path, _brush);

    // Apply the update
    BlitApp::app()->copyOntoCel(celBuff);
    celBuff.save("/tmp/celBuff.png");
}
