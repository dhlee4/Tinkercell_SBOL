/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)

 see COPYRIGHT.TXT

 The Octave interpreter that runs as a separate thread and can accept strings to parse and execute
 
****************************************************************************/
#ifndef TINKERCELL_OCTAVEINTERPRETERTHREAD_H
#define TINKERCELL_OCTAVEINTERPRETERTHREAD_H

#include <QRegExp>
#include "InterpreterThread.h"


namespace Tinkercell
{
	/*! \brief This class is used to embed an octave interpreter inside a TinkerCell application.
	The C library responsible for embedding octave is called runOctave.cpp and is located
	inside the octave folder. The octave interpreter uses two libraries -- one for embedding octave in TinkerCell
	and another for extending Octave with the TinkerCell C API.
	\sa PythonInterpreterThread
	\ingroup CAPI
	*/
    class TINKERCELLCOREEXPORT OctaveInterpreterThread : public InterpreterThread
	{
		Q_OBJECT
		
		typedef void (*initFunc)();
		typedef int (*execFunc)(const char*,const char*,const char*);
		typedef void (*finalFunc)();
		
	public:
		/*! \brief initialize the thread that will embed and extend octave. 
		* \param QString folder where the two octave libraries are located
		* \param QString name of the octave embed library (e.g. tinkercell.oct)
		*/
		OctaveInterpreterThread(const QString&, const QString&, MainWindow* main);
		/*! \brief the folder where tinkercell will look for octave files, defaults to /octave*/
		static QString OCTAVE_FOLDER;
		/*! \brief the file where tinkercell will write outputs, defaults to tmp/octav.out*/
		static QString OUTPUT_FILE;
		/*! \brief the file where tinkercell will write errors, defaults to tmp/octav.err*/
		static QString ERROR_FILE;
		/*! \brief requests main window to load all the C pointers for the C API inside the embedded library
		*/
		virtual void setCPointers();
	
	public slots:
		virtual void initialize();
		virtual void finalize();
		virtual void toolLoaded(Tool*);

	protected:
		virtual void run();
		execFunc f;
		bool addpathDone;
		/*! \brief library with all the C API functions */
		QLibrary * swigLib;
		QRegExp fromTC;
		QRegExp toTC;
	};
}

#endif
