// File:         template.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Source file for the Template class.


/*!
    \inmodule MODULE
    \class Template
    \brief BRIEF

    DETAILS
*/


//#include <QDebug>
#include "template.h"


/*!
    TODO Document
*/
Template::Template(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[Template created] " << this;
}


/*!
    TODO Document
*/
Template::~Template() {
    qDebug() << "[Template destroyed] " << this;
}

