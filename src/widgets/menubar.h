// File:         menubar.h
// Author:       Ben Summerton (define-private-public)
// Description:  Similar to the StatusBar, MenuBar is supposed to be a singleton class for the Main Window
//               widget in the applicaiton (which is BlitApp).  It provides access to menu functionality.
//
//               Most of the meat for this class can be found in the source file (menubar.cpp).  This
//               module just mainly serves as some sort of organizational module


#ifndef MENU_BAR_H
#define MENU_BAR_H


#include <QMenuBar>
class BlitApp;
class Animation;
class QAction;
class QMenu;
class QDockWidget;
#include <QList>


class MenuBar : public QMenuBar {
    Q_OBJECT;

public:
    MenuBar(BlitApp *parent, QList<QDockWidget *> &docks);


public slots:
    void onAnimLoaded(Animation *anim);


private slots:
    void _onAnimPropsClicked();


private:
    // GUI stuff
    QMenu *_fileMenu;
    QMenu *_importMenu;
    QMenu *_exportMenu;
    QMenu *_animMenu;
    QMenu *_canvasMenu;
    QMenu *_viewMenu;
    QMenu *_helpMenu;

    QAction *_newAnimAction;
    QAction *_openAnimAction;
    QAction *_saveAsAction;
    QAction *_quitAppAction;
    QAction *_animPropsAction;
    QAction *_importStillImageAction;
    QAction *_exportSpritesheetAction;
    QAction *_exportStillImageAction;
    QAction *_showGridAction;
    QAction *_setBackdropAction;
    QAction *_aboutBlitAction;


};

#endif // MENU_BAR_H

