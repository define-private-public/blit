// File:         lighttablewindow.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the LightTableWindow class.


/*!
    \inmodule Widgets
    \class LightTableWindow
    \brief The LightTableWindow is a widget that is used to control the Canvas' Light Table features.

    A very simple UI wrapper.  It talks to the main canvas inside of BlitApp and can turn things on/off.
    There should only be one instance of this class.
*/


//#include <QDebug>
#include "widgets/lighttablewindow.h"
#include "ui_light_table_window.h"
#include "blitapp.h"
#include "widgets/drawing/canvas.h"
#include <QCloseEvent>
#include <QDebug>


/*!
    Constructs the Light Table Window
*/
LightTableWindow::LightTableWindow(BlitApp *blitApp, QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::LightTableWindow)
{
    Q_ASSERT(parent != NULL);

    // GUI
    _ui->setupUi(this);
//    setWindowFlags(Qt::Tool);
    setFixedSize(sizeHint());

    // Signals & slots
    void (QSlider::*sliderValueChanged)(int) = &QSlider::valueChanged;
    connect(_ui->fadeStepSlider, sliderValueChanged, this, &LightTableWindow::_onFadeStepSliderValueChanged);

    qDebug() << "[LightTableWindow created] " << this;
}


/*!
    Does all of the cleanup for the LightTableWindow.
*/
LightTableWindow::~LightTableWindow() {
    delete _ui;

    qDebug() << "[LightTableWindow destroyed] " << this;
}


/*!
    Hooks up UI slots/signals with \a canvas.  Will disconnect a previously Canvas.
    It is possible to pass NULL to \a canvas, but that will only do the disconnection.
*/
void LightTableWindow::setCanvas(Canvas *canvas) {
    // Needed for signals & slots
    void (QSpinBox::*spinnerValueChanged)(int) = &QSpinBox::valueChanged;

    // Disconnect the one before
    if (_canvas) {
        disconnect(_ui->lightTableOnButton, 0, _canvas, 0);
        disconnect(_ui->lightTableLoopButton, 0, _canvas, 0);
    }

    // Only connect if canvas != NULL
    _canvas = canvas;
    if (!_canvas)
        return;
    
    // Add the new canvas.
    connect(_ui->lightTableOnButton, &QToolButton::clicked, _canvas, &Canvas::turnOnLightTable);
    connect(_ui->lightTableLoopButton, &QToolButton::clicked, _canvas, &Canvas::setLightTableLooping);
    connect(_ui->numBeforeSpinner, spinnerValueChanged, _canvas, &Canvas::setLightTableNumBefore);
    connect(_ui->numAfterSpinner, spinnerValueChanged, _canvas, &Canvas::setLightTableNumAfter);
    connect(this, &LightTableWindow::fadeStepSliderValueChanged, _canvas, &Canvas::setLightTableFadeStep);
}


/*!
    Connected to a QSlider::valueChanged() signal, this will emit the
    fadeStepSliderValueChanged() signal.
*/
void LightTableWindow::_onFadeStepSliderValueChanged(int value) {
    // Invert it
    qreal intensity = value / 100.0;    // value = [100, 500] -> [1.0, 5.0]
    intensity = (5 + 1) - intensity;    // +1 there is because slider starts at 100

    emit fadeStepSliderValueChanged(intensity);
}


/*!
    Overloaded function.

    Re-implemented to make sure the window doesn't close on the "X," button
    clicked
*/
void LightTableWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}
