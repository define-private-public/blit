#!/usr/bin/env python3
#
# Takes in a filename for a GIF image, does some magic, then saves everyting as the Blit File Format
# 
# Usage:
#   ./git2blit.py <gif> <output_dir>
#
#          gif -- A file that is really a GIF, it's undefined what will happen if you don't provide a
#                 real GIF.
#   output_dir -- The directoy to output this project to (along with all of the files), it would be best
#                 if this directory didn't exist
#
#   Please keep in mind that the Blit File Format is really just a directory with image and a JSON file
#   in it.  So make sure you grab everything and not just that JSON file.

# TODO list:
#  - Maybe add in an easy way to adjust the FPS, setting it to "50," isn't really the best policy
#    - Maybe make it so that it can sort of detect what would be the best FPS, though that would require
#      some sort of analysis on the amount of frames, each delay, and what would work to get a nice and
#      even integer

# NOTE:
#  This application is deprected
#



import sys, os, time
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../src')))
from animation import Animation
from xsheet import XSheet
from frame import Frame
from cel import Cel
from file_ops import *


class GifConverter:

    def __init__(self, filename, parent=None):
        # Assume is valid
        self.gif = QMovie(filename)

        # Get each frame, and put it on a buffer image
        print(self.gif.nextFrameDelay())
        self.gif.jumpToNextFrame()
        print(self.gif.nextFrameDelay())

    
    def convert(self):
        # Paint each frame onto the sheet
        numFrames = self.gif.frameCount()
        filenameFormat = '%0' + str(len(str(numFrames))) + 'i.png'

        for i in range(self.gif.frameCount()):
            img = self.gif.currentImage().convertToFormat(QImage.Format_ARGB32_Premultiplied)
            filename = filenameFormat%(i + 1)
            img.save(filename)
            
            # Draw and goto the next frame
            self.gif.jumpToNextFrame()
        
            


def main():
    # Make sure we have enough stuff
    if len(sys.argv) < 3:
        print('Usage:')
        print('    ./gif2blit.py <gif> <output_dir>')
        sys.exit(0)

    # Need to make the QApp first
    app = QApplication(sys.argv)

    # Assume that all of the input is valid
    gifName = sys.argv[1]
    outputDirName = sys.argv[2]

    # Open of the animated GIF
    gif = QMovie(gifName)

    # Make the directory if it doesn't exist
    if not os.path.exists(outputDirName):
        os.makedirs(outputDirName)

    # Build the strucutres, get some data
    xsheet = XSheet(fps=50)
    anim = Animation(xsheet=xsheet, name=os.path.basename(gifName), created=time.ctime())
    frameCount = gif.frameCount()
    msDelayPerFrame = 1000 // xsheet.fps()
    timingOff = False
    filename = ''

    # Start the loop, but just look at the first frame so we can get the size
    gif.jumpToNextFrame()
    anim.setSize(gif.currentImage().size())

    # Add all of the frames together
    for i in range(0, frameCount):
        # Make the new frame and add it
        f = Frame()
        c = Cel(gif.currentImage().convertToFormat(QImage.Format_ARGB32_Premultiplied))
        f.setHold(gif.nextFrameDelay() // msDelayPerFrame)
        f.addCel(c)
        xsheet.addFrame(f)

        # Save images as we go along
        try:
            filename = os.path.join(outputDirName, '%s.png'%c.UUID())
            c.image().save(filename)
            print(filename)
        except:
            print('There was an error in trying to save a Cel to:\n  %s'%filename)
            sys.exit(1)

        # Checking for not so good shifts in timing
        if (gif.nextFrameDelay() % msDelayPerFrame) != 0:
            timingOff = True

        # Dump it
        gif.jumpToNextFrame()

    # And save the JSON, then we're done
    try:
        filename = os.path.join(outputDirName, 'sequence.json')
        f = open(filename, 'w')
        f.write(structToJSON(animationToJSONStruct(anim)))
        f.close()
    except:
        print('There was an error in tyring to save the JSON file to:\n  %s'%filename)
        sys.exit(1)

    # Metrics:
    print('Converted a GIF with %i frames.'%xsheet.numFrames())
    if timingOff:
        print('Because of the set FPS and the delay of the frames in the GIF, the timing might be off in the converted animation.')



main()

