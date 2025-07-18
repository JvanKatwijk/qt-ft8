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
#include	<stdint.h>
#include	<stdio.h>
#include	<sdrplay_api.h>

class	sdrplayHandler_v3;

class Rsp_device : public QObject {
Q_OBJECT
protected:
	sdrplay_api_DeviceT *chosenDevice;
	int		sampleRate;
	int		freq;
	bool		agcMode;
	int		lnaState;
	int		GRdBValue;
	sdrplay_api_RxChannelParamsT	*chParams;
	sdrplay_api_DeviceParamsT	*deviceParams;
	sdrplayHandler_v3	*parent;
	int		lna_upperBound;
	QString		deviceModel;
	bool		antennaSelect;
	int		nrBits;
	bool		biasT;
public:
		Rsp_device 	(sdrplayHandler_v3 *parent,
	                         sdrplay_api_DeviceT *chosenDevice,
	                         int sampleRate,
	                         int startFrequency,
	                         bool agcMode,
	                         int lnaState,
	                         int GRdB, bool biasT);
	virtual	~Rsp_device	();
virtual int	lnaStates	(int frequency);

virtual	bool	restart		(int freq);
virtual	bool	set_agc		(int setPoint, bool on);
virtual	bool	set_lna		(int lnaState);
virtual	bool	set_GRdB	(int GRdBValue);
virtual	bool	set_ppm		(int ppm);
virtual	bool	set_antenna	(int antenna);
virtual	bool	set_amPort 	(int amPort);
virtual	bool	set_biasT 	(bool biasT);
signals:
	void	set_lnabounds_signal	(int, int);
	void	set_deviceName_signal	(const QString &);
	void	set_antennaSelect_signal (int);
	void	set_nrBits_signal	(int);
	void	show_lnaGain		(int);
};



