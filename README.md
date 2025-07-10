
# Qt-FT8

-------------------------------------------------------------------

![1.1](/pictures/front-picture.png?raw=true)

--------------------------------------------------------------------------
About Qt-FT8
-------------------------------------------------------------------------

Qt-FT8 is a separate program dedicated to decoding FT8 messages in the
various radio amateur bands.

For Limux (x64) an AppImage is available, for Windows an installer.

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

FT8 transmissions consists of short encoded messages, transmitted on
defined regions within different amateur bands.

The program consists of a main widget - one with the controls, 
a device control widget and a widget for preset frequencies.

The program uses Qt (version 6) and qwt (6.2) for the GUI, and
is written in C++.

To ease use, precompiled versions (a Windows installer and an AppImage
for x64 based Linux boxes) are available.

For generating an executable, see the relevant section of this README

![1.1](/pictures/picture-1.png?raw=true)

![1.2](/pictures/picture-2.png?raw=true)

The main widget consists on three parts, at the top the controls,
in the middle a spectrum display, and at the bottom a widget showing
received messages.

Received messages can be saved into a file, and can be transmitted
to the so-called pskReporter.

On program startup, a widget appears that allows selection of one
of the configured devices.
Notice that since most of the transmissions are in the frequecy range until
30 Mhz, RTLSDR devices (i.e. DABsticks) are not configured

Once a device is selected, the software reads in samples from the
device, and continuously tries to detect and decode FT8 messages.

The results are shown on the bottom part of the main widget.

Supported devices and selecting a device
====================================================================

![1.3](/pictures/devices.png?raw=true)

Qt-ft8 supports

 * SDRplay devices, both with lib 2.13 as 3.XX support;
 * hackrf devices;
 * DABstick, but V3 DABsticks will only work from 24 MHz upwards
 * an spyServer client, configured for 8 bits RTLSDR byte encoding

Note that for both Windows and Linux, one shoudl install the support
libraries from SDRplay.com

While the Windows installer provides support for the DABsticks and
the HACKrf devices, the AppImage does not. One needs to install
driver software first.

Tuning and presets
====================================================================

![1.4](/pictures/frequency-control.png?raw=true)

The top line of the main widget has four selectors:

 * the most left one, labeled **select freq** when touched shows a
small widget with a keypad.

![1.5](/pictures/keypad.png?raw=true)

The keypas can be used to specify a frequency. Touching tje "Khz" button
passes the selected freq - as KHz values - on to the system, touching the
"Mhz" button as MHz values.a

The frequency can be modified by clicking on the spectrum display,
or - for small modifications - with the mousewheel. The frequency
step per click of the mousewheel is determined by the spinbox on the
top line, default value is 5 Hz.

Since the frequencies om which FT8 messages are transmitted are predefined,
it is useful to have some **presets**.

 * touching the button labeled **frequency save** adds the currently selected frequency to a list of presets.

![1.5](/pictures/presets.png?raw=true)

The list of presets can be made visible (or hidden) by touching the button
labeled **presets**

On initial program startup the list of presets is filled with a
limited number of known frequencies. As said above, frequencies can be
added to the list.

Selecting a frequency from the preset list is simply by clicking on it.

**Removing** a frequency from the preset list is also possible,
**double clicking** on the element in the list.

The list itself will be maintained between program invocations as
an ".xml" file in the user's homedirectory.

The **spectrum display** shows the 96000 Hz wide frequency range, centered sround the selected frequency.
Clicking with the **right** mouse button the display switching between
a regular spectrum view and a waterfall view.

Setting parameters for the search process
===================================================================

Two parameters can be set (or altered) for searching FT8 messages
in the incoming sample stream.
The third line on the main widgets shows some selectors for that

 * the spinbox left, current value 20, specifies the number of
iterations done when the program feels that there is data;

 * the spinbox next, current value 4000, specifies the search width, i.e. the region with the selected frequency in the center, where the software is searching for potential FT8 mesagges.

In both cases it (obviously) holds that higher values requires more processing effort.

  * the selector labeled "All items" let you choose between searching
for - as the name suggests - all items, or just **CQ** messages.

Saving messages
====================================================================

The button labeled "saving" on the third row of the main widget allows
selecting an output file for saving the textual results/


![1.6](/pictures/results.png?raw=true)

Format is such that with e.g. LibreOffice Calc a decent overview is obtained.

Using the pskReporter
====================================================================

Messages can be send to the so-called **pskReporter**.
Before sending it is required to have an identify filled in.
Touching the button labeled **id** (fourth row of the main widget) shows
a small widget where one can enter its callsign etc.


![1.7](/pictures/identity.png?raw=true)

Assuming an idenitity is known to the system, touching the button
labeled **Recorder** prepares the system to send (unique) messages
to the psk reporter and it will start up the pskReporter website
as shown on top of this README.

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


