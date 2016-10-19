// File:         toolswindow.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementatoin of the ToolsWindow class.



#include "widgets/toolswindow.h"
#include "ui_tools_window.h"
#include "blitapp.h"
#include "widgets/toolbox.h"
#include "widgets/colorpalette.h"
#include "widgets/colorchoosers/colorchooser.h"
#include "widgets/colorchoosers/rgbslider.h"
#include "tools/tool.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QToolButton>
#include <QScrollArea>
#include <QColor>
#include <QPushButton>



ToolsWindow::ToolsWindow(BlitApp *blitapp, QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ToolsWindow)
{
    // GUI
    _ui->setupUi(this);
    _colorChooser = _ui->colorChooser;        // Get the RGBSlider

    // Frame data
    setWindowTitle("Tools");
//    setWindowFlags(Qt::Tool);
    setMinimumWidth(TOOL_OPTIONS_PANEL_MAX_WIDTH);
    setFixedSize(sizeHint());

    // Signals & Slots
    // TODO what happens of the colorchooser changes?
    connect(_ui->toolbox, &Toolbox::curToolChanged, this, &ToolsWindow::onCurToolChanged);
    connect(_ui->colorChooser, &ColorChooser::colorChanged, blitapp, &BlitApp::setCurColor);
    connect(_ui->colorPalette, &ColorPalette::swatchSelected, _ui->colorChooser, &ColorChooser::setCurrentColor);
    connect(blitapp, &BlitApp::curColorChanged, _ui->colorChooser, &ColorChooser::setCurrentColor);
    connect(_ui->colorPalette->addSwatchButton(), &QPushButton::clicked, this, &ToolsWindow::_onAddSwatchButtonClicked);


    // Last things
    onCurToolChanged(_ui->toolbox->curTool());
}

ToolsWindow::~ToolsWindow() {
    delete _ui;
}


void ToolsWindow::_onAddSwatchButtonClicked(bool checked) {
    // Slot is activated when the Add Swatch Button of the ColorPalette widget is clicked.  It will
    // Add the current color to the color palette (if it isn't already there).
    _ui->colorPalette->addColor(BlitApp::app()->curColor());
    BlitApp::app()->savePalette();
}


Toolbox *ToolsWindow::toolbox() {
    // Returns a pointer to the Toolbox widget
    return _ui->toolbox;
}


ColorPalette *ToolsWindow::colorPalette() {
    // Returns a pointer to the ColorPalette widget
    return _ui->colorPalette;
}


void ToolsWindow::onCurToolChanged(Tool *tool) {
    // When the current tool is changed, this slot will swap out the old tool's options panel with the
    // newly selected Tool's panel.
    QWidget *op = tool->options();
    _ui->toolOptionsPanel->setWidget(op);

    if (op)
        _ui->toolOptionsPanel->show();
    else
        _ui->toolOptionsPanel->hide();

}


void ToolsWindow::closeEvent(QCloseEvent *event) {
    // Overloaded function.  Re-implemented to make sure the window doesn't close on the "X," button clicked
    event->accept();
}

