// File:         stagedcelwidget.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the StagedCelWidget class.


#ifndef STAGED_CEL_WIDGET_H
#define STAGED_CEL_WIDGET_H

#define STAGED_CEL_WIDGET_THUMB_SIZE 64
#define STAGED_CEL_WIDGET_WIDTH 300
#define STAGED_CEL_WIDGET_HEIGHT 80
#define STAGED_CEL_WIDGET_SEPARATOR_HEIGHT 2


#include <QWidget>
#include <QPointer>
class CelRef;
class QToolButton;


namespace Ui {
    class StagedCelWidget;
}


class StagedCelWidget : public QWidget {
    Q_OBJECT;

public:
    // Constructors
    StagedCelWidget(CelRef *ref, QWidget *parent=NULL);
    ~StagedCelWidget();

    // Accessors
    CelRef *ref();

    // Vars
    int index = -1;            // for the CelsWindow


public slots:
    void select();
    void deselect();
    

private slots:
    void _onGeometryChanged();
    void _onRaiseButtonClicked();
    void _onLowerButtonClicked();


signals:
    void mousePressed();
    void raiseButtonClicked();
    void lowerButtonClicked();


protected:
    void mousePressEvent(QMouseEvent *event);


private:
    // GUI
    Ui::StagedCelWidget *_ui;

    // Member functions
    void _setBGColor(QColor clr);
    QPixmap _mkThumbnail();

    // Member vars
    QPointer<CelRef> _ref;


};


#endif // STAGED_CEL_WIDGET_H

