// File:         colorpalette.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the Color Palette widget.


#include "widgets/colorpalette.h"
#include "widgets/colorframe.h"
#include <QColor>
#include <QPalette>
#include <QFrame>
#include <QToolButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QMouseEvent>


/*== OVERLOADED qHash function ==*/
inline uint qHash(const QColor key) {
    // Should be unique.
    return (key.alpha() << 24) | (key.red() << 16) | (key.green() << 8) | key.blue();
}
/*== OVERLOADED WARNING ==*/


ColorPalette::ColorPalette(QWidget *parent) :
    QWidget(parent)
{
    // Init
    _createWidgets();
    _setupLayout();

    // Layout, need to do this once before adding colors
    _swatchContainerLayout = new QGridLayout(_swatchContainer);
}


void ColorPalette::_createWidgets() {
    // Internal function to setup the widgets (not including the swatches)3
    // Sizing
    int maxWidth = COLOR_PALETTE_SWATCH_SIZE * COLOR_PALETTE_SWATCHES_PER_ROW;    // Swatches
    maxWidth += 2 * 2;                                                            // Margins
    maxWidth += 2 * (COLOR_PALETTE_SWATCHES_PER_ROW - 1);                        // Spacing

    // Swatch container and scroller
    _swatchContainer = new QWidget;
    _swatchContainer->setFixedWidth(maxWidth);
    _scroller = new QScrollArea(this);
    _scroller->setWidget(_swatchContainer);
    _scroller->setWidgetResizable(true);
    _scroller->setFixedHeight(120);

    // Make the background of the scroller white
    QPalette pal = QPalette(_scroller->palette());
    pal.setColor(QPalette::Window, Qt::white);
    _scroller->setPalette(pal);

    // Add swatch button
    _addSwatchButton = new QToolButton(this);
    _addSwatchButton->setIcon(QIcon(":/color_add_icon"));
    _addSwatchButton->setIconSize(QSize(18, 18));
//    _addSwatchButton->setFixedSize(24, 24);                // TODO bad, very bad, get a better size (or an icon)
}


void ColorPalette::_setupLayout() {
    // Interval function to setup the layout
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(_scroller, 0, 0, 2, 1);
    layout->addWidget(_addSwatchButton, 0, 1);

    // Looks
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
}


QList<QColor> ColorPalette::colors() {
    // Returns all of the swatches set in the palette
    return _orderedColors;
}


QToolButton *ColorPalette::addSwatchButton() {
    // Returns  pointer to the Add Swatch Button
    return _addSwatchButton;
}


void ColorPalette::clear() {
    // Will clear all of the colors in the color palette
    _curSwatch = NULL;
    _colors.clear();
    _orderedColors.clear();
    QListIterator<ClickableColorFrame *> iter(_swatches);
    while (iter.hasNext()) {
        ClickableColorFrame *ccf = iter.next();
        ccf->setParent(this);        // Change parent to this widget
        delete ccf;
    }
    _swatches.clear();
    
    // Adjust layout
    _adjustSwatchWidgetsLayout();
}


void ColorPalette::addList(QList<QColor> colors) {
    // Will add an entire list of colors
    QListIterator<QColor> iter(colors);
    while (iter.hasNext())
        addColor(iter.next());
}


void ColorPalette::_adjustSwatchWidgetsLayout() {
    // Internal function to readjust the layout of all of the swatches.  Should only be
    // called when a swatch is added or removed.
    
    // First delete the old layout and set a new one
    delete _swatchContainerLayout;
    _swatchContainerLayout = new QGridLayout(_swatchContainer);
    _swatchContainerLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _swatchContainerLayout->setContentsMargins(2, 2, 2, 2);
    _swatchContainerLayout->setSpacing(2);

    // Add the swatches to the layout
    for (int i = 0; i < _swatches.size(); i++)
        _swatchContainerLayout->addWidget(_swatches[i], i / COLOR_PALETTE_SWATCHES_PER_ROW, i % COLOR_PALETTE_SWATCHES_PER_ROW);
}


void ColorPalette::_onSwatchPressed(QMouseEvent *event) {
    // When a swatch is pressed, this slot will be triggered.
    // On:
    //   Left Mouse -- Select Swatch
    //   Right Mouse -- Delete swatch

    switch (event->button()) {
        case Qt::LeftButton:
            // Select swatch
            _curSwatch = qobject_cast<ClickableColorFrame *>(sender());        // Guarenteed to be in the widget
            emit swatchSelected(_curSwatch->color());
            break;

        case Qt::RightButton:
            // Delete a swatch
            ClickableColorFrame *ccf = qobject_cast<ClickableColorFrame *>(sender());        // Guarenteed to be in the widget
            removeColor(ccf->color());
            break;
    }
}


void ColorPalette::addColor(QColor clr) {
    // Will add a color to the palette if it isn't already in there.  Creates an extra
    // swatch and adjusts the layout acordingly.  Supplied color must be valid
//    qDebug() << "[hash]" << "0x" + QString::number(qHash(clr), 16).toUpper();

    if (!_colors.contains(clr) && clr.isValid()) {
        // It doesn't have it, add it
        _colors.insert(clr);
        _orderedColors.append(clr);

        // Make the Color Frame
        QSize size(COLOR_PALETTE_SWATCH_SIZE, COLOR_PALETTE_SWATCH_SIZE);
        QSize squareSize = size / 2;
        ClickableColorFrame *ccf = new ClickableColorFrame(_swatchContainer);
        ccf->setColor(clr);
        ccf->setFixedSize(size);
        ccf->setSquareSize(squareSize);
        _swatches.append(ccf);

        // connect the signals/slots
        connect(ccf, &ClickableColorFrame::pressed, this, &ColorPalette::_onSwatchPressed);

        // Fixup the layout
        _adjustSwatchWidgetsLayout();

        // If no current swatch is set, set this one
        if (!_curSwatch)
            _curSwatch = ccf;
    }
}


void ColorPalette::removeColor(QColor clr) {
    // Removes a color from the plaette if it is there.  If it isn't nothing will happen.
    // Will adjust the layout accordingly.  Supplied color must be valid
    if (_colors.contains(clr) && clr.isValid()) {
        // Remove the color
        _colors.remove(clr);

        // TODO back algorithm, replace it
        // Find the swatch with the color and remove it
        for (int i = 0; i < _swatches.size(); i++) {
            ClickableColorFrame *ccf = _swatches[i];

            if (ccf->color() == clr) {
                // found it, remove it, delete it
                
                // Change the current swatch to nothing
                if (_curSwatch == ccf)
                    _curSwatch = NULL;

                // Do the removal
                _swatches.removeAt(i);
                _orderedColors.removeAt(i);
                ccf->deleteLater();                // Call this so we don't segfault
            }
        }

        // It has it, remove it
        _adjustSwatchWidgetsLayout();
    }
}

