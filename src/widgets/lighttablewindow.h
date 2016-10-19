// File:         lighttablewindow.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the LightTableWindow class.


#ifndef LIGHT_TABLE_WINDOW_H
#define LIGHT_TABLE_WINDOW_H


#include <QWidget>
#include <QPointer>
class BlitApp;
class Canvas;


namespace Ui {
    class LightTableWindow; 
}


class LightTableWindow : public QWidget {
    Q_OBJECT;

public:
    // Constructor/Deconstructor
    LightTableWindow(BlitApp *blitApp, QWidget *parent=NULL);
    ~LightTableWindow();

    // Canvas
    void setCanvas(Canvas *canvas);


signals:
    void fadeStepSliderValueChanged(qreal value);        // Value is between [1.0, 5.0]


public slots:


private slots:
    void _onFadeStepSliderValueChanged(int value);


protected:
    void closeEvent(QCloseEvent *event);


private:
    // Member vars
    Ui::LightTableWindow *_ui;
    QPointer<Canvas> _canvas;

};


#endif // LIGHT_TABLE_WINDOW_H

