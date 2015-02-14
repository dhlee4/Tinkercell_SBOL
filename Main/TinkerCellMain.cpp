/****************************************************************************

 Copyright (C) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYWRITE.TXT

 This is the main application file for Tinkercell. It constructs a MainWindow
 and loads a list of default plugins.

****************************************************************************/

#include "GlobalSettings.h"
#include "PythonTool.h"
#include "OctaveTool.h"
#include "RubyTool.h"
#include "ParametersTool.h"
#include "StoichiometryTool.h"
#include "FunctionDeclarationsTool.h"
#include "ModelSummaryTool.h"
#include "ModelSummaryWidget.h"
#include "ModelFileGenerator.h"
#include "SimulationEventTool.h"
#include "ModuleTool.h"
#include "WetLabTool.h"
#include "AutoGeneRegulatoryTool.h"
#include "DNASequenceTool.h"
#include "ViewTablesTool.h"
#include "LPSolveInput.h"
#include "CellPositionUpdatingTool.h"
#include "OctaveExporter.h"
#include "EnglishExporter.h"
#include "AntimonyEditor.h"
#include "SBMLImportExport.h"
#include "CopasiExporter.h"
#include "TinkerCellAboutBox.h"
#include "CollisionDetection.h"
#include "ConnectionInsertion.h"
#include "ConnectionMaker.h"
#include "NodeInsertion.h"
#include "GraphicsReplaceTool.h"
#include "GraphicsTransformTool.h"
#include "GroupHandlerTool.h"
#include "NameFamilyDialog.h"
#include "ConnectionSelection.h"
#include "NodeSelection.h"
#include "GraphicsScene.h"
#include "ConsoleWindow.h"
#include "ContainerTool.h"
<<<<<<< HEAD
#include "SBOLTool.h"
=======
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
#include <QColor>
#include <QBrush>

using namespace Tinkercell;

void LoadPluginsFromDir(const QString&,MainWindow *,QSplashScreen*);

#if defined(Q_WS_WIN) && !defined(MINGW)
int WinMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	//these two lines are required by all QT apps
    QApplication::setColorSpec (QApplication::ManyColor);
    QApplication app(argc, argv);
<<<<<<< HEAD

    /*****************************************
=======
    
    /*****************************************	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
           Optional configuration
    (must be done before creating MainWindow)
    ******************************************/

	GlobalSettings::ENABLE_HISTORY_WINDOW = true;
	GlobalSettings::ENABLE_CONSOLE_WINDOW = true;
	GlobalSettings::ENABLE_GRAPHING_TOOLS = true;
	GlobalSettings::ENABLE_CODING_TOOLS = true;
	GlobalSettings::ENABLE_PYTHON = true;
	GlobalSettings::ENABLE_RUBY = true;
	GlobalSettings::ENABLE_OCTAVE = true;
	GlobalSettings::ENABLE_LOADSAVE_TOOL = true;
<<<<<<< HEAD

=======
    
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
    GlobalSettings::PROJECTWEBSITE = QObject::tr("http://www.tinkercell.com");
    GlobalSettings::ORGANIZATIONNAME = QObject::tr("TinkerCell");
    GlobalSettings::PROJECTNAME = QObject::tr("TinkerCell");
    GlobalSettings::PLUGINS_SVN_URL = QObject::tr("https://tinkercellextra.svn.sourceforge.net/svnroot/tinkercellextra"); //for updating
   	GlobalSettings::SETTINGS_FILE_NAME = QString("settings.ini"); //for distributing TinkerCell in USB drive with the settings
<<<<<<< HEAD

    ConsoleWindow::Prompt = QObject::tr(">");
	ConsoleWindow::BackgroundColor = QColor("#555555");

=======
	
    ConsoleWindow::Prompt = QObject::tr(">");
	ConsoleWindow::BackgroundColor = QColor("#555555");
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	QColor color("#00EE00");
	color.setAlpha(50);
	GraphicsScene::SelectionRectangleBrush = QBrush(color);

<<<<<<< HEAD
    QString appDir = QCoreApplication::applicationDirPath();
=======
    QString appDir = QCoreApplication::applicationDirPath();	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
    QFile styleFile(appDir + QString("/tinkercell.qss"));

    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
<<<<<<< HEAD

    /*******  Main Window ***********/

	MainWindow mainWindow;
	mainWindow.hide();

  /*******  title , etc ***********/
    mainWindow.setWindowTitle(QObject::tr("TinkerCell"));
    mainWindow.statusBar()->showMessage(QObject::tr("Welcome to TinkerCell"));

	//setup the tabs
	CatalogWidget::tabGroups
=======
    
    /*******  Main Window ***********/
    
	MainWindow mainWindow;
	mainWindow.hide();
  
  /*******  title , etc ***********/
    mainWindow.setWindowTitle(QObject::tr("TinkerCell"));
    mainWindow.statusBar()->showMessage(QObject::tr("Welcome to TinkerCell"));
    
	//setup the tabs
	CatalogWidget::tabGroups	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
					<< QPair<QString, QStringList>(
													"Molecules",
													QStringList() << "molecule")
					<< QPair<QString, QStringList>(
													"Parts",
													QStringList() << "part")
					<< QPair<QString, QStringList>(
													"Compartments",
													QStringList() << "compartment")
                   << QPair<QString, QStringList>(
													"Regulation",
													QStringList()  << "regulation")
					<< QPair<QString, QStringList>(
													"Reaction",
													QStringList()  << "1 to 1"  << "1 to 2"  << "1 to 3"
																		  << "2 to 1" << "2 to 2" << "2 to 3"
<<<<<<< HEAD
																		  << "3 to 1" << "3 to 2" << "3 to 3")
                    << QPair<QString, QStringList>(
                                                   "SBOL",
                                                   QStringList() << "a" << "b" << "c");

=======
																		  << "3 to 1" << "3 to 2" << "3 to 3");
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

	/****  load plugins  ******/
	mainWindow.addTool(new CatalogWidget);
	mainWindow.addTool(new CollisionDetection);
	mainWindow.addTool(new ConnectionInsertion);
	mainWindow.addTool(new NodeInsertion);
	mainWindow.addTool(new NodeSelection);
	mainWindow.addTool(new ConnectionSelection);
	mainWindow.addTool(new TinkercellAboutBox);
	mainWindow.addTool(new GraphicsReplaceTool);
	mainWindow.addTool(new GraphicsTransformTool);
	mainWindow.addTool(new GroupHandlerTool);
	mainWindow.addTool(new NameFamilyDialog);
<<<<<<< HEAD
	mainWindow.addTool(new ConnectionMaker);
=======
	mainWindow.addTool(new ConnectionMaker);	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	mainWindow.addTool(new ParametersTool(QString("text")));
	mainWindow.addTool(new ParametersTool(QString("numerical")));
	mainWindow.addTool(new AssignmentFunctionsTool);
	mainWindow.addTool(new StoichiometryTool);
	mainWindow.addTool(new ModelSummaryTool);
	mainWindow.addTool(new ModelFileGenerator);
<<<<<<< HEAD
	mainWindow.addTool(new SimulationEventsTool);
	mainWindow.addTool(new SBOLTool);
	mainWindow.addTool(new ModelSummaryWidget);
	mainWindow.addTool(new ContainerTool);
=======
	mainWindow.addTool(new SimulationEventsTool);	
	mainWindow.addTool(new ModelSummaryWidget);
	mainWindow.addTool(new ContainerTool);	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	mainWindow.addTool(new OctaveExporter);
	mainWindow.addTool(new SBMLImportExport);
	mainWindow.addTool(new CopasiExporter);
	mainWindow.addTool(new AntimonyEditor);
<<<<<<< HEAD
	mainWindow.addTool(new EnglishExporter);
=======
	mainWindow.addTool(new EnglishExporter);	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	mainWindow.addTool(new ModuleTool);
	mainWindow.addTool(new WetLabTool);
	mainWindow.addTool(new AutoGeneRegulatoryTool);
	mainWindow.addTool(new DNASequenceViewer);
	mainWindow.addTool(new ViewTablesTool);
	mainWindow.addTool(new LPSolveInputWindow);
	mainWindow.addTool(new CellPositionUpdateTool);

	/********* load python, octave, ruby plugins ***********/
	if (mainWindow.tool("Python Interpreter"))
	{
		PythonTool * tool = (PythonTool*)mainWindow.tool("Python Interpreter");
		tool->loadFilesInDir("python");
	}

	if (mainWindow.tool("Octave Interpreter"))
	{
		OctaveTool * tool = (OctaveTool*)mainWindow.tool("Octave Interpreter");
		tool->loadFilesInDir("octave");
	}

	if (mainWindow.tool("Ruby Interpreter"))
	{
		RubyTool * tool = (RubyTool*)mainWindow.tool("Ruby Interpreter");
		tool->loadFilesInDir("ruby");
	}
    /*******  Splash screen ***********/

    QString splashFile(":/images/Tinkercell.png");
	QPixmap pixmap(splashFile);
	QSplashScreen splash(pixmap,Qt::SplashScreen);//|Qt::WindowStaysOnTopHint);
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
    QSize sz = mainWindow.size();
    QPoint pos = mainWindow.pos();
    splash.move(pos + QPoint(sz.width()-pixmap.width(), sz.height()-pixmap.height())/2 );

    splash.setWindowOpacity(0.75);

    splash.show();
    splash.showMessage(QString("loading ..."));

<<<<<<< HEAD
    /*******  Dynamically load plugins from folders ***********/
=======
    /*******  Dynamically load plugins from folders ***********/    
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	mainWindow.setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowNestedDocks);

    QString home = GlobalSettings::homeDir();

    LoadPluginsFromDir(appDir + QString("/") + QString(TINKERCELL_CPP_PLUGINS_FOLDER),&mainWindow, &splash);
	LoadPluginsFromDir(home + QString("/") + QString(TINKERCELL_CPP_PLUGINS_FOLDER),&mainWindow, &splash);

	LoadPluginsFromDir(appDir + QString("/") + QString(TINKERCELL_C_PLUGINS_FOLDER),&mainWindow, &splash);
	LoadPluginsFromDir(home + QString("/") +  QString(TINKERCELL_C_PLUGINS_FOLDER),&mainWindow, &splash);
<<<<<<< HEAD


    /*******  START TINKERCELL ***********/

=======
	

    /*******  START TINKERCELL ***********/
    
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
    mainWindow.readSettings();
	GraphicsScene * scene = mainWindow.newScene();
	mainWindow.show();
    splash.finish(&mainWindow);

    /*******  process command line arguments, if any ***********/
	if (argc > 1)
		for (int i=1; i < argc; ++i)
		{
			if (mainWindow.console())
				mainWindow.console()->eval(QString(argv[i]));
			else
				mainWindow.open(QString(argv[i]));
		}

    /* main loop */
<<<<<<< HEAD
    int output = app.exec();
=======
    int output = app.exec();	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	app.closeAllWindows();
    return output;
}

void LoadPluginsFromDir(const QString& dirname,MainWindow * main,QSplashScreen * splash)
{
    QDir dir(dirname);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Time);
    QFileInfoList list = dir.entryInfoList();

    for (int i = (list.size()-1); i >= 0; --i)
    {
        QFileInfo fileInfo = list.at(i);
        QString filename = fileInfo.absoluteFilePath();
        if (!QLibrary::isLibrary(filename)) continue;

        if (splash)
            splash->showMessage(QString("loading ") + fileInfo.fileName() + QString("..."));
        main->loadDynamicLibrary(filename);
    }
}

