// File:         colorchooser.h
// Author:       Ben Summerton (define-private-public)
// Description:  ColorChooser is an abstract interface for Color Choosing widtets.  For example, there
//               might be a "SliderColorChooser," or  "WheelColorChooser."  They all will share this
//               common interace for access


#ifndef COLOR_CHOOSER_H
#define COLOR_CHOOSER_H


#include <QWidget>
class QColor;


class ColorChooser: public QWidget {
    Q_OBJECT;

public:
    ColorChooser(QWidget *parent=NULL) : QWidget(parent) {}

    virtual QColor currentColor() = 0;


signals:
    void colorChanged(QColor clr);


public slots:
    virtual void setCurrentColor(QColor clr) = 0;

};


#endif // COLOR_CHOOSER_H

