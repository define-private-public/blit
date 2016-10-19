#!/usr/bin/env python3
#
# Small script to convert the XML for blit files from version 1 to version 2.
# Requires libxml extensions for python to be installed.
#
# Useage:
#    python3 <OLD_BLIT_DIR> <NEW_BLIT_DIR> [-o]
#
#    <OLD_BLIT_DIR> -- old Blit file directory (v1); required
#    <NEW_BLIT_DIR> -- new Blit file directory (v2); required
#    -o -- If <NEW_BLIT_DIR> exists, then adding this flag will overwrite the contents; optional
#   

import sys, os, io
import shutil
from lxml import etree


class V1_Cel:
    __slots__ = ('uuid', 'x', 'y', 'w', 'h')

    def __init__(self, uuid=None, x=None, y=None, w=None, h=None):
        self.uuid = uuid
        self.x = x
        self.y = y
        self.w = w
        self.h = h

    def to_V2_Cel(self):
        return V2_Cel(self.uuid, self.w, self.h)

    def to_V2_CelRef(self):
        return V2_CelRef(self.uuid, self.x, self.y, 0)



class V2_Cel:
    __slots__ = ('t', 'name', 'w', 'h')

    def __init__(self, name=None, w=None, h=None):
        self.t = 'PNG'  # Default PNG Type
        self.name = name
        self.w = w
        self.h = h

    def get_xml(self):
        xml = etree.Element('cel')
        xml.set('type', self.t)
        xml.set('name', self.name)
        xml.set('width', str(self.w))
        xml.set('height', str(self.h))
#        print(etree.tostring(xml, pretty_print=True))
        return xml


class V2_CelRef:
    __slots__ = ('cel', 'x', 'y', 'z_order')

    def __init__(self, cel=None, x=None, y=None, z_order=None):
        self.cel = cel
        self.x = x
        self.y = y
        self.z_order = z_order

    def get_xml(self):
        xml = etree.Element('staged_cel')
        xml.set('cel', self.cel)
        xml.set('x', str(self.x))
        xml.set('y', str(self.y))
        xml.set('z_order', str(self.z_order))
#        print(etree.tostring(xml, pretty_print=True))
        return xml



class V1_Frame:
    __slots__ = ('name', 'num', 'hold', 'cels')

    def __init__(self, name=None, num=None, hold=None):
        self.name = name
        self.num = num
        self.hold = hold
        self.cels = []
    
    def num_cels(self):
        return len(self.cels)

    def get_all_cels(self):
        return self.cels

    def to_V2_Frame(self):
        # Make the Frame
        v2f = V2_Frame(self.name)

        # Add in the Cel Refs
        v2f.staged_cels = []
        for i in range(0, len(self.cels)):
            v2cr = self.cels[i].to_V2_CelRef()
            v2cr.z_order = i + 1
            v2f.staged_cels.append(v2cr)

        return v2f

    def to_V2_FrameRef(self):
        # Make the FrameRef
        return V2_FrameRef(self.num, self.hold, self.name)


class V2_Frame:
    __slots__ = ('name', 'staged_cels')

    def __init__(self, name=None):
        self.name = name
        self.staged_cels = []

    def get_xml(self):
        xml = etree.Element('frame')
        xml.set('name', self.name)

        # Attach the Cel Refs
        for cr in self.staged_cels:
            xml.append(cr.get_xml())

#        print(etree.tostring(xml, pretty_print=True))
        return xml


class V2_FrameRef:
    __slots__ = ('num', 'hold', 'frame')

    def __init__(self, num=None, hold=None, frame=None):
        self.num = num
        self.hold = hold
        self.frame = frame

    def get_xml(self):
        xml = etree.Element('timed_frame')
        xml.set('frame', self.frame)
        xml.set('number', str(self.num))
        xml.set('hold', str(self.hold))

#        print(etree.tostring(xml, pretty_print=True))
        return xml
    

class V1_XSheet:
    __slots__ = ('fps', 'seq_length', 'frames')

    def __init__(self, fps=None, seq_length=None):
        self.fps = fps
        self.seq_length = seq_length
        self.frames = []

    def num_frames(self):
        return len(self.frames)

    def get_all_frames(self):
        return self.frames

    def get_all_cels(self):
        cels = []
        for f in self.frames:
            for c in f.get_all_cels():
                cels.append(c)
        return cels

    def to_V2_XSheet(self):
        # Make the XSheet
        v2xs = V2_XSheet(self.fps, self.seq_length)

        # Go through the frames and add their refs
        for frame in self.frames:
            v2xs.planes[0].append(frame.to_V2_FrameRef())

        return v2xs


class V2_XSheet:
    __slots__ = ('fps', 'seq_length', 'planes')

    def __init__(self, fps=None, seq_length=None):
        self.fps = fps
        self.seq_length = seq_length
        self.planes = []
        self.planes.append([])        # There should be only one plane

    def get_xml(self):
        xml = etree.Element('xsheet')
        xml.set('fps', str(self.fps))
        xml.set('seq_length', str(self.seq_length))

        # Add in the plane (ther should be only 1)
        i = 1
        for plane in self.planes:
            plane_xml = etree.Element('plane')
            plane_xml.set('number', str(i))
            plane_xml.set('count', str(len(plane)))

            # Do the timed_frames
            for fr in plane:
                plane_xml.append(fr.get_xml())
            
            xml.append(plane_xml)

            # Increment
            i+= 1

#        print(etree.tostring(xml, pretty_print=True))
        return xml


class V1_Animation:
    __slots__ = ('name', 'created', 'updated', 'fw', 'fh', 'xsheet')

    def __init__(self, name=None, created=None, updated=None, fw=None, fh=None):
        self.name = name
        self.created = created
        self.updated = updated
        self.fw = fw
        self.fh = fh
        self.xsheet = None

    def to_V2_Animation(self):
        v2a = V2_Animation(self.name, self.created, self.updated, self.fw, self.fh)

        # Get the Cels and frames
        v1cels = self.xsheet.get_all_cels()
        v1frames = self.xsheet.get_all_frames()
        for c in v1cels:
            v2a.cels.append(c.to_V2_Cel())
        for f in v1frames:
            v2a.frames.append(f.to_V2_Frame())

        # Set the xsheet
        v2a.xsheet = self.xsheet.to_V2_XSheet()
        return v2a


class V2_Animation:
    __slots__ = ('name', 'created', 'updated', 'fw', 'fh', 'cels', 'frames', 'xsheet')

    def __init__(self, name=None, created=None, updated=None, fw=None, fh=None):
        self.name = name
        self.created = created
        self.updated = updated
        self.fw = fw
        self.fh = fh
        self.cels = []
        self.frames = []
        self.xsheet = None

    def get_xml(self):
        # Create the XML structure
        xml = etree.Element('animation')
        xml.set('version', str(2))          # Should be version 2

        # Set the child tags
        name = etree.SubElement(xml, 'name')
        name.text = self.name
        created = etree.SubElement(xml, 'created')
        created.set('format', 'unix_timestamp')
        created.text = str(self.created)
        updated = etree.SubElement(xml, 'updated')
        updated.text = str(self.updated)
        updated.set('format', 'unix_timestamp')
        width = etree.SubElement(xml, 'width')
        width.text = str(self.fw)
        height = etree.SubElement(xml, 'height')
        height.text = str(self.fh)

        # Add the Cels
        cels_xml = etree.Element('cels')
        cels_xml.set('count', str(len(self.cels)))
        for c in self.cels:
            cels_xml.append(c.get_xml())
        xml.append(cels_xml)

        # Add the Frames
        frames_xml = etree.Element('frames')
        frames_xml.set('count', str(len(self.frames)))
        for f in self.frames:
            frames_xml.append(f.get_xml())
        xml.append(frames_xml)

        # Add the XSheet
        xml.append(self.xsheet.get_xml())

#        print(etree.tostring(xml, pretty_print=True))
        return xml


def build_tree(node, anim):
    # Recursivly build the Animation from the xml
    
    # Check base based upon tag name
    if node.tag == 'animation':
        # Is an animation
        anim.name = node.find('name').text
        anim.created = int(node.find('created').text)
        anim.updated = int(node.find('updated').text)
        anim.fw = int(node.find('width').text)
        anim.fh = int(node.find('height').text)

        # Recurse to build XSheet
        xsheet = node.find('xsheet')
        build_tree(xsheet, anim)
    elif node.tag == 'xsheet':
        # Is an XSheet
        xsheet = V1_XSheet()
        xsheet.fps = int(node.find('fps').text)
        xsheet.seq_length = int(node.find('seq_length').text)

        # Hookup the Xsheet
        anim.xsheet = xsheet
        
        # Recurse to build Frames
        for frame in node.findall('frame'):
            build_tree(frame, anim)
    elif node.tag == 'frame':
        # Is a Frame
        frame = V1_Frame()
        frame.name = node.find('name').text
        frame.num = int(node.get('number'))
        frame.hold = int(node.find('hold').text)

        # Add in the frame
        anim.xsheet.frames.append(frame)

        # Recurse to build the Cels
        for cel in node.findall('cel'):
            build_tree(cel, anim)
    elif node.tag == 'cel':
        # Is a Cel
        cel = V1_Cel()
        cel.uuid = node.get('uuid')
        cel.x = int(node.find('x').text)
        cel.y = int(node.find('y').text)
        cel.w = int(node.find('width').text)
        cel.h = int(node.find('height').text)

        # Add the Cel to the latest frame
        anim.xsheet.frames[-1].cels.append(cel)

        # This is the base case



def print_usage_message():
    print('Useage:')
    print('    $ python3 v1_to_v2.py <OLD_BLIT_DIR> <NEW_BLIT_DIR> [-o]')
    print('')
    print('<OLD_BLIT_DIR> -- old Blit file directory (v1); required')
    print('<NEW_BLIT_DIR> -- new Blit file directory (v2); required')
    print('-o -- If <NEW_BLIT_DIR> exists, then adding this flag will overwrite the contents; optional')


def main():
    # Main routine of the application, check for arguements, prints usage info
    # and sets up the coversion parameters.
    print('Blit v1 to v2 file format converter')
    print('(Please sit back, this could take a bit...)')

    # Check for arguments
    argc = len(sys.argv)
    if argc < 3:
        # Exit and print error message
        print_usage_message();
        sys.exit(0)

    # Pull out the arugments
    overwrite = False
    if '-o' in sys.argv:
        overwrite = True
        sys.argv.remove('-o')
        argc -= 1

    # Second chceck
    if argc < 3:
        print_usage_message();
        sys.exit(0);

    # Pull aruguments
    old_blit_dir = os.path.join(sys.argv[1].strip(), '')
    new_blit_dir = os.path.join(sys.argv[2].strip(), '')
    old_and_new_same = old_blit_dir == new_blit_dir
    old_exists = os.path.exists(old_blit_dir)
    new_exists = os.path.exists(new_blit_dir)

#    # Get parents
#    old_blit_dir_par = old_blit_dir.split(os.sep)
#    new_blit_dir_par = new_blit_dir.split(os.sep)
#    del old_blit_dir_par[-1]
#    del new_blit_dir_par[-1]
#    old_blit_dir_par = os.path.dirname(os.path.join(*old_blit_dir_par))
#    new_blit_dir_par = os.path.dirname(os.path.join(*new_blit_dir_par))
#    old_blit_dir_par = os.path.abspath(old_blit_dir_par)
#    new_blit_dir_par = os.path.abspath(new_blit_dir_par)

    # Check for blit dir
    old_dir_is_dir = os.path.isdir(old_blit_dir)
    if not old_dir_is_dir:
        # If the old directory is not a directory, just exit
        print('Error: %s is not a direcotry.'%old_blit_dir)
        sys.exit(1)
    
    # check if old blit dir contains the sequence.xml and paltette.xml files
    seq_exists = os.path.exists(os.path.join(old_blit_dir, 'sequence.xml'))
    pal_exists = os.path.exists(os.path.join(old_blit_dir, 'palette.xml'))

    # Error info
    if not seq_exists:
        print('Error: sequence.xml not found in %s'%old_blit_dir)
    if not pal_exists:
        print('Error: palette.xml not found in %s'%old_blit_dir)

    # Quit if error info
    if (not seq_exists) or (not pal_exists):
        sys.exit(1)

    # Check for overwrite
    if not overwrite and new_exists:
        print('Error, cannot overwrite %s; please provide -o flag to do so'%new_blit_dir)
        sys.exit(1)

    # TODO figure out this permission stuff later, it's cauing something but problems right now.
#    # Check for writing perms
#    if overwrite:
#        # Check that we can write to the existing one
#        if not os.access(new_blit_dir, os.W_OK):
#            print('Error, cannont write to %s; please double check permissions'%new_blit_dir)
#            sys.exit(1)
#    else:
#        if not os.access(new_blit_dir_par, os.W_OK):
#            print('Error, cannont write to %s; please double check permissions'%new_blit_dir_par)
#            sys.exit(1)
    
    # Read in the xml files
    old_seq_xml = ''
    old_pal_xml = ''
    try:
        old_seq_xml_file = open(os.path.join(old_blit_dir, 'sequence.xml'), 'r')
        old_pal_xml_file = open(os.path.join(old_blit_dir, 'palette.xml'), 'r')
        old_seq_xml = old_seq_xml_file.read().encode('utf-8')
        old_pal_xml = old_pal_xml_file.read().encode('utf-8')
        old_seq_xml_file.close()
        old_pal_xml_file.close()
    except Exception as e:
        print('Error: trouble reading XML files.')
        print(e)
        sys.exit(1)

    # Get XML
    old_seq_root = None
    old_pal_root = None
    try:
        parser = etree.XMLParser(ns_clean=True, recover=True, encoding='utf-8')
        old_seq_root = etree.fromstring(old_seq_xml, parser=parser)
        old_pal_root = etree.fromstring(old_pal_xml, parser=parser)
    except Exception as e:
        print('Error: wasn\'t able to parse XML.')
        print(e)
        sys.exit(1)

    # Check Animation versions, look for v1
    old_seq_v = int(old_seq_root.get('version'))
    old_pal_v = int(old_pal_root.get('version'))
    if (old_seq_v != 1) or (old_pal_v != 1):
       print('Error: Blit file in %s was not detected to be version 1'%old_blit_dir)
       sys.exit(1)

    # Create some blit structures
    try:
        # The sequence.xml
        anim = V1_Animation()
        build_tree(old_seq_root, anim)
        anim = anim.to_V2_Animation()

        # The palette.xml
        old_pal_root.set('version', str(2))
    except Exception as e:
        print('Error, wasn\'t able to read old format.  XML is possibly malformed.')
        sys.exit(1)

    # Check for directories
    if not new_exists:
        # Make the directory
        try:
            os.mkdir(new_blit_dir)
        except Exception as e:
            print('Error, wasn\'t able to make %s'%new_blit_dir)
            print(e)
            sys.exit(1)
    elif new_exists and not overwrite:
        print('Error, must supply overwrite flag (-o) to write to an existing directory')
        sys.exit(1)

    # Save the XML
    new_seq_xml = etree.tostring(anim.get_xml(), encoding=str, pretty_print=True).replace('  ', '\t')
    new_pal_xml = etree.tostring(old_pal_root,  encoding=str, pretty_print=True).replace('  ', '\t')
    try:
        new_seq_file = open(os.path.join(new_blit_dir, 'sequence.xml'), 'w')
        new_seq_file.write('<?xml version="1.0" encoding="utf-8"?>\n')
        new_seq_file.write(new_seq_xml)
        new_seq_file.close()

        new_pal_file = open(os.path.join(new_blit_dir, 'palette.xml'), 'w')
        new_pal_file.write('<?xml version="1.0" encoding="utf-8"?>\n')
        new_pal_file.write(new_pal_xml)
        new_pal_file.close()
    except Exception as e:
        print('Error, wasn\'t able to save XML files to %s'%new_blit_dir)
        print(e)
        sys.exit(1)

    # Copy over the image files
    if not old_and_new_same:
        try:
            for c in anim.cels:
                png_filename = '%s.png'%c.name
                old_path = os.path.join(old_blit_dir, png_filename)
                new_path = os.path.join(new_blit_dir, png_filename)
                shutil.copy(old_path, new_path)
        except Exception as e:
            print('Error, wasn\'t able to copy of PNGs for Cels')
            print(e)
            sys.exit(1)
    
    print('Conversion Complete')



# Run this python script if it's being directly called
if __name__ == '__main__':
    main()
