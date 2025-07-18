#
/*
 *    Copyright (C) 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-ft8 decoder
 *
 *    qt-ft8 decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-ft8 decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-ft8 decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    A simple client for spyServer
 *	
 *	Inspired by the spyserver client from Mike Weber
 *	and some functions are copied.
 *	8 bit codes and a samplerate of SAMPLERATE (96000) S/s
 *	for Functions copied (more or less) from Mike weber's version
 *	copyrights are gratefully acknowledged
 */

#include	<QtNetwork>
#include	<QSettings>
#include	<QLabel>
#include	<QMessageBox>
#include	<QTcpSocket>
#include	<QDir>
#include	"radio-constants.h"
#include	"spyserver-client.h"

#define	DEFAULT_FREQUENCY	(Khz (137600))

	spyServer_client::spyServer_client (RadioInterface *mr,
	                                    QSettings *s,
	                                    RingBuffer<std::complex<float>>*b):
	                                           myFrame (nullptr),
	                                           hostLineEdit (nullptr),
	                                           tmpBuffer (32 * 32768) {

	spyServer_settings	= s;
	this	-> _I_Buffer	= b;
	setupUi (&myFrame);
	myFrame. show		();
	hostLineEdit. hide ();

	theServer	= nullptr;
  //	setting the defaults and constants
	s -> beginGroup ("SPYSERVER_SETTINGS");
	settings. gain		=
	               s ->  value ("spyServer-gain", 20). toInt ();
	settings. auto_gain     =
	               s ->  value ("spyServer-auto_gain", 0). toInt ();
	settings. basePort	=
	               s ->  value ("spyServer+port", 5555). toInt ();
	s -> endGroup ();
	portNumber      -> setValue     (settings. basePort); 
        if (settings. auto_gain != 0)
           autogain_selector    -> setChecked (true);
	spyServer_gain	-> setValue (settings. gain);
	connected	= false;
	dumping		= false;
	settings. resample_quality	= 2;
	settings. batchSize		= 4096;
	settings. sample_bits		= 8;
//
	connect (spyServer_connect, &QPushButton::clicked,
                 this, &spyServer_client::wantConnect);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 2)
        connect (autogain_selector, &QCheckBox::checkStateChanged,
#else
        connect (autogain_selector, &QCheckBox::stateChanged,
#endif
                 this, &spyServer_client::handle_autogain);
        connect (spyServer_gain, qOverload<int>(&QSpinBox::valueChanged),
                 this, &spyServer_client::setGain);
        connect (portNumber, qOverload<int>(&QSpinBox::valueChanged),
                 this, &spyServer_client::set_portNumber);
	theState	-> setText ("waiting to start");
}

	spyServer_client::~spyServer_client () {
	if (connected) {		// close previous connection
	   fprintf (stderr, "stopReader is called\n");
	   if (theServer == nullptr)
	      return;
	   if (!connected || !running)	// seems double???
	      return;
	   if (!theServer -> is_streaming ())
	      return;
	   theServer	-> stop_running ();
	   running		= false;
	}
	if (theServer != nullptr)
	   delete theServer;
	theServer	= nullptr;
}
//
void	spyServer_client::wantConnect () {
QString ipAddress;
QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

	if (connected)
	   return;
	// use the first non-localhost IPv4 address
	for (uint16_t i = 0; i < ipAddressesList.size (); i ++) {
	   if (ipAddressesList.at (i) != QHostAddress::LocalHost &&
	      ipAddressesList. at (i). toIPv4Address()) {
	      ipAddress = ipAddressesList. at(i). toString();
	      break;
	   }
	}
	// if we did not find one, use IPv4 localhost
	if (ipAddress. isEmpty())
	   ipAddress = QHostAddress (QHostAddress::LocalHost).toString();

	spyServer_settings	-> beginGroup ("SPYSERVER_SETTINGS");
	ipAddress	= spyServer_settings -> value ("remote-server", ipAddress). toString ();
	spyServer_settings	-> endGroup ();
	hostLineEdit. setText (ipAddress);
	fprintf (stderr, "extracted %s\n", ipAddress. toLatin1 (). data ());

	hostLineEdit. setInputMask ("000.000.000.000");
//	Setting default IP address
	hostLineEdit. show ();
	theState	-> setText ("Enter IP address, \nthen press return");
	connect (&hostLineEdit, &QLineEdit::returnPressed,
	         this, &spyServer_client::setConnection);
}

//	if/when a return is pressed in the line edit,
//	a signal appears and we are able to collect the
//	inserted text. The format is the IP-V4 format.
//	Using this text, we try to connect,
void	spyServer_client::setConnection () {
QString s	= hostLineEdit. text();
QString theAddress	= QHostAddress (s). toString ();
	onConnect. store (false);
	spyServer_settings	-> beginGroup ("SPYSERVER_SETTINGS");
	spyServer_settings	-> setValue ("remote-server", theAddress);
	spyServer_settings	-> endGroup ();
	fprintf (stderr, "we storen %s\n", theAddress. toLatin1 (). data ());
	settings. basePort	= portNumber -> value ();
	try {
	   if (theServer != nullptr)
	      delete theServer;
	   theServer	= nullptr;
	   fprintf (stderr, "Trying to connect to %s %d\n",
	                                      theAddress. toLatin1 (). data (),
	                                      settings. basePort);
	   theServer	= new spyHandler (this, theAddress,
	                                    (int)settings. basePort,
	                                    &tmpBuffer);
	} catch (...) {
	   QMessageBox::warning (nullptr, tr ("Warning"),
                                          tr ("Connection failed"));
	   return;
	}
	if (theServer == nullptr) {
	   fprintf (stderr, "Connecting failed\n");
	   return;
	}

	connect (&checkTimer, &QTimer::timeout,
	         this, &spyServer_client::handle_checkTimer);
	timedOut	= false;
	checkTimer. start (2000);
	while (!onConnect. load () && !timedOut) 
	   usleep (1000);
	if (timedOut) {
	   QMessageBox::warning (nullptr, tr ("Warning"),
                                          tr ("no answers, fatal"));
	   delete theServer;
	   theServer	= nullptr;
	   connected = false;
	   return;
	}
	checkTimer. stop ();	
	disconnect (&checkTimer, &QTimer::timeout,
	            this, &spyServer_client::handle_checkTimer);
	theServer	-> connection_set ();

	struct DeviceInfo theDevice;
	theServer	-> get_deviceInfo (theDevice);

	if (theDevice. DeviceType == DEVICE_AIRSPY_HF) {
	   nameOfDevice ->setText("AIRSPY HF");
	}
	else {
	   theState -> setText ("not supported device");
	   return;
	}

	this -> deviceNumber	-> setText (QString::number (theDevice. DeviceSerial));
	uint32_t max_samp_rate	= theDevice. MaximumSampleRate;
	uint32_t decim_stages	= theDevice. DecimationStageCount;
	int desired_decim_stage = -1;
	double resample_ratio	= 1.0;

	if (max_samp_rate == 0) {
	   if (theServer != nullptr)
	      delete theServer;
	   theServer = nullptr;
	   return;
	}

	for (uint16_t i = 0; i < decim_stages; ++i ) {
	   int targetRate = (uint32_t)(max_samp_rate / (1 << i));
	   if (targetRate == SAMPLERATE) {
	      desired_decim_stage = i;
	      resample_ratio = 1;
	      break;
	   } else
	   if (targetRate > SAMPLERATE) {
//	remember the next-largest rate that is available
	      desired_decim_stage = i;
	      resample_ratio = SAMPLERATE / (double)targetRate;
	      settings. sample_rate = targetRate;
	   }

	   if (desired_decim_stage < 0) {
	      if (theServer != nullptr) 
	         delete theServer;
	      theServer = nullptr;
	      return;
	   }

	   std::cerr << "Desired decimation stage: " <<
	                 desired_decim_stage <<
	                 " (" << max_samp_rate << " / " <<
	                 (1 << desired_decim_stage) <<
	                 " = " << max_samp_rate / (1 <<
	                 desired_decim_stage) << ") resample ratio: " <<
	                 resample_ratio << std::endl;
 
	}

	int maxGain		= theDevice. MaximumGainIndex;
	spyServer_gain		-> setMaximum (maxGain);
	settings. resample_ratio	= resample_ratio;
	settings. desired_decim_stage	= desired_decim_stage;
	connected	= true;
	fprintf (stderr, "going to set samplerate stage %d\n",
	                                    desired_decim_stage);
	if (!theServer -> set_sample_rate_by_decim_stage (
	                                    desired_decim_stage)) {
	   std::cerr << "Failed to set sample rate " <<
	                           desired_decim_stage << "\n";
	   return;
	}

	disconnect (spyServer_connect, &QPushButton::clicked,
	            this, &spyServer_client::wantConnect);
	theState	-> setText ("connected");

//	Since we are down sampling, creating an outputbuffer with the
//	same size as the input buffer is OK
	if (settings. resample_ratio != 1.0 ) {
//	we process chunks of 1 msec
	   convBufferSize          = settings. sample_rate / 1000;
	   convBuffer. resize (convBufferSize + 1);
	   float samplesPerMsec = SAMPLERATE / 1000.0;
	   for (int i = 0; i < SAMPLERATE / 1000; i ++) {
	      float inVal  = float (settings. sample_rate / 1000);
	      mapTable_int [i]     = int (floor (i * (inVal / samplesPerMsec)));
	      mapTable_float [i]   = i * (inVal / samplesPerMsec) - mapTable_int [i];
	   }
	   convIndex       = 0;
	}
	theServer -> start_running ();
	running	= true;
}


bool	spyServer_client::restartReader	() {
	fprintf (stderr, "restart reader\n");
	return true;
}

void	spyServer_client::stopReader	() { }

void	spyServer_client::setVFOFrequency	(int32_t freq) {
	if (!connected || !running)
	   return;
	std::cerr << "spy-handler: setting center_freq to " <<
	                                            freq << std::endl;
	if (!theServer -> set_iq_center_freq (freq)) {
	   std::cerr << "Failed to set freq\n";
	   return;
	}
	if (!theServer -> set_gain (settings. gain)) {
	   std::cerr << "Failed to set gain\n";
	   return;
	}
	lastFrequency	= freq;
}

int32_t	spyServer_client::getVFOFrequency	() {
	return lastFrequency;
}
//
void	spyServer_client::resetBuffer	() {
	_I_Buffer -> FlushRingBuffer ();
}

int16_t	spyServer_client::bitDepth () {
	return 8;
}

void	spyServer_client::setGain	(int gain) {
	settings. gain = gain;
	spyServer_settings	-> beginGroup ("SPYSERVERVER_SETTINGS");
	spyServer_settings	-> setValue (
                                      "spyServer_client-gain", settings. gain);
	spyServer_settings	-> endGroup ();
	
	if (theServer == nullptr)
	   return;
	if (!theServer -> set_gain (settings.gain)) {
	   std::cerr << "Failed to set gain\n";
	   return;
	}
}

void    spyServer_client::handle_autogain       (int d) {
        (void)d;
        int x = autogain_selector -> isChecked ();
        settings. auto_gain     = x != 0;
	spyServer_settings	-> beginGroup ("SPYSERVERVER_SETTINGS");
        spyServer_settings 	-> setValue (
                                     "spyServer-auto_gain", x ? 1 : 0);
	spyServer_settings	-> endGroup ();
	if (theServer == nullptr)
	   return;
        if (connected)
           theServer -> set_gain_mode (d != x, 0);
}

void	spyServer_client::connect_on () {
	onConnect. store (true);
}

static 
float convTable [] = {
 -128 / 128.0 , -127 / 128.0 , -126 / 128.0 , -125 / 128.0 , -124 / 128.0 , -123 / 128.0 , -122 / 128.0 , -121 / 128.0 , -120 / 128.0 , -119 / 128.0 , -118 / 128.0 , -117 / 128.0 , -116 / 128.0 , -115 / 128.0 , -114 / 128.0 , -113 / 128.0 
, -112 / 128.0 , -111 / 128.0 , -110 / 128.0 , -109 / 128.0 , -108 / 128.0 , -107 / 128.0 , -106 / 128.0 , -105 / 128.0 , -104 / 128.0 , -103 / 128.0 , -102 / 128.0 , -101 / 128.0 , -100 / 128.0 , -99 / 128.0 , -98 / 128.0 , -97 / 128.0 
, -96 / 128.0 , -95 / 128.0 , -94 / 128.0 , -93 / 128.0 , -92 / 128.0 , -91 / 128.0 , -90 / 128.0 , -89 / 128.0 , -88 / 128.0 , -87 / 128.0 , -86 / 128.0 , -85 / 128.0 , -84 / 128.0 , -83 / 128.0 , -82 / 128.0 , -81 / 128.0 
, -80 / 128.0 , -79 / 128.0 , -78 / 128.0 , -77 / 128.0 , -76 / 128.0 , -75 / 128.0 , -74 / 128.0 , -73 / 128.0 , -72 / 128.0 , -71 / 128.0 , -70 / 128.0 , -69 / 128.0 , -68 / 128.0 , -67 / 128.0 , -66 / 128.0 , -65 / 128.0 
, -64 / 128.0 , -63 / 128.0 , -62 / 128.0 , -61 / 128.0 , -60 / 128.0 , -59 / 128.0 , -58 / 128.0 , -57 / 128.0 , -56 / 128.0 , -55 / 128.0 , -54 / 128.0 , -53 / 128.0 , -52 / 128.0 , -51 / 128.0 , -50 / 128.0 , -49 / 128.0 
, -48 / 128.0 , -47 / 128.0 , -46 / 128.0 , -45 / 128.0 , -44 / 128.0 , -43 / 128.0 , -42 / 128.0 , -41 / 128.0 , -40 / 128.0 , -39 / 128.0 , -38 / 128.0 , -37 / 128.0 , -36 / 128.0 , -35 / 128.0 , -34 / 128.0 , -33 / 128.0 
, -32 / 128.0 , -31 / 128.0 , -30 / 128.0 , -29 / 128.0 , -28 / 128.0 , -27 / 128.0 , -26 / 128.0 , -25 / 128.0 , -24 / 128.0 , -23 / 128.0 , -22 / 128.0 , -21 / 128.0 , -20 / 128.0 , -19 / 128.0 , -18 / 128.0 , -17 / 128.0 
, -16 / 128.0 , -15 / 128.0 , -14 / 128.0 , -13 / 128.0 , -12 / 128.0 , -11 / 128.0 , -10 / 128.0 , -9 / 128.0 , -8 / 128.0 , -7 / 128.0 , -6 / 128.0 , -5 / 128.0 , -4 / 128.0 , -3 / 128.0 , -2 / 128.0 , -1 / 128.0 
, 0 / 128.0 , 1 / 128.0 , 2 / 128.0 , 3 / 128.0 , 4 / 128.0 , 5 / 128.0 , 6 / 128.0 , 7 / 128.0 , 8 / 128.0 , 9 / 128.0 , 10 / 128.0 , 11 / 128.0 , 12 / 128.0 , 13 / 128.0 , 14 / 128.0 , 15 / 128.0 
, 16 / 128.0 , 17 / 128.0 , 18 / 128.0 , 19 / 128.0 , 20 / 128.0 , 21 / 128.0 , 22 / 128.0 , 23 / 128.0 , 24 / 128.0 , 25 / 128.0 , 26 / 128.0 , 27 / 128.0 , 28 / 128.0 , 29 / 128.0 , 30 / 128.0 , 31 / 128.0 
, 32 / 128.0 , 33 / 128.0 , 34 / 128.0 , 35 / 128.0 , 36 / 128.0 , 37 / 128.0 , 38 / 128.0 , 39 / 128.0 , 40 / 128.0 , 41 / 128.0 , 42 / 128.0 , 43 / 128.0 , 44 / 128.0 , 45 / 128.0 , 46 / 128.0 , 47 / 128.0 
, 48 / 128.0 , 49 / 128.0 , 50 / 128.0 , 51 / 128.0 , 52 / 128.0 , 53 / 128.0 , 54 / 128.0 , 55 / 128.0 , 56 / 128.0 , 57 / 128.0 , 58 / 128.0 , 59 / 128.0 , 60 / 128.0 , 61 / 128.0 , 62 / 128.0 , 63 / 128.0 
, 64 / 128.0 , 65 / 128.0 , 66 / 128.0 , 67 / 128.0 , 68 / 128.0 , 69 / 128.0 , 70 / 128.0 , 71 / 128.0 , 72 / 128.0 , 73 / 128.0 , 74 / 128.0 , 75 / 128.0 , 76 / 128.0 , 77 / 128.0 , 78 / 128.0 , 79 / 128.0 
, 80 / 128.0 , 81 / 128.0 , 82 / 128.0 , 83 / 128.0 , 84 / 128.0 , 85 / 128.0 , 86 / 128.0 , 87 / 128.0 , 88 / 128.0 , 89 / 128.0 , 90 / 128.0 , 91 / 128.0 , 92 / 128.0 , 93 / 128.0 , 94 / 128.0 , 95 / 128.0 
, 96 / 128.0 , 97 / 128.0 , 98 / 128.0 , 99 / 128.0 , 100 / 128.0 , 101 / 128.0 , 102 / 128.0 , 103 / 128.0 , 104 / 128.0 , 105 / 128.0 , 106 / 128.0 , 107 / 128.0 , 108 / 128.0 , 109 / 128.0 , 110 / 128.0 , 111 / 128.0 
, 112 / 128.0 , 113 / 128.0 , 114 / 128.0 , 115 / 128.0 , 116 / 128.0 , 117 / 128.0 , 118 / 128.0 , 119 / 128.0 , 120 / 128.0 , 121 / 128.0 , 122 / 128.0 , 123 / 128.0 , 124 / 128.0 , 125 / 128.0 , 126 / 128.0 , 127 / 128.0 };

void	spyServer_client::data_ready	() {
uint8_t buffer_8 [settings. batchSize * 2];

	while (connected && 
	          (tmpBuffer. GetRingBufferReadAvailable () > 2 * settings. batchSize)) {
	   uint32_t samps =	
	            tmpBuffer. getDataFromBuffer (buffer_8,
	                                         2 * settings. batchSize) / 2;
	   if (!running)
	      continue;

	   if (settings. resample_ratio != 1) {
	      std::complex<float> temp [SAMPLERATE / 1000];
	      for (uint32_t i = 0; i < samps; i ++) {
                 convBuffer [convIndex ++] =
                             std::complex<float> (
                                          convTable [buffer_8 [2 * i]],
                                          convTable [buffer_8 [2 * i + 1]]);
              
                 if (convIndex > convBufferSize) {
                    for (int j = 0; j < SAMPLERATE / 1000; j ++) {
                       int16_t  inpBase    = mapTable_int [j];
                       float    inpRatio   = mapTable_float [j];
                       temp [j]    = convBuffer [inpBase + 1] * inpRatio +
                                     convBuffer [inpBase] * (1 - inpRatio);
                    }
                    _I_Buffer -> putDataIntoBuffer (temp, SAMPLERATE / 1000);
                    convBuffer [0] = convBuffer [convBufferSize];
                    convIndex = 1;
                 }
              }
	   }
	   else {	// no resmpling
	      std::complex<float> outB [samps];
	      for (uint32_t i = 0; i < samps; i ++) 
	         outB [i] = std::complex<float> (
	                       convTable [buffer_8 [2 * i]],
	                       convTable [buffer_8 [2 * i + 1]]);
	      _I_Buffer -> putDataIntoBuffer (outB, samps);
	   }
	}
	dataAvailable (0);
}

void	spyServer_client::handle_checkTimer () {
	timedOut = true;
}

void    spyServer_client::set_portNumber        (int v) {
        settings. basePort = v;

	spyServer_settings	-> beginGroup ("SPYSERVER_SETTINGS");
        spyServer_settings	-> setValue ("spyServer-port", v);
	spyServer_settings	-> endGroup ();
}

