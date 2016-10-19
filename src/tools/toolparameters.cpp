// File:         toolparameters.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  ToolParameters source implementation.


/*!
    \inmodule Tools
    \brief A module that lets Tools have common paramter data.

    This module has a member Tools::commonParams that is a QHash<QString, QVariant> that is used to
    store common options/paramters for tools.  E.g. pen/brush/eraser size, turn on/off antilasiang,
    etc.  If you are adding paramters and you're not sure if the Key is taken, just use the
    QHash::contains to check first.  Be also careful when overwriting a value.
*/


#include "tools/toolparameters.h"
#include <QVariant>


namespace Tools {
    QHash<QString, QVariant> commonParams;

    /*!
        Sets up some common paramters for Tools::commonParams.  Here is a list of what string
        cooresponds to which datatype.  These are the default values.  The user should be able to 
        add their own.

        \code
        "pen-size" -> double           [1.0]
        "anti-aliasing" -> bool        [false]
        \endcode
    */
    void setupHashDefaults() {
//        QHash<QString, QVariant> commonParams();
        commonParams["pen-size"] = QVariant(1.0);
        commonParams["anti-aliasing"] = QVariant(false);
    }
}

