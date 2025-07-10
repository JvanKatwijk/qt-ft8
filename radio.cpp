#
/*
 *    Copyright (C)  2020 .. 2025
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
 */
#include	<unistd.h>
#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QHeaderView>
#include	"radio.h"
#include	"fft-scope.h"
#include	"fft-filters.h"
#include        "popup-keypad.h"
#include	"identity-dialog.h"
#include	"ft8-decoder.h"
#include	"psk-writer.h"
//
//      devices
#include        "device-handler.h"
#include        "filereader.h"
#ifdef	HAVE_SDRPLAY_V2
#include        "sdrplay-handler-v2.h"
#endif
#ifdef	HAVE_SDRPLAY_V3
#include        "sdrplay-handler-v3.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SPYSERVER
#include	"spyserver-client.h"
#endif
//
#include	"deviceselect.h"
#include	"ft8-decoder.h"

#define	D_SDRPLAY_V2	"sdrplay_v2"
#define	D_SDRPLAY_V3	"sdrplay"
#define	D_RTL_TCP	"rtl_tcp"
#define	D_HACKRF	"hackrf"
#define	D_RTLSDR	"dabstick"
#define	D_SPYSERVER	"spyserver_client"
static 
const char *deviceTable [] = {
#ifdef	HAVE_SDRPLAY_V2
	D_SDRPLAY_V2,
#endif
#ifdef	HAVE_SDRPLAY_V3
	D_SDRPLAY_V3,
#endif
#ifdef	HAVE_HACKRF
	D_HACKRF,
#endif
#ifdef	HAVE_RTLSDR
	D_RTLSDR,
#endif
#ifdef	HAVE_RTL_TCP
	D_RTL_TCP,
#endif
#ifdef	HAVE_SPYSERVER
	D_SPYSERVER,
#endif
	nullptr
};

static int startKnop;
static	QTimer	*starter;

static inline
int twoPower (int a) {
int	res	= 1;
	while (--a >= 0)
	   res <<= 1;
	return res;
}

QString	FrequencytoString (int32_t freq) {
	if (freq < 10)
	   return QString (QChar ('0' + (uint8_t)(freq % 10)));
	return 
	   FrequencytoString (freq / 10). append (QChar ('0' + (uint8_t)(freq % 10)));
}

uint32_t freqTable [] =
	{1840, 3753, 5357, 7056, 7071, 7074, 10136,
	14075, 18104, 21075, 24915, 28074, 0};

	RadioInterface::RadioInterface (QSettings	*sI,
	                                const QString	&presetFile,
	                                QWidget		*parent):
	                                    QMainWindow (parent),
	                                    inputData	(1024 * 1024),
	                                    hfShifter	(96000),
	                                    hfFilter	(2048, 377),
	                                    my_bandPlan (":res/sw-bandplan.xml"),
	                                    thePresets (this, presetFile),
	                                    theDecimator (96000, 12000) {

	this	-> settings	= sI;
	this	-> inputRate	= 96000;
	setupUi (this);

	tableWidget	= new QTableWidget (0, 5);
	tableWidget	-> setColumnWidth (0, 100);	// date
	tableWidget	-> setColumnWidth (1, 20);
	tableWidget	-> setColumnWidth (2, 100);
	tableWidget	-> setColumnWidth (3, 300);
	tableWidget	-> setColumnWidth (4, 50);
	QHeaderView * headerView = tableWidget -> horizontalHeader ();
	headerView	-> setSectionResizeMode (1, QHeaderView::Stretch);
	tableWidget	-> setHorizontalHeaderLabels (
	       QStringList () << tr ("date") << tr ("???") << tr ("freq") <<
	                      tr ("message") << tr ("strength"));
	displayArea	-> setWidget (tableWidget);
//      and some buffers

//	and the decoders
	displaySize		= 1024;
	scopeWidth		= inputRate;
	centerFrequency		= KHz (14070);
	selectedFrequency	= KHz (14070);

	theBand. currentOffset	= 0;
	theBand. lowF		= -3000;
	theBand. highF		= 3000;
	mouseIncrement		= 5;
//	scope and settings
	hfScopeSlider	-> setValue (50);
        hfScope		= new fftScope (hfSpectrum,
                                        displaySize,
                                        kHz (1),        // scale
                                        inputRate,
                                        hfScopeSlider -> value (),
                                        8);
	hfScope		-> set_bitDepth (12);	// default
	centerFrequency		= KHz (14070);	// default
	selectedFrequency	= KHz (14070);	// default
	hfScope	-> setScope (centerFrequency, selectedFrequency - centerFrequency);
	settings	-> beginGroup ("ft8Settings");
	int val		= settings -> value ("width", 4000). toInt ();
	spectrumWidth_selector -> setValue (val);
	val         = settings -> value ("maxIterations", 20). toInt ();
        iterationSelector -> setValue (val);
	settings	-> endGroup ();
        mykeyPad                = new keyPad (this);

        connect (hfScope, &fftScope::clickedwithLeft,
                 this, &RadioInterface::adjustFrequency_khz);
	connect (hfScope, &fftScope::clickedwithRight,
	         this, &RadioInterface::switch_hfViewMode);
        connect (hfScopeSlider, &QSlider::valueChanged,
                 this, &RadioInterface::set_hfscopeLevel);
        connect (freqButton, &QPushButton::clicked,
                 this, &RadioInterface::handle_freqButton);
	connect (mouse_Inc, SIGNAL (valueChanged (int)),
	         this, SLOT (set_mouseIncrement (int)));
	connect (freqSave, &QPushButton::clicked,
                 this, &RadioInterface::set_freqSave);
	connect (cq_selector, &QComboBox::textActivated,
                 this, &RadioInterface::handle_cq_selector);
	connect (identityButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_identityButton);
	connect (spectrumWidth_selector, &QSpinBox::valueChanged,
                 this, &RadioInterface::set_spectrumWidth);
	connect (iterationSelector, &QSpinBox::valueChanged,
                 this, &RadioInterface::set_maxIterations);
	connect (presetButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_presetButton);
	connect	(pskReporterButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_pskReporterButton);
	connect (filesaveButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_filesaveButton);
	theWriter		= nullptr;
	filePointer. store (nullptr);
	theDevice		= nullptr;
	delayCount		= 0;

	secondsTimer. setInterval (1000);
	connect (&secondsTimer, SIGNAL (timeout (void)),
                 this, SLOT (updateTime (void)));
        secondsTimer. start (1000);

//	and off we go
	theDecoder	= new ft8_Decoder (this, 12000, settings);
//	bind the signals in the decoder to some slots
//
//	and now for the device
	QString device =
	            settings -> value ("device", "no device").toString ();
	int k = -1;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   if (deviceTable [i] == device) {
	      k = i;
	      break;
	}
	if (k != -1) {
	   starter	= new QTimer;
	   startKnop	= k;
	   starter -> setSingleShot (true);
	   starter -> setInterval (500);
	   connect (starter, &QTimer::timeout, 
	            this, &RadioInterface::quickStart);
	   starter -> start (500);
	}
	else {
	   startKnop	= 0;
	   theDevice 	= setDevice (settings, &inputData);
	   if (theDevice != nullptr) {
	      hfScope	 -> set_bitDepth (theDevice -> bitDepth ());
	      connect (theDevice, &deviceHandler::dataAvailable,
	               this, &RadioInterface::sampleHandler);
	      theDevice	-> restartReader ();
	      settings	-> beginGroup ("ft8Settings");
	      int freq	= settings	-> value ("frequency", 14075). toInt ();
	      settings	-> endGroup ();
	      setFrequency (freq * KHz (1));
	   }
	   else
	      throw (24);
	}
}

void    RadioInterface::quickStart () {
        disconnect (starter, &QTimer::timeout,
                    this, &RadioInterface::quickStart);
        delete starter;
        if (getDevice (deviceTable [startKnop], settings, &inputData) == nullptr) {
           if (setDevice (settings, &inputData) == nullptr) {
	      hfScope	 -> set_bitDepth (theDevice -> bitDepth ());
	      connect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	      theDevice	-> restartReader ();
	      settings	-> beginGroup ("ft8Settings");
	      int freq	= settings	-> value ("frequency", 14075). toInt ();
	      settings	-> endGroup ();
	      setFrequency (freq);
	   }
	   else
	      throw (31);
	}
}

//      The end of all
        RadioInterface::~RadioInterface () {
}
//
//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	() {
	if (theDevice != NULL) {
	   theDevice	-> stopReader ();
	   disconnect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	   delete  theDevice;
	}
	thePresets. hide ();
	delete		theDecoder;
	secondsTimer. stop ();
	delete		hfScope;
        delete		mykeyPad;
	if (filePointer. load () != nullptr)
	   fclose (filePointer. load ());
}

deviceHandler	*RadioInterface::
	            setDevice (QSettings *s,
	                       RingBuffer<std::complex<float>> *hfBuffer) {
deviceSelect	deviceSelect;
deviceHandler	*theDevice	= nullptr;
QStringList devices;

	(void)s;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   devices += deviceTable [i];
//	devices	+= "filereader";
	devices	+= "quit";
	deviceSelect. addList (devices);
	int theIndex = -1;
	while (theDevice == nullptr) {
	   theIndex = deviceSelect. QDialog::exec ();
	   if (theIndex < 0)
	      continue;
	   QString s = devices. at (theIndex);
	   if (s == "quit")
	      return nullptr;
//	   fprintf (stderr, "we go for %s\n", s. toLatin1 (). data ());
	   theDevice	= getDevice (s, settings, hfBuffer);
	   if (theDevice != nullptr) {
	      theDevice -> restartReader ();
	      return theDevice;
	   }
	}
	return theDevice;
}

deviceHandler	*RadioInterface::
	                      getDevice (const QString &s,
	                                 QSettings *settings,
	                                 RingBuffer<std::complex<float>> *b) {
#ifdef HAVE_SDRPLAY_V2
	if (s == D_SDRPLAY_V2) {
	   try {
	      return  new sdrplayHandler_v2 (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_SDRPLAY_V3
	if (s == D_SDRPLAY_V3) {
	   try {
	      return new sdrplayHandler_v3 (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_HACKRF
	if (s == D_HACKRF) {
	   try {
	      return  new hackrfHandler (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_RTLSDR
	if (s == D_RTLSDR) {
	   try {
	      return new rtlsdrHandler (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_SPYSERVER
	if (s == D_SPYSERVER) {
	   try {
	      deviceHandler *tt =  new spyServer_client (this, settings, b);
	      tt -> restartReader ();
	      return tt;
	   } catch (int e) {
	   }
	}
	else
#endif
	if (s == "filereader") {
	   try {
	      return new fileReader (this, settings, b);
	   } catch (int e) {
	   }
	}
	else {
	   QMessageBox::warning (this, tr ("sdr"),
	                            tr ("loading device failed"));
	}
	return nullptr;
}
//
//	
void    RadioInterface::handle_freqButton (void) {
        if (mykeyPad -> isVisible ())
           mykeyPad -> hidePad ();
        else
           mykeyPad     -> showPad ();
}

//	setFrequency is called from the frequency panel,
//	from inside to change VFO and offset,
//	and from the decoder
void	RadioInterface::setFrequency (int32_t frequency) {
	centerFrequency		= frequency;
	selectedFrequency	= frequency;
	theDevice	-> setVFOFrequency (centerFrequency);
	hfScope		-> setScope  (centerFrequency,
	                              centerFrequency - selectedFrequency);
	hfFilter. setBand (theBand. currentOffset + theBand. lowF,
                           theBand. currentOffset + theBand. highF,
                                                  inputRate);
//	fprintf (stderr, "Low %d high %d\n", centerFrequency + theBand. lowF,
//	                                     centerFrequency + theBand. highF);
	QString ff	= FrequencytoString (selectedFrequency);
	frequencyDisplay	-> display (ff);
	bandLabel	-> setText (my_bandPlan. getFrequencyLabel (selectedFrequency));        
	settings	-> beginGroup ("ft8Settings");
	settings	-> setValue ("frequency", selectedFrequency / 1000);
	settings	-> endGroup ();
}
//
//	adjustFrequency is called whenever clicking the mouse
void	RadioInterface::adjustFrequency_khz (int32_t n) {
	adjustFrequency_hz (1000 * n);
//	fprintf (stderr, "adjusting %d Hz\n", n * 1000);
}

void	RadioInterface::adjustFrequency_hz (int32_t n) {
int	lowF	= theBand. lowF;
int	highF	= theBand. highF;
int	currOff	= theBand. currentOffset;

	if ((currOff + highF + n >= scopeWidth / 2 - scopeWidth / 20) ||
	    (currOff + lowF + n <= - scopeWidth / 2 + scopeWidth / 20) ) {
	   quint64 newFreq = theDevice -> getVFOFrequency () +
	                                   theBand. currentOffset + n;
	   setFrequency (newFreq);
	   return;
	}
	else {
	   theBand. currentOffset += n;
	   hfScope -> setScope (theDevice -> getVFOFrequency (),
	                                   theBand. currentOffset);
	   hfFilter. setBand (theBand. currentOffset + theBand. lowF,
	                      theBand. currentOffset + theBand. highF,
	                            inputRate);
	}

	int freq		= theDevice -> getVFOFrequency () +
	                                    theBand. currentOffset;
	QString ff = FrequencytoString ((quint64)freq);
	frequencyDisplay	-> display (ff);
}

//      just a convenience button
void    RadioInterface::set_inMiddle () {
        quint64 newFreq = theDevice -> getVFOFrequency () +
                                            theBand. currentOffset;
        setFrequency (newFreq);
}

int32_t	RadioInterface::get_selectedFrequency	() {
	return selectedFrequency;
}

int32_t	RadioInterface::get_centerFrequency	() {
	return centerFrequency;
}
//
void    RadioInterface::set_freqSave    () {
int frequency	= theDevice -> getVFOFrequency ();
	thePresets. addElement (frequency);
}

void	RadioInterface::set_mouseIncrement (int inc) {
	mouseIncrement = inc;
}

void	RadioInterface::wheelEvent (QWheelEvent *e) {
	adjustFrequency_hz ((e -> angleDelta (). ry () > 0) ?
	                        mouseIncrement : -mouseIncrement);
}

//////////////////////////////////////////////////////////////////
//
void	RadioInterface::sampleHandler (int amount) {
std::complex<float>   buffer [theDecimator. inSize ()]; 
std::complex<float> ifBuffer [theDecimator. outSize ()];

	(void)amount;
	while (inputData. GetRingBufferReadAvailable () > 512) {
	   inputData. getDataFromBuffer (buffer, 512);
	   hfScope	-> addElements (buffer, 512);
	   for (int i = 0; i < 512; i ++) {
              std::complex<float> temp = hfFilter. Pass (buffer [i]);
              temp = hfShifter. do_shift (temp, theBand. currentOffset);
              if (theDecimator. add (temp, ifBuffer)) {
                 for (int j = 0; j < theDecimator. outSize (); j ++) {
	            theDecoder -> process (ifBuffer [j]);
                 }
              }
           }
        }
}
//
//
void    RadioInterface::set_hfscopeLevel (int level) {
        hfScope -> setLevel (level);
}

void	RadioInterface::switch_hfViewMode	(int d) {
	(void)d;
	hfScope	-> switch_viewMode ();
}

void	RadioInterface::updateTime		(void) {
QDateTime currentTime = QDateTime::currentDateTime ();

	timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
           handle_quitButton ();
           event -> accept ();
        }
}

void	RadioInterface::handle_preset	(const QString &freqText) {
bool	ok;
int32_t frequency = freqText. toInt (&ok);
	if (!ok)
	   return;
	setFrequency (KHz (frequency));
}

void	RadioInterface::show_pskStatus	(bool b) {
	if (b) {
	   pskStatus_label -> setStyleSheet ("QLabel {background-color : green}");
	   pskStatus_label -> setText ("sending to pskServer");
	}
	else {
	   pskStatus_label -> setStyleSheet ("QLabel {background-color : red}");
	   pskStatus_label -> setText ("");
	}
}

void	RadioInterface::handle_cq_selector	(const QString &s) {
	if (theDecoder != nullptr)
	   theDecoder -> handle_cq_selector (s);
}

void	RadioInterface::handle_identityButton () {
identityDialog Identity (settings);
        Identity. QDialog::exec ();
}

void	RadioInterface::set_spectrumWidth (int n) {
	if (theDecoder != nullptr) {
	   theDecoder -> set_spectrumWidth (n);
	   settings     -> beginGroup ("ft8Settings");
           settings     -> setValue ("width", n);
           settings     -> endGroup ();
	   theBand. lowF	= -n / 2;
	   theBand. highF	= n / 2;
	}
}

void	RadioInterface::set_maxIterations	(int n) {
	if (theDecoder != nullptr) {
	   theDecoder -> set_maxIterations	(n);
	   settings	-> beginGroup ("ft8Settings");
	   settings	-> setValue ("maxIterations", n);
	   settings	-> endGroup ();
	}
}

void	RadioInterface::addMessage	(const QString  &call,
	                         const QString  &grid,
	                         int frequency,
	                         int snr) {
	int freq	= theDevice -> getVFOFrequency () + frequency;
	if (theWriter != nullptr) 
	   theWriter ->  addMessage (call. toStdString (),
	                             grid. toStdString (),
		                     freq, snr);
}

void	RadioInterface::printLine	(const QString &s,
	                                 int val, int freq, 
	                                 const QString &res, int strength) {

int row	= tableWidget -> rowCount ();
	freq		= freq + theDevice -> getVFOFrequency ();
	tableWidget -> insertRow (row);
	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 0, item0);

	QTableWidgetItem *item1	= new QTableWidgetItem;
	item1		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 1, item1);

	QTableWidgetItem *item2	= new QTableWidgetItem;
	item2		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 2, item2);

	QTableWidgetItem *item3	= new QTableWidgetItem;
	item3		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 3, item3);

	QTableWidgetItem *item4	= new QTableWidgetItem;
	item4		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 4, item4);

	tableWidget	-> item (row, 0) -> setText (s);
	tableWidget	-> item (row, 1) -> setText (QString::number (val));
	tableWidget	-> item (row, 2) -> setText (QString::number (freq));
	tableWidget	-> item (row, 3) -> setText (res);
	tableWidget	-> item (row, 4) -> setText (QString::number (strength));
QString result	= s + ";" + QString::number (val) + ";" +
	          QString::number (freq + theDevice -> getVFOFrequency ()) + 
	          ";" + res + ";" + QString::number (strength);
	if (theResults. size () >= 50)
           theResults. pop_front ();
        theResults += result;
	if (filePointer. load () != nullptr)
	   fprintf (filePointer. load (), "%s\n",
	                           result. toUtf8 (). data ());
	fprintf (stderr, "%s\n", result. toLatin1 (). data ());
}

void	RadioInterface::handle_filesaveButton	() {
	if (filePointer. load () != nullptr) {
	   fclose (filePointer);
	   filesaveButton -> setText ("save file");
	   filePointer. store (nullptr);
	   return;
	}

	QString saveName = 
		 QFileDialog::getSaveFileName (nullptr,
                                                 tr ("save file as .."),
                                                 QDir::homePath (),
                                                 tr ("Images (*.txt)"));
	if (saveName == "")
	   return;

	filePointer. store (fopen (saveName. toUtf8 (). data (), "w"));
	if (filePointer. load () == nullptr)
	   return;

	filesaveButton -> setText ("saving");
}

void	RadioInterface::handle_pskReporterButton	() {
	locker. lock ();
	if (theWriter != nullptr) {
	   delete theWriter;
	   pskReady	= false;
	   theWriter	= nullptr;
	   locker. unlock ();
	   show_pskStatus (false);
	   return;
	}

	try {
	   theWriter	= new reporterWriter (settings);
	   pskReady = true;
	} catch (int e) {
	   pskReady	= false;
	}
	locker. unlock ();
	if (pskReady) {
#ifdef	__MINGW32__
	   std::wstring browserAddress = 
	                  std::wstring (L"https://pskreporter.info/pskmap.html");
	   fprintf (stderr, "browser address %s\n", browserAddress. c_str ());
	   ShellExecute (nullptr, L"open", browserAddress. c_str (),
	                                   nullptr, nullptr, SW_SHOWNORMAL);
#else
	   std::string browserAddress = 
	                  std::string ("https://pskreporter.info/pskmap.html");
	   std::string x = std::string ("xdg-open ") + browserAddress;
	   (void)system (x. c_str ());
#endif
	}
	show_pskStatus (pskReady);
}

void	RadioInterface::sendMessages () {
	if (theWriter != nullptr) {
	   theWriter -> sendMessages ();
	}
}

void	RadioInterface::handle_presetButton	() {
	if (thePresets. isVisible ())
	   thePresets. hide ();
	else
	   thePresets. show ();
}

