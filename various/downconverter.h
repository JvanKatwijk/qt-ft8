#
/*
 *    Copyright (C) 2014 .. 2024
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the  drm-receiver
 *
 *    drm-receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm-receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	<stdint.h>
#include	<samplerate.h>
#include	"radio-constants.h"
#include	"ringbuffer.h"

class	fftFilter;

class	downConverter {
public:
			downConverter	(int32_t rateIn, int32_t  rateOut);
			~downConverter	();
	int32_t		bufferSize_in	();
	int32_t		bufferSize_out	();
	void		convert_in	(std::complex<float> *);
	uint32_t	hasData		();
	int32_t		dataOut		(std::complex<float> *, int32_t);
private:
	int32_t         inRate;
	int32_t         outRate;
	double          ratio;
	int32_t         outputLimit;
	int32_t         inputLimit;
	SRC_STATE       *src_converter;
	SRC_DATA        *src_data;
	float           *inBuffer;
	float           *outBuffer;
	int32_t         inp;
	fftFilter	*hfFilter;
	RingBuffer<std::complex<float>> *dataBuffer;
	bool		filtering_needed;
};

