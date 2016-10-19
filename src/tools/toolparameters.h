// File:         toolparameters.h
// Author:       Ben Summerton (define-private-public)
// Description:  ToolParamters is a common singleton QHash that is used to share common paramter
//               values across different tools.


#ifndef TOOL_PARAMETERS_H
#define TOOL_PARAMETERS_H


#include <QHash>
class QString;
class QVariant;


namespace Tools {
    // hash, check toolsparameters.cpp for which ones are already in there.
    extern QHash<QString, QVariant> commonParams;

    void setupHashDefaults();
};



#endif // TOOL_PARAMETERS_H

