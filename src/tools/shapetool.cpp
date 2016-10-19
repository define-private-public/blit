// File:         shapetool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the ShapeTool Tool


// TODO list
//   - Later on, add options to only make "perfect boxes," or "perfect ellipses."
//     a.k.a squares and circles.  It would be good for the user if they could
//     have this type of control.
//   - right now the box and the ellipse modes have a click n' drag method of
//     drawing.  Allow the user to make a start click and an end click (similar
//     to what I want in the future for the Line Tool).  Currently the Polygon
//     mode is the click n' plot (w/ end on a double click)
//   - Alternative ways of drawing circles and rectangles:
//     - click the center for the circle, then drag out to its radius
//       - this might also work well for ellipses
//   - Triangle
//   - N-sided, even sided, convex polygon
//   - Add an anti-aliasing option
//     - Will do nothing for the polygon tool.


#include "tools/shapetool.h"
#include "tools/toolparameters.h"
#include "blitapp.h"
#include "util.h"
#include "animation/celref.h"
#include <QtMath>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QGraphicsSceneMouseEvent>


/*== OVERLOADED qHash function ==*/
inline uint qHash(const QPoint p) {
    // Should be unique as possible
    return qFloor(qPow(p.x(), p.y()));
}

inline uint qHash(const QPointF p) {
    return qHash(p.toPoint());
}
/*== OVERLOADED WARNING ==*/


// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_pen, ShapeTool);


ShapeTool::ShapeTool(QObject *parent) :
    Tool(parent),
    _pen(BlitApp::app()->curColor(), Tools::commonParams["pen-size"].toDouble(), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin),
    _shape(Box)
{
    // Do nothing
}


ShapeTool::~ShapeTool() {
    // Do nothing
}


QString ShapeTool::name() {
    // returns the name of the Tool
    return tr("Shape");
}


QString ShapeTool::desc() {
    // A small tooltip
    return tr("Draws elipses, boxes, and polygons.");
}


QIcon ShapeTool::icon() {
    return QIcon(":/shape_tool_icon");
}


QWidget *ShapeTool::options() {
    // Return apanel to change tool options
    void (QSpinBox::*spinnerValueChanged)(int) = &QSpinBox::valueChanged;
    void (QComboBox::*currentIndexChanged)(int) = &QComboBox::currentIndexChanged;

    // Make the options panel
    QWidget *optionsPanel = new QWidget();
    optionsPanel->setMaximumWidth(TOOL_OPTIONS_PANEL_MAX_WIDTH);

    // Size Spinner
    QSpinBox *sizeSpinner = new QSpinBox(optionsPanel);
    sizeSpinner->setValue(Tools::commonParams["pen-size"].toDouble());
    sizeSpinner->setMinimum(1);

    // Shape selector
    QComboBox *shapeBox = new QComboBox(optionsPanel);
    shapeBox->insertItem(Box, tr("Box"));
    shapeBox->insertItem(Ellipse, tr("Ellipse"));
    shapeBox->insertItem(Polygon, tr("Polygon"));
    shapeBox->setCurrentIndex((int)_shape);

//    // Same Size option
//    _sameSizeBox = new QCheckBox(optionsPanel);
//    _sameSizeBox->setChecked(_sameSize);
//    _sameSizeBox->setEnabled(_shape != Polygon);

    // Layout
    QFormLayout *layout = new QFormLayout(optionsPanel);
    layout->addRow(tr("Size:"), sizeSpinner);
    layout->addRow(tr("Shape:"), shapeBox);
//    layout->addRow(tr("Same Size:"), _sameSizeBox);

    // Signals & slots
    connect(sizeSpinner, spinnerValueChanged, this, &ShapeTool::_onSizeSpinnerValueChanged);
    connect(shapeBox, currentIndexChanged, this, &ShapeTool::_onShapeBoxChanged);
//    connect(_sameSizeBox, &QCheckBox::clicked, this, &ShapeTool::_onSameSizeBoxClicked);

    return optionsPanel;
}


void ShapeTool::onMouseDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // Used to stop drawing polygons
    //  Only do stuff if the pen is down, and the shape is a polygon
    if (!_penDown)
        return;

    // Release on double-click if shape is a polygon
    if (_shape == Polygon)
        _reset();
}


void ShapeTool::onMouseMoved(QGraphicsSceneMouseEvent *event) {
    // If the pen is down, then well, draw
    if (!_penDown)
        return;

    // Get the current point
    _curPoint = event->scenePos();
    _curPoint.setX(qFloor(_curPoint.x()));
    _curPoint.setY(qFloor(_curPoint.y()));

    _transferDrawing();
}


void ShapeTool::onMousePressed(QGraphicsSceneMouseEvent *event) {
    // First check for current cel
    CelRef *ref = BlitApp::app()->curCelRef();
    if (!ref)
        return;


    // If the pen isn't down
    if (!_penDown) {
        // Put the pen in the down state, start drawing
        _penDown = true;
        _pen.setWidth(Tools::commonParams["pen-size"].toDouble());
        _pen.setColor(BlitApp::app()->curColor());

        // Cel stuff
        _celPos = ref->pos();
        _celImage = new QImage(BlitApp::app()->celImage());

        // Calculate the start point
        _startPoint = event->scenePos(); //.toPoint();
        _startPoint.setX(qFloor(_startPoint.x()));
        _startPoint.setY(qFloor(_startPoint.y()));
        _curPoint = _startPoint;

        // If polygon, start line
        if (_shape == Polygon) {
            _path.moveTo(_startPoint);
            _points.push_back(_curPoint);
        }
    } else if (_penDown && (_shape == Polygon)) {
        // On a click, add a line to the polygon
        _path.lineTo(_curPoint);
        _points.push_back(_curPoint);
    }

    // Draw the first point
    _transferDrawing();
}


void ShapeTool::onMouseReleased(QGraphicsSceneMouseEvent *event) {
    if (!_penDown)
        return;

    // Reset, only if the shape is a box or an ellipse, polygon is handled in double-click
    if ((_shape == Box) || (_shape == Ellipse))
        _reset();
}


void ShapeTool::_onSizeSpinnerValueChanged(int value) {
    // When the spinbox for the size of the ShapeTool is changed, it should change the size of the size
    // variable.  This is very self-explainable.
    Tools::commonParams["pen-size"] = QVariant((double)value);
}


/*!
    For when the Shape QComboBox's selection is changed, this will tell the
    tool to change the shape drawing mode.  Will no do anything if the the
    mouse is currently down.  Will cause other widgets to disable themselves.
*/
void ShapeTool::_onShapeBoxChanged(int index) {
    _reset();
    _shape = (Shape)index;

    // Enable/disable the same size box
    if (_sameSizeBox)
        _sameSizeBox->setEnabled(_shape != Polygon);
}


/*!
    If a shape supports it, it can be drawn with the same width and heigh.  If
    the _sameSizeBox is checked, then this will enable it to do so.
*/
void ShapeTool::_onSameSizeBoxClicked(bool checked) {
    _sameSize = checked;
}


void ShapeTool::_transferDrawing() {
    // Internal function.  Used to transfer the current drawing to the Cel.  Requires that the 
    // ShapeTool is down.
    if (!_penDown)
        return;

    // for pen
    QPen drawBuffPen(_pen);
    drawBuffPen.setColor(Qt::black);

    // Drawing buffer
    QImage drawBuff = util::mkBlankImage(_celImage->size());
    QPainter drawBuffPainter(&drawBuff);
    drawBuffPainter.setPen(drawBuffPen);

    // compute the bounds
    QPoint tl(_startPoint.toPoint() - _celPos.toPoint());        // Top Left
    QPoint wh(_curPoint.toPoint() - _celPos.toPoint());            // Width & Height
    wh -= tl;

//    // Should only work for Ellipse and Box (turn them into circle and Square)
//    if (_sameSize) {
//        // Same size contraints
//        int size = qMax(wh.x(), wh.y());
//        wh.setX(size);
//        wh.setY(size);
//    }

    if (_sameSize) {
        int whx = wh.x();
        int why = wh.y();
        int wMag = qAbs(wh.x());
        int hMag = qAbs(wh.y());

        if (why < 0) {
            // Top-

            if (whx < 0) {
                // -Left
                int size = qMin(whx, why);
                wh.setX(size);
                wh.setY(size);
            } else if (whx > 0) {
                // -Right
                if (whx > qAbs(why))
                    wh.setY(whx);
                else if (qAbs(why) > whx)
                    wh.setX(qAbs(why));
            }
        } else if (why > 0) {
            // Bottom-

            if (whx < 0) {
                // -Left
                if (qAbs(whx) > qAbs(why))
                    wh.setY(qAbs(whx));
                else if (qAbs(why) > qAbs(whx))
                    wh.setX(qAbs(why));
            } else if (whx > 0) {
                // -Right
                int size = qMax(whx, why);
                wh.setX(size);
                wh.setY(size);
            }
        }

    } else {
        // Make it so that tl is always the Top-Left point
        if (wh.x() <= -1) {
            tl.rx() += wh.x();
            wh.rx() = qAbs(wh.x());
        }
        if (wh.y() <= -1) {
            tl.ry() += wh.y();
            wh.ry() = qAbs(wh.y());
        }
    }


    // Chose a shape to draw
    switch (_shape) {
        case Box:
            // Draw the four corners (have to do this because of how QPainter works
            drawBuffPainter.drawPoint(tl);                        // Top Left
            drawBuffPainter.drawPoint(tl.x() + wh.x(), tl.y());    // Top Right
            drawBuffPainter.drawPoint(tl + wh);                    // Bottom Right
            drawBuffPainter.drawPoint(tl.x(), tl.y() + wh.y());

            // Draw the box
            drawBuffPainter.drawRect(tl.x(), tl.y(), wh.x(), wh.y());
            break;

        case Ellipse:
            // Always draw a point
            if (wh.isNull())
                drawBuffPainter.drawPoint(tl);
            
            drawBuffPainter.drawEllipse(tl.x(), tl.y(), wh.x(), wh.y());
            break;

        case Polygon:
            // TODO can't really rely on QPainter's line drawing system since wern't not getting
            //      lines from pixel to pixel.  It's kind of difficult to explain, but we need
            //      to figure out how to work with it so we Don't have to do this janky hack
            //      down here and implement the bresenham line algorithm for what should be a
            //      minor task.  the current method also makes it so Antialising cannot work.
            // If we have some points, then draw them.
            if (_points.size() > 0) {
                QVector<QPoint> toDraw;
                _points.push_back(_curPoint);        // Temporary

                // Go thorugh all of the points
                QList<QPointF>::const_iterator iter = _points.begin();
                QPointF cur, prev = *iter;
                toDraw << prev.toPoint();            // Add the first
                while ((iter + 1) < _points.end()) {
                    // goto the next point
                    iter++;
                    cur = *iter;

                    // Add the point to the vector to draw
                    toDraw << cur.toPoint();
                    toDraw << math::bresenhamLinePoints(prev, cur).toVector();

                    // Setup for the next point
                    prev = cur;
                }

                // Do the drawing of the points
                drawBuffPainter.save();
                drawBuffPainter.translate(-_celPos);
                drawBuffPainter.drawPoints(toDraw.data(), toDraw.size());
                drawBuffPainter.restore();

                _points.pop_back();                // And take it off
            }
            break;
    }


    drawBuffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    drawBuffPainter.fillRect(0, 0, drawBuff.width(), drawBuff.height(), _pen.color());

    // Then put it onto the Cel
    QImage celBuff = _celImage->copy();
    QPainter celBuffPainter(&celBuff);
    celBuffPainter.drawImage(0, 0, drawBuff);

    // Apply the update
    BlitApp::app()->copyOntoCel(celBuff);
}


/*!
    Used to reset the state of the tool after a drawing is done.
*/
void ShapeTool::_reset() {
    // Only do stuff if the pen is down
    if (!_penDown)
        return;

    // Null out
    _startPoint = QPointF();
    _curPoint = QPointF();
    if (_shape == Polygon) {
        _path = QPainterPath();
        _points.clear();
    }

    // Cel stuff
    _celPos = QPointF();
    delete _celImage;
    _celImage = NULL;

    // Bring the pen back up
    _penDown = false;
}

