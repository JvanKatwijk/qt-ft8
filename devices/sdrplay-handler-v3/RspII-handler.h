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

#include	"Rsp-device.h"

class	sdrplayHandler;

class	RspII_handler: public Rsp_device {
public:
		RspII_handler (sdrplayHandler_v3 *parent,
	                       sdrplay_api_DeviceT *chosenDevice,
	                       int	sampleRate,
	                       int	freq,
	                       bool	agcMode,
	                       int	lnaState,
	                       int 	GRdB,
	                       bool	biasT);
		~RspII_handler	();

	int	lnaStates	(int frequency);
	bool	restart		(int freq);
	bool	set_agc		(int setPoint, bool on);
	bool	set_GRdB	(int GRdBValue);
	bool	set_ppm		(int ppm);
	bool	set_lna		(int lnaState);
	bool	set_antenna	(int antenna);
	bool	set_biasT	(bool biasT);
private:
	int16_t	bankFor_rspII 	(int freq);
	int	get_lnaGain	(int, int);
};



