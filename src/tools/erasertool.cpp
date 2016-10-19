// File:         erasertool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the EraserTool Tool
//
//               This tool uses most of the source from the PenTool tool.  The only difference
//               is that this uses the drawn pixels to erase stuff.



#include "tools/erasertool.h"
#include "tools/toolparameters.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPointF>
#include <QIcon>
#include <QColor>
#include <QPixmap>
#include <QLabel>
#include <QSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGraphicsSceneMouseEvent>


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_eraser, EraserTool);


EraserTool::EraserTool(QObject *parent) :
    Tool(parent),
    _pen(Qt::black, Tools::commonParams["pen-size"].toDouble(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
{
    // Do nothing
}

EraserTool::~EraserTool() {
    // Do nothing
}


QString EraserTool::name() {
    // returns the name of the Tool
    return tr("Eraser");
}


QString EraserTool::desc() {
    // A small tooltip
    return tr("Erases drawn pixels.");
}


QIcon EraserTool::icon() {
    return QIcon(":/eraser_tool_icon");
}


QWidget *EraserTool::options() {
    // Return apanel to change tool options
    void (QSpinBox::*spinnerValueChanged)(int) = &QSpinBox::valueChanged;

    // Make the options panel
    QWidget *optionsPanel = new QWidget();
    optionsPanel->setMaximumWidth(TOOL_OPTIONS_PANEL_MAX_WIDTH);

    // Size
    QSpinBox *sizeSpinner = new QSpinBox(optionsPanel);
    sizeSpinner->setMinimum(1);
    sizeSpinner->setValue(Tools::commonParams["pen-size"].toDouble());

    // Hardness
    QSpinBox *hardnessSpinner = new QSpinBox(optionsPanel);
    hardnessLabel = new QLabel(optionsPanel);
    hardnessLabel->setText(QString::number((_hardness / 255.0) * 100.0, 'f', 2) + "%");
    hardnessSpinner->setMinimum(0x00);
    hardnessSpinner->setMaximum(0xFF);
    hardnessSpinner->setValue(_hardness);

    // Layout
    QFormLayout *layout = new QFormLayout(optionsPanel);
    QHBoxLayout *hardnessLayout = new QHBoxLayout();
    hardnessLayout->addWidget(hardnessSpinner);
    hardnessLayout->addWidget(hardnessLabel);
    layout->addRow(tr("Size:"), sizeSpinner);
    layout->addRow(tr("Hardness:"), hardnessLayout);

    // Signals & slots
    connect(sizeSpinner, spinnerValueChanged, this, &EraserTool::_onSizeSpinnerValueChanged);
    connect(hardnessSpinner, spinnerValueChanged, this, &EraserTool::_onHardnessSpinnerValueChanged);

    return optionsPanel;
}


void EraserTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Do nothing
}


void EraserTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // If the eraser is down, then well, erase (actually draw)
    if (_eraserDown) {
        // Get Add a new line to the list of strokes
        QPointF curPoint = event->scenePos();
        _painter.drawLine(curPoint - _celPos, _lastPoint - _celPos);
        _transferDrawing();

        // Save the point
        _lastPoint = curPoint;
    }
}


void EraserTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;

    // Put the eraser in the down state
    _eraserDown = true;

    // Setup the Painter and state
    BlitApp *bApp = BlitApp::app();
    _celPos = ref->pos();
    _lastPoint = event->scenePos();
    _drawBuffer = new QImage(bApp->getPaintableImage());
    _celImage = new QImage(bApp->celImage());

    // Erase the first point
    _painter.begin(_drawBuffer);
    _pen.setWidth(Tools::commonParams["pen-size"].toDouble());
    _painter.setPen(_pen);
    QPointF tmp(qFloor(_lastPoint.x()), qFloor(_lastPoint.y()));
    _painter.drawPoint(tmp - _celPos);
    _transferDrawing();
}


void EraserTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    if (_eraserDown) {
        //  Needs to be in an if here becuase of Double-Clicking
        // Stop erasing
        _painter.end();

        // Cleanup state and Painter
        _lastPoint = QPointF();        // Null out
        _celPos = QPointF();
        delete _drawBuffer;
        _drawBuffer = NULL;
        delete _celImage;
        _celImage = NULL;
    
        // Bring the pen back up
        _eraserDown = false;
    }

}


void EraserTool::_onSizeSpinnerValueChanged(int value) {
    // When the spinbox for the size of the EraserTool is changed, it should change the size of the size
    // variable.  This is very self-explainable.
    Tools::commonParams["pen-size"] = QVariant((double)value);
//    _pen.setWidth(value);
}


void EraserTool::_onHardnessSpinnerValueChanged(int value) {
    // When the spinbox for the hardness of the EraserTool is changed, it will adjust how intense the
    // erasure is.
    _hardness = value;
    hardnessLabel->setText(QString::number((_hardness / 255.0) * 100.0, 'f', 2) + "%");
}


void EraserTool::_transferDrawing() {
    // TODO along with the PenTool Too, investigate the drawing speeds
    //
    // Internal function.  Used to transfer the current drawing to the Cel.  Requires that the 
    // EraserTool is down.
    if (!_eraserDown)
        return;

    // convert the drawing buffer to the desired intensity of the hardness
    QImage drawBuff = _drawBuffer->copy();
    QPainter drawBuffPainter(&drawBuff);
    drawBuffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    drawBuffPainter.fillRect(0, 0, drawBuff.width(), drawBuff.height(), QColor(0x00, 0x00, 0x00, _hardness));

    // Then Erase that much from the Cel
    QImage celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    celBuffPainter.drawImage(0, 0, drawBuff);

//    // Go through the alpha adjusted drawing bufer.  If a pixel's alpha value is someing, then
//    // subtract that alpha value from the second buffer
//    for (int y = 0; y < drawBufferCopy.width(); y++) {
//        for (int x = 0; x < drawBufferCopy.height(); x++) {
//            int alpha = qAlpha(drawBufferCopy.pixel(x, y));
//
//            // Subtract pixel alpha by value if there is some erasure
//            if (alpha != 0) {
//                QRgb pixel = celCopy.pixel(x, y);
//                int destAlpha = qAlpha(pixel);
//
//                // Before
////                qDebug() << "before";
////                qDebug() << "  r:" << qRed(pixel);
////                qDebug() << "  g:" << qGreen(pixel);
////                qDebug() << "  b:" << qBlue(pixel);
////                qDebug() << "  a:" << qAlpha(pixel);
//
//                // Do the subtaction
//                destAlpha -= _hardness;
//                if (destAlpha < 0)
//                    destAlpha = 0;
//
//                // Before
////                qDebug() << "Dest";
////                qDebug() << "  a:" << destAlpha;
//
//                // And set the pixel
//                celCopy.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), destAlpha));
//
//                // After
//                pixel = celCopy.pixel(x, y);
////                qDebug() << "after";
////                qDebug() << "  r:" << qRed(pixel);
////                qDebug() << "  g:" << qGreen(pixel);
////                qDebug() << "  b:" << qBlue(pixel);
////                qDebug() << "  a:" << qAlpha(pixel);
////                qDebug();
//            }
//        }
//    }
//    celCopy.save("/tmp/celCopy.png");

    // Apply the update
    BlitApp::app()->copyOntoCel(celBuff);
}

