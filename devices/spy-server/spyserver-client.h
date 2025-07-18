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
#include	<QSettings>
#include	<QTimer>
#include	<atomic>
#include	<QLabel>
#include	<QFrame>
#include	<QMessageBox>
#include	<QLineEdit>
#include	<QHostAddress>
#include	<QByteArray>
#include	<QScopedPointer>
#include	<cstdio>
#include	"radio-constants.h"
#include	"device-handler.h"
#include	"ringbuffer.h"
#include	"ui_spyserver-widget.h"

#include	"spy-handler.h"

#define	SAMPLERATE	96000

class	spyServer_client: //public QObject,
	                 public deviceHandler, Ui_spyServer_widget_8 {
public:
			spyServer_client	(RadioInterface *,
	                                         QSettings *,
	                                         RingBuffer<std::complex<float>> *);
			~spyServer_client	();
	void		setVFOFrequency	(int32_t);
	int		getVFOFrequency	();
	bool		restartReader	();
	void		stopReader	();
	void		resetBuffer	();
	int16_t		bitDepth	();
	void		connect_on	();
struct {
	int		gain;
	qint64		basePort;
	uint32_t	sample_rate;
	float		resample_ratio;
	int		desired_decim_stage;
	int		resample_quality;
	int		batchSize;
	int		sample_bits;
	int		auto_gain;
} settings;

private slots:
	void		setConnection	();
	void		wantConnect	();
	void		setGain		(int);
	void		handle_autogain (int);
        void            handle_checkTimer       ();
        void            set_portNumber  (int);
public slots:
	void		data_ready	();
private:
	QFrame		myFrame;
	RingBuffer<std::complex<float>> *_I_Buffer;
	QLineEdit	hostLineEdit;
	RingBuffer<uint8_t>	tmpBuffer;
	QTimer		checkTimer;
	spyHandler	*theServer;
	bool		isvalidRate	(int32_t);
	QSettings	*spyServer_settings;
	int32_t		theRate;
	bool		connected;
	bool		running;
	QHostAddress	serverAddress;
	qint64		basePort;
	bool		dumping;
	FILE		*dumpfilePointer;
	std::atomic<bool>	onConnect;
	bool		timedOut;

	int16_t         convBufferSize; 
        int16_t         convIndex;
        std::vector <std::complex<float> >      convBuffer;
        int16_t         mapTable_int   [SAMPLERATE / 1000];
        float           mapTable_float [SAMPLERATE / 1000];
        int             selectedRate;
};


