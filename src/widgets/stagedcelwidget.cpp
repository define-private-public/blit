// File:         stagedcelwidget.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the StagedCelWidget class.


/*!
    \inmodule Widgets
    \class StagedCelWidget
    \brief The StagedCelWidget is a widget that represents a CelRef in the CelsWindow.

    It gives a small visualization of the CelRef, allows the uesr to adjust layering
    within the Frame, opacity of CelRef, toggling to show Cel Info (etc.)  This widget
    should only be used via a QGraphicsProxyWidget.
*/




//#include <QDebug>
#include "stagedcelwidget.h"
#include "ui_staged_cel_widget.h"
#include "animation/celref.h"
#include "animation/cel.h"
#include "util.h"
#include <QPainter>
#include <QDebug>



/*!
    Creates a new StagedCelWidget with the supplied CelRef.  It's possible to
    set \a ref to NULL, but probably not a good idea.
*/
StagedCelWidget::StagedCelWidget(CelRef *ref, QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::StagedCelWidget),
    _ref(ref)
{
    _ui->setupUi(this);

    if (ref) {
        Cel *cel = _ref->cel();

        // Populate the UI
        _ui->thumbLabel->setPixmap(_mkThumbnail());
        _ui->nameLabel->setText(cel->name());
        _onGeometryChanged();        // Reuse function to set label
        _ui->showCelInfoButton->setChecked(ref->showCelInfo());

        // Show it as deselected
        _setBGColor(Qt::white);

        // Signals & slots
        connect(_ui->showCelInfoButton, &QToolButton::clicked, _ref, &CelRef::setShowCelInfo);
        connect(cel, &Cel::nameChanged, _ui->nameLabel, &QLabel::setText);
        connect(cel, &Cel::resized, this, &StagedCelWidget::_onGeometryChanged);
        connect(_ref, &CelRef::positionChanged, this, &StagedCelWidget::_onGeometryChanged);
        connect(_ui->raiseButton, &QToolButton::clicked, this, &StagedCelWidget::_onRaiseButtonClicked);
        connect(_ui->lowerButton, &QToolButton::clicked, this, &StagedCelWidget::_onLowerButtonClicked);
    } else
        _setBGColor(QColor(0xFF, 0x8C, 0x8C));

    // Info
    qDebug() << "[StagedCelWidget created] ref=" << _ref;
}


/*!
    Deconstructions the StagedCelWidgets (mainly cleans up the UI).
*/
StagedCelWidget::~StagedCelWidget() {
    delete _ui;

    // Info
    qDebug() << "[StagedCelWidget deleted] ref=" << _ref;
}


/*!
    Returns a pointer to the CelRef this widget is for.  May return NULL.
*/
CelRef *StagedCelWidget::ref() {
    return _ref;
}


/*!
    Overrided function that intercepts a mouse event.  Will pass it down to its
    parent, but will also emit the mousePressed() signal.
*/
void StagedCelWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    emit mousePressed();
}


/*!
    Will "select," the widget.  Will change the background color to a nice blue,
    signifying that the corresponding CelRef is the current one (for operations
    like drawing).
*/
void StagedCelWidget::select() {
    _setBGColor(QColor(0xB4, 0xFF, 0xFF));

    qDebug() << "[StagedCelWidget select]";
}


/*!
    Will "deselct," the widget.  Does nothing more than change the background
    color to white.
*/
void StagedCelWidget::deselect() {
    _setBGColor(Qt::white);
    
    qDebug() << "[StagedCelWidget deselect]";
}


/*!
    Called by both signals Cel::resized() and CelRef::positionChanged(), this will
    modify the text of the Geometry label to reflect that change.
*/
void StagedCelWidget::_onGeometryChanged() {
    QPoint p = _ref->pos().toPoint();
    QSize size(_ref->cel()->size());
    _ui->geometryLabel->setText(QString("[%1, %2]  (%3, %4)").arg(size.width()).arg(size.height()).arg(p.x()).arg(p.y()));
}


/*!
    When the Cel's raiseButton emits it clicked signal, this will emit
    the raiseButtonClicked() signal.
*/
void StagedCelWidget::_onRaiseButtonClicked() {
    emit raiseButtonClicked();
}


/*!
    When the Cel's lowerButton emits it clicked signal, this will emit
    the lowerButtonClicked() signal.
*/
void StagedCelWidget::_onLowerButtonClicked() {
    emit lowerButtonClicked();
}


/*!
    Small utilty function to set the background of the widget to
    \a clr.
*/
void StagedCelWidget::_setBGColor(QColor clr) {
    QPalette pal(palette());
    pal.setColor(QPalette::Window, clr);
    setPalette(pal);
}


/*!
    Generates a preview/thumbnail of the Cel.
*/
QPixmap StagedCelWidget::_mkThumbnail() {
    // Will draw the Pixmap onto the Preivew Item
    QImage img = _ref->cel()->image();
    QImage preview = util::mkBlankImage(QSize(STAGED_CEL_WIDGET_THUMB_SIZE, STAGED_CEL_WIDGET_THUMB_SIZE));

    // Sizing;
    QRect src, dest;
    src = QRect(QPoint(0, 0), img.size());
    dest.setSize(preview.size());
    
    // Adjustments, center the destination
    bool widthLess = src.width() < dest.width();
    bool heightLess = src.height() << dest.height();
    if (widthLess || heightLess) {
        // Change dest width
        if (widthLess)
            dest.setWidth(src.width());
        if (heightLess)
            dest.setHeight(src.height());

        int midSize = STAGED_CEL_WIDGET_THUMB_SIZE / 2;
        dest.moveCenter(QPoint(midSize, midSize));
    }

    // Draw
    QPainter p(&preview);
    p.fillRect(0, 0, preview.width(), preview.height(), Qt::white);
    p.drawImage(dest, img, src);
    p.setPen(Qt::black);
    p.drawRect(0, 0, preview.width() - 1, preview.height() - 1);

    // Return it
    return QPixmap::fromImage(preview);
}
