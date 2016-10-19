// File:         rgbslider.h
// Author:       Ben Summerton (define-private-public)
// Description:  RGBSlider is a ColorChooser implementation that choosers colors using three sliders (RGB)
//               and as well includes one more slider for the Alpha channel.  Also features spin boxes


#ifndef RGB_SLIDER_H
#define RGB_SLIDER_H


#define RGB_SLIDER_DEFAULT_SLIDER_WIDTH 150


#include "widgets/colorchoosers/colorchooser.h"
class ColorFrame;
class QLabel;
class QLineEdit;
class QSpinBox;
class QSlider;


namespace Ui {
    class RGBSlider;
}


class RGBSlider : public ColorChooser {
    Q_OBJECT;

public:
    RGBSlider(QWidget *parent=NULL);
    ~RGBSlider();

    // Methods
    QColor currentColor();


public slots:
    void setCurrentColor(QColor clr);

    // For updating the internal widgets
    void onHexEdited(QString hexClr);
    void onHexEditFinished();
    void onSliderSpinnerChanged();


private:
    // Variables
    QColor _clr = Qt::black;
    int _sliderWidth = RGB_SLIDER_DEFAULT_SLIDER_WIDTH;
    bool _lockSlidersSpinners = false;

    // Widgets
    ColorFrame *_colorBox;
    QLineEdit *_hexEdit;
    Ui::RGBSlider *_ui;

//    QSlider *_rSlider;
//    QSlider *_gSlider;
//    QSlider *_bSlider;
//    QSlider *_aSlider;
//    QSpinBox *_rSpinner;
//    QSpinBox *_gSpinner;
//    QSpinBox *_bSpinner;
//    QSpinBox *_aSpinner;
//    QLabel *_rLabel;
//    QLabel *_gLabel;
//    QLabel *_bLabel;
//    QLabel *_aLabel;

    // internal functions
    void _mkWidgets();
    void _setupLayout();
    void _connectWidgets();

};


#endif // RGB_SLIDER_H

