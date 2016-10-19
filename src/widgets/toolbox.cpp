// File:         toolbox.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementatoin of the Toolbox widget


// TODO remove the individual tools once they're plugins
#include "widgets/toolbox.h"
#include "tools/tool.h"
#include "tools/pentool.h"
#include "tools/brushtool.h"
#include "tools/erasertool.h"
#include "tools/linetool.h"
#include "tools/shapetool.h"
#include "tools/filltool.h"
#include "tools/movetool.h"
#include "tools/resizetool.h"
#include "tools/colorpickertool.h"
#include <QGridLayout>
#include <QToolButton>


Toolbox::Toolbox(QWidget *parent) :
    QWidget(parent)
{
    // Load up the tools
    // TODO should be plugins
    _tools.append(new PenTool(this));
//    _tools.append(new BrushTool(this));
    _tools.append(new EraserTool(this));
    _tools.append(new LineTool(this));
    _tools.append(new ShapeTool(this));
    _tools.append(new FillTool(this));
    _tools.append(new MoveTool(this));
//    _tools.append(new ResizeTool(this));
    _tools.append(new ColorPickerTool(this));

    // Widget construction and signals/slots
    _createWidgets();
    _setupLayout();
    for (int t = 0; t < _toolButtons.size(); t++)
        connect(_toolButtons[t], &QToolButton::clicked, this, &Toolbox::onToolButtonClicked);

    // Map the tools to the tool buttons. 
    // TODO make should be in a different funciton.  It's kind of bad to assume they are created
    //      in the same order
    for (int i = 0; i < _tools.size(); i++) {
        Tool *t = _tools[i];
        QToolButton *tb = _toolButtons[i];
        _toolMap.insert(t, tb);
        _toolButtonMap.insert(tb, t);
    }

    // Setup the first tool
    _curTool = _tools[0];
    _toolButtons[0]->click();
    emit curToolChanged(_curTool);
//    // TODO redundant code w/ the onToolButtonClicked method, consolidate it to one funciton
//    if (_tools.size() > 6) {
//        _curTool = _tools[6];
//        _toolButtons[6]->click();
//        emit curToolChanged(_curTool);
//    }
}


Tool *Toolbox::curTool() {
    // Returns a pointer to the currently loaded Tool.  There is the possiblilty that this funciton
    // could return a NULL pointer, but that's very unlikely.
    return _curTool;
}


void Toolbox::_createWidgets() {
    // Internal organizational function to create the widgets of the Toolbox.

    // Make the QToolButtons 
    for (int t = 0; t < _tools.size(); t++) {
        QToolButton *tb = new QToolButton(this);
        tb->setCheckable(true);
        tb->setIconSize(QSize(20, 20));
        tb->setIcon(_tools[t]->icon());
        tb->setToolTip(_tools[t]->name() + ": " + _tools[t]->desc());

        _toolButtons.append(tb);
    }
}


void Toolbox::_setupLayout() {
    // Internal organizational funciton to layout all of the widget in the Dialog
    // Pre-condition for this function is that all of the widgets have been setup
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    // Add the Tools
    int r, c;
    for (int t = 0; t < _toolButtons.size(); t++) {
        c = t % TOOLBOX_MAX_TOOLS_PER_ROW;
        r = t / TOOLBOX_MAX_TOOLS_PER_ROW;
        layout->addWidget(_toolButtons[t], r, c);
    }

}


void Toolbox::onToolButtonClicked(bool checked) {
    // When a tool is clicked, this method will be called.  When a Tool Button is clicked, it will
    // make that the current tool.  Only one tool at a time can be the current tool

    // Get the sender Tool.  Check to make sure that sender is a button.  If it is, assume its
    // in the map
    QToolButton *tb = qobject_cast<QToolButton *>(sender());
    if (!tb)
        return;
    Tool *tool = _toolButtonMap.value(tb);

    if (tool != _curTool) {
        // New Tool selected
        // Unset the old Tool
        QToolButton *oldTb = _toolMap.value(_curTool);
        oldTb->setChecked(false);

        // Set the new Tool
        _curTool = tool;
        emit curToolChanged(_curTool);
    } else {
        // Same tool was pressed, just make sure the button stays checked
        tb->setChecked(true);
    }
}
