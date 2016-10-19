TEMPLATE = app
QT += widgets 
RESOURCES = blit.qrc
TARGET = blit
CONFIG += c++14 plugin warn_off debug


# If you want to compile with clang instead of GCC, uncomment the two lines below
#QMAKE_CC = clang
#QMAKE_CXX = clang++



SOURCES += main.cpp

HEADERS += util.h
SOURCES += util.cpp

HEADERS += blitapp.h
SOURCES += blitapp.cpp



# Animation Module
HEADERS += animation/cel.h
SOURCES += animation/cel.cpp

HEADERS += animation/celref.h
SOURCES += animation/celref.cpp

HEADERS += animation/celrefitem.h
SOURCES += animation/celrefitem.cpp

HEADERS += animation/pngcel.h
SOURCES += animation/pngcel.cpp

HEADERS += animation/cellibrary.h
SOURCES += animation/cellibrary.cpp

HEADERS += animation/frame.h
SOURCES += animation/frame.cpp

HEADERS += animation/frameitem.h
SOURCES += animation/frameitem.cpp

HEADERS += animation/timedframe.h
SOURCES += animation/timedframe.cpp

HEADERS += animation/framelibrary.h
SOURCES += animation/framelibrary.cpp

HEADERS += animation/xsheet.h
SOURCES += animation/xsheet.cpp

HEADERS += animation/animation.h
SOURCES += animation/animation.cpp 

HEADERS += fileops.h
SOURCES += fileops.cpp

HEADERS += spritesheet.h
SOURCES += spritesheet.cpp



# Unclassified widgets
HEADERS += widgets/colorframe.h
SOURCES += widgets/colorframe.cpp

HEADERS += widgets/statusbar.h
SOURCES += widgets/statusbar.cpp

HEADERS += widgets/menubar.h
SOURCES += widgets/menubar.cpp

HEADERS += widgets/timelinewindow.h
SOURCES += widgets/timelinewindow.cpp

HEADERS += widgets/toolbox.h
SOURCES += widgets/toolbox.cpp

HEADERS += widgets/toolswindow.h
SOURCES += widgets/toolswindow.cpp

HEADERS += widgets/editorcontainer.h
SOURCES += widgets/editorcontainer.cpp

HEADERS += widgets/animationproperties.h
SOURCES += widgets/animationproperties.cpp

HEADERS += widgets/colorpalette.h
SOURCES += widgets/colorpalette.cpp

HEADERS += widgets/celswindow.h
SOURCES += widgets/celswindow.cpp

HEADERS += widgets/stagedcelwidget.h
SOURCES += widgets/stagedcelwidget.cpp

HEADERS += widgets/lighttablewindow.h
SOURCES += widgets/lighttablewindow.cpp



# Color chooser widgets
HEADERS += widgets/colorchoosers/colorchooser.h

HEADERS += widgets/colorchoosers/rgbslider.h
SOURCES += widgets/colorchoosers/rgbslider.cpp



# Drawing Widgets
HEADERS += widgets/drawing/canvas.h
SOURCES += widgets/drawing/canvas.cpp

HEADERS += widgets/drawing/backdrop.h
SOURCES += widgets/drawing/backdrop.cpp



# Tools
HEADERS += tools/toolparameters.h
SOURCES += tools/toolparameters.cpp

HEADERS += tools/tool.h
SOURCES += tools/tool.cpp

HEADERS += tools/pentool.h
SOURCES += tools/pentool.cpp

HEADERS += tools/brushtool.h
SOURCES += tools/brushtool.cpp

HEADERS += tools/erasertool.h
SOURCES += tools/erasertool.cpp

HEADERS += tools/linetool.h
SOURCES += tools/linetool.cpp

HEADERS += tools/shapetool.h
SOURCES += tools/shapetool.cpp

HEADERS += tools/filltool.h
SOURCES += tools/filltool.cpp

HEADERS += tools/movetool.h
SOURCES += tools/movetool.cpp

HEADERS += tools/resizetool.h
SOURCES += tools/resizetool.cpp

HEADERS += tools/colorpickertool.h
SOURCES += tools/colorpickertool.cpp



# Timeline Widgets
HEADERS += widgets/timeline/tick.h
SOURCES += widgets/timeline/tick.cpp

HEADERS    += widgets/timeline/ruler.h
SOURCES += widgets/timeline/ruler.cpp

HEADERS += widgets/timeline/trianglemarker.h
SOURCES += widgets/timeline/trianglemarker.cpp

HEADERS += widgets/timeline/bracketmarker.h
SOURCES += widgets/timeline/bracketmarker.cpp

HEADERS += widgets/timeline/cursor.h
SOURCES += widgets/timeline/cursor.cpp

HEADERS += widgets/timeline/timeline.h
SOURCES += widgets/timeline/timeline.cpp

FORMS += \
    ui/animation_properties_dialog.ui \
    ui/cels_window.ui \
    ui/timeline_window.ui \
    ui/rgb_slider.ui \
    ui/spritesheet_dialog.ui \
    ui/tools_window.ui \
    ui/staged_cel_widget.ui \
    ui/light_table_window.ui

