// File:         rgbslider.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the RGBSlider color chooser



#include "widgets/colorchoosers/rgbslider.h"
#include "ui_rgb_slider.h"
#include "widgets/colorframe.h"
#include <QColor>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QGridLayout>


RGBSlider::RGBSlider(QWidget *parent) :
    ColorChooser(parent),
    _ui(new Ui::RGBSlider)
{
//    _mkWidgets();
//    _setupLayout();

    // Setup the UI
    _ui->setupUi(this);
    _colorBox = _ui->colorBox;
    _hexEdit = _ui->hexEdit;

    // Set some defautls
    _colorBox->setColor(_clr);
    _hexEdit->setText(_clr.name().toUpper());

    _connectWidgets();
}


RGBSlider::~RGBSlider() {
    delete _ui;
}


QColor RGBSlider::currentColor() {
    // Returns the current stored color
    return _clr;
}


void RGBSlider::setCurrentColor(QColor clr) {
    // Will set the color of the RGB Slider, only if the supplied color is valid
    if (clr.isValid()) {
        _lockSlidersSpinners = true;

        // Change out the colors
        _clr = clr;
        _ui->rSlider->setValue(_clr.red());
        _ui->gSlider->setValue(_clr.green());
        _ui->bSlider->setValue(_clr.blue());
        _ui->aSlider->setValue(_clr.alpha());
        _hexEdit->setText(_clr.name().toUpper());

        // Emit a signal
        emit colorChanged(_clr);

        _lockSlidersSpinners = false;
    }
}


void RGBSlider::onHexEdited(QString hexClr) {
    // If the Hex Code Line edit is given a valid Hex color, this will update the sliders and spinners
    // emit the colorChanged signal.  If an invalid color is given, then it will do nothing.  You can also
    // supply a 6 character hex code color without the # infront.

    // Preliminary stuff
    if (hexClr.size() == 0)
        return;
    hexClr = hexClr.toUpper();

    // First check for no #
    QColor clr(hexClr);
    if (hexClr.size() == 6) {
        QColor tmp("#" + hexClr);
        if (tmp.isValid())
            clr = tmp;
    }

    // Say they have a #, but a named color
    if (hexClr.startsWith("#")) {
        QColor tmp(hexClr.mid(1));
        if (tmp.isValid())
            clr = tmp;
    }

    // Check for validity
    if (clr.isValid()) {
        // Prelim stuff
        _lockSlidersSpinners = true;
        _hexEdit->setStyleSheet("");
        
        // Change out the colors
        _clr = clr;
        _ui->rSlider->setValue(_clr.red());
        _ui->gSlider->setValue(_clr.green());
        _ui->bSlider->setValue(_clr.blue());
        _clr.setAlpha(_ui->aSlider->value());
        // Preserve the Alpha

        // Update the box
        emit colorChanged(_clr);
        _lockSlidersSpinners = false;
    } else 
        _hexEdit->setStyleSheet("background-color:#FF6060");    // Make the hex edit red
}


void RGBSlider::onHexEditFinished() {
    // Small little slot used to "normalize," the hex edit when editing is finished (i.e. Return Key
    // pressed or widget is unfocused).
    if (_clr.isValid()) {
        _hexEdit->setStyleSheet("");
        _hexEdit->setText(_clr.name().toUpper());
        setFocus();
    }
}


void RGBSlider::onSliderSpinnerChanged() {
    // When a Slider or Spinner's value is changed, this will figure out the newly set color, then emit
    // the colorChanged signal.  Which intern will update the ColorBox.  It will also update the Hex Code
    // Color box below.

    // Only work of the Hex edit box isn't being used.
    if (!_lockSlidersSpinners) {
        // Slider/Spinner values are guarenteed to be the same
        _clr.setRed(_ui->rSlider->value());
        _clr.setGreen(_ui->gSlider->value());
        _clr.setBlue(_ui->bSlider->value());
        _clr.setAlpha(_ui->aSlider->value());
    
        // Change the text box
        _hexEdit->setStyleSheet("");
        _hexEdit->setText(_clr.name().toUpper());

        emit colorChanged(_clr);
    }
}


void RGBSlider::_mkWidgets() {
//    // Small internal function to make all of the widgets
//
//    // Color Box is simple
//    _colorBox = new ColorFrame(this);
//    _colorBox->setColor(_clr);
//
//    // Hex Code box
//    _hexEdit = new QLineEdit(this);
//    _hexEdit->setText(_clr.name().toUpper());
//    _hexEdit->setFixedWidth(_colorBox->width());
//
//    // Sliders
//    _rSlider = new QSlider(Qt::Horizontal, this);
//    _gSlider = new QSlider(Qt::Horizontal, this);
//    _bSlider = new QSlider(Qt::Horizontal, this);
//    _aSlider = new QSlider(Qt::Horizontal, this);
//
//    _rSlider->setSingleStep(1);
//    _gSlider->setSingleStep(1);
//    _bSlider->setSingleStep(1);
//    _aSlider->setSingleStep(1);
//
//    _rSlider->setPageStep(8);
//    _gSlider->setPageStep(8);
//    _bSlider->setPageStep(8);
//    _aSlider->setPageStep(8);
//
//    _rSlider->setRange(0x00, 0xFF);
//    _gSlider->setRange(0x00, 0xFF);
//    _bSlider->setRange(0x00, 0xFF);
//    _aSlider->setRange(0x00, 0xFF);
//
//    _rSlider->setFixedWidth(_sliderWidth);
//    _gSlider->setFixedWidth(_sliderWidth);
//    _bSlider->setFixedWidth(_sliderWidth);
//    _aSlider->setFixedWidth(_sliderWidth);
//
//    // Spinners
//    _rSpinner = new QSpinBox(this);
//    _gSpinner = new QSpinBox(this);
//    _bSpinner = new QSpinBox(this);
//    _aSpinner = new QSpinBox(this);
//
////    _rSpinner->setPageStep(8);
////    _gSpinner->setPageStep(8);
////    _bSpinner->setPageStep(8);
////    _aSpinner->setPageStep(8);
//    
//    _rSpinner->setRange(0x00, 0xFF);
//    _gSpinner->setRange(0x00, 0xFF);
//    _bSpinner->setRange(0x00, 0xFF);
//    _aSpinner->setRange(0x00, 0xFF);
//
//    // Labels
//    _rLabel = new QLabel("R", this);
//    _gLabel = new QLabel("G", this);
//    _bLabel = new QLabel("B", this);
//    _aLabel = new QLabel("A", this);
//
//    // Last few things
//    _aSlider->setValue(0xFF);
//    _aSpinner->setValue(0xFF);
}


void RGBSlider::_setupLayout() {
//    // Small interal function (for code organization) to setup the layout
//    QGridLayout *layout = new QGridLayout(this);
//
//    // Red
//    layout->addWidget(_rLabel, 0, 0);
//    layout->addWidget(_rSlider, 0, 1);
//    layout->addWidget(_rSpinner, 0, 2);
//
//    // Green
//    layout->addWidget(_gLabel, 1, 0);
//    layout->addWidget(_gSlider, 1, 1);
//    layout->addWidget(_gSpinner, 1, 2);
//
//    // Blue
//    layout->addWidget(_bLabel, 2, 0);
//    layout->addWidget(_bSlider, 2, 1);
//    layout->addWidget(_bSpinner, 2, 2);
//
//    // Alpha
//    layout->addWidget(_aLabel, 3, 0);
//    layout->addWidget(_aSlider, 3, 1);
//    layout->addWidget(_aSpinner, 3, 2);
//
//    // Other widgets
//    layout->addWidget(_colorBox, 0, 3, 2, 1);
//    layout->addWidget(_hexEdit, 2, 3);
//
//    // Set layout and geometry for widget
//    layout->setSpacing(2);
//    layout->setContentsMargins(0, 0, 0, 0);
//    setLayout(layout);
//    setFixedSize(sizeHint());
}


void RGBSlider::_connectWidgets() {
    // Small internal function to connect all of the widgets
    void (QSlider::*sliderValueChanged)(int) = &QSlider::valueChanged;
    void (QSpinBox::*spinnerValueChanged)(int) = &QSpinBox::valueChanged;

    /*== Make the sliders and sliders talk to eachother ==*/
    // sliders -> spinners
    connect(_ui->rSlider, sliderValueChanged, _ui->rSpinner, &QSpinBox::setValue);
    connect(_ui->gSlider, sliderValueChanged, _ui->gSpinner, &QSpinBox::setValue);
    connect(_ui->bSlider, sliderValueChanged, _ui->bSpinner, &QSpinBox::setValue);
    connect(_ui->aSlider, sliderValueChanged, _ui->aSpinner, &QSpinBox::setValue);

    // spinners -> sliders
    connect(_ui->rSpinner, spinnerValueChanged, _ui->rSlider, &QSlider::setValue);
    connect(_ui->gSpinner, spinnerValueChanged, _ui->gSlider, &QSlider::setValue);
    connect(_ui->bSpinner, spinnerValueChanged, _ui->bSlider, &QSlider::setValue);
    connect(_ui->aSpinner, spinnerValueChanged, _ui->aSlider, &QSlider::setValue);

    // sliders -> widget
    connect(_ui->rSlider, sliderValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->gSlider, sliderValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->bSlider, sliderValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->aSlider, sliderValueChanged, this, &RGBSlider::onSliderSpinnerChanged);

    // spinenrs -> widget
    connect(_ui->rSpinner, spinnerValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->gSpinner, spinnerValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->bSpinner, spinnerValueChanged, this, &RGBSlider::onSliderSpinnerChanged);
    connect(_ui->aSpinner, spinnerValueChanged, this, &RGBSlider::onSliderSpinnerChanged);

    // Hex Code Edit -> Widget
    connect(_hexEdit, &QLineEdit::textEdited, this, &RGBSlider::onHexEdited);
    connect(_hexEdit, &QLineEdit::editingFinished, this, &RGBSlider::onHexEditFinished);

    // Widget -> Color Box
    connect(this, &RGBSlider::colorChanged, _colorBox, &ColorFrame::setColor);

}


