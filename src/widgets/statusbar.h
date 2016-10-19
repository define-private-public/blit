// File:         statusbar.h
// Author:       Ben Summerton (define-private-public)
// Description:  This class is a singletone for the MainWindow (BlitApp in this case).  It just contains
//               a few other widgets for mainly editing functionality.
//
//               NOTE: This class may disappear or get moved somewhere in the near future.  It's kind of
//                     temporary for the moment


#ifndef STATUS_BAR_H
#define STATUS_BAR_H


#include <QStatusBar>
#include <QList>
class BlitApp;
class QLabel;
class QComboBox;



class StatusBar : public QStatusBar {
    Q_OBJECT;

public:
    StatusBar(BlitApp *parent = NULL);
    ~StatusBar();


public slots:
    void onZoomChanged(double zoom);
    void onZoomBoxChanged(int index);


private slots:
    void _onCanvasMouseMoved(QPointF pos);


private:
    // Internal functions
    void _setupChangeZoomBox();

    // Members
    QLabel *_canvasMouseLabel;
    QComboBox *_changeZoomBox;
    QList<double> _zoomList;

};


#endif // STATUS_BAR_H

