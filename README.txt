DiskImagery64
-------------

A drag & drop Disk Image Editor for Commodore 64 D64 Images

  Written by Christian Vogelgsang  <chris@vogelgsang.org>
          under the GNU Public License V2

  Official Page is: http://www.lallafa.de/blog (DiskImagery64 Page)

Introduction
------------

If you are a C64-addict then you usually like to setup own disk images with
new software or with new arrangements of existing collections. I often use the
great "c1541" command line tool of VICE (www.viceteam.org) to create and
modify disk images on my Mac. Working this way gets cumbersome if you arrange
new images from a large collection of other images or if you want to
hand-craft an image with many files from different sources. For this task I
wrote DiskImagery64...

DiskImagery64 is a GUI application that allows to quickly view and edit a
large set of disk images and allows to copy or move files by simple dragging
them from one image to the other. Furthermore, a file browser allows access to
your local file system and from there you can also drag files to an image and
vice versa.

Installation
------------

DiskImagery64 is written with the portable Qt-library (www.trolltech.com) and
is open source under the GNU Public License V2. You can either download the
source code or a compiled binary. The source compiles on all Qt-platforms:
Mac, Linux/Unix with X11, and Windows. Simply call "qmake" and "make" in the
source tree to build it. Make sure to use at least Version 4.2.x of Qt.

For the low-level disk image I/O DiskImagery64 uses the "diskimage" library
written by Per Olofsson. For your convenience the source code is included in
this source distribution, but you can also find the source with documentation
on Per's official "diskimage"-Page:

  http://www.paradroid.net/diskimage/

  diskimage - D64/D71/D81 library
    Copyright (c) 2003-2006, Per Olofsson
    All rights reserved.

Fonts
-----

For authentic reproduction of file names in disk images, DiskImagery64 uses
two fonts: CBM and CBMShift for the unshifted and shifted commodore char set.
In this distribution you find both of them as scalable TrueType fonts. Install
them on your system before running DiskImagery64.

Mac users simply double-click on the CBM.dfont and CBMShift.dfont files and
select "Install" in the Font Manager to install them on their systems. Windows
and Linux users can use the provided CBM.ttf and CBMShift.ttf files.

The fonts were not created totally by myself. I had a scalable commodore
TrueType font lying around on on my hard disk and I used it as a starting
point. The existing font told me its origins in the header: based on a font by
Devin D. Cook with fixes from Chris McBride.

I loaded this font in the great free font editor FontForge
(http://fontforge.sf.net/) and started fixing it again: I wanted a one-to-one
petscii mapping and that was not present. Furthermore, this requires to have
two fonts: one for unshifted and one for the shifted commodore char set. I
created the two new fonts CBM and CBMShift with a "Unicode" TrueType mapping
but with each petscii character at the correct hex position. The characters
were copied from the other TrueType font and missing characters were drawn by
myself. 

I hope the mapping of all petscii characters is correct now. For reference I
have provided the editable FontForge font (*.sfd) files in the "fonts"
directory. If you find any errors then please send me your fixes!

The fonts are NOT suitable to use them as a replacement for any other unicode
font as the embedded mapping is called unicode but petscii actually.
Nevertheless, they are perfectly suited to directly print petscii strings.

Usage
-----

1. Startup

There are several ways to launch DiskImagery64:
 
 - Double-click on the application icon
 - Double-click on a *.d64 image file (on Macs)
 - Drag a *.d64 image file onto the application icon

If DiskImagery64 is launched with a disk image then an Image Browser window
opens and shows the directory of the image. If no image is given then the
File Browser window opens.

2. Image Browser

An Image Browser windows shows the contents of a disk images. You can open as
many image browsers as you like.

You can create a new disk image with the "File/New Image" menu command. A new
and empty image browser is opened. The new disk image is empty and has a
default disk name and id. 

"File/Open Image" opens a new image browser with an already existing image.

You can change the disk name and id by formatting the image with the
"Tools/Format Disk" command. Enter a new disk name and id in the dialog.
This will also erase all files on the disk.

A disk image is altered by copying files from and to the directory shown in
the corresponding image browser. Simply select one or more files in one disk
image and drag-and-drop them to another image. It is also possible to drag
files from the File Browser to the image. Then a local file is copied onto
the image.

Files on a disk image can be altered with the common "Cut", "Copy", "Paste"
and "Delete" commands found in the "Edit" menu. First select one or more of
them and issue a command.

If a disk image is modified the changes are at first only performed in memory.
You need to save your image to disk to make the changes permanently. Either
select "File/Save" to save the image with the already given name or use
"File/Save as..." to save a copy or a currently unsaved image.

"File/Close" closes the current image browser. The image is not automatically
saved onto disk in this case. The applications warns you if you want to close
an unsaved image. If the last browser in DiskImagery64 is closed then the
application quits.

3. File Browser

The File Browser shows you a directory of your machine's file system. The
browser is used as a drag source or a drop target if you want to move local
files to or from a disk image.

You can browse your file system by opening and closing the tree hierarchy
shown in the browser. Furthermore, the root of the shown file tree can be
entered in the top line edit field. Simply enter a valid path there and press
enter. Additionally, a click on the directory icon lets you select the root
directory in a file dialog.

Select a single or multiple files in the file browser and drag them onto a
disk image to copy them there. The file names are automatically converted from
Unicode to Petscii. Additionally, known extensions like ".prg", ".del", ...
are automatically stripped for the Petscii name and converted into the
corresponding CBM file type.

The transfer of files from a disk image to the local file system works
similar: Simply select one or more files in the image browser and drag them
onto a directory in the file browser. Again, the file names are converted
automatically and the CBM file type is added as a file extension. Invalid
characters (":","/","\") are automatically stripped from the name.

4. Tools

The Tools Menu offers some tools while working with a disk image. "Format
Image" allows to completely format the virtual disk. Enter the new name and
disk id.

"Add Separator" is used to add a separator special file to the current disk
image. A separator file is usually empty and only used in the directory
listing to separate entries or to group application files. The Add Separator
command has already some predefined separator styles available, but you can
design your own separator (see Preferences).

5. Emulator

DiskImagery64 allows to call your favorite C64 Emulator to mount an opened
image. Use the preferences to setup your emulator. The "Mount Image" command
mounts/attaches the disk image to a virtual drive in your emulator and
launches the program.

"Run Program" allows to run the selected disk entry inside your emulator. Make
sure to have a single program selected. The emulator is run and the disk image
name and file name on the disk image is passed as arguments.

6. Networking

DiskImagery64 can directly work with a real Commodore 64 if its connected via
ethernet. For the C64 a popular network adapter is the RR-Net, a 10 MBit NIC
mounted on the Retro-Replay cartidge (available from Individual Computers
http://ami.ga) which is an Action Replay clone.

To set up your network, I suggest to use a cross-cable to directly connect the
C=64 to your Mac. This avoids traffic from other sources that can disturb the
good old 8-bitter. Furthermore, The Final Replay ROM (short: TFR) image is
required to use CodeNet or NetDrive features described below. The ROM is
suitable for the Retro-Replay and available at
http://www.oxyron.de/html/freplay.html. Setup the correct IP addresses for the
C64 and your Mac in the ROM with the deliverd tool and flash the image on your
cartidge. Finally store both IP addresses in DiskImagery64's preferences and
you are ready to go!

The following network services are available in DiskImagery64:

* CodeNet: Press F6 on the C64 with TFR running to enter CodeNet mode. In this
  mode the C64 waits for instructions from the network. You can fill memory,
  download data directly to C64 memory, jump to memory or run a program. This
  is all implemented in DiskImagery64 but currently only used to download a
  PRG and run it.

  In DiskImager64 simply select a program file in a disk image or a local file
  and select "Network/Run Program". The file is downloaded in a second and run
  on the real machine. This works only for one-filers as loading other files
  is not supported in this mode.

* NetDrive: TFR allows to access a "virtual" IEC network drive on device id 6.
  So a "LOAD "$",6" on your C64 will load the directory from the network
  drive. In DiskImagery64 you can create a network drive from every disk image
  or selection of files (also local ones) by selecting "Network/Share Files in
  NetDrive".

  The NetDrive allows to use multi-file-programs as a program can load data
  files from the virtual device later on. The program must only use the kernal
  load routines (no fastloader, custom load routines...) as the NetDrive works
  on kernal level and is bypassed by custom routines. Kernal loading is often
  required nowadays (e.g. on IDE64, Dreamload on MMC64,...), so many
  multi-file progs are already available in patched versions.

* WarpCopy64 support (http://www.oxyron.de/html/wc64.html). WC64 is a server
  program running on the C64 and a client on a host (PC/Mac). Now you can
  control your C64-attached real disk drives (e.g. 1541) directly via network
  from your Mac. You can format a disk, verify a disk, send direct DOS
  commands to the drive and of course copy disk images in both directions. You
  can then directly transfer a real disk into a disk image in DiskImagery64 or
  a disk image back onto a real disk. A slow (most IEC compatible) mode taking
  several minutes per side and a warp mode only taking tens of seconds is
  available.

  First of all enter CodeNet by pressing F6 in TFR. "Network/WarpCopy64/Start
  WarpCopy" now transfers the WC64 server program to your C64 and launches it.
  Make sure to have the correct IP adresses selected in preferences! You can
  pick "Network/WarpCopy64/Read Disk" to transfer a real disk into a disk
  image of DiskImagery64. If you have a disk image opened then the
  "Network/WarpCopy64/Write Disk" command is available and transfers the image
  directly onto a real disk.

Reference Manual
----------------

1. Menus

   File/New Image

          		Create a new disk image in a new Image Browser Window

   File/New Browser

				Create a new File Browser Window

   File/Open Image

				Open an existing disk image in a new Image Browser Window

   File/Save Image

				Save the current image to disk if it was changed.
				Class "Save Image as..." if disk image is currently unnamed.

   File/Save Image as...
 				Save the current disk image with a new name to disk

   File/Close
				Close the current Image or File Browser window.
				If the last window is closed then the application quits.
   
   Edit/Cut
				Cut the selected files from the current disk image and
				store them in the Clipboard
   Edit/Copy

				Copy the selected files from the current disk image to the
				Clipboard
   Edit/Paste

				Paste the files in the clipboard into the current disk image
   Edit/Delete

				Remove the selected files from the current disk image.

   View/Shift Charset

				Toggle the CBM Font between shifted and unshifted chars.
				Changes only the current Image Browser.

   View/Fixed Font Size

				If enbled the CBM Font is fixed to 8 Points (i.e. the Original
				Font Size).

   View/Show Charset

			    A new tool window shows all printable PETSCII characters in
                the current charset. Select and copy chars from there to the
                input fields if you need special characters.

   Tools/Format Disk

				Format the current disk image. Give the disk title and the id.
				
   Tools/Add Separator

				Add a separator to the current disk image. Edit an own
                separator or picked one of the predefined ones.

   Emulator/Run Program

				Saves the current image and runs your favorite C64 emulator
                and loads the currently selected entry from your Image
                Browser.

   Emulator/Mount Image

				Saves the current image and runs your favorite C64 emulator 
				with the current image attached.

   Network/Run Program

				Requires TFR and CodeNet started! Transfer the selected file
                in a disk image or a local file directly to a real C64 and
                launch it there.

   Network/Share Files in NetDrive

				Requires TFR! The selected files in a disk image or on the
                local file system are grouped together into a virtual disk
                drive for the C64. Use device id 6 on the C64 to access the
                files.

   Network/Show Log Window

				All Network commands are logged in the Log Window. Have a look
                there to troubleshoot your network problems. The Abort button
                allows to stop all running network services.

   Network/WarpCopy64/Start WarpCopy

				Requires TFR and CodeNet started! The WAPRCOPY06.PRG server
                program is transferred to your C64 and launched. Make sure to
                setup the location of the binary in the preferences of
                DiskImagery64. The binary is in the official Windows server
                distribution available here:
                http://www.oxyron.de/html/wc64.html

   Network/WarpCopy64/Read Disk (Warp/Slow)

				Requires WC64 server running! Transfer the real disk residing
                in drive 8 of your C64 to a disk image in DiskImagery64.
                Tested currently only with a 1541. On read errors the
                defective blocks are read again. Error handling is done, but
                may be improved ;) Use the slow mode if the warp mode does not
                work for you.

   Network/WarpCopy64/Write Disk (Warp/Slow)

				Requires WC64 server running! Transfer the current disk image
                directly to a disk residing in drive 8 of your connected C64.
                Use the slow mode if the warp mode does not work for you.

   Network/WarpCopy64/Format Disk

				Requires WC64 server running! Enter name (and id) of the disk
                to be formatted in the attached drive. If only a name is given
                a quick format is performed otherwise a full format.

   Network/WarpCopy64/Verify Disk

				Requires WC64 server running! Issue 'V' CBM DOS Command to
                verify the inserted disk.

   Network/WarpCopy64/Send DOS Command

				The entered string is transferred to the attached drive as a
                CBM Dos command.

   Network/WarpCopy64/Get Drive Status

				Return the drive status of the attached disk drive and print
                it in the network log window.

2. Preferences

   Image Section:

		File/Title/Id:
		
				Define the naming of newly created images and their default
				file name. You can either set a static name here or use the
				auto naming scheme of DiskImagery64. By adding a special tag
				%c or %C to your name, title or id the tag is replaced by the
				current image counter before creating a new image. The syntax
				%<pad>c allows to use 0-padding of the counter, e.g. %3c pads
				all numbers to three digits.
				
		Counter:
		
				The value of the current disk image counter. After creating a
				new disk image the counter is incremented automatically. Use
				this entry here to manually reset or preset the counter.

   Font Section:

		Use shifted CBM Font:
		Use fixed 8 Pixel Font Size:

		        The values are the presets for the corresponding entries in
                the View menu. Every newly created or opened image will have
                these settings.

   Separator Section:

		Plus Button:

		 		A new seperator template entry is added to the list of
                available templates. Double-click on the new entry to alter
                the default appearance.
				
		Minus Button:

				Remove the selected separator template from the list of
                available templates.

   Emulator Section:

		Application:

				Enter (or copy & paste) the full path to your favorite
                Emulator. This binary is launched for the "Mount Image" and
                "Run Program" commands.

		Mount Image Arguments:

				A space separated list of command line arguments. The argument
                list is appended to the application name to execute the "Mount
                Image" command. The placeholder "%i" is replaced by the full
                path of the current disk image.
				
		Run Program Arguments:

				A space separated list of command line arguments. The argument
                list is appended to the application name to execute the "Run
                Program" command. The placeholder "%i" is replaced by the full
                path of the current disk image. The placeholder "%p" is
                replaced by the unicode string of the currently selected file
                in the disk image. Use "%P" if you need the raw PETSCII name
                of the program.

   Network Section:

		My Address:

				Enter the IP address of your Mac. Make sure to use the same
                address in your system's network adapter where the C64 is
                connected. Also the TFR ROM needs this address.

		C64 Address:

				Enter the IP address of you C64. Same as defined in TFR ROM.

		WarpCopy PRG File:

		 		The location of the WarpCopy06.PRG server program on your
                local file system. The binary is in the official Windows
                server distribution here: http://www.oxyron.de/html/wc64.html

		Patch WarpCopy Ips:

				This is enabled by default and adjusts the C64 address
                hard-coded inside the WarpCopy server program to match the
                values given here.

Feedback
--------

The program is still very young and certainly contains bugs. If you find them
then I'd appreciate your feedback.

If you have other suggestions, ideas or general feedback then feel free to
drop me a mail, too!

Enjoy!
-Chris