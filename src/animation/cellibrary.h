// File:         cellibrary.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the CelLibrary class.


#ifndef CEL_LIBRARY_H
#define CEL_LIBRARY_H


#include <QObject>
#include <QHash>
class Cel;
class Animation;


class CelLibrary : public QObject {
    Q_OBJECT;

public:
    // Constructor and deconstructor
    CelLibrary(Animation *anim);
    ~CelLibrary();


    // Cel operators
    bool addCel(Cel *cel);
    bool removeCel(Cel *cel);
    int numCels();
    bool nameTaken(QString name);
    Cel *getCel(QString name);
    QList<Cel *> cels();


private slots:
    void _onCelNameChanged(QString name);
    void _onCelDestroyed(QObject *obj);


private:
    friend Animation;
    bool _clear();                                                    // Should only be called by Animation

    // Member vars
    QHash<QString, Cel *> _cels;        // Hash of all the Cels in use.


};


#endif // CEL_LIBRARY_H

