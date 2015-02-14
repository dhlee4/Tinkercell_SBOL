/****************************************************************************
Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
see COPYRIGHT.TXT

****************************************************************************/
#include <QVBoxLayout>
#include <QDockWidget>
#include <QProcess>
#include <QCompleter>
#include <QStringListModel>
#include <QLibrary>
#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include "GraphicsScene.h"
#include "MainWindow.h"
#include "ConsoleWindow.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "PythonTool.h"
#include <QtDebug>
#include "GlobalSettings.h"

namespace Tinkercell
{
	PythonTool::PythonTool() : Tool(tr("Python Interpreter"),tr("Coding")), actionsGroup(this), buttonsGroup(this)
	{
		PythonTool::fToS = new PythonTool_FToS;
		PythonTool::fToS->setParent(this);
		pythonInterpreter = 0;

		connect(&actionsGroup,SIGNAL(triggered ( QAction *  )),this,SLOT(actionTriggered ( QAction *  )));
		connect(&buttonsGroup,SIGNAL(buttonPressed ( int  )),this,SLOT(buttonPressed ( int  )));
		connectTCFunctions();
	}

	bool PythonTool::loadFilesInDir(const QString& dirname)
	{
		QString name[] = {
			dirname, 
			GlobalSettings::tempDir() + tr("/") + dirname,
			GlobalSettings::homeDir() + tr("/") + dirname,
			QDir::currentPath() + tr("/") + dirname,
			QCoreApplication::applicationDirPath() + tr("/") + dirname
		};

		QWidget * widget = mainWindow->tool(tr("Dynamic Library Menu"));
		bool opened = false;
		if (widget)
		{
			DynamicLibraryMenu * libMenu = static_cast<DynamicLibraryMenu*>(widget);
			for (int i=0; i < 5; ++i)
			{
				QDir dir(name[i]);
				if (dir.exists())
				{
					if (loadFilesInDir(libMenu, dir))
						opened = true;
				}
			}
		}
		return opened;
	}

	bool PythonTool::loadFile(const QString& filename)
	{
		if (!mainWindow) return false;

		QWidget * widget = mainWindow->tool(tr("Dynamic Library Menu"));
		if (widget)
		{
			DynamicLibraryMenu * libMenu = static_cast<DynamicLibraryMenu*>(widget);
			return loadFile(libMenu, QFileInfo(filename));
		}
		return false;
	}

	bool PythonTool::loadFile(DynamicLibraryMenu * libMenu, const QFileInfo& fileInfo)
	{
		if (!libMenu) return false;

		QString pyFile = fileInfo.absoluteFilePath();

		if (pyFileNames.contains(pyFile)) return false;

		QFile file(pyFile);

		if (!file.open(QFile::ReadOnly | QFile::Text)) return false;

		if (fileInfo.completeSuffix().toLower() != tr("py")) return false;

		QString category, name, descr, icon, specific;
		bool menu = true, tool = true;
		bool startedParsing = false;

		while (!file.atEnd()) //inside python script file
		{
			QString line(file.readLine());
			if (line.toLower().contains(tr("\"\"\"")))
				if (startedParsing)
					break;
				else
					startedParsing = true;

			if (!startedParsing) continue;

			QStringList words = line.split(tr(":"));
			if (words.size() == 2)
			{
				QString s1 = words[0].remove(tr("#")).trimmed(), 
					s2 = words[1].trimmed();

				if (s1 == tr("category"))
					category = s2;
				else
					if (s1 == tr("name"))
						name = s2;
					else
						if (s1 == tr("description"))
							descr = s2;
						else
							if (s1 == tr("icon"))
								icon = s2;
							else
								if (s1 == tr("menu"))
									menu = (s2.toLower().contains("yes"));
								else
									if (s1 == tr("tool"))
										tool = (s2.toLower().contains("yes"));
									else
										if (s1.contains(tr("specific")))
											specific = s2;
			}
		}

		file.close();

		if (name.isNull() || name.isEmpty()) return false;

		QString appDir = QCoreApplication::applicationDirPath();
		QString homeDir = GlobalSettings::homeDir();

		if (!icon.isEmpty() && !QFile(icon).exists())
		{
			if (QFile(appDir + tr("/") + icon).exists())
				icon = appDir + tr("/") + icon;
			else
				if (QFile(tr(":/images/") + icon).exists())
					icon = tr(":/images/") + icon;
				else
					if (QFile(homeDir + tr("/") + icon).exists())
						icon = homeDir + tr("/") + icon;
					else
						if (QFile(homeDir + tr("/python/") + icon).exists())
							icon = homeDir + tr("/python/") + icon;
		}

		if (icon.isEmpty() || !QFile(icon).exists())
			icon = tr(":/images/function.png");
		QPixmap pixmap(icon);

		QToolButton * button = libMenu->addFunction(category, name, QIcon(pixmap));

		if (button)
		{
			button->setToolTip(descr);
			buttonsGroup.addButton(button,pyFileNames.size());
			pyFileNames << pyFile;
		}

		if (menu)
		{
			QAction * menuItem = libMenu->addMenuItem(category, name, QIcon(pixmap));
			if (menuItem)
			{
				menuItem->setToolTip(descr);
				actionsGroup.addAction(menuItem);
				hashPyFile[menuItem] = pyFile;
			}
		}

		if (!specific.isEmpty())
		{
			QAction * contextAction = libMenu->addContextMenuItem(specific, name, pixmap, tool);
			if (contextAction)
			{
				contextAction->setToolTip(descr);
				actionsGroup.addAction(contextAction);
				hashPyFile[contextAction] = pyFile;
			}
		}
		return true;
	}

	bool PythonTool::loadFilesInDir(DynamicLibraryMenu * libMenu, QDir& dir)
	{
		bool filesFound = false;
		if (!libMenu) return filesFound;

		dir.setFilter(QDir::Files);
		dir.setSorting(QDir::Name);

		QFileInfoList list = dir.entryInfoList();

		for (int i = 0; i < list.size(); ++i)
		{
			QFileInfo fileInfo = list.at(i);
			if (loadFile(libMenu, fileInfo))
				filesFound = true;
		} //done reading one py script file

		return filesFound;
	}

	bool PythonTool::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);
		if (mainWindow)
		{
			QString appDir = QCoreApplication::applicationDirPath();
			pythonInterpreter = new PythonInterpreterThread(tr("python/_tinkercell"), mainWindow);
			pythonInterpreter->initialize();

			connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));

			if (console())
			{
				if (!(pythonInterpreter->library() && pythonInterpreter->library()->isLoaded()))
					console()->message(tr("Python not loaded\n"));
			}

			QFile file(appDir + tr("/python/init.py"));
			QString s;
			if (file.open(QFile::ReadOnly | QFile::Text))
			{
				s = file.readAll();
				file.close();
			}

			if (!s.contains(tr("from tinkercell import *")))
				s = tr("from tinkercell import *\n") + s;

			runPythonCode(s);

			ConsoleWindow * outWin = console();
			if (outWin)
			{
				if (!outWin->lastError().isEmpty())
					QDesktopServices::openUrl(QUrl(appDir + tr("/python/init.py")));
				if (!outWin->interpreter() && pythonInterpreter)
					outWin->setInterpreter(pythonInterpreter);
			}

			return true;
		}

		return false;
	}

	void PythonTool::buttonPressed ( int id )
	{
		if (pyFileNames.size() <= id)
			return;

		QString pyfile = pyFileNames[id];

		if (!pyfile.isEmpty())
		{
			runPythonFile(pyfile); //go
		}
	}

	void PythonTool::actionTriggered(QAction * item)
	{
		if (!item || !hashPyFile.contains(item))
			return;

		QString pyfile = hashPyFile[item];

		if (!pyfile.isEmpty())
		{
			runPythonFile(pyfile); //go
		}
	}

	void PythonTool::connectTCFunctions()
	{
		connect(fToS,SIGNAL(runPythonCode(QSemaphore*,const QString&)),this,SLOT(runPythonCode(QSemaphore*,const QString&)));
		connect(fToS,SIGNAL(runPythonFile(QSemaphore*,const QString&)),this,SLOT(runPythonFile(QSemaphore*,const QString&)));
		connect(fToS,SIGNAL(addPythonPlugin(QSemaphore*,const QString&,const QString&,const QString&,const QString&, const QString&)),
			this,SLOT(addPythonPlugin(QSemaphore*,const QString&,const QString&,const QString&,const QString&, const QString&)));
	}

	typedef void (*tc_PythonTool_api)(
		void (*runPythonCode)(const char*),
		void (*runPythonFile)(const char*),
		void (*addPythonPlugin)(const char*,const char*,const char*,const char*,const char*)
		);

	void PythonTool::setupFunctionPointers( QLibrary * library)
	{
		tc_PythonTool_api f = (tc_PythonTool_api)library->resolve("tc_PythonTool_api");
		if (f)
		{
			//qDebug() << "tc_PythonTool_api resolved";
			f(
				&(_runPythonCode),
				&(_runPythonFile),
				&(_addPythonPlugin)
				);
		}
	}

	/******************************************************/

	PythonTool_FToS * PythonTool::fToS;


	void PythonTool::_runPythonCode(const char* c)
	{
		return fToS->runPythonCode(c);
	}

	void PythonTool::_runPythonFile(const char* c)
	{
		return fToS->runPythonFile(c);
	}

	void PythonTool::_addPythonPlugin(const char* file,const char* name,const char* descr,const char* category,const char* icon)
	{
		return fToS->addPythonPlugin(file,name,descr,category,icon);
	}

	void PythonTool_FToS::runPythonCode(const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit runPythonCode(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	void PythonTool_FToS::runPythonFile(const char* c)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit runPythonFile(s,ConvertValue(c));
		s->acquire();
		s->release();
		delete s;
	}

	void PythonTool_FToS::addPythonPlugin(const char* file,const char* name,const char* descr,const char* category,const char* icon)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit addPythonPlugin(s,tr(file),tr(name),tr(descr),tr(category),tr(icon));
		s->acquire();
		s->release();
		delete s;
	}

	/*******************
	PYTHON STUFF
	*********************/

	void PythonTool::runPythonCode(QSemaphore* sem,const QString& code)
	{
		runPythonCode(code);
		if (sem)
			sem->release();
	}

	void PythonTool::runPythonFile(QSemaphore* sem,const QString& file)
	{
		runPythonFile(file);
		if (sem)
			sem->release();
	}

	void PythonTool::addPythonPlugin(QSemaphore * sem,const QString& pyFile,const QString& name,const QString& descr,const QString& category, const QString& icon0)
	{
		QWidget * widget = mainWindow->tool(tr("Dynamic Library Menu"));

		if (!widget) return;

		DynamicLibraryMenu * libMenu = static_cast<DynamicLibraryMenu*>(widget);

		if (name.isNull() || name.isEmpty())
		{
			if (sem) 
				sem->release();
			return;
		}

		QString appDir = QCoreApplication::applicationDirPath();
		QString homeDir = GlobalSettings::homeDir();

		QString icon = icon0;

		if (!QFile(icon).exists())
		{
			if (QFile(appDir + tr("/") + icon).exists())
				icon = appDir + tr("/") + icon;
			else
				if (QFile(tr(":/images/") + icon).exists())
					icon = tr(":/images/") + icon;
				else
					if (QFile(homeDir + tr("/") + icon).exists())
						icon = homeDir + tr("/") + icon;
					else
						if (QFile(homeDir + tr("/python/") + icon).exists())
							icon = homeDir + tr("/python/") + icon;
		}

		if (icon.isEmpty() || !QFile(icon).exists())
			icon = tr(":/images/function.png");
		QPixmap pixmap(icon);

		QToolButton * button = libMenu->addFunction(category, name, QIcon(pixmap));

		if (button)
		{
			button->setToolTip(descr);
			buttonsGroup.addButton(button,pyFileNames.size());
			pyFileNames << pyFile;
		}

		QAction * menuItem = libMenu->addMenuItem(category, name, QIcon(pixmap));
		if (menuItem)
		{
			menuItem->setToolTip(descr);
			actionsGroup.addAction(menuItem);
			hashPyFile[menuItem] = pyFile;
		}

		/*
		if (!specific.isEmpty())
		{
		QAction * contextAction = libMenu->addContextMenuItem(specific, name, pixmap, tool);
		if (contextAction)
		{
		contextAction->setToolTip(descr);
		actionsGroup.addAction(contextAction);
		hashPyFile[contextAction] = pyFile;
		}
		}*/

		if (sem)
			sem->release();
	}

	void PythonTool::runPythonCode(const QString& code)
	{	
		if (pythonInterpreter)
			pythonInterpreter->exec(code);
	}

	void PythonTool::runPythonFile(const QString& filename)
	{
		if (pythonInterpreter)
		{
			QString appDir = QCoreApplication::applicationDirPath();

			QString name[] = {	GlobalSettings::homeDir() + tr("/") + filename,
				GlobalSettings::homeDir() + tr("/python/") + filename,
				GlobalSettings::tempDir() + tr("/") + filename,
				GlobalSettings::tempDir() + tr("/python/") + filename,
				filename,
				QDir::currentPath() + tr("/") + filename,
				appDir + tr("/python/") + filename ,
				appDir + tr("/") + filename };

			QFile file;
			bool opened = false;
			for (int i=0; i < 8; ++i)
			{
				file.setFileName(name[i]);
				if (file.open(QFile::ReadOnly | QFile::Text))
				{
					opened = true;
					break;
				}
			}
			if (!opened)
			{
				if (console())
					console()->error( filename + tr("file not found"));
			}
			else
			{
				QString code(file.readAll());
				runPythonCode(code);
				file.close();
			}
		}
	}

}
