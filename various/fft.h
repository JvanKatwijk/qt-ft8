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

//
//	Simple wrapper around fftw
#include	"radio-constants.h"
#include	<complex>
#include	<stdint.h>
//
#define	FFTW_MALLOC		fftwf_malloc
#define	FFTW_PLAN_DFT_1D	fftwf_plan_dft_1d
#define FFTW_DESTROY_PLAN	fftwf_destroy_plan
#define	FFTW_FREE		fftwf_free
#define	FFTW_PLAN		fftwf_plan
#define	FFTW_EXECUTE		fftwf_execute
#include	<fftw3.h>
/*
 *	a simple wrapper
 */

class	fft_handler {
public:
			fft_handler	(int32_t);
			~fft_handler	();
	void		do_FFT		(std::complex<float> *);
private:
	int32_t		fftSize;
	FFTW_PLAN	plan;
	std::complex<float>	*vector;
};

class	ifft_handler {
public:
			ifft_handler	(int32_t);
			~ifft_handler	();
	void		do_IFFT		(std::complex<float> *);
private:
	int		fftSize;
	FFTW_PLAN	plan;
	std::complex<float>	*vector;
	void		Scale		(std::complex<float> *);
};
	
