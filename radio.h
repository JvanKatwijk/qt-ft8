#
/*
 *    Copyright (C)  2022 - 2025
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

#include        <QMainWindow>
#include        <QTimer>
#include        <QWheelEvent>
#include        <QLineEdit>
#include	<QTimer>
#include        "ui_qt-ft8.h"
#include        "radio-constants.h"
#include	"fft-filters.h"
#include        "ringbuffer.h"
#include	"shifter.h"
#include	"decimator.h"
#include	"bandplan.h"
#include        <QStandardItemModel>
#include	<mutex>
#include	"preset-handler.h"
#include	<QTableWidget>
class		deviceHandler;
class		ft8_Decoder;
class           QSettings;
class           fftScope;
class           fft_scope;
class           keyPad;
class		reporterWriter;

class	RadioInterface:public QMainWindow,
	               private Ui_MainWindow {
Q_OBJECT
public:
		RadioInterface (QSettings	*sI,
	                        const QString	&presetFile,
	                        QWidget		*parent = nullptr);
		~RadioInterface	();

	int32_t		get_selectedFrequency	();
	int32_t		get_centerFrequency	();
private:
        RingBuffer<std::complex<float> > inputData;

	struct band {
           int32_t      lowF;
           int32_t      highF;
           int32_t      currentOffset;
        }theBand;

	QTableWidget	*tableWidget;
	shifter		hfShifter;
	fftFilter	hfFilter;
	bandPlan	my_bandPlan;
	presetHandler	thePresets;
	decimator	theDecimator;
	QTimer		secondsTimer;

	int32_t		centerFrequency;
	int32_t		selectedFrequency;
	QSettings       *settings;
        int32_t         inputRate;
	int32_t		scopeWidth;
        int16_t         displaySize;
        int16_t         spectrumSize;
        double          *displayBuffer;
        deviceHandler	*theDevice;
	ft8_Decoder	*theDecoder;
        fftScope        *hfScope;
	int16_t		mouseIncrement;
	int16_t		delayCount;
	void		set_inMiddle	();
//
        keyPad          *mykeyPad;

	QStandardItemModel	model;
	QStringList	theResults;
	int		teller;
	std::atomic<int>	spectrumWidth;

	reporterWriter	*theWriter;
	bool		pskReady;
	std::mutex	locker;
	std::atomic<FILE *> filePointer;
public	slots:
	void		printLine		(const QString &,
	                                         int, int,
	                                         const QString &, int);
	void		addMessage		(const QString &,
	                                         const QString &,
	                                         int, int);
private slots:
        deviceHandler	*getDevice		(const QString &,
	                                         QSettings *,
	                                      RingBuffer<std::complex<float>> *);
        deviceHandler	*setDevice		(QSettings *,
	                                      RingBuffer<std::complex<float>> *);
        void            adjustFrequency_hz	(int);
        void            adjustFrequency_khz	(int);
        void            set_hfscopeLevel	(int);
        void            handle_freqButton       ();
        void            wheelEvent              (QWheelEvent *);
	void		set_mouseIncrement	(int);
	void		set_freqSave		();
	void		handle_quitButton	();
	void		switch_hfViewMode	(int);
	void		updateTime		();
	void		closeEvent		(QCloseEvent *event);
	void		handle_cq_selector	(const QString &);
	void		handle_identityButton	();
	void		set_spectrumWidth	(int);
	void		set_maxIterations	(int);

	void		handle_filesaveButton	();
	void		handle_pskReporterButton	();

	void		handle_presetButton	();
public slots:
	void		setFrequency		(int32_t);
	void		quickStart		();
	void		sampleHandler		(int amount);
	void		show_pskStatus		(bool);
	void		sendMessages		();
	void		handle_preset		(const QString &);
};

