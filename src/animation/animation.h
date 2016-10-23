// File:         animation/animation.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the Animation class.


#ifndef ANIMATION_H
#define ANIMATION_H


// Definitions
#define ANIMATION_DATETIME_STRING_FORMAT "ddd MMM dd HH:mm:ss yyyy t"
#define ANIMATION_FRAME_SIZE_MAX 2160


#include <QObject>
#include <QPointer>
#include <QSize>
#include <QDateTime>
class XSheet;
class CelLibrary;
class FrameLibrary;
class QString;



class Animation : public QObject {
    Q_OBJECT;

public:
    explicit Animation(QString name="", QSize size=QSize(1, 1),
                       QDateTime created=QDateTime::currentDateTime(), QDateTime updated=QDateTime(),
                       QObject *parent=NULL);
    ~Animation();
    bool isEmpty();

    QPointer<XSheet> xsheet();
    QString name();
    void setName(QString name);

    QSize frameSize();
    int frameWidth();
    int frameHeight();
    void setFrameSize(QSize size);
    void setFrameWidth(int width);
    void setFrameHeight(int height);
    
    QDateTime created();
    void setCreated(QDateTime created);
    quint32 createdTimestamp();
    QString createdString();
    QDateTime updated();
    void setUpdated(QDateTime updated);
    quint32 updatedTimestamp();
    QString updatedString();
    void update();

    QPointer<CelLibrary> cl();
    QPointer<FrameLibrary> fl();

    // File operators
    QString resourceDir();
    void setResourceDir(QString path);
    void copyResourcesTo(QString path);


signals:
    void resourceDirChanged(QString path);
    void XSheetChanged(QPointer<XSheet> xsheet);
    void nameChanged(QString str);
    void frameSizeChanged(QSize size);


private slots:
    void _onXSheetDestroyed(QObject *obj=NULL);


private:
    void setXSheet(XSheet *xsheet);

    // Memeber vars
    QString _resourceDir;                // Location of files for Cels
    QPointer<CelLibrary> _cl;        // Collection of all the Cels
    QPointer<FrameLibrary> _fl;        // Collection of all the Frames
    QPointer<XSheet> _xsheet;        // Pointer to XSheet in use
    QString _name = "";                // Name of the Animation, can be set to none
    QSize _size;                    // Dimensions of each individual Frame in the animation
    QDateTime _created;                // Time that the animation was created
    QDateTime _updated;                // Time that the animation was last changed
};


#endif // ANIMATION_H

