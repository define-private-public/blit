// File:         spritesheet.h
// Author:       Ben Summerton (define-private-public)
// Description:  This module cotains classes and utility functions for turning Animation sequences int
//               usable spritesheets and vice-versa.  The Spritesheet dialog class should be used as a 
//               singlton.  It will retain values set to it by its last useage.
//
//               It should also only be used as a Modal

// TODO:
//   - Add overlay to grid preview
//   - spritesheet importing


#ifndef SPRITESHEET_H
#define SPRITESHEET_H


// Definitnios
#define SPRITESHEET_ROW_MAJOR 1
#define SPRITESHEET_COLUMN_MAJOR 2
#define SPRITESHEET_TRANSPARENT QColor(0x00, 0x00, 0x00, 0x00)
#define SPRITESHEET_DEFAULT_NUM_PER_ORDER 10
#define SPRITESHEET_GRID_COLOR QColor(0x00, 0x00, 0x00)


#include <QDialog>
class Animation;
class ColorFrame;
class QColor;
class QImage;
class QLabel;
class QRadioButton;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QScrollArea;


// Utility functions
namespace Spritesheet {
    QImage animationToSpritesheet(Animation *anim, int order=SPRITESHEET_ROW_MAJOR, int numPerOrder=SPRITESHEET_DEFAULT_NUM_PER_ORDER, QColor background=SPRITESHEET_TRANSPARENT, int scale=1);
};




namespace Ui {
    class SpritesheetDialog;
}


class SpritesheetDialog : public QDialog {
    Q_OBJECT;

public:
    SpritesheetDialog(QWidget *parent = NULL);
    ~SpritesheetDialog();

    void setAnim(Animation *anim);
    QImage mkSpritesheet();


public slots:
    bool saveSpritesheet();
    void togglePreview(int state);
    void toggleGrid(int state);
    void setOrder();
//    void setDimensions(int val);        // val is unused
    void setDimensions();
    void setScale(int scale);
    void setBGColor(QColor clr);


private:
    // utilty functions
    void _generatePreview();

    // Member variables (static)
    static SpritesheetDialog *_instance;        // Singleton

    static QString _lastFilename;
    static QString _selectedFilter;
    static bool _showPreview;
    static bool _showGrid;
    static int _order;
    static int _numPerOrder;
    static QColor _bgColor;
    static int _scale;
    static int _rows;
    static int _columns;
    
    // non static member vars
    // Animation
    Animation *_anim;

    // GUI Widgets
    Ui::SpritesheetDialog *_ui;


};


#endif // SPRITESHEET_H

