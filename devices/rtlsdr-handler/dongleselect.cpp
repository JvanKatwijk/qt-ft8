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

#include	"dongleselect.h"
#include	<stdio.h>
#include	<QVBoxLayout>

	dongleSelect::dongleSelect () {
	toptext		= new QLabel (this);
	toptext	-> setText ("Select a dongle");
	selectorDisplay	= new QListView (this);
	QVBoxLayout	*layOut = new QVBoxLayout;
	layOut	-> addWidget (selectorDisplay);
	layOut	-> addWidget (toptext);
	setWindowTitle (tr("dongle select"));
	setLayout (layOut);

	dongleList. setStringList (Dongles);
	Dongles = QStringList ();
	dongleList. setStringList (Dongles);
	selectorDisplay	-> setModel (&dongleList);
	connect (selectorDisplay, SIGNAL (clicked (QModelIndex)),
	         this, SLOT (selectDongle (QModelIndex)));
	selectedItem	= -1;
}

	dongleSelect::~dongleSelect () {
}

void	dongleSelect::addtoDongleList (const char *v) {
QString s (v);

	Dongles << s;
	dongleList. setStringList (Dongles);
	selectorDisplay	-> setModel (&dongleList);
	selectorDisplay	-> adjustSize ();
	adjustSize ();
}

void	dongleSelect::selectDongle (QModelIndex s) {
	fprintf (stderr, "we have chosen %d\n", s.row ());
	selectedItem = s. row ();
	QDialog::done (s. row ());
//	close ();
}

int16_t	dongleSelect::getSelectedItem	() {
	return selectedItem;
}

