// File:         linetool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the LineTool Tool


// TODO list:
//  - Add in methods to change the Pen's cap style
//  - Right now you're supposed to click and drag to draw a line, add in a way
//    where the user can draw a line but, without drawing (start point on first
//    click, end line on second click).


#include "tools/linetool.h"
#include "tools/toolparameters.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPointF>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QFormLayout>
#include <QGraphicsSceneMouseEvent>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_pen, LineTool);


LineTool::LineTool(QObject *parent) :
    Tool(parent),
    _pen(BlitApp::app()->curColor(), Tools::commonParams["pen-size"].toDouble(), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin)
{
    // Do nothing
}


LineTool::~LineTool() {
    // Do nothing
}


QString LineTool::name() {
    // returns the name of the Tool
    return tr("Line");
}


QString LineTool::desc() {
    // A small tooltip
    return tr("Draws single straight lines.");
}


QIcon LineTool::icon() {
    return QIcon(":/line_tool_icon");
}


QWidget *LineTool::options() {
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
    connect(sizeSpinner, spinnerValueChanged, this, &LineTool::_onSizeSpinnerValueChanged);

    return optionsPanel;
    // Do nothing
}


void LineTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Do nothing
}


void LineTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // If the pen is down, then well, draw
    if (_penDown) {
        // Get Add a new line to the list of strokes
        _curPoint = event->scenePos().toPoint();
        _curPoint.setX(qFloor(_curPoint.x()));
        _curPoint.setY(qFloor(_curPoint.y()));
        _transferDrawing();
    }
}


void LineTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Put the pen in the down state
    _penDown = true;
    _pen.setWidth(Tools::commonParams["pen-size"].toDouble());
    _pen.setColor(BlitApp::app()->curColor());

    // Setup the Painter and state
    _celPos = ref->pos();
    _startPoint = event->scenePos(); //.toPoint();
    _startPoint.setX(qFloor(_startPoint.x()));
    _startPoint.setY(qFloor(_startPoint.y()));
    _curPoint = _startPoint;
    _celImage = new QImage(BlitApp::app()->celImage());

    // Draw the first point
    _transferDrawing();
}


void LineTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    if (_penDown) {
        //  Needs to be in an if here becuase of Double-Clicking
        // Stop painting

        // Cleanup state and Painter
        _startPoint = QPointF();        // Null out
        _curPoint = QPointF();
        _celPos = QPointF();
        delete _celImage;
        _celImage = NULL;
    
        // Bring the pen back up
        _penDown = false;
    }

}


void LineTool::_onSizeSpinnerValueChanged(int value) {
    // When the spinbox for the size of the LineTool is changed, it should change the size of the size
    // variable.  This is very self-explainable.
    Tools::commonParams["pen-size"] = QVariant((double)value);
}


void LineTool::_transferDrawing() {
    // Internal function.  Used to transfer the current drawing to the Cel.  Requires that the 
    // LineTool is down.
    if (!_penDown)
        return;

    // Then put it onto the Cel
    QImage celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.setPen(_pen);
//    celBuffPainter.translate(-0.5, -0.5);
//    celBuffPainter.drawLine(_startPoint, _curPoint);
    QList<QPoint> points = math::bresenhamLinePoints(_startPoint - _celPos, _curPoint - _celPos);
    QList<QPoint>::const_iterator iter;
    for (iter = points.begin(); iter != points.end(); iter++) {
        celBuffPainter.drawPoint(*iter);
    }
    celBuffPainter.drawPoint(_startPoint - _celPos);
    celBuffPainter.drawPoint(_curPoint - _celPos);

    // Apply the update
    BlitApp::app()->copyOntoCel(celBuff);
}

