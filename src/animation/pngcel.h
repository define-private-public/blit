// File:         pngcel.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the PNGCel object


// TODO List
//  - Constructor
//    - When CelManager is made, make one constructor that checks if it is new or a file
//      - simplify the construction so that we don't have two
//  - Add some sort of timer, that after a set time, will deactivate the Cel
//    - For example, if a Cell hasn't be touched in about half an hour, it would be save to deactivate if
//      (if it isn't currently selected)
//    - Would be good for memory consumption and usefel so you don't have to wait so long of each frame
//      to load



#ifndef PNG_CEL_H
#define PNG_CEL_H


#define PNG_CEL_TYPE 1


#include "animation/cel.h"
class QImage;


class PNGCel : public Cel {
    Q_OBJECT;

public:
    enum { Type = Type + PNG_CEL_TYPE};

    // Constructors/Deconstructors
    explicit PNGCel(Animation *anim, QString name, QSize size);        // Brand new Cel
    explicit PNGCel(Animation *anim, QString name);                    // Existing Cel
    PNGCel *copy(QString name="");
    ~PNGCel();

    // Important info
    bool isEmpty();
    bool hasFileResources();
    QStringList fileResources();
    void save(QString basename="");
    int type();
    bool setName(QString name);

    // A PNG Cel special
    QImage image();
    void setImage(QImage &image);

    // Cel Delection functions
    void remove(bool deletePNG=true);
    bool toBeRemoved();

    // Overloads
    void paint(QPainter *painter);

    // sizing information
    // TODO add in simple width/height resizing
    void resize(int x, int y, int width, int height);
    void resize(QPoint topLeft, QSize size);


private slots:
    // For loading / closing
    void _loadPNG();    // signal = Cel::activated
    void _closePNG();    // signal = Cel::deactivated


protected:
    // Data members
    QImage *_png = NULL;        // In the format of Premultiplied 32 Bit ARGB
    bool _deletePNG = false;    // To delete the PNG file upon PNGCel deletion

    // Functions
    void _mkPNG();

};


#endif // PNG_CEL_H

