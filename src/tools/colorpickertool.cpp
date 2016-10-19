// File:         colorpickertool.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the ColorPickerTool.


#include "tools/colorpickertool.h"
#include "blitapp.h"
#include "animation/celref.h"
#include <QtMath>
#include <QPixmap>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <QGuiApplication>
#include <QScreen>



// Export the Tool
//Q_EXPORT_PLUGIN2(blit_tool_brush, ColorPickerTool);


/*!
    Sets up the Color Picker Tool.  \a parent should be the Toolbox that contains
    this tool.
*/
ColorPickerTool::ColorPickerTool(QObject *parent) :
    Tool(parent)
{

}

/*!
    Cleans up the tool.
*/
ColorPickerTool::~ColorPickerTool() {

}


/*!
    returns the name of the Tool
*/
QString ColorPickerTool::name() {
    return tr("Color Picker");
}


/*!
    A Tool tip that describes what the tool does
*/
QString ColorPickerTool::desc() {
    return tr("Pick a color.");
}


/*!
    Returns a picture/icon for the tool.
*/
QIcon ColorPickerTool::icon() {
    return QIcon(":/color_picker_tool_icon");
}


/*!
    If the color picking process is started, this will change the color that is
    being chosen.
*/
void ColorPickerTool::onMouseMoved(QGraphicsSceneMouseEvent *event){
    if (_picking) {
        // Set the current color
        QPoint pos = event->screenPos();
        QColor clr = _getColorAt(pos.x(), pos.y());
        if (clr.isValid())
            BlitApp::app()->setCurColor(clr);
    }
}


/*!
    When the mouse is pressed, the color picking process will start.
    
    It will take a screenshot of the current screen.
*/
void ColorPickerTool::onMousePressed(QGraphicsSceneMouseEvent *event){
    if (!_picking) {
        // Onyly start picking if we aren't already
        _picking = true;

        // Take a screenshot
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen)
            _screenshot = screen->grabWindow(0).toImage();

        // Set the current color
        QPoint pos = event->screenPos();
        QColor clr = _getColorAt(pos.x(), pos.y());
        if (clr.isValid())
            BlitApp::app()->setCurColor(clr);
    }
}


/*!
    When the mouse is released, the color will be selected.
*/
void ColorPickerTool::onMouseReleased(QGraphicsSceneMouseEvent *event){
    if (_picking) {
        // Free resources
        _picking = false;
    }
}


/*!
    Returns the color at pixel location (\a x, \a y).  Returns an invalid
    QColor of the location doesn't exist.
*/
QColor ColorPickerTool::_getColorAt(int x, int y) {
    QColor clr;

    // Check location first
    if ((x >= _screenshot.width()) || (y >= _screenshot.height()))
        return clr;            // Returns as invalid
    
    // good location
    clr.setRgb(_screenshot.pixel(x, y));
    return clr;
}

