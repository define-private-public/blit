// File:         fileops.h
// Author:       Ben Summerton (define-private-public)
// Description:  File Ops. (File Operatiosn) is a collection of functions and variables that are used to
//               load in aniamtions from the Blit File Format.


#ifndef FILE_OPS_H
#define FILE_OPS_H


class Cel;
class PNGCel;
class CelRef;
class Frame;
class TimedFrame;
class XSheet;
class CelLibrary;
class FrameLibrary;
class Animation;
class QString;
class QStringList;
class QSize;
class QUuid;
class QImage;
class QXmlStreamReader;
class QXmlStreamWriter;
class QColor;
#include <QList>
#include <QHash>
#include <QPointer>


namespace FileOps {
    // Some constant "variables"
    const int curFileFormatVersion = 2;            // Current version of the Blit File Format
    QStringList okayExtensions();                // Okay extensions to read
    QStringList validFilters();                    // Okay filters for file dialogs
    QHash<QString, QString> extensionMap();        // Used to map filters to extensions

    // utility functions
    bool extensionOkay(QString filename, bool caseSensitive=false);
    bool isBlitDir(QString path);
    bool pngExists(QString path, QString name);
    bool mkEmptyPNG(QString path, QString name, QSize size);
    bool rmPNG(QString path, QString name);
    bool renameFile(QString dir, QString oldName, QString newName);



    // Animation -> XML
    void celToXML(QXmlStreamWriter &xml, Cel *cel);
    void celsToXML(QXmlStreamWriter &xml, CelLibrary *cl);
    void celRefToXML(QXmlStreamWriter &xml, CelRef *cr);
    void frameToXML(QXmlStreamWriter &xml, Frame *frame);
    void framesToXML(QXmlStreamWriter &xml, FrameLibrary *fl);
    void timedFrameToXML(QXmlStreamWriter &xml, TimedFrame *tf);
    void planeToXML(QXmlStreamWriter &xml, QList<QPointer<TimedFrame>> &plane, int num);
    void xsheetToXML(QXmlStreamWriter &xml, XSheet *xsheet);
    void animationToXML(QXmlStreamWriter &xml, Animation *anim);

    // XML -> Animation
    Cel *xmlToCel(QXmlStreamReader &xml, Animation *anim);
    void xmlToCels(QXmlStreamReader &xml, Animation *anim);
    CelRef *xmlToCelRef(QXmlStreamReader &xml, Animation *anim);
    Frame *xmlToFrame(QXmlStreamReader &xml, Animation *anim);
    void xmlToFrames(QXmlStreamReader &xml, Animation *anim);
    TimedFrame *xmlToTimedFrame(QXmlStreamReader &xml, Animation *anim);
    QList<TimedFrame *> xmlToPlane(QXmlStreamReader &xml, Animation *anim);
    XSheet *xmlToXSheet(QXmlStreamReader &xml, Animation *anim);
    Animation *xmlToAnimation(QXmlStreamReader &xml, QString resourceDir);



    // Colors/Palette
    void colorsToXML(QXmlStreamWriter &xml, QList<QColor> &colors);
    QList<QColor> xmlToColors(QXmlStreamReader &xml);



    // saving/loading
    bool saveAnimation(Animation *anim, QString path);
    Animation *loadAnimation(QString path);
    bool savePalette(QList<QColor> &colors, QString path);
    QList<QColor> loadPalette(QString path);

    // Imports
    PNGCel *loadStillImage(QString filename, Animation *anim);

    // Exports
    bool saveStillFrame(Frame *frame, QString dest);
    bool saveSpritesheet(QImage spritesheet, QString dest);
};

#endif // FILE_OPS_H

