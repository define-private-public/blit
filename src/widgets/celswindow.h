// File:         celswindow.h
// Author:       Ben Summerton (define-private-public)
// Description:  The CelsWindow is a Qt::Tool widget that is used to display the Cels in the
//               currently open Frame.  The user can modify te layering & size.



#ifndef CELS_WINDOW_H
#define CELS_WINDOW_H


#include <QWidget>
#include <QPointer>
#include <QList>
#include <QHash>
class BlitApp;
class CelRef;
class Frame;
class TimedFrame;
class StagedCelWidget;
class QToolButton;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsRectItem;
class QGraphicsSceneMouseEvent;
class QGraphicsProxyWidget;


namespace Ui {
    class CelsWindow;
}


class CelsWindow : public QWidget {
    Q_OBJECT;

public:
    CelsWindow(BlitApp *blitApp, QWidget *parent=NULL);
    ~CelsWindow();


signals:
    void animationPlaybackStateChanged(bool isPlaying);


public slots:
    void setFrame(TimedFrame *tf);                        // Also "onCurTimedFrameChanged"
    void onCurCelRefChanged(CelRef *ref);                // Colors a Cel



private slots:
    // Signals from BlitApp
    void _onAnimationPlaybackStateChanged(bool isPlaying);    // Technically It's from TimelineWindow

    // Signals from the Frame
    void _onFrameDestroyed(QObject *obj=NULL);                // Cleanup method for when _curFrame is deleted
    void _onFrameCelAdded(CelRef *ref);
    void _onFrameCelRemoved(CelRef *ref);
    void _onFrameCelMoved(CelRef *ref);

    // Signals from the StagedCelWidgets
    void _onCelWidgetPressed();
    void _onCelWidgetRaiseButtonClicked();
    void _onCelWidgetLowerButtonClicked();

    // From the UI buttons
    void _onCopyCelButtonClicked(bool checked);
    void _onAddCelButtonClicked(bool checked);
    void _onDeleteCelButtonClicked(bool checked);

    void _enableWidgets(bool enable=true);


protected:
    void closeEvent(QCloseEvent *event);


private:
    // Internal organizational functions
    void _addCel(CelRef *cel);
    void _addSeparator(int i);
    qreal _yValueForIndex(int i);

    // Member Vars
    Frame *_curFrame = NULL;
    CelRef *_curRef = NULL;
    StagedCelWidget *_curSCW = NULL;
    QList<StagedCelWidget *> _scWidgets;
    QHash<CelRef *, StagedCelWidget *> _scwMap;
    QList<QGraphicsProxyWidget *> _proxies;
    QHash<CelRef *, QGraphicsProxyWidget *> _proxyMap;
    QList<QGraphicsRectItem *> _separators;

    // GUI
    bool _isPlayingBack = false;
    Ui::CelsWindow *_ui;
    QGraphicsScene *_scene;


};


#endif // CELS_WINDOW_H

