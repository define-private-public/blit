// File:         template.h
// Author:       Ben Summerton (define-private-public)
// Description:  Header file for the Template class.


#ifndef TEMPLATE_H
#define TEMPLATE_H


#include <QObject>


class Template : public QObject {
    Q_OBJECT;

public:
    // Constrcutor & Deconstructor
    Template(QObject *parent=NULL);
    ~Template();


signals:


public slots:


private slots:


private:


};


#endif // TEMPLATE_H

