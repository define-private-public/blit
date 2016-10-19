Blit - A Bitmap Based Animation Tool
====================================

![The Main Interface](https://gitlab.com/define-private-public/blit/raw/master/web/images/screenshot-main-window.png)

![The Spritsheet Exporter](https://gitlab.com/define-private-public/blit/raw/master/web/images/screenshot-spritesheet-exporter.png)

Project site: https://gitlab.com/define-private-public/blit

Doxygen source docs: http://blit.gitlab.io/SourceDocs/index.html

**Please note that Blit is no longer in active development.**  

The goals of Blit were:
 1. Be cross platform (developed on Linux, made using C++ & Qt)
 2. Be a modern Spriting & Sprite Animation tool
 3. Be useable for traditional 2D Animation
 4. Handle Pencil tests
 5. A simple Stop Motion interface

The source  or this application is being released here in case anyone is
interested in it.  I don't think I'll be working on this anymore, but if
someone wants to, I'm willing to answer any questions you have, redocument
things, or provide any guidance.  There is some out of date documentation in the
source code, but most of it should be marked as such.

All of the known issues (and plans) that I have made, are logged in the issue
tracker on GitLab.

When going through the source code you might notice some bad spellings or not so
great grammar; please ignore it.  If it is causing some confusion though, tell
me and I'll fix it up.

While I feel like this was one of my "higher quality," projects, not everything
in here reflects on my current programming standards.  I haven't touched almost
any the code in more than a year.

I've written a small memoir on my development of Blit.  You can read it on my
personal blog:
https://16bpp.net/blog/post/blit-a-retrospective-on-my-largest-project-ever


Licenses
--------
All of the source code, documentation, and everything else is licensed under the
GNU GPLv3, unless otherwise noted.  The text can be found in the file `./LICENSE`.

This program uses some of Google's Material Design Icons, and those are licensed
under the Apache License v2.0.  See the file `./APACHE_LICENSE` for details.


Required Software
-----------------
Blit runs on Linux, Windows, and OS X.  There are no binaries available, so you
will need to build it yourself.

To Compile:
 - A C++14 compiler, I used [clang](http://clang.llvm.org)
 - Qt 5.x, [Qt Open Source](https://www.qt.io/download-open-source/) is fine

For Documentation:
 - Qt comes with `qdoc`, you can use that
 - [Doxygen](https://www.stack.nl/~dimitri/doxygen/) is recommended though
 - [Graphviz](http://www.graphviz.org/) is optional, but needed for some stuff


Building
--------
If you feel comfortable in the command line:

 1. cd `./src/`
 2. `qmake`
 3. `make`

Else, load `./src/blit.pro` in Qt Creator and then click the "Build," button.


Basic Usage
-----------
One of my friends told me that Blit was kind of hard to use.  I'd have to agree
with him.

### Blit Files
Blit will store all of the necessary files in a single directory.  If that
directory contains a `sequence.xml` and a `palette.xml` Blit will consider it to
be a valid Blit file.  So when opening files, you are supposed to open up a
directory.  Likewise when creating a new Animation, you specify a directory
name.

All edits that you make are automatically saved to the disc.  The exception is
when editing Cels.  Those are saved when you change the current Frame.

There is a sample animation in `./samples/blue_spirit_v2/`

If you are interested in Blit's file format/structure, read
`./doc/file_format_v2.txt`.

### Cels & Drawing
Blit is a Cel based animation tool.  You can add/copy/delete Cels using the
"Cels Window," (on the right side).  The Cel size is determined by the
Animation's set Frame size.

Colors can be chosen from the RGB sliders on the left.  If you want to add a
color to your swatches, press the "Palette +" button.  If you want to remove a
swatch, right click on it.

You have some basic tools available for editing:

 1. Pen 
 2. Eraser
 3. Line
 4. Shape
 5. Fill
 6. Move Cel
 7. Color Picker

Some of the tools have different properties that can be adjusted below the RGB
sliders.

### Frames & Timing
Frames are what contains a collection of Cels.  Each Frame has a sequence number
and hold value.  You can adjust the FPS of the Animation in the lower left hand
corner of the "Timeline Window,"  Like Cels, you can add/copy/delete them too.
If you want to move Frames, you can drag and drop them within the timeline.

Playback buttons are in the lower right hand corner.  There is an option to loop
the Animation and to limit playback to a smaller section.

### Other Things

 - All of the sub windows are "Dock Widgets," which can be pulled off of the
   main window and moved around
 - There is also a "Light Table," window; it can be used to enable Onion
   Skinning.  It is found under the `View` menu
 - You can adjust the color of the Canvas by going to `Canvas -> Set Backdrop
   Color`
 - There is a grid that is shown when you change the zoom to 800% or greater.
   It's visibility can be toggled via `Canvas -> Grid`
 - If you want to adjust the frame size of the Animation.  Go to `Animation ->
   Properties`
 - You can import existing images as Cels from `File -> Import -> Still Image as
   Cel`
 - You can export Animations as a spritesheet from `File -> Export -> As
   Spritehseet`
   - To change the background color on the spritsheet, double click the
     `Background Color` swatch


Documentation
-------------
Most of the source for Blit is documented.  Using Doxygen or `qdoc` you should
be able to generate documentation from the `./doc/` directory.  There is some
additional documentation in the `./web/` folder, which was part of a
[DokuWiki](https://dokuwiki.org/) instance.

