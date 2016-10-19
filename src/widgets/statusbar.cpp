// File:         statusbar.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation for StatusBar class.



#include "widgets/statusbar.h"
#include "blitapp.h"
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QtMath>


StatusBar::StatusBar(BlitApp *parent) :
    QStatusBar(parent)
{
    // Add some default values to the zoomList
    _zoomList << 1;
    _zoomList << 2;
    _zoomList << 3;
    _zoomList << 4;
    _zoomList << 5;
    _zoomList << 6;
    _zoomList << 7;
    _zoomList << 8;
    _zoomList << 12;
    _zoomList << 16;
    _zoomList << 20;
    _zoomList << 24;
    _zoomList << 32;

    // Setup widgets
    _changeZoomBox = new QComboBox(this);
    _setupChangeZoomBox();

    _canvasMouseLabel = new QLabel("(X, Y)", this);

    // Default size
//    _changeZoomBox->setCurrentIndex(0);
    onZoomChanged(1.0);

    // Add widget to the layout
    addWidget(_canvasMouseLabel);
    addWidget(new QWidget(this), 1);
    addWidget(_changeZoomBox);

    // Hoopup some slots and singals
    void (QComboBox::*currentIndexChangedSignal)(int) = &QComboBox::currentIndexChanged;
    connect(_changeZoomBox, currentIndexChangedSignal, this, &StatusBar::onZoomBoxChanged);
    connect(parent, &BlitApp::zoomChanged, this, &StatusBar::onZoomChanged);
    connect(parent, &BlitApp::canvasMouseMoved, this, &StatusBar::_onCanvasMouseMoved);
}


StatusBar::~StatusBar() {
    // Do Nothing
}


void StatusBar::onZoomChanged(double zoom) {
    // Called when BlitApp's zoom value has been changed, this will progmatically set the zoom
    // percent in the drop down.  If the zoom value isn't found, then it will add it to the
    // zoom list


    // Check for new zoom value
    if (!_zoomList.contains(zoom)) {
        _zoomList << zoom;
        qSort(_zoomList);
        _setupChangeZoomBox();
    }

    // Set zoom box
    _changeZoomBox->setCurrentIndex(_zoomList.indexOf(zoom));
}


void StatusBar::onZoomBoxChanged(int index) {
    // Called by the drop down box's 'currentIndexChanged' singal,  Will change the current zoom for the
    // application. Index should be [0, _zoomList.size()]
    if ((index < 0) || (index > _zoomList.size()))
        return;

    BlitApp::app()->setZoom(_zoomList[index]);
}


/*!
    Triggered by the signal BlitApp::canvasMouseMoved(), this will change the text
    oof the mouse canvas label to the position of the mouse.
*/
void StatusBar::_onCanvasMouseMoved(QPointF pos) {
    _canvasMouseLabel->setText(QString("(%1, %2)").arg(qFloor(pos.x())).arg(qFloor(pos.y())));
}


void StatusBar::_setupChangeZoomBox() {
    // Internal funciton to setup the elements in the chagne zoom box.
    _changeZoomBox->clear();

    // Insert elements
    for (int i = 0; i < _zoomList.size(); i++) {
        QString zoomStr = QString::number(_zoomList[i] * 100, 'f', 0) + "%";
        _changeZoomBox->insertItem(i, zoomStr);
    }
}

