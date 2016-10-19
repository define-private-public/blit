// File:         main.cpp
// Author:       Ben Summerton (define-private-public)
// Description:  Main source file for the Blit Application.  It doesn't do that much other than
//               startup BlitApp.  It may be used for some testing purposes (e.g. testing a new
//               module, or pre-loading a file).  Remember to clear this out for when releaseing
//               stuff.


// Qt includes
#include <QApplication>
#include <QStringList>

// Blit includes
#include "blitapp.h"


// Filter List for Debugging mesages, feel free to modify as needed
QStringList filterOutClasses({
    "Cel", "CelRef", "CelRefItem", "PNGCel",
    "Frame", "TimedFrame",
    "StagedCelWidget",
    "Canvas"
});


// Custom qDebug() message handler
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // Check the filter list
    foreach (QString className, filterOutClasses) {
        if (msg.trimmed().startsWith("[" + className + " "))
            return;
    }

    // Convert and print
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;

        case QtInfoMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;

        case QtWarningMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;

        case QtCriticalMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;

        case QtFatalMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            abort();
    }
}


// the main show
int main(int argc, char *argv[]) {
    // Custom message handler
    qInstallMessageHandler(customMessageHandler);

    // Setup the App
    QApplication app(argc, argv);
    BlitApp blit;
    blit.show();

    return app.exec();
}

