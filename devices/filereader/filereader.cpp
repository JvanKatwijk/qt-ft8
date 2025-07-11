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
 */
#include	<QLabel>
#include	<QFileDialog>
#include	"radio-constants.h"
#include	"radio.h"
#include	"filehulp.h"
#include	"filereader.h"

	fileReader::fileReader	(RadioInterface *mr,
	                         QSettings *s,
	                         RingBuffer<std::complex<float>> *b):
	                           myFrame (nullptr) {
	theRate		= 96000;
	(void)s;
	setupUi		(&myFrame);
	myFrame. show ();
	myReader	= nullptr;
	QString	replayFile
	              = QFileDialog::
	                 getOpenFileName (nullptr,
	                                  tr ("load file .."),
	                                  QDir::homePath (),
	                                  tr ("sound (*.wav)"));
	replayFile	= QDir::toNativeSeparators (replayFile);
	myReader	= new fileHulp (replayFile, theRate, b);
	connect (myReader, SIGNAL (setProgress (int, float, float)),
	         this, SLOT (setProgress (int, float, float)));
	connect (myReader, SIGNAL (dataAvailable (int)),
	         this, SIGNAL (dataAvailable (int)));
	nameofFile	-> setText (replayFile);
	setProgress	(0, 0, 0);
	rateDisplay	-> display (theRate);
	this	-> lastFrequency	= Khz (14070);
	connect (resetButton, SIGNAL (clicked (void)),
	         this, SLOT (reset (void)));
	connect (progressBar, SIGNAL (sliderMoved (int)),
	         this, SLOT (handle_progressBar (int)));
}
//
	fileReader::~fileReader	() {
	if (myReader != nullptr)
	   delete myReader;
	myReader	= nullptr;
	myFrame. hide ();
}

void	fileReader::handle_progressBar		(int f) {
	myReader	-> setFileat (f);
}


void	fileReader::setProgress	(int progress,
	                         float  timelength, float playTime) {
//	progressBar	-> setValue (progress);
	progressBar	-> setValue (timelength / playTime * 100);
	currentTime	-> display (timelength);
	totalTime	-> display (playTime);
	
}

bool	fileReader::restartReader		() {
	fprintf (stderr, "reader restart met %d\n", myReader != nullptr);
	if (myReader != nullptr)
	   return myReader -> restartReader ();
	else
	   return false;
}

void	fileReader::stopReader			() {
	if (myReader != nullptr)
	   myReader	-> stopReader ();
}

void	fileReader::exit			() {
	if (myReader != nullptr)
	   myReader	-> stopReader ();
}

int16_t	fileReader::bitDepth			() {
	return 16;
}

void	fileReader::reset			() {
	if (myReader != nullptr)
	   myReader -> reset ();
}

int32_t	fileReader::getRate			() {
	return theRate;
}

