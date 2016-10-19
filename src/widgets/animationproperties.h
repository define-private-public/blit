// File:         animationproperties.h
// Author:       Ben Summerton (define-private-public)
// Description:  The Animation Properties Dialog is a modal dialog that is used to edit
//               properties about a supplied Animation.  It is a singleton.  Make sure to
//               Delete the widget when you're done with it.  That way it cleans up the
//               singleton.


#ifndef ANIMATION_PROPERTIES_H
#define ANIMATION_PROPERTIES_H


#include <QDialog>
class Animation;
class QLabel;
class QPushButton;
class QSpinBox;
class QLineEdit;


namespace Ui {
    class AnimationPropertiesDialog;
}


class AnimationPropertiesDialog : public QDialog {
    Q_OBJECT;

public:
    AnimationPropertiesDialog(Animation *anim, QWidget *parent = NULL);
    ~AnimationPropertiesDialog();

    // "Return," varibles
    QString name();
    QSize size();


public slots:


signals:


private:
    // Member variables (static)
    static AnimationPropertiesDialog *_instance;        // Singleton

    // Other memeber vars
    Animation *_anim = NULL;

    // GUI Widgets
    Ui::AnimationPropertiesDialog *_ui;


protected:
    void keyPressEvent(QKeyEvent *event);

};


#endif // ANIMATION_PROPERTIES_H

