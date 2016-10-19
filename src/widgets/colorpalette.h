// File:         colorpalette.h
// Author:       Ben Summerton (define-private-public)
// Description:  Color Palette is (currently) a simple widget that is a collection of 
//               of swatches that can be used to select a color and to save other ones.
//
//               Under normal operation, it will contain all of the colors that are used
//               in the animation.
//
//               Please note that the Add Swatch Button doesn't do anything unless it is
//               hooked up outside the widget.  (E.g. the ToolsWindow will actually perform
//               the addition).
//
// NOTE: in order to get QSet<QColor> working, we had to overload qHash

// TODO List:
//   - Maybe should be a ColorChooser widget
//   - Check speed for QSet w/ color profiler when color profiler is done
//   - Check if the hash function is good or not


#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H


#define COLOR_PALETTE_SWATCH_SIZE 20
#define COLOR_PALETTE_SWATCHES_PER_ROW 10


#include <QWidget>
#include <QSet>
#include <QList>
#include <QHash>
#include <QPointer>
class ClickableColorFrame;
class QColor;
class QToolButton;
class QScrollArea;
class QGridLayout;


class ColorPalette: public QWidget {
    Q_OBJECT;

public:
    ColorPalette(QWidget *parent=NULL);

    // Accessors
    QList<QColor> colors();
    QToolButton *addSwatchButton();

    // Mutators
    void clear();
    void addList(QList<QColor> colors);


public slots:
    void addColor(QColor clr);
    void removeColor(QColor clr);


private slots:
    void _onSwatchPressed(QMouseEvent *event);


signals:
    void swatchSelected(QColor clr);


private:
    // Internal functions
    void _createWidgets();
    void _setupLayout();
    void _adjustSwatchWidgetsLayout();

    // Member vars
    QPointer<ClickableColorFrame> _curSwatch;    // Also GUI
    QList<ClickableColorFrame *> _swatches;        // Also GUI
    QSet<QColor> _colors;
    QList<QColor> _orderedColors;

    // GUI
    QWidget *_swatchContainer;
    QGridLayout *_swatchContainerLayout;
    QToolButton *_addSwatchButton;
    QScrollArea *_scroller;

};


#endif // COLOR_PALETTE_H

