// File:         colouranimation/frame.h
// Author:       Ben Summerton (define-private-public)
// Description:  ColorFrame is a class that shows a solid colour inside of its bounds.  If you give it a
//               colour with an alpha value, it will show some boxes in the background.
//
//               There is also a subclass of ColorFrame called "SelfChaningColorFrame," when once its
//               double-clicked, it will bring up a dialog window to set the color.
//
//               And another one called "ClickableColorFrame," which allows a user to click it.  
//               It will emit signals when its clicked and whatnot.  Also changes outline frame if
//               some code in the function `ClickableColorFrame::setSelected` is uncommented.

// TODO List:
//   - Implement other ways of showing transparencies (not just a right angle box in the bagkround)


#ifndef COLOR_FRAME_H
#define COLOR_FRAME_H


#define COLOR_FRAME_DEFAULT_COLOR Qt::black
#define COLOR_FRAME_DEFAULT_SIZE QSize(64, 64)
#define COLOR_FRAME_DFEAULT_SQUARE_SIZE QSize(8, 8)


#include <QFrame>


/*== Main Color Frame class ==*/
class ColorFrame : public QFrame {
    Q_OBJECT;

public:
    ColorFrame(QWidget *parent=NULL);

    void setSquareSize(QSize size);
    QSize squareSize();

    QColor color();


public slots:
    void setColor(QColor clr);


signals:
    void colorChanged(QColor clr);


private:
    // Member vars
    QSize _squareSize;        // Size of the square for the checkerboard in the back (transparency related)


protected:
    QColor _clr;            // Stored color

    void paintEvent(QPaintEvent *event);

};



/*== Subclass that allows for changes on double clicks (brings up a dialog) ==*/
class SelfChangingColorFrame : public ColorFrame {
    Q_OBJECT;

public:
    SelfChangingColorFrame(QWidget *parent=NULL);


protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

};



/*== Subclass that allows for the user to Select the color Frame (emtis extra signals) ==*/
class ClickableColorFrame : public ColorFrame {
    Q_OBJECT;

public:
    ClickableColorFrame(QWidget *parent=NULL);


signals:
    void pressed(QMouseEvent *event);
    void released(QMouseEvent *event);
    void doubleClicked(QMouseEvent *event);


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

};


#endif // COLOR_FRAME_H

