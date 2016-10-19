// File:         colourframe.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the ColorFrame, SelfChangingColorFrame, and
//               ClickableColorFrame classes.


#include "widgets/colorframe.h"
#include "util.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>
#include <QColorDialog>
#include <QPaintEvent>


/*== Color Frame methods ==*/
ColorFrame::ColorFrame(QWidget *parent) :
    QFrame(parent)
{
    // Defaults
    QColor clr = COLOR_FRAME_DEFAULT_COLOR;
    QSize size = COLOR_FRAME_DEFAULT_SIZE;

    // Layout stuff
    setFrameStyle(QFrame::Box);
    setFixedSize(size);

    // Initial vlues
    setSquareSize(COLOR_FRAME_DFEAULT_SQUARE_SIZE);
    setColor(clr);
}


void ColorFrame::setSquareSize(QSize size) {
    // Sets the size of the transparency square in the background, width and height must be positive
    // integers, or the size will not be changed.  Will cause a repaint on successful change
    if ((size.width() < 1) || (size.height() < 1))
        return;

    _squareSize = size;
    repaint();
}


QSize ColorFrame::squareSize() {
    // Returns the size of the transparency square for the background
    return _squareSize;
}


QColor ColorFrame::color() {
    // Returns the color that is currently set in the box
    return _clr;
}


void ColorFrame::setColor(QColor clr) {
    // Will change the color set in the box (causing a repaitn).  Will only work of the color is valid.
    // Will also emit the colorChagned signal
    if (!clr.isValid())
        return;
    
    // Okay, set it, update and redraw
    _clr = clr;
    emit colorChanged(_clr);
    repaint();
}


void ColorFrame::paintEvent(QPaintEvent *event) {
    // Overridden method used to draw the color over the box.  
    QPainter qp(this);

    // Chose how to paint
    if (_clr.alpha() == 0xFF)
        qp.fillRect(event->rect(), _clr);    // Opaque draw
    else {
        // Draw some transparency
        int x = 0, y = 0;
        QColor startRowWith = Qt::white;        // What color to start the row with
        QColor nextClr;

        // Draw boxes
        while (y < event->rect().height()) {

            nextClr = startRowWith;

            // Where the boxes are really drawn
            while (x < event->rect().width()) {
                qp.fillRect(x, y, _squareSize.width(), _squareSize.height(), nextClr);

                // Flip colors and set next position
                nextClr = (nextClr == Qt::white) ? Qt::gray : Qt::white;
                x += _squareSize.width();
            }
        
            // Flip colors and set next positions
            startRowWith = (startRowWith == Qt::white) ? Qt::gray : Qt::white;
            x = 0;
            y += _squareSize.height();
        }

        // Draw the color as an overlay
        if (_clr.alpha() != 0x00)
            qp.fillRect(event->rect(), _clr);
    }

    QFrame::paintEvent(event);    // We need that box
}








/*== Self Changing Color Frame methods ==*/
SelfChangingColorFrame::SelfChangingColorFrame(QWidget *parent) :
    ColorFrame(parent)
{
}


void SelfChangingColorFrame::mouseDoubleClickEvent(QMouseEvent *event) {
    // Upon double clicking this, it will bring up a dialog window to change the color
    QColor clr = QColorDialog::getColor(_clr, this, "Select Color", QColorDialog::ShowAlphaChannel);

    // Check for validity and chnage if so
    if (clr.isValid() && (clr != _clr))
        setColor(clr);        
}







/*== Clickable Color Frame methods ==*/
ClickableColorFrame::ClickableColorFrame(QWidget *parent) :
    ColorFrame(parent)
{
    // Nothing extra
}


void ClickableColorFrame::mousePressEvent(QMouseEvent *event) {
    // Passes up a mouse press event
    emit pressed(event);
    QWidget::mousePressEvent(event);
}


void ClickableColorFrame::mouseReleaseEvent(QMouseEvent *event) {
    // Passes up a mouse release event
    emit released(event);
    QWidget::mouseReleaseEvent(event);
}


void ClickableColorFrame::mouseDoubleClickEvent(QMouseEvent *event) {
    // Passes up a double click event
    emit doubleClicked(event);
    QWidget::mouseDoubleClickEvent(event);
}




//bool ClickableColorFrame::isSelected() {
//    // Returns the status if the widget is selected or not
//    return _selected;
//}
//
//
//void ClickableColorFrame::setSelected(bool yesOrNo) {
//    // Will progmatically select the widget.  Does nothing if the state doesn't change.
//    if (_selected != yesOrNo) {
//        // different
//        _selected = yesOrNo;
//
//        // TODO maybe changes size instead
////        // Set an inverse color for border, and line width
////        QPalette pal = QPalette(palette());
////        QColor invClr = util::invert(_clr);
////        invClr.setAlpha(0xFF);
////        pal.setColor(QPalette::WindowText, invClr);
////        setLineWidth(_selected ? 2 : 1);
////        setPalette(pal);
//
//        // Emit signals last
//        if (_selected)
//            emit selected();
//    }
//}
