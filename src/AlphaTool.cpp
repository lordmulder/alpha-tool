///////////////////////////////////////////////////////////////////////////////
// Alpha Tool - compute alpha-channel from diff values
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>. Some Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// http://www.gnu.org/licenses/gpl-2.0.txt
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <QCoreApplication>
#include <QStringList>
#include <QImage>
#include <QLibraryInfo>

#define DIFF(X,Y) (((X)>(Y))?((X)-(Y)):((Y)-(X)))
#define AVRG(X,Y) qRound((double((X))+double((Y)))/2.0)

//========================================================================
// TYPES
//========================================================================

typedef struct
{
	int r; int g; int b;
}
pixval_t;

typedef int (*mix_func_t)(const pixval_t &p);

//========================================================================
// MIX FUNCTIONS
//========================================================================

static int mix_average(const pixval_t &p)
{
	return qBound(0, qRound((double(p.r) + double(p.g) + double(p.b)) / 3.0), 255);
}

static int mix_luminosity(const pixval_t &p)
{
	return qBound(0, qRound((0.21 * double(p.r)) + (0.72 * double(p.g)) + (0.07 * double(p.b))), 255);
}

static int mix_lightness(const pixval_t &p)
{
	return qBound(0, qRound((qMax(qMax(p.r,p.g),p.b) +  qMin(qMin(p.r,p.g),p.b)) / 2.0), 255);
}

static const struct
{
	const wchar_t *const name;
	const mix_func_t ptr;
}
MIX_MODE[] =
{
	{ L"average",    &mix_average    },
	{ L"luminosity", &mix_luminosity },
	{ L"lightness",  &mix_lightness  },
	{ NULL, NULL }
};

//========================================================================
// UTILITY FUNCTIONS
//========================================================================

static void print_logo(void)
{
	std::cerr << "Alpha Tool - compute alpha-channel from diff values [" __DATE__ "]"          << std::endl;
	std::cerr << "Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>. Some Rights Reserved.\n"    << std::endl;
	std::cerr << "This program is free software: you can redistribute it and/or modify"        << std::endl;
	std::cerr << "it under the terms of the GNU General Public License <http://www.gnu.org/>." << std::endl;
	std::cerr << "Note that this program is distributed with ABSOLUTELY NO WARRANTY.\n"        << std::endl;
}

static QCoreApplication *initialize_qt(void)
{
	int qt_argc = 1; char *qt_agrv[] = { "AlphaTool.exe" };
	QCoreApplication *application = new QCoreApplication(qt_argc, qt_agrv);
	if(application)
	{
		std::wcerr << L"Using Qt Framework v" << qVersion();
		std::wcerr << L", " << (const wchar_t*) QLibraryInfo::buildDate().toString(Qt::ISODate).constData();
		std::wcerr << L" [" << (const wchar_t*) QLibraryInfo::buildKey().constData() << L"]\n" << std::endl;
		application->setLibraryPaths(QStringList() << QCoreApplication::applicationDirPath());
	}
	return application;
}

static QImage *load_image(const wchar_t *const fileName)
{
	QScopedPointer<QImage> image(new QImage(QString::fromUtf16((const ushort*)fileName)));
	if(image.isNull() || image->isNull())
	{
		std::cerr << "\nError: Faild to read input file #1!\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	if((image->format() != QImage::Format_RGB32) && (image->format() != QImage::Format_ARGB32))
	{
		std::cerr << "\nError: Input file is not in 24/32-Bit format!\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	return image.take();
}

static inline pixval_t getPixVal(const QRgb *const line, const int offset)
{
	const pixval_t ret =
	{
		qRed(line[offset]), qGreen(line[offset]), qBlue(line[offset])
	};
	return ret;
}

static inline pixval_t average(const pixval_t &p1, const pixval_t &p2)
{
	const pixval_t ret =
	{
		AVRG(p1.r, p2.r), AVRG(p1.g, p2.g), AVRG(p1.b, p2.b)
	};
	return ret;
}

static inline void updateBounds(const int &x, const int &y, int *bound_x, int *bound_y)
{
	if(x < bound_x[0]) bound_x[0] = x;
	if(x > bound_x[1]) bound_x[1] = x;
	if(y < bound_y[0]) bound_y[0] = y;
	if(y > bound_y[1]) bound_y[1] = y;
}

static void print_status(const int &val, const int &max)
{
	std::cerr << '\r' << (val) << '/' << max << " (" << qRound(100.0*(double(val)/double(max))) << "%)" << std::flush;
}

//========================================================================
// MAIN
//========================================================================

static int alpha_main(int argc, wchar_t* argv[])
{
	print_logo();

	QScopedPointer<QCoreApplication> application(initialize_qt());
	if(application.isNull())
	{
		std::cerr << "Failed to initialized Qt!\n" << std::endl;
		return EXIT_FAILURE;
	}

	if(argc < 4)
	{
		std::cerr << "Usage:\n   AlphaTool.exe <in_1.png> <in_2.png> <out.png> [<mix_mode>]\n" << std::endl;
		std::cerr << "Mix Modes:\n   " << std::flush;
		for(size_t i = 0; MIX_MODE[i].name && MIX_MODE[i].ptr; i++)
		{
			if(i) std::cerr << ", " << std::flush;
			std::wcerr << MIX_MODE[i].name << std::flush;
		}
		std::cerr << "\n" << std::endl;
		return EXIT_FAILURE;
	}

	//-------------------------------------------------------------------------
	// Setup mix function
	//-------------------------------------------------------------------------

	mix_func_t mix_func = NULL;
	
	if(argc > 4)
	{
		for(size_t i = 0; MIX_MODE[i].name && MIX_MODE[i].ptr; i++)
		{
			if(!_wcsicmp(MIX_MODE[i].name, argv[4]))
			{
				mix_func = MIX_MODE[i].ptr;
				break;
			}
		}
		if(!mix_func)
		{
			std::cerr << "Invalid mixing mode has been specified!\n" << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		mix_func = &mix_luminosity;
	}

	//-------------------------------------------------------------------------
	// Load imag files
	//-------------------------------------------------------------------------

	std::cerr << "Loding input images..." << std::endl;

	QScopedPointer<QImage> input[2];
	input[0].reset(load_image(argv[1]));
	input[1].reset(load_image(argv[2]));

	std::cerr << "Okay.\n\nImage size: " << input[0]->width() << " x " << input[0]->height() << '\n' << std::endl;

	if(input[0]->size() != input[1]->size())
	{
		std::cerr << "Error: Input files don't match in size!\n" << std::endl;
		return EXIT_FAILURE;
	}

	//-------------------------------------------------------------------------
	// Compare files
	//-------------------------------------------------------------------------

	std::cerr << "Processing image, please be patient:" << std::endl;

	const QSize size = input[0]->size();
	int bound_x[2] = { size.width()  - 1, 0 };
	int bound_y[2] = { size.height() - 1, 0 };

	QScopedPointer<QImage> output(new QImage(size, QImage::Format_ARGB32));

	for(int y = 0; y < size.height(); y++)
	{
		print_status(y, size.height());

		QRgb *outPtr = (QRgb*) output->scanLine(y);
		const QRgb *line1 = (const QRgb*) input[0]->constScanLine(y);
		const QRgb *line2 = (const QRgb*) input[1]->constScanLine(y);

		for(int x = 0; x < size.width(); x++)
		{
			const pixval_t p1 = getPixVal(line1, x);
			const pixval_t p2 = getPixVal(line2, x);

			const pixval_t avg = average(p1, p2);
			const int alpha = 255 - DIFF(mix_func(p1), mix_func(p2));
			outPtr[x] = qRgba(avg.r, avg.g, avg.b, alpha);

			//Update bound values
			if(alpha > 0)
			{
				updateBounds(x, y, bound_x, bound_y);
			}
		}
	}

	print_status(size.height(), size.height());
	std::cerr << '\n' << std::endl;

	//-------------------------------------------------------------------------
	// Auto Cropping
	//-------------------------------------------------------------------------

	std::cerr << "Image bounds: " << std::flush;
	std::cerr << "x(" << bound_x[0] << "," << bound_x[1] << ") "  << std::flush;
	std::cerr << "y(" << bound_y[0] << "," << bound_y[1] << ")\n" << std::flush;

	if((bound_x[0] > 0) || (bound_y[0] > 0) || (bound_x[1] < size.width()-1) || (bound_y[1] < size.height()-1))
	{
		const int cropped_w = bound_x[1] + 1 - bound_x[0];
		const int cropped_h = bound_y[1] + 1 - bound_y[0];
		std::cerr << "Cropped size: " << cropped_w << " x " << cropped_h << '\n' << std::flush;
		(*output) = output->copy(bound_x[0], bound_y[0], cropped_w, cropped_h);
	}

	std::cerr << std::endl;

	//-------------------------------------------------------------------------
	// Save result
	//-------------------------------------------------------------------------

	std::cerr << "Saving output image..." << std::endl;

	if(!output->save(QString::fromUtf16((const ushort*)argv[3]), "PNG"))
	{
		std::cerr << "Error: Failed to save output file!\n" << std::endl;
		return EXIT_FAILURE;
	}

	std::cerr << "Okay.\n\nCompleted.\n" << std::endl;
	return EXIT_SUCCESS;
}

//========================================================================
// MAIN
//========================================================================

int wmain(int argc, wchar_t* argv[])
{
	int ret = -1;

	__try
	{
		ret = alpha_main(argc, argv);
	}
	__except(1)
	{
		fprintf(stderr, "\n\nUNHANDELED EXCEPTION ERROR !!!\n\n");
		fflush(stderr);
		_exit(-1);
	}

	return ret;
}
