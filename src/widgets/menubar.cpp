// File:         menubar.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for MenuBar class


#include "widgets/menubar.h"
#include "blitapp.h"
#include "animation/animation.h"
#include "widgets/drawing/canvas.h"
#include <QAction>
#include <QMenu>
#include <QDockWidget>


MenuBar::MenuBar(BlitApp *parent, QList<QDockWidget *> &docks) :
    QMenuBar(parent)
{
    // Make all of the actions
    // File Menu
    _newAnimAction = new QAction(tr("&New"), this);
    _openAnimAction = new QAction(tr("&Open"), this);
    _saveAsAction = new QAction(tr("Save &As..."), this);
    _quitAppAction = new QAction(tr("&Quit"), this);

    // Animation Menu
    _animPropsAction = new QAction(tr("&Properties"), this);

    // Canvas Menu
    _showGridAction = new QAction(tr("&Grid"), this);
    _showGridAction->setCheckable(true);
    _showGridAction->setChecked(true);

    _setBackdropAction = new QAction(tr("Set &Backdrop Color"), this);


    // Import
    _importStillImageAction = new QAction(tr("Still Image as &Cel"), this);

    // Export
    _exportSpritesheetAction = new QAction(tr("As &Spritesheet"), this);
    _exportStillImageAction = new QAction(tr("Frame as Still &Image"), this);

    // View Menu
    _showGridAction = new QAction(tr("&Grid"), this);
    _showGridAction->setCheckable(true);
    _showGridAction->setChecked(true);

    // Help Menu
    _aboutBlitAction = new QAction(tr("&About Blit"), this);


    // Make the menus
    // file Menu
    _fileMenu = new QMenu(tr("&File"));
    _fileMenu->addAction(_newAnimAction);
    _fileMenu->addAction(_openAnimAction);
    _fileMenu->addAction(_saveAsAction);
    _fileMenu->addSeparator();
    _importMenu = _fileMenu->addMenu(tr("&Import..."));
        _importMenu->addAction(_importStillImageAction);
    _exportMenu = _fileMenu->addMenu(tr("&Export..."));
        _exportMenu->addAction(_exportSpritesheetAction);
        _exportMenu->addAction(_exportStillImageAction);
    _fileMenu->addSeparator();
    _fileMenu->addAction(_quitAppAction);

    // Animation Menu
    _animMenu = new QMenu(tr("&Animation"));
    _animMenu->addAction(_animPropsAction);
    _animMenu->hide();        // Hidden by default

    // Canvas Menu
    _canvasMenu = new QMenu(tr("&Canvas"));
    _canvasMenu->addAction(_showGridAction);
    _canvasMenu->addAction(_setBackdropAction);
    _canvasMenu->hide();    // Hidden by default

    /*== View Menu ==*/
    _viewMenu = new QMenu(tr("&View"));

    // windows/docks
    QList<QDockWidget *>::const_iterator iter;
    for (iter = docks.begin(); iter != docks.end(); iter++)
        _viewMenu->addAction((*iter)->toggleViewAction());

    // Help Menu
    _helpMenu = new QMenu(tr("&Help"));
    _helpMenu->addAction(_aboutBlitAction);



    // Add the menus
    addMenu(_fileMenu);
//    addMenu(_animMenu);        // Don't show till anim loaded
//    addMenu(_canvasMenu);    // Don't show till anim loaded
    addMenu(_viewMenu);
    addMenu(_helpMenu);

    // Some default stuff, isn't enabled until anim is loaded
    _importMenu->setEnabled(false);
    _exportMenu->setEnabled(false);
    _saveAsAction->setEnabled(false);




    // Connect their signals/slots
    connect(_newAnimAction, &QAction::triggered, parent, &BlitApp::onNewAnim);
    connect(_openAnimAction, &QAction::triggered, parent, &BlitApp::onOpenAnim);
    connect(_saveAsAction, &QAction::triggered, parent, &BlitApp::onSaveAs);
    connect(_quitAppAction, &QAction::triggered, parent, &BlitApp::close);
    connect(_animPropsAction, &QAction::triggered, this, &MenuBar::_onAnimPropsClicked);
    connect(_showGridAction, &QAction::toggled, parent->canvas(), &Canvas::showGrid);
    connect(_setBackdropAction, &QAction::triggered, parent, &BlitApp::onSetBackdrop);
    connect(_importStillImageAction, &QAction::triggered, parent, &BlitApp::showImportStillImage);
    connect(_exportSpritesheetAction, &QAction::triggered, parent, &BlitApp::showExportSpritesheet);
    connect(_exportStillImageAction, &QAction::triggered, parent, &BlitApp::showExportStillImage);
    connect(_aboutBlitAction, &QAction::triggered, parent, &BlitApp::showAboutBlit);

    // From the parent
    connect(parent, &BlitApp::animLoaded, this, &MenuBar::onAnimLoaded);
}


/*!
    Called by BlitApp:animLoaded.  If an Animation was loaded, this function will clear out the
    current menubar, but then reconstruct it with the Animation Menu included.  Will also
    renabled the menu and actions
*/
void MenuBar::onAnimLoaded(Animation *anim) {
    // Add all menus
    addMenu(_fileMenu);
    addMenu(_animMenu);
    addMenu(_canvasMenu);
    addMenu(_viewMenu);
    addMenu(_helpMenu);

    // Renable some options
    _importMenu->setEnabled(true);
    _exportMenu->setEnabled(true);
    _saveAsAction->setEnabled(true);
}


/*!
    Need this extra slot to show the Animation properties dialog.
*/
void MenuBar::_onAnimPropsClicked() {
    BlitApp::app()->showAnimationProperties();
}

