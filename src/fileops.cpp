// File:         fileops.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Implementation of the File oprattions functions

// TODO List
//   - Notice how there is a lot of similar code for checking if a Dir exists, is a Dir, and the propor
//     permissions are set?  Write a reuseable function for it
//   - See saveAnimation() regarding Cels in a new directory
//   - The export functions may be kind of useless now, they used to do a lot more in the python version



#include "fileops.h"
#include "util.h"
#include "animation/cellibrary.h"
#include "animation/framelibrary.h"
#include "animation/pngcel.h"
#include "animation/celref.h"
#include "animation/frame.h"
#include "animation/timedframe.h"
#include "animation/xsheet.h"
#include "animation/animation.h"
#include <QSize>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QColor>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QImage>
#include <QDebug>


namespace FileOps {
    /*!
        Returns a QStringList of the file extensions that make up a valid Blit Animation file.
    */
    QStringList okayExtensions() {
        QStringList ret;
        ret
        << ".xml"
        << ".png";
        return ret;
    }

    /*!
        Returns a QStringList of file extension filters for dialog windows.
    */
    QStringList validFilters() {
        QStringList ret;
        ret
        << "PNG Image (*.png)"
        << "BMP Image (*.bmp)"
        << "JPG Image (*.jpg)"
        << "JPEG Image (*.jpeg)";
        return ret;
    }
    
    /*!
        Used to map filters to extensions.
    */
    QHash<QString, QString> extensionMap() {
        QHash<QString, QString> ret;
        QStringList vf = validFilters();
        ret[vf[0]] = ".png";
        ret[vf[1]] = ".bmp";
        ret[vf[2]] = ".jpg";
        ret[vf[3]] = ".jpeg";
        return ret;
    }


    /*!
        Small helper function to see if the extension for \a filename is part of the Blit File Format.
        Will return True if so, False otherwise,  By default it doesn't care about the case of the
        extensions, though you can set it do though via \a caseSensitive (default is false).
    */
    bool extensionOkay(QString filename, bool caseSensitive) {
        // Upper case it
        if (!caseSensitive) 
            filename = filename.toLower();

        // Run the filename through the gaunlet of okay extensions
        QListIterator<QString> iter(okayExtensions());
        while (iter.hasNext()) {
            if (filename.endsWith(iter.next()))
                return true;
        }

        // None of them were okay, return false
        return false;
    }


    /*!
        Checks to see if the supplied directory could contains a valid Blit Animation.  It checks:
         1. Are the correct permissions set for the containing files?
         2. Is a 'sequence.xml' file found?
         3. Is a 'palette.xml' file found?
        
        If it passes these smell tets, then it is safe to assume that this is a valid Blit Animation.

        It doesn't check at all if it's the current blit version or not.
    */
    bool isBlitDir(QString path) {
        // Directory and path info
        QDir dir(path);
        QFileInfo pathInfo(path);

        // First make sure the folder exits, and the permissions are okay
        if (!pathInfo.exists())
            return false;
        else if (!pathInfo.isDir())
            return false;
        else if (!(pathInfo.isReadable() | pathInfo.isWritable() | pathInfo.isExecutable()))
            return false;

        // Loop through looking for the XML files, and make sure we can read each file
        bool seqFound = false;
        bool palFound = false;
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fi = list[i];

            // Only look at files that we care about
            if (extensionOkay(fi.fileName())) {
                if (!fi.isReadable())
                    return false;

                if (fi.fileName() == "sequence.xml")
                    seqFound = true;
                else if (fi.fileName() == "palette.xml")
                    palFound = true;
            }
        }

        // It all rests on if we found the XML files or not
        return (seqFound && palFound);
    }


    /*!
        Most likely will be used by the Cel class, will check to see if a PNG file exists with the
        supplied name as a filename.  \a path should be the location of a valid Blit Animation Project,
        and \a is the filename of the PNG image.
    */
    bool pngExists(QString path, QString name) {
        QFileInfo fi(path + name + ".png");
        return fi.exists();
    }


    /*!
        Will make an empty (transparent) PNG (32 Bit ARGB) file in a specified directory \a dirPath.
        \a size must be 1x1 or greater.  Filename of the PNG will be \a name (doesn't require ".png"
        at the end, that will be added automatically.).  It is unspecified what will happen if the
        PNG file already exists (most likely will overwrite it). 
        Will return true on success.
    */
    bool mkEmptyPNG(QString dirPath, QString name, QSize size) {
        // Check size
        if (size.isEmpty())
            return false;

        QString filePath = dirPath + name + ".png";
        QImage png(size, QImage::Format_ARGB32_Premultiplied);
        png.fill(Qt::transparent);
        return png.save(filePath);
    }


    /*!
        Will delete a Cel's PNG Image.  Supply the PNGCel's file \a name (without ".png"), and
        \a path as the directory where it's stored.

        Will return true upon success, false upon failure.
    */
    bool rmPNG(QString dirPath, QString name) {
        QFile pngFile(dirPath + name + ".png");
        return pngFile.remove();
    }
    

    /*!
        Renames a file in a given directory.  \a dirPath must have a trailing slash.
        \a oldName is the name of the file you want to rename to \a newName.

        Returns true on success, false on failure
    */
    bool renameFile(QString dirPath, QString oldName, QString newName) {
        QFile file(dirPath + oldName);
        file.open(QIODevice::ReadOnly);
        bool success = file.rename(dirPath + newName);
        file.close();

        return success;
    }



    /*== Blit Objects -> XML ==*/
   
    /*!
        Writes a Cel object to an XML stream.
    */
    void celToXML(QXmlStreamWriter &xml, Cel *cel) {
        // <cel>
        xml.writeStartElement("cel");

        // Give it a type
        QString type;
        switch (cel->type()) {
            case CEL_BASE_TYPE: type = "base"; break;
            case PNG_CEL_TYPE: type = "PNG"; break;
            default: type = "base";
        }
        xml.writeAttribute("type", type);

        // The rest of them
        xml.writeAttribute("name", cel->name());
        xml.writeAttribute("width", QString::number(cel->width()));
        xml.writeAttribute("height", QString::number(cel->height()));

        xml.writeEndElement();
        // </cel>
    }


    /*!
        Writes out the CelLibrary XML.  Will add a <cels></cels> block.
    */
    void celsToXML(QXmlStreamWriter &xml, CelLibrary *cl) {
        // <cels>
        xml.writeStartElement("cels");
        xml.writeAttribute("count", QString::number(cl->numCels()));

        // Write out all of the Cels
        QList<Cel *> cels = cl->cels();
        for (auto iter = cels.begin(); iter != cels.end(); iter++)
            celToXML(xml, *iter);

        xml.writeEndElement();
        // </cels>
    }
    
    
    /*!
        Writes out a CelRef to an XML stream.  Will add a <staged_cel/> tag.
    */
    void celRefToXML(QXmlStreamWriter &xml, CelRef *cr) {

        // <staged_cel>
        xml.writeStartElement("staged_cel");
        xml.writeAttribute("cel", cr->cel()->name());
        xml.writeAttribute("x", QString::number(cr->x()));
        xml.writeAttribute("y", QString::number(cr->y()));
        xml.writeAttribute("z_order", QString::number(cr->zValue()));
        xml.writeEndElement();
    }


    /*!
        Write a Frame object to an XML stream.  Will add a <frame></frame> block.
    */
    void frameToXML(QXmlStreamWriter &xml, Frame *frame) {

        // <frame>
        xml.writeStartElement("frame");
        xml.writeAttribute("name", frame->name());

        // Write the cels
        for (int i = 0; i < frame->numCels(); i++)
            celRefToXML(xml, frame->cel(i));

        xml.writeEndElement();
        // </frame>
    }


    /*!
        Writes out the FrameLibrary to the XML stream.  Adds a <frames></frames> block.
    */
    void framesToXML(QXmlStreamWriter &xml, FrameLibrary *fl) {
        // <frames>
        xml.writeStartElement("frames");
        xml.writeAttribute("count", QString::number(fl->numFrames()));

        // Write out all of the Frames
        QList<Frame *> frames = fl->frames();
        for (auto iter = frames.begin(); iter != frames.end(); iter++)
            frameToXML(xml, *iter);

        xml.writeEndElement();
        // </frames>
    }
    
   
    /*!
        // Writes out a TimedFrame to a <timedfrme/> tag to an XML Stream.
    */
    void timedFrameToXML(QXmlStreamWriter &xml, TimedFrame *tf) {
        // <timed_frame/>
        xml.writeStartElement("timed_frame");
        xml.writeAttribute("frame", tf->frame()->name());
        xml.writeAttribute("number", QString::number(tf->seqNum()));
        xml.writeAttribute("hold", QString::number(tf->hold()));
        xml.writeEndElement();
    }
    
    
    /*!
        Writes a Plane to the XML stream.  Adds a <plane></plane> block.
    */
    void planeToXML(QXmlStreamWriter &xml, QList<QPointer<TimedFrame>> &plane, int num) {

        // <plane>
        xml.writeStartElement("plane");
        xml.writeAttribute("number", QString::number(num));
        xml.writeAttribute("count", QString::number(plane.size()));

        // Write out the TimedFrames
        for (auto iter = plane.begin(); iter != plane.end(); iter++)
            timedFrameToXML(xml, *iter);

        xml.writeEndElement();
        // </plane>
    }

   
    /*!
        Write an XSheet object to an XML Stream.  Adds a <xsheet></xsheet> block.
    */
    void xsheetToXML(QXmlStreamWriter &xml, XSheet *xsheet) {
        // <xsheet>
        xml.writeStartElement("xsheet");
        xml.writeAttribute("fps", QString::number(xsheet->FPS()));
        xml.writeAttribute("seq_length", QString::number(xsheet->seqLength()));
        
        // Write the plane(s)
        // TODO there is currently only one plane, need to add support for multiple in the future
        QList<QPointer<TimedFrame>> firstPlane = xsheet->frames();
        planeToXML(xml, firstPlane, 1);

        xml.writeEndElement();
        // </xsheet>
    }


    /*!
        Write an Animation object to an XML stream.
    */
    void animationToXML(QXmlStreamWriter &xml, Animation *anim) {
        // <animation>
        xml.writeStartElement("animation");
        xml.writeAttribute("version", QString::number(curFileFormatVersion));

        // Meta Data
        xml.writeTextElement("name", anim->name());
    
        // Timestamps
        xml.writeStartElement("created");
        xml.writeAttribute("format", "unix_timestamp");
        xml.writeCharacters(QString::number(anim->createdTimestamp()));
        xml.writeEndElement();

        xml.writeStartElement("updated");
        xml.writeAttribute("format", "unix_timestamp");
        xml.writeCharacters(QString::number(anim->updatedTimestamp()));
        xml.writeEndElement();

        // Frame Size
        xml.writeTextElement("width", QString::number(anim->frameWidth()));
        xml.writeTextElement("height", QString::number(anim->frameHeight()));

        // Libraries
        celsToXML(xml, anim->cl());
        framesToXML(xml, anim->fl());

        // The XSheet
        xsheetToXML(xml, anim->xsheet());
    
        xml.writeEndElement();
        // </animation>
    }



    /*== XML -> Blit Objects ==*/

    /*!
        Takes in an XML Stream (and removes some data from it) to produce a Cel object.  This
        fuction will aways return a Cel, though if the Cel is invalid, it may return a NULL pointer.
        
        It's assumed that the XML stream provided has just read <cel> as a start element.  And will
        exit once a </cel> tag has just been read as an end element
    */
    Cel *xmlToCel(QXmlStreamReader &xml, Animation *anim) {
        // Variable to build the Cel
        QSize size;

        // Read the data
        QString type = xml.attributes().value("type").toString();
        QString name = xml.attributes().value("name").toString();
        size.setWidth(xml.attributes().value("width").toInt());
        size.setHeight(xml.attributes().value("height").toInt());

        // Last checks, uuid is guarenteed to be valid
        if (!size.isEmpty()) {
            // Good to return a valid one, check types
            if (type == "base")
                return new Cel(anim, name, size);
            else if (type == "PNG")
                return new PNGCel(anim, name);            // Existing constructor
        }

        // Unkown type or it was empty.
        return NULL;
    }


    /*!
        Takes in a XML stream that has just read a <cels> tag.  Will peel out
        all of the Cels and put them into the CelLibrary for the Animation
    */
    void xmlToCels(QXmlStreamReader &xml, Animation *anim) {
        // Read the data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "cel") {
                    // Add the Cel to the Animation
                    Cel *cel = xmlToCel(xml, anim);

                    if (cel)
                        anim->cl()->addCel(cel);
                    else
                        qDebug() << "Error, detected NULL Cel when reading in Frame from XML";
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Exit loop on </cels> reached
                if (tag == "cels")
                    break;
            }
        }
    }


    /*!
        Check an XML stream, that has just read a "staged_cel" token and will output
        A CelRef that is inside that frame.
    */
    CelRef *xmlToCelRef(QXmlStreamReader &xml, Animation *anim) {
        // Read the data
        QPoint pos;
        QString celName = xml.attributes().value("cel").toString();
        pos.setX(xml.attributes().value("x").toInt());
        pos.setY(xml.attributes().value("y").toInt());
        qreal zOrder = xml.attributes().value("z_order").toDouble();

        // Get the Cel by name
        Cel *cel = anim->cl()->getCel(celName);
        if (cel) {
            CelRef *cr = new CelRef(cel);
            cr->setPos(pos);
            cr->setZValue(zOrder);
            return cr;
        } else
            return NULL;
    }


    /*!
        Take in an XML Stream and will output a Frame object.  If any of the XML is invalid, then
        a NULL Pointer will be retruned.  This function will modify the XML Stream.
        
        It's assumed that the XML stream provided has just read <frame> as a start element. And will
        exit once a </frame> end element has been reached.
    */
    Frame *xmlToFrame(QXmlStreamReader &xml, Animation *anim) {
        // Read the data
        QString name = xml.attributes().value("name").toString();
        QList<CelRef *> celRefs;
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "staged_cel") {
                    // Add the Cel to the Frame
                    CelRef *ref = xmlToCelRef(xml, anim);

                    if (ref)
                        celRefs.append(ref);
                    else
                        qDebug() << "Error, detected NULL CelRef when reading in Frame from XML";
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Exit loop on </frame> reached
                if (tag == "frame")
                    break;
            }
        }

        // Build and return the Frame (by adding from smallest z to largest), I love C++11 lambdas <3!
        Frame *frame = new Frame(anim, name);
        qSort(celRefs.begin(), celRefs.end(),
            [](auto a, auto b) {
                return (a->zValue() <= b->zValue());
            }
        );
        for (auto iter = celRefs.begin(); iter != celRefs.end(); iter++)
            frame->addCel(*iter);

        return frame;
    }


    /*!
        Takes in a XML stream that has just read a <cels> tag.  Will peel out
        all of the Frames and put them into the FrameLibrary for the Animation.
    */
    void xmlToFrames(QXmlStreamReader &xml, Animation *anim) {
        // Read the data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "frame") {
                    // Add the Frame to the Animation
                    Frame *frame = xmlToFrame(xml, anim);

                    if (frame)
                        anim->fl()->addFrame(frame);
                    else
                        qDebug() << "Error, detected NULL Frame when reading in Frame from XML";
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Exit loop on </frames> reached
                if (tag == "frames")
                    break;
            }
        }
    }


    /*!
        Takes in an XML stream that has just read a <timed_frame> token.  Will
        return a pointer to a TimedFrame if the Frame is found, if not, then will
        return a NULL pointer.
    */
    TimedFrame *xmlToTimedFrame(QXmlStreamReader &xml, Animation *anim) {
        // Read the data
        QString frameName = xml.attributes().value("frame").toString();
        int num = xml.attributes().value("number").toInt();
        int hold = xml.attributes().value("hold").toInt();

        // Get the Frame by name
        Frame *frame = anim->fl()->getFrame(frameName);
        if (frame) {
            TimedFrame *tf = new TimedFrame(frame, num, hold);
            return tf;
        } else
            return NULL;
    }


    /*!
        Once a <plane> tag has been read in by an XML stream, this will turn
        that into a lit of TimedFrame pointers for a XSheet plane.  It's
        possible that this coult return an empty list
    */
    QList<TimedFrame *> xmlToPlane(QXmlStreamReader &xml, Animation *anim) {
        // Variables
        QList<TimedFrame *> timedFrames;

        // Read the data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "timed_frame") {
                    TimedFrame *tf = xmlToTimedFrame(xml, anim);

                    if (tf)
                        timedFrames.append(tf);
                    else
                        qDebug() << "Error, detected NULL TimedFrame when reading in Frame from XML";
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Exit loop on </plane> reached
                if (tag == "plane")
                    break;
            }
        }

        return timedFrames;
    }


    /*!
        Will take in an XML stream and output an XSheet object.  If any bit of the XML parsing is
        bad (or any other child Blit objects are bad), then an XSheet that is not similar to the
        XML will be returned.
        
        It is assumed that <xsheet> has already been read as a starting element.  And will exit
        once the </xsheet> end element has been read.
    */
    XSheet *xmlToXSheet(QXmlStreamReader &xml, Animation *anim) {
        // Variables
        XSheet *xsheet = anim->xsheet();
        int fps = xml.attributes().value("fps").toInt();

        // Read the data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                // Strip elements
                if (tag == "plane") {
                    QList<TimedFrame *> timedFrames = xmlToPlane(xml, anim);
                    for (auto iter = timedFrames.begin(); iter != timedFrames.end(); iter++)
                        xsheet->addFrame(*iter);
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Break out if </xsheet> reached
                if (tag == "xsheet")
                    break;
            }
        }

        // Set some data then return
        xsheet->setFPS(fps);
        return xsheet;
    }


    /*!
        Takes stuff away from an XML Stream and spits out an Animation object.  If any bit of the
        XML was invalid (or child Blit objects, e.g. XSheet, Frame, Cel), a NULL pointer will be
        returned.
        
        When calling this function, it has been assumed that <animation> has jut been read as a 
        start element.  And when exiting, </animation> will have been read as an end element.
    */
    Animation *xmlToAnimation(QXmlStreamReader &xml, QString resourceDir) {
        // Variables
        QString name;
        QDateTime created, updated;
        QSize frameSize;

        // Return a Null animation if the version number isn't good
        if (xml.attributes().value("version") != QString::number(curFileFormatVersion))
            return NULL;
        
        // Create the Animation
        Animation *anim = new Animation();
        anim->setResourceDir(resourceDir);

        // Read data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                // Parse data
                if (tag == "name")
                    name = xml.readElementText();
                else if (tag == "created")
                    created.setTime_t(xml.readElementText().toInt());
                else if (tag == "updated")
                    updated.setTime_t(xml.readElementText().toInt());
                else if (tag == "width")
                    frameSize.setWidth(xml.readElementText().toInt());
                else if (tag == "height")
                    frameSize.setHeight(xml.readElementText().toInt());
                else if (tag == "cels")
                    xmlToCels(xml, anim);
                else if (tag == "frames")
                    xmlToFrames(xml, anim);
                else if (tag == "xsheet")
                    xmlToXSheet(xml, anim);
            } else if (token == QXmlStreamReader::EndElement) {
                // Break out if </animation> reached
                if (tag == "animation")
                    break;
            }

        }

        // And hope for the best!
        anim->setName(name);
        anim->setFrameSize(frameSize);
        anim->setCreated(created);
        anim->setUpdated(updated);
        return anim;
    }
    


    /*== XML Color stuff ==*/

    /*!
        Takes in a list of QColor objects (all are assumed to be valid RGBA), and will put them in
        nice XML format.
    */
    void colorsToXML(QXmlStreamWriter &xml, QList<QColor> &colors)  {
        QListIterator<QColor> iter(colors);
        char buff[9];
        while (iter.hasNext()) {
            QColor clr(iter.next());
            sprintf(buff, "%02X%02X%02X%02X", clr.alpha(), clr.red(), clr.green(), clr.blue());
    
            // <color value="XXXXXXXX" />
            xml.writeStartElement("color");
            xml.writeAttribute("value", buff);
            xml.writeEndElement();
        }
    }

    
    /*!
        Takes in an XML stream and ouputs a list of QColor objects.  For this funciton to work, the
        <palette> tag must have just been read in as a start tag.  It will check the version number.
        if anything goes bad, this funciton will return an empty list of QColors.
    */
    QList<QColor> xmlToColors(QXmlStreamReader &xml) {
        // Vars
        QList<QColor> colors;

        // Check
        if (xml.attributes().value("version") != QString::number(curFileFormatVersion))
            return colors;

        // Read the data
        QString tag;
        QXmlStreamReader::TokenType token;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {

                // Get data from the tags
                if (tag == "color") {
                    // Peel the data apart
                    QColor clr;
                    QString hex = xml.attributes().value("value").toString();
                    QString a, r, g, b;
                    a = hex.mid(0, 2);
                    r = hex.mid(2, 2);
                    g = hex.mid(4, 2);
                    b = hex.mid(6, 2);

                    clr.setAlpha(a.toInt(NULL, 16));
                    clr.setRed(r.toInt(NULL, 16));
                    clr.setGreen(g.toInt(NULL, 16));
                    clr.setBlue(b.toInt(NULL, 16));

                    // Last sanity check
                    if (clr.isValid())
                        colors.append(clr);
                    else
                        qDebug() << "Error, wasn't able to get color=" << clr;
                }
            } else if (token == QXmlStreamReader::EndElement) {
                // Break out of loop if </palette> tag reached
                if (tag == "palette")
                    break;
            }
        }

        // Return the colors
        return colors;
    }
    




    /*== Everything Else ==*/

    /*!
        Saves an Animation object to a desitionation directory (path in thise case).  It will look at
        the Anim. object, put its data into a JSON format, and thens save it in the directory.  If you
        want to perform a "saveAs" function, make sure that the target path/directoy doesn't have a
        sequence.xml file inside of it.
    */
    bool saveAnimation(Animation *anim, QString path) {
        QDir dir(path);
        QFileInfo pathInfo(path);

        // First make sure the folder exits, and the permissions are okay
        if (!pathInfo.exists())
            return false;
        else if (!pathInfo.isDir())
            return false;
        else if (!(pathInfo.isReadable() | pathInfo.isWritable() | pathInfo.isExecutable()))
            return false;

        // Save the Seq file.
        QFile seqFile(path + "/sequence.xml");
        bool saveCels = !seqFile.exists();

        if (!seqFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;        // Wasn't able to open the file for writing

        // Save all of the Cels from the old directory to the new one
        if (saveCels) {
            // Doing this is kind of ineffcient, but works
            QListIterator<QPointer<TimedFrame>> frames(anim->xsheet()->frames());

            // go through each frame
            while (frames.hasNext()) {
                Frame *frame = frames.next()->frame();
                QListIterator<CelRef *> refs(frame->cels());

                // Go through each Cel
                while (refs.hasNext()) {
                    CelRef *ref = refs.next();
                    ref->cel()->image().save(path + "/" + ref->cel()->name() + ".png");
                }
            }


        }
        
        // Create the XML stream and write it all out
        QXmlStreamWriter xml(&seqFile);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(-1);

        xml.writeStartDocument();
        animationToXML(xml, anim);
        xml.writeEndDocument();
         
        seqFile.close();

        return true;
    }


    /*!
        Takes in a folder (for path) and will try to read in it's XML sequence file.  It will then put
        all of the information into their associated dats structures (Animation, XSheet, Frame, Cel).
        If there is any failure at all, a NULL pointer will be returned On success, you get the Animation
        object you so long desire.
    */
    Animation *loadAnimation(QString path) {
        // Preiliminary checks
        // If it passes these smell tets, then it is safe to assume that this is a valid Blit Animation
        QDir dir(path);
        QFileInfo pathInfo(path);

        // First make sure the folder exits, and the permissions are okay
        if (!pathInfo.exists())
            return NULL;
        else if (!pathInfo.isDir())
            return NULL;
        else if (!(pathInfo.isReadable() | pathInfo.isWritable() | pathInfo.isExecutable()))
            return NULL;

        // Loop through all of the files looing for the .xml file
        // TODO should have a better method of getting the sequence file...
        QFileInfoList list = dir.entryInfoList();
        QString seqFilename = "";
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fi = list[i];

            if (fi.fileName() == "sequence.xml") {    
                seqFilename = fi.filePath();
                break;
            }
        }

        // REturn false if we haven't found it
        if (seqFilename.isEmpty())
            return NULL;

        // Load up the XML data
        QFile xmlFile(seqFilename);
        if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return NULL;

        // Create the Stream and, do preliminary stuff, then read it
        QXmlStreamReader xml(&xmlFile);
        xml.readNext();

        // Read
        Animation *anim = NULL;
        QXmlStreamReader::TokenType token;
        QString tag;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "animation")
                    anim = xmlToAnimation(xml, path);
            }
        }

        // Return something either way
        return anim;
    }


    /*!
        Will save a list of colors (i.e. a Color Palette) to an xml file.  All of the values in colors
        should be unique.  path should be a valid Blit File Format directory.  Will return true on
        success, false otherwise.
    */
    bool savePalette(QList<QColor> &colors, QString path) {
        QDir dir(path);
        QFileInfo pathInfo(path);

        // First make sure the folder exits, and the permissions are okay
        if (!pathInfo.exists())
            return false;
        else if (!pathInfo.isDir())
            return false;
        else if (!(pathInfo.isReadable() | pathInfo.isWritable() | pathInfo.isExecutable()))
            return false;

        // Save the Palette File
        QFile palFile(path + "/palette.xml");
        if (!palFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;        // Wasn't able to open the file for writing
        
        // Create the XML stream and write it all out
        QXmlStreamWriter xml(&palFile);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(-1);

        // Write some XML, and then the colors
        xml.writeStartDocument();

        // <palette>
        xml.writeStartElement("palette");
        xml.writeAttribute("version", QString::number(curFileFormatVersion));

        colorsToXML(xml, colors);

        xml.writeEndElement();
        // </pallete>
    
        xml.writeEndDocument();
         
        palFile.close();

        return true;
    }


    /*!
        Will try to load a list of colors from the "palette.xml" file in valid blit animation file.
        For the variable path, just supply the path to the blit animation file folder.  On success,
        this function will return someting.  On failure, it will return an empty list.
    */
    QList<QColor> loadPalette(QString path) {
        // Preiliminary checks
        // If it passes these smell tests, then it is safe to assume that this is a valid Blit Animation
        QList<QColor> colors;
        QDir dir(path);
        QFileInfo pathInfo(path);

        // First make sure the folder exits, and the permissions are okay
        if (!pathInfo.exists())
            return colors;
        else if (!pathInfo.isDir())
            return colors;
        else if (!(pathInfo.isReadable() | pathInfo.isWritable() | pathInfo.isExecutable()))
            return colors;

        // Loop through all of the files looing for the .xml file
        // TODO should have a better method of getting the palette file...
        QFileInfoList list = dir.entryInfoList();
        QString palFilename = "";
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fi = list[i];

            if (fi.fileName() == "palette.xml") {    
                palFilename = fi.filePath();
                break;
            }
        }

        // REturn false if we haven't found it
        if (palFilename.isEmpty())
            return colors;

        // Load up the XML data
        QFile xmlFile(palFilename);
        if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return colors;

        // Create the Stream and, do preliminary stuff, then read it
        QXmlStreamReader xml(&xmlFile);
        xml.readNext();

        // Read
        QXmlStreamReader::TokenType token;
        QString tag;
        while (!xml.atEnd() && !xml.hasError()) {
            token = xml.readNext();
            tag = xml.name().toString();

            if (token == QXmlStreamReader::StartElement) {
                if (tag == "palette")
                    colors = xmlToColors(xml);
            }
        }

        // Return the list
        return colors;
    }



    /*== Import functions ==*/

    /*!
        Loads up an image file (preferably a PNG) and will output a pointer to a newly allocated
        PNGCel.  If anything fails, this will return a NULL pointer.
    */
    PNGCel *loadStillImage(QString filename, Animation *anim) {
        QImage tmp(filename);
        if (tmp.isNull())
            return NULL;
        else {
            // Image was good, turn it into a Cel object
            QFileInfo fi(filename);
            PNGCel *cel = new PNGCel(anim, fi.baseName(), tmp.size());
            cel->setImage(tmp);
            return cel;
        }
    }


    /*== Export functions ==*/

    /*!
        Looks at a frame object, and will save it as an image to dest.  Please make sure to supply a
        valid image extension (like .png).  Will return true on success, false otherwise.  Will always
        overwrite the existing image (if there is one).  Please note that this function did more in
        the python version of Blit, but it seems that stuff may be a bit redundant.
    */
    bool saveStillFrame(Frame *frame, QString dest) {
        return frame->render().save(dest);
    } 


    /*!
        Takes in a QImage that represents a spritesheet and will save it to dest.  Please supply a valid
        image extension for dest (like .png).  Will return true on success, false otherwise.  Will 
        overwrite existing data.
    */
    bool saveSpritesheet(QImage spritesheet, QString dest) {
        return spritesheet.save(dest);
    }
};
