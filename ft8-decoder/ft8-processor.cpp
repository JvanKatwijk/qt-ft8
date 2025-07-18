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

#include	"ft8-constants.h"
#include	"ft8-decoder.h"
#include	"ldpc.h"
#include	"pack-handler.h"
#include	"psk-writer.h"
#include	"radio.h"
#include	<time.h>

	ft8_processor::ft8_processor	(RadioInterface	*theDecoder,
	                                 int		maxIterations):
	                                        theCache (30),
	                                        freeSlots (nrBlocks) {
	                           
	this	-> maxIterations	= maxIterations;
	this	-> blockToRead	= 0;
	this	-> blockToWrite	= 0;
	cqSelector. store (false);
	running. store (false);
	connect (this, &ft8_processor::printLine,
	         theDecoder, &RadioInterface::printLine);
	connect (this, &ft8_processor::show_pskStatus,
	         theDecoder, &RadioInterface::show_pskStatus);
	connect (this, &ft8_processor::addMessage,
	         theDecoder, &RadioInterface::addMessage);
	threadHandle    = std::thread (&ft8_processor::run, this);
}

	ft8_processor::~ft8_processor	() {
	if (running. load ()) {
           running. store (false);
           sleep (1);
           threadHandle. join ();
        }
}

static int blocks_in	= 0;

//	entry for processing
void	ft8_processor::PassOn (int lineno,
	                       float refVal, float strengthValue,
	                       int frequency, float *log174) {
	if (blocks_in >= 4)
	   return;
        while (!freeSlots. tryAcquire (200)) 
	   if (!running. load ())
	      return;
	blocks_in ++;
	theBuffer [blockToWrite]. lineno	= lineno;
	theBuffer [blockToWrite]. value 	= refVal;
	theBuffer [blockToWrite]. frequency	= frequency;
	theBuffer [blockToWrite]. strengthValue	= strengthValue;
	for (int i = 0; i < FT8_LDPC_BITS; i ++)
	   theBuffer [blockToWrite]. log174 [i] = log174 [i];
	blockToWrite = (blockToWrite + 1 ) % nrBlocks;;
	usedSlots. Release ();
}

void	ft8_processor::set_maxIterations	(int n) {
	maxIterations. store (n);
}

void	insertString (char *target, int pos, const QString &s) {
	for (int i = 0; s.toLatin1 (). data () [i] != 0; i ++)
	   target [pos + i] = s. toLatin1 (). data () [i];
}

void	insertNumber (char *target, int pos, int number) {
QString s = QString::number (number);
	insertString (target, pos, s);
}

void	insert_2_Number (char *target, int pos, int number) {
QString s;
	if ((number >= 100) || (number < 0)) {
	   insertString (target, pos, QString::number (number));
	   return;
	}
	if (number >= 10) {
	   s. push_back (QChar ('0' + number / 10));
	   s. push_back (QChar ('0' + number % 10));
	   insertString (target, pos, s);
	   return;
	}
	s. push_back (' ');
	s. push_back (QChar ('0' + number));
	insertString (target, pos, s);
}

QString	makeLine (QString time,
	          int value, int freq,
	          QString message, int strength)  {
char res [256];
static
int posTable [] = {0, 15, 20, 40, 75, 85};

	for (int i = 0; i < 256; i ++)
	   res [i] = ' ';

	insertString (res, posTable [0], time);
	insert_2_Number (res, posTable [1], value > 100 ? 101 : value);
	insertNumber (res, posTable [2], freq);
	insertString (res, posTable [3], message);
	insert_2_Number (res, posTable [4], strength);
	res [posTable [5]] = 0;
	return QString (res);
}

void	ft8_processor::run () {
uint8_t ldpcBits [FT8_LDPC_BITS];
ldpc ldpcHandler;
int	errors;

	usleep (1000000);
	running. store (true);
        while (running. load()) {
	   while (!usedSlots. tryAcquire (200)) 
	      if (!running. load ())
	         return;
//	   ldpcHandler. bp_decode (theBuffer [blockToRead]. log174,
//	                           maxIterations. load (),
//	                           ldpcBits, &errors);
	   ldpcHandler. ldpc_decode (theBuffer [blockToRead]. log174,
	                             maxIterations. load (),
	   	                     ldpcBits, &errors);

//	   Release the buffer now
	   freeSlots. Release ();
	   blocks_in --;
           blockToRead = (blockToRead + 1) % (nrBlocks);
//
	   if (errors != 0) 
	      continue;

//	the check bits are to be moved from 77 up to 82 up
//	and 77 .. 82 should be 0
	   for (int i = 96; i > 82; i --)
	      ldpcBits [i] = ldpcBits [i - 14];

	   for (int i = 77; i < 82; i ++)
	      ldpcBits [i] = 0;

	   if (check_crc_bits (ldpcBits, 96)) {
//	      fprintf (stderr, "crc OK\n");
//	crc is correct, unpack  the message
	      bool is_CQ	= false;
	      QString res = unpackHandler. unpackMessage (ldpcBits, is_CQ);
	      if (res != "") {
	         if ((cqSelector. load () && is_CQ) || !cqSelector. load ()) {
	            showLine (theBuffer [blockToRead]. lineno,
	                      theBuffer [blockToRead]. value,
	                      theBuffer [blockToRead]. frequency,
	                      theBuffer [blockToRead]. strengthValue,
	                      res);
	         }
	         QStringList call = unpackHandler. extractCall (ldpcBits);
	         if (call. length () > 0) {
	            QString callIdent	= call. at (0);
	            QString locator;
	            if (call. length () == 2)
	               locator = call. at (1);
	            int snr	= theBuffer [blockToRead]. value;
	            int freq	= theBuffer [blockToRead]. frequency;
	            addMessage (callIdent, locator, freq, snr);
	         }
	      }
	   }
//	prepare for the next round
	}
}

void	ft8_processor::print_statistics () {
}

void    ft8_processor::showLine (int line, int val, int freq,
	                                  float strength, const QString &res) {
	if (theCache. update (val, freq, res. toStdString ()))
	   return;

//	int currentFreq	= theDecoder -> tunedFrequency ();
	time_t rawTime;
	struct tm *timeinfo;
	char buffer [100];
	time (&rawTime);
	timeinfo = localtime (&rawTime);
	strftime (buffer, 80, "%I:%M:%S%p", timeinfo);
	QString s = QString (buffer);
//	QString result = makeLine (s, val, freq, res, (int)strength);
	printLine (s, val, freq, res, (int)strength);
}

static
uint8_t crcPolynome [] = 
	{1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1};	// MSB .. LSB
bool	ft8_processor::check_crc_bits (uint8_t *in, int nrBits) {
uint8_t	b [14];
int16_t	Sum	= 0;

	memset (b, 0, 14);

	for (int i = nrBits - 14; i < nrBits; i ++)
	   in [i] = ~in [i];

	for (int i = 0; i < nrBits; i++) {
	   if ((b [0] ^ in [i]) == 1) {
	      for (int f = 0; f < 13; f++) 
	         b [f] = b [f + 1]; 
	      b [13] = 0;
	      for (int f = 0; f < 14; f ++)
	         b [f] ^= crcPolynome [f];
	   }
	   else {
	      memmove (&b [0], &b[1], sizeof (uint8_t ) * 13); // Shift
	      b [13] = 0;
	   }
	}
	for (int i = 0; i < 14; i ++)
	   Sum += b [i];
	return Sum == 0;
}

void	ft8_processor::set_cqSelector	(bool b) {
	cqSelector. store (b);
}

