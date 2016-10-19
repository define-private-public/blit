// File:         cel.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the Cel object

// TODO
//  - Add slot for QObject::destroyed (?)
//  - Add Copy Constructor (make sure name is unique)
//    - Possibly something for the subclasses.


#ifndef CEL_H
#define CEL_H

#define CEL_BASE_TYPE 0
#define CEL_MIN_SIZE QSize(1, 1)
#define CEL_UUID_POSTFIX_SIZE 8


#include <QObject>
#include <QPointer>
#include <QSize>
#include <QSet>
class Animation;
class CelRef;
class QStringList;
class QImage;
class QPainter;


class Cel : public QObject {
    Q_OBJECT;

public:
    enum { Type = CEL_BASE_TYPE };

    // Constructors/Deconstructors
    explicit Cel(Animation *anim, QString name="", QSize size=CEL_MIN_SIZE);
    virtual Cel *copy(QString name="");
    virtual ~Cel();

    // Important info
    virtual bool isEmpty();
    virtual bool hasFileResources();
    virtual QStringList fileResources();
    virtual void save(QString basename="");
    virtual int type();
    QString name();
    QString oldName();
    virtual bool setName(QString name);

    // Image
    virtual QImage image();

    // For file resources
    virtual void remove(bool deleteFiles=true);
    virtual bool toBeRemoved();

    // Dimensions
    QSize size();
    int width();
    int height();
    virtual void resize(int width, int height);
    virtual void resize(QSize size);

    // Misc info
    bool usingRandomName();
    bool usingUUIDPostfix();

    // Painting info for the QGraphicsScene
    virtual void paint(QPainter *painter);

    // Cel Referecnes
    void registerRef(CelRef *ref);
    void unregisterRef(CelRef *ref);
    QList<QPointer<CelRef>> celRefs();


public slots:
    // State
    bool active();
    void activate();
    void deactivate();


signals:
    void activated();
    void deactivated();
    void nameChanged(QString name);
    void resized(QSize size);


protected:
    // Member vars
    QPointer<Animation> _anim;                        // Animathion that this XSheet is a part of
    QString _name = "";                    // Name of the Cel, must be unique vs. all other instances of Cel
    QString _oldName = "";                // Previous name of the Cel, is used in conjunction with the nameChanged() signal
    bool _usingRandomName = false;        // If a name was randomly generated for the Cel, then this flag is set, the postfix is it's given name
    bool _usingUUIDPostfix = false;        // When assigning a name, if the name is taken, then it will automatically assing a UUID postfix
    bool _active = false;                // Flag to see if the Cel is currently marked as active or not
    QSize _size;                        // Dimensions of the Cel
    QList<QPointer<CelRef>> _celRefs;    // Cel Refs that are pointing to this Cel

};


#endif // CEL_H

