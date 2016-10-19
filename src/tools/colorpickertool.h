// File:         colorpickertool.h
// Author:       Ben Summerton (define-private-public)
// Description:  ColorPickerTool is an implementation of the Tool interface.  It comes standard in Blit.
//               It will let the user colorpicker around Cels in the Frame.
//
//               Right now it only allows the user to colorpicker the currently selected Cel


#ifndef COLOR_PICKER_TOOL_H
#define COLOR_PICKER_TOOL_H


#include "tools/tool.h"
#include <QPointF>
#include <QImage>


class ColorPickerTool : public Tool {
    Q_OBJECT;
    Q_INTERFACES(Tool);


public:
    ColorPickerTool(QObject *parent=NULL);
    ~ColorPickerTool();

    // Tool Info
    QString name();
    QString desc();
    QIcon icon();


public slots:
    // For drawing
    void onMouseMoved(QGraphicsSceneMouseEvent *event);
    void onMousePressed(QGraphicsSceneMouseEvent *event);
    void onMouseReleased(QGraphicsSceneMouseEvent *event);


private:
    // Member functions
    QColor _getColorAt(int x, int y);

    // Member vars
    bool _picking = false;
    QImage _screenshot;
};


#endif // COLOR_PICKER_TOOL_H

