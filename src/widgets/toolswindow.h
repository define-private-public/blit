// File:         toolswindow.h
// Author:       Ben Summerton (define-private-public)
// Description:  ToolsWindow is a collection of widgets that go on the left side of the QMainWindow
//               (BlitApp) and contains tools and other options for editing the animaiton.


#ifndef TOOLS_WINDOW_H
#define TOOLS_WINDOW_H


//#define TOOLS_WINDOW_WIDTH 200
//#define TOOLS_WINDOW_HEIGHT 670


#include <QWidget>
class BlitApp;
class Tool;
class Toolbox;
class ColorPalette;
class ColorChooser;
class QColor;
class QCloseEvent;
class QScrollArea;


namespace Ui {
    class ToolsWindow;
}


class ToolsWindow : public QWidget {
    Q_OBJECT;

public:
    ToolsWindow(BlitApp *blitapp, QWidget *parent=NULL);
    ~ToolsWindow();

    // Accessors methods
    Toolbox *toolbox();
    ColorPalette *colorPalette();


public slots:
    void onCurToolChanged(Tool *tool);


private slots:
    void _onAddSwatchButtonClicked(bool checked=false);


protected:
    void closeEvent(QCloseEvent *event);


private:
    // Member vars
    // GUI Widgets
    Ui::ToolsWindow *_ui;
    ColorChooser *_colorChooser;
//    ColorPalette *_colorPalette;
//    Toolbox *_toolbox;
//    QScrollArea *_toolOptionsPanel;

};


#endif // TOOLS_WINDOW_H

