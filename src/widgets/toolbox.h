// File:         toolbox.h
// Author:       Ben Summerton (define-private-public)
// Description:  The Toolbox is a widget that contains a set list of Tool objects.  It is used in
//               the topmost portion of the ToolsWindow.  It's mainly organizational.
//
//               There are two maps in the class that are used to map Tool objects to QToolButtons
//               and vice-versa.  Pointers are used as keys and as values.  There are also two
//               lists that have the Tools and QToolButtons in order (these might be removed in the
//               future though).


#ifndef TOOLBOX_H
#define TOOLBOX_H

#define TOOLBOX_MAX_TOOLS_PER_ROW 10


#include <QWidget>
#include <QHash>
class Tool;
class QToolButton;


class Toolbox: public QWidget {
    Q_OBJECT;

public:
    // constructors
    Toolbox(QWidget *parent=NULL);

    // Accessors
    Tool *curTool();


public slots:
    void onToolButtonClicked(bool checked);


signals:
    void curToolChanged(Tool *tool);


private:
    // Internal organizational functions
    void _createWidgets();
    void _setupLayout();

    // Member vars
    Tool *_curTool;
    QHash<Tool *, QToolButton *> _toolMap;            // Maps a Tool to a QToolButton
    QHash<QToolButton *, Tool *> _toolButtonMap;    // Maps a QToolButton to a Tool

    QList<Tool *> _tools;                            // List of all the Tools in order
    QList<QToolButton *> _toolButtons;                // List of all the QToolButtons in order

};


#endif // TOOLBOX_H

