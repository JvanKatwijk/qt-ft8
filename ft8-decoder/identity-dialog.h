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

#include	<QDialog>
#include	<QDialogButtonBox>
#include	<QMessageBox>
#include	<QLabel>
#include	<QLineEdit>
#include	<QGridLayout>
#include	<QSettings>

class	identityDialog: public QDialog {
public:
		identityDialog	(QSettings *, QWidget *parent = nullptr);
		~identityDialog	();
private slots:
	QSettings	*ft8Settings;
	void		verify	();
	void		reject	();

	QLabel		*callsignLabel;
	QLineEdit	*callsign;
	QLabel		*gridLabel;
	QLineEdit	*homeGrid;
	QLabel		*antennaLabel;
	QLineEdit	*antenna;
	QDialogButtonBox *buttonBox;

};


