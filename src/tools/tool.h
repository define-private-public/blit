// File:         tool.h
// Author:       Ben Summerton (define-private-public)
// Description:  Tool is an abstract interface for defining Drawing tools.  For example, the 
//               PenTool, Brush, and Eraser Tools all sublcass/implement this interfacee.
//
//               If you want to implement a subclass, be sure to read all the comments on what to do for each method


#ifndef TOOL_H
#define TOOL_H


#define TOOL_OPTIONS_PANEL_MAX_WIDTH 280


#include <QObject>
class QString;
class QIcon;
class QWidget;
class QGraphicsSceneMouseEvent;


class Tool : public QObject {
    Q_OBJECT;

public:
    Tool(QObject *parent=NULL);
    virtual ~Tool();

    // Tool properties that must be implmeented
    virtual QString name() = 0;            // Must be unique
    virtual QString desc() = 0;            // Tooltip info
    virtual QIcon icon() = 0;            // Recommended to be constructed with XPM data

    // Optional properties
    virtual QWidget *options();            // Panel of options to modify the tool paparmeters, can return a NULL pointer.
                                        // Should always return a newly allocated widget

public slots:
    // For drawing
    virtual void onMouseDoubleClicked(QGraphicsSceneMouseEvent *event);
    virtual void onMouseMoved(QGraphicsSceneMouseEvent *event);
    virtual void onMousePressed(QGraphicsSceneMouseEvent *event);
    virtual void onMouseReleased(QGraphicsSceneMouseEvent *event);

};


// Make in a plugin interface
Q_DECLARE_INTERFACE(Tool, "net.16bpp.blit.Tool/1.0")


#endif // TOOL_H

