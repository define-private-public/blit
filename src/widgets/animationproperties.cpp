// File:         animationproperties.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for animationproperties.h


#include "widgets/animationproperties.h"
#include "ui_animation_properties_dialog.h"
#include "animation/xsheet.h"
#include "animation/animation.h"
#include <QSize>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QKeyEvent>
#include <QFormLayout>
#include <QDebug>


/*== Static State variables ==*/
AnimationPropertiesDialog *AnimationPropertiesDialog::_instance = NULL;        // Singleton


AnimationPropertiesDialog::AnimationPropertiesDialog(Animation *anim, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::AnimationPropertiesDialog),
    _anim(anim)
{
    // Make sure there is just a singletone
    if (_instance != NULL) {
        qDebug() << "Error, the animation properties dialog can only have one instance at at time (it's a singleton).";
        return;
    }
    
    // Other checks
    Q_ASSERT(_anim != NULL);
    _instance = this;

    // Else, it's all good; widgets and layout
    _ui->setupUi(this);
    _ui->nameEdit->setText(_anim->name());
    _ui->frameWidthSpinner->setRange(1, ANIMATION_FRAME_SIZE_MAX);
    _ui->frameWidthSpinner->setValue(_anim->frameWidth());
    _ui->frameHeightSpinner->setRange(1, ANIMATION_FRAME_SIZE_MAX);
    _ui->frameHeightSpinner->setValue(_anim->frameHeight());
    _ui->createdTimeLabel->setText(_anim->createdString());
    _ui->updatedTimeLabel->setText(_anim->updatedString());

    // Add some extra signals & slots
    connect(_ui->okButton, &QPushButton::clicked, this, &AnimationPropertiesDialog::accept);
    connect(_ui->cancelButton, &QPushButton::clicked, this, &AnimationPropertiesDialog::reject);

    // Set contraints
    setWindowTitle("Animation Properties");
    setFixedSize(sizeHint());
}


AnimationPropertiesDialog::~AnimationPropertiesDialog() {
    // Clear the singleton
    _instance = NULL;
    delete _ui;
}


QString AnimationPropertiesDialog::name(){
    // Returns the name that was set in the Name Edit
    return _ui->nameEdit->text();
}


QSize AnimationPropertiesDialog::size(){
    // Returns the size that was set in the Size spinners
    return QSize(_ui->frameWidthSpinner->value(), _ui->frameHeightSpinner->value());
}


void AnimationPropertiesDialog::keyPressEvent(QKeyEvent *event) {
    // Does all the same actions as a normal key press event, but will close on the Enter key pressed
    // as well
    if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
        event->accept();
        accept();
    }

    QDialog::keyPressEvent(event);
}
