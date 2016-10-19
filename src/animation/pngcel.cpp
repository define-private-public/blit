// File:         pngcel.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source implementation of the PNGCel class


/*!
    \inmodule Animation
    \class PNGCel
    \brief PNGCel is a sublcass of Cel, where the image data is stored in a PNG.
    
    All image data is stored in a 32 bit PNG image.  The name of the file is the
    same as the unique name given to the Cel.
*/


#include "animation/pngcel.h"
#include "animation/celref.h"
#include "animation/animation.h"
#include "util.h"
#include "fileops.h"
#include <QStringList>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <QDebug>



/*!
    Creates a new PNG Cel.  If name is already taken, it will append some
    randomly generated characters.  Size must be at least 1x1, it will be
    resized if not.
*/
PNGCel::PNGCel(Animation *anim, QString name, QSize size) :
    Cel(anim, name, size)
{
    // Make the new PNG file
    _mkPNG();

    // Connect the slots
    connect(this, &Cel::activated, this, &PNGCel::_loadPNG);
    connect(this, &Cel::deactivated, this, &PNGCel::_closePNG);

    // Debug info
    qDebug() << "  [PNGCel New" << _name << "]";
}


/*!
    Loads up an exisiting PNG Cel.  It is assumed that name is not already taken
    and exists as an image file in the PNG Directory
*/
PNGCel::PNGCel(Animation *anim, QString name) :
    Cel(anim, name, QSize(1, 1))
{
    // Load up the PNG to get the size
    _loadPNG();
    if (_png) {
        Cel::resize(_png->size());        // Call the parent one, not this one
        _closePNG();
    } else {
        // Bad things
        qDebug() << "Error, wasn't able to load PNG file for " << _name;
    }

    // Connect the slots
    connect(this, &Cel::activated, this, &PNGCel::_loadPNG);
    connect(this, &Cel::deactivated, this, &PNGCel::_closePNG);

    // Debug info
    qDebug() << "  [PNGCel Existing" << _name << "] size=" << _size;
}


/*!
    The same as the base class's description, this will do a deepy copy of the
    PNGCel object.  \a name follows the same rules as creating any new Cel.
    The new Cel will share the same parent QObject.

    If the PNGCel that is set to be removed upon deletion, the copy will also
    be set to be removed.

    \sa setName()
    \sa remove()
*/
PNGCel *PNGCel::copy(QString name) {
    qDebug() << "[PNGCel copy]";

    // Create blank brand new, then copy of state and QImage data
    PNGCel *cel = new PNGCel(_anim, name, _size);
    cel->remove(_deletePNG);
    
    // Load temp and copy
    QImage tmp(image());
    cel->setImage(tmp);

    return cel;
}


/*!
    Deconstructor for the PNG based Cel.  If the PNG was marked to be removed,
    this will delete the file associated with the cel.

    \sa remove()
    \sa toBeRemoved()
*/
PNGCel::~PNGCel() {
    // Cleanup mem (will close the PNG if it's open)
    deactivate();

    // Remove the PNG if flagged to do so
    if (_deletePNG)
        FileOps::rmPNG(_anim->resourceDir(), _name);
}


/*!
    Used to check if the PNG Cel is considered empty or not.  Currently, it is
    always considered not empty (e.g. "full").
*/
bool PNGCel::isEmpty() {
    // Consider the PNG Cel to never be empty
    return false;
}


/*!
    Reimplemented from base class, the PNGCel has an underlying PNG file, so
    this does use File resources.

    Returns true
*/
bool PNGCel::hasFileResources() {
    return true;
}


/*!
    The PNGCel has one file resource, it's name PNG for the Cel
*/
QStringList PNGCel::fileResources() {
    return QStringList() << _name + ".png";
}


/*!
    Will have the internal PNG/QImage to the disk.  By default \a basename
    is empty.  It's not recommended to provide a file extension or directory,
    the PNGCel will take care of that.  Passing in an empty string will use
    the PNGCel's name as the basename.  Passing in the PNGCel's name will do
    nothing.
*/
void PNGCel::save(QString basename) {
    // Dubs check, PNG should already exists
    if (basename == _name)
        return;

    // Check for empty (default)
    if (basename.isEmpty())
        basename = _name;

    // Perform the action
    if (_png) {
        // If loaded, just save it
        QString path = _anim->resourceDir() + basename + ".png";

        if (!_png->save(path))
            qDebug() << "[PNGCel save]" << this << "couldn't save" << _name << "to" << path;
    } else {
        // Else not loaded, copy it.
        QString src = _anim->resourceDir() + _name + ".png";
        QString dest = _anim->resourceDir() + basename + ".png";

        if (!QFile::copy(src, dest)) {
            qDebug() << "[PNGCel save]" << this << "couldn't copy from" << src  << "to" << dest;
            qDebug() << "              destination file might already exists.";
        }
    }

}


/*!
    Returns the type of the cel. Should be (Type + PNG_CEL_TYPE).
*/ 
int PNGCel::type() {
     return Type;
}


/*!
    Has all of the same functionality of the parent classes setName() function
    But this will also rename the underlying PNG if setting the name was a 
    success.

    returns true on success, false on failure
*/
bool PNGCel::setName(QString name) {
    // Save old name and call parent
    QString oldName = _name;
    bool success = Cel::setName(name);

    if (success) {
        // If it was a success, then rename the underlying png file
        // But if the rename didn't work, then set the Cel to its old name
        success = FileOps::renameFile(_anim->resourceDir(), oldName + ".png", _name + ".png");
        if (!success) {
            Cel::setName(oldName);
            qDebug() << "[PNGCel setName; able to rename Cel, but not able to rename underlying file, switching to old name]";
        }
    }

    return success;
}


/*!
    Returns a copy of the QImage that this PNGCel contains.  Overrides base 
    class function.
*/
QImage PNGCel::image() {
    if (_png)
        return *_png;
    else {
        // Need to temporarly load up the image
        _loadPNG();
        QImage img(*_png);        // Copy
        _closePNG();

        return img;                // Give it out
    }
}


/*!
    This function will copy the image data provided over to the Cels' image data.  It is
    assumed that the dimensions of the supplied image maatch up with the Cel.  It is also
    assumed that the color format of the pixels is the same.  Think that this function
    will replace the Cel's image/PNG with this one.
*/
void PNGCel::setImage(QImage &image) {
    // If the image is currently loaded
    bool imageLoaded = (_png != NULL);        // Refers to if the image has been loaded or not previously
    if (!imageLoaded)
        _loadPNG();
    
    // Do the swap
    QImage *newImage = new QImage(image.copy());        // Deep copy
    QImage *oldImage = _png;
    _png = newImage;

    // cleanup if needed
    if (!imageLoaded)
        _closePNG();
    else
        delete oldImage;

    // Send a signal to repaint if active
    if (_active) {
        for (auto crIter = _celRefs.begin(); crIter != _celRefs.end(); crIter++)
            (*crIter)->update();
    }
}


/*!
    Not necessarly a deconstructor, but calling this function will mark the PNG
    to be removed upon the delection of the Cel.  By default deletePNG is set to
    true.  If you want to unmark it, just pass in false.

    Overrides base class function

    \sa toBeRemoved()
*/
void PNGCel::remove(bool deletePNG) {
    _deletePNG = deletePNG;
}


/*!
    Returns if the underlying PNG image is marked to be removed or not.

    Overrides base class function

    \sa remove()
*/
bool PNGCel::toBeRemoved() {
    return _deletePNG;
}


/*!
    Will paint the image data to the QGraphicsScene.
*/
void PNGCel::paint(QPainter *painter) {
    // Don't paint an image unless something is loaded up
    if (_png)
        painter->drawImage(0, 0, *_png);

    // Call parent class's method
    Cel::paint(painter);
}


/*!
    Does a resize of the PNG Cel.  width and height are the new dimensions of the
    Cel object.    x and y are where to place the old Cel image onto the new one.  
    For example,  if you want to resize the Cel, but have all the image data in
    the upper left conrer, you would call:
    \code
        cel->resize(0, 0, new_width, new_height);
    \endcode
    
    And if you wanted it in the lower right hand cornder:
    \code
        cel->resize(new_width - width(), new_height - height(), new_width, new_height);
    \endcode
    
    There is a chance that the Cel's image data might be cutoff if you give the
    x or y varaibles a postion greater than (new - old).  It's your choice. 
    The underlying PNG image will also be overwritten.
    
    It will return true on success, or false on falure.  If an invalid width or
    height are given, this funtion will return false and do nothing.
*/
void PNGCel::resize(int x, int y, int width, int height) {
    // TODO it's undefined what will happen if x or y are negative.  figure out what
    //      will happen.

    // Bounds
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    
    // don't do anything if still the same size
    if ((width == _size.width()) && (height == _size.height()))
        return;
    
    // TODO renable later
//
//    // Load up the image
//    bool imageLoaded = (_png != NULL);        // Refers to if the image has been loaded or not previously
//    if (!imageLoaded)
//        _loadPNG();
//    
//    // Vars
//    QSize newSize(width, height);
//    QImage *tmp = new QImage(util::mkBlankImage(newSize));
//    QImage *oldImage = NULL;
//    QString path = _anim->resourceDir() + _name + ".png";
//
//    // Copy the older one over and save it
//    QPainter p(tmp);
//    p.drawImage(x, y, *_png);
//    p.end();
//
//    // Swap variables
//    if (!imageLoaded)
//        oldImage = _png;
//    _png = tmp;
//
//    // cleanup if needed
//    if (!imageLoaded)
//        _closePNG();
//    else
//        delete oldImage;
//
//    // Repaint all of our CelRefs
//    if (_active) {
//        for (auto crp : _celRefs)
//            crp->update();
//    }
//
    // Call the parent function to resize
    Cel::resize(width, height);
}


/*!
    Will resize the png image to the specified size.  Minimum size will be at
    least 1x1.  This is an overloaded function
*/
void PNGCel::resize(QPoint topLeft, QSize size) {
    // TODO it's undefined what will happen if x or y are negative.  figure out what
    //      will happen.
    resize(topLeft.x(), topLeft.y(), size.width(), size.height());
}


/*!
    Small internal utility function to make a PNG image, and report an error if it wasn't successful.
    It will save a new blank PNG image to the _png directory
*/
void PNGCel::_mkPNG() {
//    qDebug() << "PNGCel::_mkPNG()";
    bool madePNG = FileOps::mkEmptyPNG(_anim->resourceDir(), _name, _size);
    if (!madePNG)
        qWarning() << "Wasn't able to make an Empty PNG for Cel:" << _name;
}


/*!
    Internal funciton to load up the PNG image for the Cel.

    \sa _closePNG()
*/
void PNGCel::_loadPNG() {
//    qDebug () << "PNGCel::_loadPNG()";
    if (!_png) {
        // Only load up if the _png is NULL, and converter it the correct format
        QString path = _anim->resourceDir() + _name + ".png";
        QImage tmp(path);
        _png = new QImage(tmp.convertToFormat(QImage::Format_ARGB32_Premultiplied));

        // Error
        if (!_png)
            qDebug() << "Error, wasn't able to open the PNG for:" << _name;
    }
}


/*!
    Saves the PNG image if it's open.

    \sa _loadPNG()
*/
void PNGCel::_closePNG() {
//    qDebug () << "PNGCel::_closePNG()";
    if (_png) {
        // Save the image and free the memory
        QString path = _anim->resourceDir() + _name + ".png";
        
        if (!_png->save(path))
            qDebug() << "Error, couldn't save" << _name << "to" << path;

        delete _png;
        _png = NULL;
    }
}


