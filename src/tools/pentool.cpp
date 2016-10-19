// File:         pen.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the PenTool Tool



#include "tools/pentool.h"
#include "tools/toolparameters.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPointF>
#include <QPixmap>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QFormLayout>
#include <QGraphicsSceneMouseEvent>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_pen, PenTool);


PenTool::PenTool(QObject *parent) :
    Tool(parent),
    _pen(BlitApp::app()->curColor(), Tools::commonParams["pen-size"].toDouble(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
{
    // Do nothing
}

PenTool::~PenTool() {
    // Do nothing
}


QString PenTool::name() {
    // returns the name of the Tool
    return tr("Pen");
}


QString PenTool::desc() {
    // A small tooltip
    return tr("Draws single solid strokes.");
}


QIcon PenTool::icon() {
    return QIcon(":/pen_tool_icon");
}


QWidget *PenTool::options() {
    // Return apanel to change tool options
    void (QSpinBox::*spinnerValueChanged)(int) = &QSpinBox::valueChanged;

    // Make the options panel
    QWidget *optionsPanel = new QWidget();
    optionsPanel->setMaximumWidth(TOOL_OPTIONS_PANEL_MAX_WIDTH);
    QSpinBox *sizeSpinner = new QSpinBox(optionsPanel);
    sizeSpinner->setValue(Tools::commonParams["pen-size"].toDouble());
    sizeSpinner->setMinimum(1);

    // Layout
    QFormLayout *layout = new QFormLayout(optionsPanel);
    layout->addRow(tr("Size:"), sizeSpinner);

    // Signals & slots
    connect(sizeSpinner, spinnerValueChanged, this, &PenTool::_onSizeSpinnerValueChanged);

    return optionsPanel;
    // Do nothing
}


void PenTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Do nothing
}


void PenTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // If the pen is down, then well, draw
    if (_penDown) {
        // Get Add a new line to the list of strokes
        QPointF curPoint = event->scenePos();
        _painter.drawLine(curPoint - _celPos, _lastPoint - _celPos);
        _transferDrawing();

        // Save the point
        _lastPoint = curPoint;
    }
}


void PenTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Put the pen in the down state
    _penDown = true;

    // Setup the Painter and state
    BlitApp *bApp = BlitApp::app();
    _celPos = ref->pos();
    _lastPoint = event->scenePos();
    _drawBuffer = new QImage(bApp->getPaintableImage());
    _celImage = new QImage(bApp->celImage());

    // Draw the first point
    _painter.begin(_drawBuffer);
    _pen.setWidth(Tools::commonParams["pen-size"].toDouble());
    _painter.setPen(_pen);
    QPointF tmp(qFloor(_lastPoint.x()), qFloor(_lastPoint.y()));
    _painter.drawPoint(tmp - _celPos);
    _transferDrawing();
}


void PenTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    if (_penDown) {
        //  Needs to be in an if here becuase of Double-Clicking
        // Stop painting
        _painter.end();

        // Cleanup state and Painter
        _lastPoint = QPointF();        // Null out
        _celPos = QPointF();
        delete _drawBuffer;
        _drawBuffer = NULL;
        delete _celImage;
        _celImage = NULL;
    
        // Bring the pen back up
        _penDown = false;
    }

}


void PenTool::_onSizeSpinnerValueChanged(int value) {
    // When the spinbox for the size of the PenTool is changed, it should change the size of the size
    // variable.  This is very self-explainable.
    Tools::commonParams["pen-size"] = QVariant((double)value);
//    _pen.setWidth(value);
}


void PenTool::_transferDrawing() {
    // Internal function.  Used to transfer the current drawing to the Cel.  Requires that the 
    // PenTool is down.
    if (!_penDown)
        return;

    // Using the draw buffer, create another painter instance that will
    // convert all of the black lines to the color
    QColor clr = BlitApp::app()->curColor();
    QImage drawBuff = _drawBuffer->copy();
    QPainter drawBuffPainter(&drawBuff);
    drawBuffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    drawBuffPainter.fillRect(0, 0, drawBuff.width(), drawBuff.height(), clr);

    // Then put it onto the Cel
    QImage celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.drawImage(0, 0, drawBuff);

    // Apply the update
    BlitApp::app()->copyOntoCel(celBuff);
}

