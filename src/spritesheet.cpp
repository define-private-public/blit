// File:         spritesheet.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for spritesheet.h



#include "spritesheet.h"
#include "ui_spritesheet_dialog.h"
#include "util.h"
#include "fileops.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include "animation/animation.h"
#include "widgets/colorframe.h"
#include <QtCore/qmath.h>
#include <QPainter>
#include <QRadioButton>
#include <QLabel>
#include <QSpinBox>
#include <QScrollArea>
#include <QFrame>
#include <QPalette>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPixmap>
#include <QFileDialog>
#include <QColor>
#include <QImage>
#include <QTimer>
#include <QDebug>



namespace Spritesheet {
    /*!
        Converts the Frames in an Animation to a spritesheet (in a QImage).  This function assumes
        that anim is not Null, order is either set to SPRITESHEET_ROW_MAJOR or SPRITESHEET_COLUMN_MAJOR,
        numPerOrder is a positive integer, and scale is a positive integer as well.  If any of these
        variables do not pass the sniff test, animationToSpritesheet() will return a Null QImage.
    */
    QImage animationToSpritesheet(Animation *anim, int order, int numPerOrder, QColor background, int scale) {
        if (anim->isEmpty())
            return QImage();
        if ((order != SPRITESHEET_ROW_MAJOR) && (order != SPRITESHEET_COLUMN_MAJOR))
            return QImage();
        if (numPerOrder < 1)
            return QImage();
        if (scale < 1)
            return QImage();

        // Passed smell test, More vars
        QSize frameSize(anim->frameSize() * scale);
        QList<QPointer<TimedFrame>> frames = anim->xsheet()->frames();
        int numFrames = frames.size();
        numPerOrder = (numPerOrder < numFrames) ? numPerOrder : numFrames;    // Trim to a good size

        // set rows and columns
        int rows = 0, columns = 0;
        if (order == SPRITESHEET_ROW_MAJOR) {
            // Row Major
            columns = numPerOrder;
            rows = qCeil((float)numFrames / columns);
        } else {
            // Column Major
            rows = numPerOrder;
            columns = qCeil((float)numFrames / rows);
        }

        // Make the spritesheet
        QImage spritesheet(frameSize.width() * columns, frameSize.height() * rows, QImage::Format_ARGB32_Premultiplied);
        spritesheet.fill(background);    // Set the BG colour

        // Start painting
        QPainter qp(&spritesheet);
        QRect rect;
        for (int i = 0; i < numFrames; i++) {
            // Choose some coordinates
            if (order == SPRITESHEET_ROW_MAJOR) {
                // Row Major
                rect.setX(frameSize.width() * (i % columns));
                rect.setY(frameSize.height() * (i / columns));
            } else {
                // column major
                rect.setX(frameSize.width() * (i / rows));
                rect.setY(frameSize.height() * (i % rows));
            }

            // Draw the inage
            rect.setSize(frameSize);
            qp.drawImage(rect, frames[i]->frame()->render());
        }

        // All done!
        return spritesheet;
    }
};




/*== Spritesheet Dialog class ==*/
// Statics (persistance variables), and their default values
SpritesheetDialog *SpritesheetDialog::_instance = NULL;
QString SpritesheetDialog::_lastFilename = "";
QString SpritesheetDialog::_selectedFilter = FileOps::validFilters()[0];
bool SpritesheetDialog::_showPreview = true;
bool SpritesheetDialog::_showGrid = false;
int SpritesheetDialog::_order = SPRITESHEET_ROW_MAJOR;
int SpritesheetDialog::_numPerOrder = SPRITESHEET_DEFAULT_NUM_PER_ORDER;
QColor SpritesheetDialog::_bgColor = SPRITESHEET_TRANSPARENT;
int SpritesheetDialog::_scale = 1;
int SpritesheetDialog::_rows = 1;
int SpritesheetDialog::_columns = 1;


/*!
    Creates a new SpritesheetDialoag.  \a parent should be the parent window this dialog belongs to.

    NOTE: this class is a singleton
*/
SpritesheetDialog::SpritesheetDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SpritesheetDialog)
{
    // Make sure there is just a singletone
    if (_instance != NULL) {
        qDebug() << "Error, the export spritesheet dialog can only have one instance at at time (it's a singleton).";
        return;
    }

    // Else, it's all good; widgets and layout
    _instance = this;
    _ui->setupUi(this);
    _ui->previewWidget->setBackgroundRole(QPalette::Light);

    // Previous settings
    if (_order == SPRITESHEET_ROW_MAJOR)
        _ui->rowRadio->setChecked(true);
    else
        _ui->columnRadio->setChecked(true);
    _ui->fpoSpinner->setValue(_numPerOrder);
    _ui->scaleSpinner->setValue(_scale);
    _ui->showPreviewCheckBox->setCheckState(_showPreview ? Qt::Checked : Qt::Unchecked);
    _ui->showGridCheckBox->setCheckState(_showGrid ? Qt::Checked : Qt::Unchecked);
    _ui->bgColorFrame->setColor(_bgColor);

    // Becuase of Qt5, we need to do some hacky-variable signal/slot loading
    void (QSpinBox::*valueChangedSignal)(int) = &QSpinBox::valueChanged;

    // Connect all of the slots and signals
    connect(_ui->rowRadio, &QRadioButton::clicked, this, &SpritesheetDialog::setOrder);
    connect(_ui->columnRadio, &QRadioButton::clicked, this, &SpritesheetDialog::setOrder);
    connect(_ui->fpoSpinner, valueChangedSignal, this, &SpritesheetDialog::setDimensions);
    connect(_ui->scaleSpinner, valueChangedSignal, this, &SpritesheetDialog::setScale); 
    connect(_ui->showPreviewCheckBox, &QCheckBox::stateChanged, this, &SpritesheetDialog::togglePreview);
    connect(_ui->showGridCheckBox, &QCheckBox::stateChanged, this, &SpritesheetDialog::toggleGrid);
    connect(_ui->bgColorFrame, &ColorFrame::colorChanged, this, &SpritesheetDialog::_generatePreview);
    connect(_ui->saveButton, &QPushButton::clicked, this, &SpritesheetDialog::saveSpritesheet);
    connect(_ui->closeButton, &QPushButton::clicked, this, &SpritesheetDialog::close);
    connect(_ui->bgColorFrame, &ColorFrame::colorChanged, this, &SpritesheetDialog::setBGColor);

    // Last few things
    setWindowTitle("Export Animation as Spritesheet");
    setFixedSize(minimumSize());
}


/*!
    Cleans up the SpritesheetDialog.  Also deletes the singleton.
*/
SpritesheetDialog::~SpritesheetDialog() {
    // cleanup the singleton
    _instance = NULL;
    delete _ui;
}


/*!
    Sets the Animation object that this dialog is trying to spritesheet.
*/
void SpritesheetDialog::setAnim(Animation *anim) {
    _anim = anim;
    if (_anim) {
        _ui->frameSizeLabel->setText(util::sizeToStr(_anim->frameSize() * _scale));
        _ui->fpoSpinner->setMaximum(_anim->xsheet()->numFrames());
    }
    _generatePreview();
}


/*!
    Calling this funciton will set the "previewImage," widget to have a QPixmap of the spriteshet set
    to it.  if _showPreview is false, nothing will happen.
*/
void SpritesheetDialog::_generatePreview() {
    if (!_showPreview)
        return;
    
    // make a new spritesheet and set it as the image
    QImage sheet(mkSpritesheet());

    if (_showGrid) {
        QSize frameSize;
        int numFrames = 0;
        int rows, columns;

        if (_anim) {
            frameSize = _anim->frameSize() * _scale;
            numFrames = _anim->xsheet()->numFrames();
        }

        QPainter painter(&sheet);
        painter.setPen(SPRITESHEET_GRID_COLOR);


        if (_order == SPRITESHEET_ROW_MAJOR) {
            // Row Major
            columns = _numPerOrder;
            rows = qCeil((float)numFrames / columns);
        } else {
            // Column Major
            rows = _numPerOrder;
            columns = qCeil((float)numFrames / rows);
        }

        QRect rect;
        for (int i = 0; i < numFrames; i++) {
            // Choose some coordinates
            if (_order == SPRITESHEET_ROW_MAJOR) {
                // Row Major
                rect.setX(frameSize.width() * (i % columns));
                rect.setY(frameSize.height() * (i / columns));
            } else {
                // column major
                rect.setX(frameSize.width() * (i / rows));
                rect.setY(frameSize.height() * (i % rows));
            }

            // Draw the inage
            rect.setWidth(frameSize.width() - 1);
            rect.setHeight(frameSize.height() - 1);
            painter.drawRect(rect);
        }
    }

    _ui->previewImage->setPixmap(QPixmap::fromImage(sheet));
    _ui->previewImage->adjustSize();

}


/*!
    small convinience function to generate a QImage spritesheet of the Animation.
*/
QImage SpritesheetDialog::mkSpritesheet() {
    if (_anim)
        return Spritesheet::animationToSpritesheet(_anim, _order, _numPerOrder, _bgColor, _scale);
    else
        return QImage();
}


/*!
    Brings up a dialog to save a generated spritesheet.
*/
bool SpritesheetDialog::saveSpritesheet() {
    // Brings up a File Dialog to save the spritesheet somewhere
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Spritesheet"), _lastFilename, FileOps::validFilters().join(";;"), &_selectedFilter);

    // Check that the dialog was a success
    if (!filename.isEmpty()) {
        // Make sure we've got a good extension
        if (!filename.endsWith(FileOps::extensionMap()[_selectedFilter]))
            filename.append(FileOps::extensionMap()[_selectedFilter]);

        // Make the save
        QImage sheet = mkSpritesheet();
        bool success = FileOps::saveSpritesheet(sheet, filename);
        
        // Set a variable
        _lastFilename = filename;

        return success;        
    }

    return false;    // No save
}


/*!
    Toggles the preview state.
*/
void SpritesheetDialog::togglePreview(int state) {
    _showPreview = (state == Qt::Checked);
    _generatePreview();
}


/*!
    Called when the "Show Grid," checkbox is toggled on.
*/
void SpritesheetDialog::toggleGrid(int state) {
    _showGrid = (state == Qt::Checked);
    _generatePreview();
}


/*!
    Should be called by ehtier radios buttons with their "clicked," signal.  This will change the order
    of the generated spritesheet.
*/
void SpritesheetDialog::setOrder() {
    int newWorldOrder = _ui->rowRadio->isChecked() ? SPRITESHEET_ROW_MAJOR : SPRITESHEET_COLUMN_MAJOR;        // Fun variable names
    if (newWorldOrder != _order) {
        _order = newWorldOrder;
//        _numLabel->setText((_order == SPRITESHEET_ROW_MAJOR) ? "Frames per Row:" : "Frames per Column:");
        _generatePreview();
    }
}


/*!
    Will set the values in the Row and Column spiner boxes, as well as set the _numPerOrder variable.
    Should be called by the "valueChanged," singal for the row/column spin boxes.
*/
void SpritesheetDialog::setDimensions() {
    _numPerOrder = _ui->fpoSpinner->value();
    _generatePreview();
}

/*!
    Changes the scale of the genreates spritesheet, will also udate some of the info displayed.
*/
void SpritesheetDialog::setScale(int scale) {
    if (_scale != scale) {
        _scale = scale;
        _generatePreview();

        // Set the frame and sheet size labels if there is an _animaiton
        if (_anim)
            _ui->frameSizeLabel->setText(util::sizeToStr(_anim->frameSize() * _scale));
    }
}


/*!
    Changes the background color that the spritesheet will have.
*/
void SpritesheetDialog::setBGColor(QColor clr) {
    if (_bgColor != clr) {
        _bgColor = clr;
        _generatePreview();
    }
}

