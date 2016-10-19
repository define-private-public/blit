// File:         editorcontainer.h
// Author:       Ben Summerton (define-private-public)
// Description:  The "Editor Container," is the central widget that appears in the MainWindow (i.e
//               BlitApp).  It will contain the Canvas widget, but may also contain other things


#ifndef EDITOR_CONTAINER_H
#define EDITOR_CONTAINER_H


#include <QWidget>
class BlitApp;
class Canvas;
class QGridLayout;


class EditorContainer : public QWidget {
    Q_OBJECT;

public:
    EditorContainer(BlitApp *parent);

    bool rulersShowing();


public slots:
    void showRulers(bool showThem);


private:
    bool _showRulers = false;        // Boolean value to show the rulers or not

    // GUI
    QGridLayout *_layout;
    Canvas *_canvas;

    // TODO add rulesr
    // QWidget *_hRuler;
    // QWidget *_vRuler;

};


#endif // EDITOR_CONTAINER_H

