<<<<<<< HEAD
/****************************************************************************
 **
 ** Copyright (c) 2008 Deepak Chandran
 ** Contact: Deepak Chandran (dchandran1@gmail.com)
 **
 ** See COPYWRITE.TXT
 **
 ****************************************************************************/

//#include <vld.h>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

#include "SegmentationWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	SegmentationAnimation::MainWindow mainWin;
	mainWin.setGeometry(100,100,700,400);
	mainWin.show();
	app.exec();

	return 0;
}
=======
/****************************************************************************
 **
 ** Copyright (c) 2008 Deepak Chandran
 ** Contact: Deepak Chandran (dchandran1@gmail.com)
 **
 ** See COPYWRITE.TXT
 **
 ****************************************************************************/

//#include <vld.h>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

#include "SegmentationWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	SegmentationAnimation::MainWindow mainWin;
	mainWin.setGeometry(100,100,700,400);
	mainWin.show();
	app.exec();

	return 0;
}
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
