
# Qt-FT8

-------------------------------------------------------------------

![1.1](/pictures/front-picture.png?raw=true)

--------------------------------------------------------------------------
About Qt-FT8
-------------------------------------------------------------------------

Qt-FT8 is a separate program dedicated to decoding FT8 messages transmitted in
 the various radio amateur bands.

For Linux (x64) an AppImage is available, for Windows an installer.

Table of Contents
=================================================================

* [Introduction](#introduction)
* [Supported devices and selecting a device](#supported-devices-and-selecting-a-device)
* [Tuning and presets](#tuning-and-presets)
* [Setting parameters for the search process](#setting-parameters-for-the-search-process)
* [Saving messages](#saving-messages)
* [Using the pskReporter](#using-the-psk-reporter)
* [Generating an executable](#generating-an-executable)
* [Copyrights](#copyrights)

Introduction
====================================================================

FT8 transmissions consist of short encoded messages, transmitted on
defined regions within different amateur bands.

This FT8 decoding program - when running - shows a main widget -
one with the controls, a device control widget and a widget for preset frequencies.

The program is written in C++ and uses Qt (version 6) and qwt (6.2) for the GUI.

To ease use, precompiled versions (a Windows installer and an AppImage
for x64 based Linux boxes) are available.

For generating an executable, see the relevant section of this README

![1.1](/pictures/picture-1.png?raw=true)

![1.2](/pictures/picture-2.png?raw=true)

The main widget consists on three parts, at the top section the controls,
in the middle a spectrum display, and at the bottom a widget showing
received messages.

Received messages can be saved into a file, and sending them
to the so-called pskReporter is possible.

Notice that since most of the transmissions are in the frequecy range until
30 Mhz, RTLSDR devices (i.e. DABsticks) are not capable of handling these
frequencies, nevertheless by default DABsticks are configured in.

Once a device is selected, the software reads in samples from the
device, and continuously tries to detect and decode FT8 messages.
The results are shown on the bottom part of the main widget.

Supported devices and selecting a device
====================================================================

Qt-ft8 supports

 * SDRplay devices, both with lib 2.13 as 3.XX support;
 * hackrf devices;
 * DABstick, but V3 DABsticks will only work from 24 MHz upwards;
 * an spyServer client, configured for 8 bits RTLSDR byte encoding.

On program startup, a small widget is shown for device selection
as shown below.

![1.3](/pictures/devices.png?raw=true)

Selecting a device is )obviously) by clicking on the name
of the device in the list.
Note, however, that for the SDRplay devices (both Windows and Linux), one should install the support libraries from SDRplay.com

While the Windows installer provides support for the DABsticks and
the HACKrf devices, the AppImage does not. One needs to install
driver software first.

A recent addition is the client for the spyserver. Since my antenna equipment
is limited, I - from time to time - connect to a - relatively nearby - 
spyServer, one that supports the AIRspyHF.

The client asks for a datastream of 96000 8bit IQ samples, that works
well, as the picture on top shows.

Tuning and presets
====================================================================

![1.4](/pictures/frequency-control.png?raw=true)

The top line of the main widget has four selectors:

 * the most left one, labeled **select freq** when touched shows a
small widget with a keypad.

![1.5](/pictures/keypad.png?raw=true)

The keypad (picture above) can be used to specify a frequency. Touching the "Khz" button passes the selected freq - as KHz values - on to the system, touching the "Mhz" button as MHz values.

The selected frequency can be modified by clicking on the spectrum display,
or - for small modifications - with the mousewheel. The frequency
step per click of the mousewheel is determined by the spinbox on the
top line, default value is 5 Hz.

Since the frequencies om which FT8 messages are transmitted are predefined,
it is useful to have some **presets**, Qt-FT8 has therefore a preset list.
This list of presets can be made visible (or hidden) by touching the button
labeled **presets** (picture below).

![1.5](/pictures/presets.png?raw=true)

Selecting a frequency from the preset list is simply by clicking on it.

* touching the button labeled **frequency save** adds the currently selected frequency to the list of presets.
**Removing** a frequency from the preset list is also possible, **double clicking** on the element in the list.

On initial program startup the list of presets is filled with a
limited number of known frequencies. As said above, frequencies can be
added to and removed from  the list. The list will always be ordered.

The list itself will be maintained between program invocations as
an ".xml" file in the user's homedirectory.

The **spectrum display** shows the 96000 Hz wide frequency range, centered around the selected frequency.
On clicking with the **right** mouse button  the display switches between
a regular spectrum view and a waterfall view.

Setting parameters for the search process
===================================================================

Two parameters can be set (or altered) for searching FT8 messages
in the incoming sample stream.
The third line on the main widgets shows some selectors:

 * the spinbox left, current value 20, specifies the number of
iterations done when the program feels that there is data;

 * the spinbox next, current value 4000, specifies the **search width **, i.e. the region with the selected frequency in the center, where the software is searching for potential FT8 mesagges.

For both selections it (obviously) holds that higher values requires more processing effort.

  * the selector labeled "All items" let you choose between searching
for - as the name suggests - all items, or just **CQ** messages.

Saving messages
====================================================================

The button labeled "saving" on the third row of the main widget allows
selecting an output file for saving the received messages in a textual form.

![1.6](/pictures/results.png?raw=true)

Format is such that with e.g. LibreOffice Calc a decent overview is obtained.

Using the pskReporter
====================================================================

Messages can be send to the so-called **pskReporter**.
Before sending it is required to have the  **identity** filled in.
Touching the button labeled **id** (fourth row of the main widget) shows
a small widget where one can enter its callsign etc (an example is given below,
(Of course, the identity info is maintained between program invocations).

![1.7](/pictures/identity.png?raw=true)

Assuming an idenitity is known to the system, touching the button
labeled **Recorder** prepares the system to send (unique) messages
to the psk reporter and it will start up the pskReporter website
as shown on top of this README.

Filling in the identity in the identity field in the top line of the
web page (and touching the "go" button) shows the identifications of
the transmitters of received messages on the map, as shown in the figure on top.

Generating an executable
====================================================================

While for both Linux (x64) and Windows precompiled versions are available
it is of course possible to build an executable yourself.

The directory contains a file "qt-ft8.pro" that can be used to
generate a Makefile. In order to do so, Qt6 and **qmake6** need to be
installed.

The AppImage is built on Ubuntu 20, the required libraries and programs can be installed as given below

 *   sudo apt-get update
 *   sudo apt-get install git
 *   sudo apt-get install cmake
 *   sudo apt-get install build-essential
 *   sudo apt-get install g++
 *   sudo apt-get install pkg-config
 *   sudo apt-get install libfftw3-dev
 *   sudo apt-get install zlib1g-dev 
 *   sudo apt-get install libusb-1.0-0-dev
 *   sudo apt-get install mesa-common-dev
 *   sudo apt-get install qt6-base-dev
 *   sudo apt-get install qt6-multimedia-dev
 *   sudo apt-get install libsamplerate0-dev

While on my development box (Fedora) a Qt6 and qwt-6.2 version are available
from the repositories, Ubuntu (20) does not provide these.

 *   Download qwt-6.30 from "https://sourceforge.net/projects/qwt/files/qwt/6.3.0/";
 *   follow the instructions (i.e. unzip, *cd* to the unzipped folder) and adapt the config file to your likings;
 *   building is then simple (takes some time though): "qmake6 qwt.pro", "make";
 *   install the library ("sudo make install") and inform the loader "sudo ldconfig";
 *   Note that the default for installation is "/usr/local/qwt/6.3.0", adjust the PATH settings accordingly.

Copyrights
====================================================================

	Copyright (C)  2022 .. 2025
	Jan van Katwijk (J.vanKatwijk@gmail.com) 
	Lazy Chair Computing

	Copyright of libraries used - Qt-DAB, qwt, fftw,
	libsamplerate, libusb-1, - is gratefully acknowledged.
	
	Some functions in the core of the decoder, in particular
	the ldpc decoding, is - with minor modifications -
	taken from Karlis Goba.
	All rights of Karlis Goba is gratefully acknowledged.

	Qt-FT8 - the sources and the precompiled versions -
	is distributed under e GPL V2 library,  in the hope that
	it will be useful, but WITHOUT ANY WARRANTY; without even the
	implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for 
	more details.


