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


#pragma once

#include	<QObject>
#include	<thread>
#include	<atomic>
#include	"semaphore.h"
#include	<QMutex>
#include	"ft8-constants.h"
#include	"pack-handler.h"
#include	"dl-cache.h"

class	RadioInterface;
class	reporterWriter;
#define	nrBlocks	100

class	ft8_processor: public QObject {
Q_OBJECT
public:
		ft8_processor	(RadioInterface *, int);
		~ft8_processor	();

	void	PassOn		(int, float, float, int, float *);
	void	set_maxIterations	(int);

	void		set_cqSelector		(bool);
private:
	packHandler	unpackHandler;
	void		run		();
	bool		check_crc_bits	(uint8_t *message, int nrBits);
	void		showLine	(int, int, int, float, const QString &);
	dlCache		theCache;
	std::atomic<bool>	cqSelector;
	struct {
           int lineno;
	   float	 value;
           int frequency;
	   float	strengthValue;
           float log174 [174];
        } theBuffer [nrBlocks];

	Semaphore	freeSlots;
	Semaphore	usedSlots;
	
	std::atomic<bool> running;
	
//	ft8_Decoder	*theDecoder;
	std::atomic<int>	maxIterations;
	int		amount;
	int		blockToRead;
	int		blockToWrite;
	std::thread	threadHandle;
        uint8_t         a91 [FT8_M_BYTES];

	void		print_statistics	();
signals:
	void		printLine	(const QString &,
	                                 int, int, const QString &, int);
	void		show_pskStatus	(bool);
	void		addMessage	(const QString &, const QString &,
	                                            int, int);
};

