// File:         editorcontainer.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the EditorContainer class.


#include "widgets/editorcontainer.h"
#include "blitapp.h"
#include "widgets/drawing/canvas.h"
#include <QGridLayout>


EditorContainer::EditorContainer(BlitApp *parent) :
    QWidget(parent)
{
    // Construct the canvas, and the rulser
    // This function will also set the Canvas pointer in the BlitApp class

    _canvas = new Canvas(this);
    // TODO add rulers
    parent->setCanvas(_canvas);            // Set the canvas in the parent

    // Hookup slots and signals
    // connect(_canvas, &Canvas::mouseMoved, _hRuler, &Ruler::setMousePos);
    // connect(_canvas, &Canvas::mouseMoved, _vRuler, &Ruler::setMousePos);

    // Layout
    _layout = new QGridLayout(this);
    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    //_layout->addWidget(_hRuler, 0, 1);
    //_layout->addWidget(_vRuler, 1, 0);
    _layout->addWidget(_canvas, 1, 1);

    // Other things
    setMinimumSize(320, 240);
    showRulers(_showRulers);

}


bool EditorContainer::rulersShowing() {
    // Tells if the rulers are showing or not
    return _showRulers;
}


void EditorContainer::showRulers(bool showThem) {
    // If "showThem," is set to true, then it will turn on the rulers else, else it will turn them off.
    if (_showRulers != showThem) {
        _showRulers = showThem;

        if (_showRulers) {
            // _hRuler->show();        
            // _vRuler->show();        
        } else {
            // _hRuler->hide();        
            // _vRuler->hide();        
        }
    }
}

