#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-ft8 decoder
 *
 *    qt-ft8 decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    qt-ft8 decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-ft8 decoder, if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QSettings>
#include	<QHBoxLayout>
#include	<QLabel>
#include	"hackrf-handler.h"

#define	DEFAULT_GAIN	30
static float convTable [256];

	hackrfHandler::hackrfHandler	(RadioInterface *mr,
                                         QSettings      *s,
                                         RingBuffer<std::complex<float>> *r):
                                                      myFrame (nullptr) {
int	err;
int	res;
int	i;

	this    -> outputRate		= 96000;
	this    -> inputRate		= 24 * outputRate;
	this	-> hackrfSettings	= s;
	setupUi (&myFrame);
	myFrame. show ();
	_I_Buffer			= r;

	for (i = 0; i < 128; i ++)
	   convTable [i] = (float)(i) / 128.0;
	for (i = 0; i < 128; i ++)
	   convTable [128 + i] =  -(float)(128 - i) / 128.0;

#ifdef  __MINGW32__
        const char *libraryString = "libhackrf.dll";
        Handle          = LoadLibrary ((wchar_t *)L"libhackrf.dll");
#else
        const char *libraryString = "libhackrf.so";
        Handle          = dlopen (libraryString, RTLD_NOW);
#endif

	if (Handle == nullptr) {
	   fprintf (stderr, "failed to open %s\n", libraryString);
	   throw (20);
	}

        libraryLoaded   = true;
        if (!load_hackrfFunctions ()) {
#ifdef __MINGW32__
           FreeLibrary (Handle);
#else
           dlclose (Handle);
#endif
           throw (21);
        }
//
	lastFrequency	= Khz (220000);
//
//	See if there are settings from previous incarnations
	hackrfSettings		-> beginGroup ("hackrfSettings");
	lnagainSlider 		-> setValue (
	            hackrfSettings -> value ("hack_lnaGain", DEFAULT_GAIN). toInt ());
	vgagainSlider 		-> setValue (
	            hackrfSettings -> value ("hack_vgaGain", DEFAULT_GAIN). toInt ());

	hackrfSettings	-> endGroup ();

//
	res	= this -> hackrf_init ();
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_init:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   throw (21);
	}

	res	= this	-> hackrf_open (&theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_open:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   throw (22);
	}

	res	= this -> hackrf_set_sample_rate (theDevice, inputRate);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_samplerate:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   throw (23);
	}

	res	= this -> hackrf_set_baseband_filter_bandwidth (theDevice,
	                                                        1536000);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_bw:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   throw (24);
	}

	res	= this -> hackrf_set_freq (theDevice, 220000000);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: ");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   throw (25);
	}

	setLNAGain	(lnagainSlider	-> value ());
	setVGAGain	(vgagainSlider	-> value ());
//	and be prepared for future changes in the settings
	connect (lnagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setLNAGain (int)));
	connect (vgagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setVGAGain (int)));

	hackrf_device_list_t *deviceList = this -> hackrf_device_list ();
	if (deviceList != nullptr) {
	   char *serial = deviceList -> serial_numbers [0];
	   serial_number_display -> setText (serial);
	   enum hackrf_usb_board_id board_id =
	                 deviceList -> usb_board_ids [0];
	   usb_board_id_display ->
	                setText (this -> hackrf_usb_board_id_name (board_id));
	}

	filter_1	= new decimatingFIR (2 * 4 + 1,
                                                     + outputRate / 2,
                                                     inputRate, 4);
        filter_2	= new decimatingFIR (2 * 8 + 1,
                                                      outputRate / 2,
                                                      inputRate / 4, 6);
	running. store (false);
}

	hackrfHandler::~hackrfHandler	() {
	stopReader ();
	hackrfSettings	-> beginGroup ("hackrfSettings");
	hackrfSettings	-> setValue ("hack_lnaGain",
	                                 lnagainSlider -> value ());
	hackrfSettings -> setValue ("hack_vgaGain",
	                                 vgagainSlider	-> value ());
	hackrfSettings	-> endGroup ();
	this	-> hackrf_close (theDevice);
	this	-> hackrf_exit ();
	delete	filter_1;
	delete	filter_2;
}
//

void	hackrfHandler::setVFOFrequency	(int32_t newFrequency) {
int	res;
	if (newFrequency < Mhz (1))
	   return;
	res	= this -> hackrf_set_freq (theDevice, newFrequency);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: \n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
	lastFrequency = newFrequency;
}

int32_t	hackrfHandler::getVFOFrequency	() {
	return lastFrequency;
}

void	hackrfHandler::setLNAGain	(int newGain) {
int	res;
	if ((newGain <= 40) && (newGain >= 0)) {
	   res	= this -> hackrf_set_lna_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_lna_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   lnagainDisplay	-> display (newGain);
	}
}

void	hackrfHandler::setVGAGain	(int newGain) {
int	res;
	if ((newGain <= 62) && (newGain >= 0)) {
	   res	= this -> hackrf_set_vga_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_vga_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   vgagainDisplay	-> display (newGain);
	}
}
//
static
int	callback (hackrf_transfer *transfer) {
hackrfHandler *ctx = static_cast <hackrfHandler *>(transfer -> rx_ctx);
uint8_t *p	= transfer -> buffer;
int	i;
RingBuffer<std::complex<float> > * q = ctx -> _I_Buffer;
std::complex<float> localBuf [transfer -> valid_length / 2];
int	cnt	= 0;

	for (i = 0; i < transfer -> valid_length / 2; i ++) {
	   int8_t re = (int8_t)(p [2 * i]);
	   int8_t im = (int8_t)(p [2 * i + 1]);
	   std::complex<float> temp  =
	              std::complex<float> (((float)re) / 128.0,
	                                   ((float)im) / 128.0);
	   std::complex<float> tmp2;

           if (ctx -> filter_1 -> Pass (temp, &tmp2))
	      if (ctx -> filter_2 -> Pass (tmp2,  &(localBuf [cnt])))
              if (localBuf [cnt] == localBuf [cnt])
                 cnt ++;
        }

        ctx -> _I_Buffer -> putDataIntoBuffer (localBuf, cnt);
        ctx -> sampleCnt += cnt;

	if (ctx -> sampleCnt > ctx -> outputRate / 8) {
	   ctx -> report_dataAvailable ();
	   ctx -> sampleCnt = 0;
	}
	return 0;
}

bool	hackrfHandler::restartReader	() {
int	res;

	if (running. load ())
	   return true;

	res	= this -> hackrf_start_rx (theDevice, callback, this);	
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_start_rx :\n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return false;
	}
	running. store (this -> hackrf_is_streaming (theDevice));
	sampleCnt	= 0;
	fprintf (stderr, "hackrf is %s running\n",
	                        running. load () ? " " : "not");
	return running. load ();
}

void	hackrfHandler::stopReader	() {
int	res;

	if (!running. load ())
	   return;

	res	= this -> hackrf_stop_rx (theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_stop_rx :\n", res);
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
	running. store (false);
}

void	hackrfHandler::resetBuffer	() {
      _I_Buffer       -> FlushRingBuffer ();
}

int16_t	hackrfHandler::bitDepth		() {
	return 8;
}

bool	hackrfHandler::load_hackrfFunctions () {
//
//	link the required procedures
	this -> hackrf_init	= (pfn_hackrf_init)
	                       GETPROCADDRESS (Handle, "hackrf_init");
	if (this -> hackrf_init == nullptr) {
	   fprintf (stderr, "Could not find hackrf_init\n");
	   return false;
	}

	this -> hackrf_open	= (pfn_hackrf_open)
	                       GETPROCADDRESS (Handle, "hackrf_open");
	if (this -> hackrf_open == nullptr) {
	   fprintf (stderr, "Could not find hackrf_open\n");
	   return false;
	}

	this -> hackrf_close	= (pfn_hackrf_close)
	                       GETPROCADDRESS (Handle, "hackrf_close");
	if (this -> hackrf_close == nullptr) {
	   fprintf (stderr, "Could not find hackrf_close\n");
	   return false;
	}

	this -> hackrf_exit	= (pfn_hackrf_exit)
	                       GETPROCADDRESS (Handle, "hackrf_exit");
	if (this -> hackrf_exit == nullptr) {
	   fprintf (stderr, "Could not find hackrf_exit\n");
	   return false;
	}

	this -> hackrf_start_rx	= (pfn_hackrf_start_rx)
	                       GETPROCADDRESS (Handle, "hackrf_start_rx");
	if (this -> hackrf_start_rx == nullptr) {
	   fprintf (stderr, "Could not find hackrf_start_rx\n");
	   return false;
	}

	this -> hackrf_stop_rx	= (pfn_hackrf_stop_rx)
	                       GETPROCADDRESS (Handle, "hackrf_stop_rx");
	if (this -> hackrf_stop_rx == nullptr) {
	   fprintf (stderr, "Could not find hackrf_stop_rx\n");
	   return false;
	}

	this -> hackrf_device_list	= (pfn_hackrf_device_list)
	                       GETPROCADDRESS (Handle, "hackrf_device_list");
	if (this -> hackrf_device_list == nullptr) {
	   fprintf (stderr, "Could not find hackrf_device_list\n");
	   return false;
	}

	this -> hackrf_set_baseband_filter_bandwidth	=
	                      (pfn_hackrf_set_baseband_filter_bandwidth)
	                      GETPROCADDRESS (Handle,
	                         "hackrf_set_baseband_filter_bandwidth");
	if (this -> hackrf_set_baseband_filter_bandwidth == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_baseband_filter_bandwidth\n");
	   return false;
	}

	this -> hackrf_set_lna_gain	= (pfn_hackrf_set_lna_gain)
	                       GETPROCADDRESS (Handle, "hackrf_set_lna_gain");
	if (this -> hackrf_set_lna_gain == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_lna_gain\n");
	   return false;
	}

	this -> hackrf_set_vga_gain	= (pfn_hackrf_set_vga_gain)
	                       GETPROCADDRESS (Handle, "hackrf_set_vga_gain");
	if (this -> hackrf_set_vga_gain == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_vga_gain\n");
	   return false;
	}

	this -> hackrf_set_freq	= (pfn_hackrf_set_freq)
	                       GETPROCADDRESS (Handle, "hackrf_set_freq");
	if (this -> hackrf_set_freq == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_freq\n");
	   return false;
	}

	this -> hackrf_set_sample_rate	= (pfn_hackrf_set_sample_rate)
	                       GETPROCADDRESS (Handle, "hackrf_set_sample_rate");
	if (this -> hackrf_set_sample_rate == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_sample_rate\n");
	   return false;
	}

	this -> hackrf_is_streaming	= (pfn_hackrf_is_streaming)
	                       GETPROCADDRESS (Handle, "hackrf_is_streaming");
	if (this -> hackrf_is_streaming == nullptr) {
	   fprintf (stderr, "Could not find hackrf_is_streaming\n");
	   return false;
	}

	this -> hackrf_error_name	= (pfn_hackrf_error_name)
	                       GETPROCADDRESS (Handle, "hackrf_error_name");
	if (this -> hackrf_error_name == nullptr) {
	   fprintf (stderr, "Could not find hackrf_error_name\n");
	   return false;
	}

	this -> hackrf_usb_board_id_name = (pfn_hackrf_usb_board_id_name)
	                       GETPROCADDRESS (Handle, "hackrf_usb_board_id_name");
	if (this -> hackrf_usb_board_id_name == nullptr) {
	   fprintf (stderr, "Could not find hackrf_usb_board_id_name\n");
	   return false;
	}

	fprintf (stderr, "OK, functions seem to be loaded\n");
	return true;
}

void	hackrfHandler::report_dataAvailable () {
        emit dataAvailable (10);
}

