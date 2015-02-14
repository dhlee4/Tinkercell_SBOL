/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT

 This tool handles module connections that merge items from two modules

****************************************************************************/
#include <math.h>
#include <QRegExp>
#include <QProcess>
#include "ItemFamily.h"
#include "CloneItems.h"
#include "NetworkHandle.h"
#include "ItemHandle.h"
#include "GraphicsScene.h"
#include "TextEditor.h"
#include "ConsoleWindow.h"
#include "UndoCommands.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "NodeGraphicsReader.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "LoadSaveTool.h"
#include "CodingWindow.h"
#include "TreeButton.h"
#include "ModuleTool.h"
#include "GlobalSettings.h"
#include "Ontology.h"
#include <iostream>

namespace Tinkercell
{
	#define WINDOW_WIDTH 200
	static QString linkerFileName(":/images/moduleLinker.xml");
	static QString interfaceFileName(":/images/moduleInterface.xml");
	static QString moduleFileName(":/images/Module.xml");
	static QString linkerClassName("module linker item");
	static QString interfaceClassName("module interface item");
	static QString connectionClassName("module connection item");

    ModuleTool::ModuleTool() : Tool(tr("Module Connection Tool"),tr("Module tools")), 
    	newModuleDialog(0), newModuleTable(0), newModuleName(0), 
		connectionsTree(0), nodesTree(0), exportModuleDialog(0), 
		snapshotToolTip(0)
    {	
    	ModuleTool::fToS = new ModuleTool_FToS;
    	ModuleTool::fToS->setParent(this);
    	
        setPalette(QPalette(QColor(255,255,255,255)));
        setAutoFillBackground(true);
		connect(fToS, SIGNAL(doSubstituteModel(QSemaphore*, ItemHandle*, const QString&)),
						this, SLOT(doSubstituteModel(QSemaphore*, ItemHandle*, const QString&)));
						
		if (!textTablesToBeRemoved.contains(tr("Units")))
			textTablesToBeRemoved << tr("Units");

		if (!textTablesToBeRemoved.contains(tr("Rate equations")))
			textTablesToBeRemoved << tr("Rate equations");
    }

	//insert interface node
    void ModuleTool::select(int)
    {
    	GraphicsScene * scene = currentScene();
    	if (!scene) return;
    	
    	QList<QGraphicsItem*> & selected = scene->selected();
    	QList<QGraphicsItem*> itemsToInsert;
		ItemHandle * h = 0;
		NodeGraphicsItem * node;
		
		NodeGraphicsReader reader;
		
		QString appDir = QCoreApplication::applicationDirPath();

		for (int i=0; i < selected.size(); ++i)
			if ((h = getHandle(selected[i])) && NodeHandle::cast(h))
			{
				for (int j=0; j < h->graphicsItems.size(); ++j)
					if ((node = NodeGraphicsItem::cast(h->graphicsItems[j])) && node->className == interfaceClassName)
						continue;

				node = new NodeGraphicsItem(interfaceFileName);
				node->setPos(
					QPointF(selected[i]->sceneBoundingRect().left() - node->sceneBoundingRect().width()/2.0,selected[i]->scenePos().y())
				);
				node->className = interfaceClassName;
				node->setHandle(h);
				itemsToInsert << node;
			}

		if (!itemsToInsert.isEmpty())
		{
			if (scene->localHandle())
				scene->insert(scene->localHandle()->name + tr(" interface created"),itemsToInsert);
			else
			if (itemsToInsert.size() == 1)
				scene->insert(getHandle(itemsToInsert[0])->name + tr(" interface created"),itemsToInsert);
			else
				scene->insert(tr("Interfaces created"),itemsToInsert);
		}
    }
	
	QStringList ModuleTool::listOfModels(ItemFamily * family)
	{
		QStringList fileNames;
		if (!family)
			return fileNames;
		
		QList<ItemFamily*> childFamilies;
		childFamilies << family;// << family->allChildren();
		QString appDir = QCoreApplication::applicationDirPath();
		QString home = homeDir();
		
		//QString emptyModelFile = emptyModel();

		for (int i=0; i < childFamilies.size(); ++i)
		{
			QString s = childFamilies[i]->name();
			s.replace(tr(" "),tr("_"));
			QString dirname = home + tr("/Modules/") + s;
			QDir dir(dirname);
	
			if (!dir.exists())
				dir.setPath(home + tr("/Modules/") + s.toLower());
		
			if (!dir.exists())
				dir.setPath(appDir + tr("/Modules/") + s);
			
			if (!dir.exists())
				dir.setPath(appDir + tr("/Modules/") + s.toLower());

			if (dir.exists())
			{
				dir.setFilter(QDir::Files);
				dir.setSorting(QDir::Size);
				QFileInfoList list = dir.entryInfoList();
				for (int j = (list.size()-1); j >= 0; --j)
				{
					QFileInfo fileInfo = list.at(j);
					if (fileInfo.suffix().contains(tr("~"))) continue;
					fileNames << fileInfo.absoluteFilePath();
				}
			}
		}
		//fileNames << emptyModelFile;
		return fileNames;
	}

	QStringList ModuleTool::checkValidityOfModule(ConnectionHandle * module)
	{
		if (!module || !ConnectionFamily::cast(module->family()) || !module->hasNumericalData(tr("Parameters"))) return QStringList();
		
		NumericalDataTable paramsTable = module->numericalDataTable(tr("Parameters"));

		ConnectionFamily * family = ConnectionFamily::cast(module->family());
		QStringList participantRoles = family->participantRoles();
		QStringList participantTypes = family->participantTypes();
		QStringList parameters = family->numericalAttributes.keys();

		QStringList missing;
		QList<ItemHandle*> children = module->children;

		for (int i=0; i < participantRoles.size(); ++i)
		{
			bool exists = false;
			for (int j=0; j < children.size(); ++j)
				if (children[j] && children[j]->name.toLower() == participantRoles[i].toLower() && children[j]->isA(participantTypes[i]))
				{
					exists = true;
					break;
				}
			if (!exists)
			{
				missing += participantRoles[i];
			}
		}

		for (int i=0; i < parameters.size(); ++i)
		{
			bool exists = paramsTable.hasRow(parameters[i]);
			if (!exists)
				missing += parameters[i];
		}
		return missing;
	}

	void ModuleTool::substituteModel(ItemHandle * parentHandle, const QString& filename0, NetworkWindow * window, QStringList * missing)
	{
		if (!parentHandle) return;

		QString groupName = parentHandle->name;

		TextDataTable oldParticipantsData (parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName));			

		if (!cachedModels.contains(parentHandle) && !parentHandle->children.isEmpty())
		{
			QList<ItemHandle*> newHandles;
			QList<QGraphicsItem*> childItems = parentHandle->graphicsItems;
			QList<QGraphicsItem*> childrenClone = cloneGraphicsItems(childItems, newHandles);

			ItemHandle * root = 0;
			for (int i=0; i < newHandles.size(); ++i)
				if (newHandles[i] && !newHandles[i]->parent && newHandles[i]->name == parentHandle->name)
				{
					root = newHandles[i];
					break;
				}

			if (root)
			{
				TextDataTable participants (parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName));
				for (int i=0; i < newHandles.size(); ++i)
					if (newHandles[i] && !newHandles[i]->parent)
						for (int j=0; j < participants.rows(); ++j)
							if (participants(j,0) == newHandles[i]->fullName())
							{
								newHandles[i]->setParent(root, false);
								newHandles[i]->name = participants.rowName(j);
							}
				cachedModels.insert(parentHandle, childrenClone);
			}
		}

		QString filename;
		QList<QGraphicsItem*> items;
		QList<ItemHandle*> handles;
		QList<QUndoCommand*> commands;

		if (filename0.isEmpty() || filename0.toLower() == tr("empty"))
			filename = emptyModel();
		else
		if (filename0.toLower() == "original" && cachedModels.contains(parentHandle))
		{
			items = cachedModels[parentHandle];
			handles = getHandle(items);

			ItemHandle * root = 0;
			for (int i=0; i < handles.size(); ++i)
				if (!handles[i]->parent)
				{
					root = handles[i];
					break;
				}

			items.clear();
			
			if (root)
			{
				handles = root->children;
				RenameCommand::findReplaceAllHandleData(handles,root->fullName(), parentHandle->fullName());

				QList<ItemHandle*> newHandles;
				cachedModels[parentHandle] = cloneGraphicsItems(root->graphicsItems, newHandles);

				for (int i=0; i < handles.size(); ++i)
					if (handles[i]->parent == root)
					{
						items = GraphicsScene::getItemsInGroup(handles[i]->graphicsItems,groupName);
						handles[i]->setParent(parentHandle,false);
					}
			}
		}
		else
			filename = filename0;

		NetworkHandle * network;
		if (window && window->handle == parentHandle)
		{
			network = window->network;
		}
		else
		{
			network = currentNetwork();
			if (!network) return;
			
			QList<GraphicsScene*> scenes = network->scenes();
			for (int i=0; i < scenes.size(); ++i)
				if (scenes[i] && scenes[i]->localHandle() == parentHandle)
				{
					window = scenes[i]->networkWindow;
					break;
				}
			
			if (!window)
			{
				QList<TextEditor*> editors = network->editors();
				for (int i=0; i < editors.size(); ++i)
					if (editors[i] && editors[i]->localHandle() == parentHandle)
					{
						window = editors[i]->networkWindow;
						break;
					}
			}
			
			if (!window || window->handle != parentHandle)
			{
				window = createNewWindow(ConnectionHandle::cast(parentHandle), network);
			}
		}
		
		QString filenames[] = {  
			filename,
			tempDir() + QObject::tr("/") + filename,
			homeDir() + QObject::tr("/") + filename,
			homeDir() + QObject::tr("/Modules/") + filename,
			QDir::currentPath() + QObject::tr("/") + filename,
			QCoreApplication::applicationDirPath() + QObject::tr("/") + filename,
			};
		
		for (int i=0; i < 6; ++i)
			if (QFile::exists(filenames[i]))
			{
				filename = filenames[i];
				break;
			}
		
		if ((QFile::exists(filename) || !handles.isEmpty()) && window && 
			network && (parentHandle == window->handle) &&
			window->handle->family()) 
		{
			QList<GraphicsScene*> scenes = network->scenes();
			QList<TextEditor*> editors = network->editors();
			
			/*for (int i=0; i < scenes.size(); ++i)
				if (scenes[i] && 
					scenes[i]->localHandle() && 
					scenes[i]->localHandle() != parentHandle &&
					scenes[i]->networkWindow && 
					scenes[i]->networkWindow->isVisible())
					scenes[i]->networkWindow->close();
			
			for (int i=0; i < editors.size(); ++i)
				if (editors[i] && 
					editors[i]->localHandle() &&
					editors[i]->localHandle() != parentHandle && 					
					editors[i]->networkWindow && 
					editors[i]->networkWindow->isVisible())
					editors[i]->networkWindow->close();*/
			
			if (handles.isEmpty())
			{
				QPair< QList<ItemHandle*> , QList<QGraphicsItem*> > pair = mainWindow->getItemsFromFile(filename,parentHandle);
				items = pair.second;
				handles = pair.first;
				if (handles.contains(parentHandle))
					handles.removeAll(parentHandle);
			}

			GraphicsScene::setItemGroup(items,groupName);
			
			if (handles.isEmpty())
			{
				if (console())
					console()->message(filename + tr(" is an empty model"));
				return;
			}

			QList<ItemHandle*> visitedHandles;
			
			if (items.isEmpty())
			{
				for (int i=0; i < handles.size(); ++i)
					if (handles[i] && !visitedHandles.contains(handles[i]))
					{
						visitedHandles << handles[i];
						if (!handles[i]->parent)
							items += handles[i]->graphicsItems;
					}
			}

			if (!items.isEmpty())
			{
				GraphicsScene * newScene = window->newScene();
				newScene->insert(tr("new model"),items,GraphicsScene::LOADED);
				QPixmap printer(WINDOW_WIDTH, WINDOW_WIDTH);
				printer.fill();
				newScene->print(&printer);
				moduleSnapshots[parentHandle] = printer;
			}
			else
			{
				QString modelText;
				TextEditor * newEditor = window->newTextEditor();	
				newEditor->insert(handles);
				emit getTextVersion(handles, &modelText);		
				newEditor->setText(modelText);
			}

			ItemHandle * h;
			
			QList<NodeHandle*> nodes;
			if (ConnectionHandle::cast(parentHandle))
				nodes = ConnectionHandle::cast(parentHandle)->nodes();

			for (int k=0; k < textTablesToBeRemoved.size(); ++k)
					if (parentHandle->hasTextData(textTablesToBeRemoved[k]))
					{
						TextDataTable * table2 = &(parentHandle->textDataTable(textTablesToBeRemoved[k]));
						TextDataTable table3;
						commands << new ChangeTextDataCommand( tr("remove text table"), table2, &(table3));
					}

			for (int k=0; k < numericalTablesToBeRemoved.size(); ++k)
					if (parentHandle->hasNumericalData(numericalTablesToBeRemoved[k]))
					{
						TextDataTable * table2 = &(parentHandle->textDataTable(numericalTablesToBeRemoved[k]));
						TextDataTable table3;
						commands << new ChangeTextDataCommand( tr("remove num table"), table2, &(table3));
					}

			QList<ItemHandle*> allHandles = network->handles();
			if (!allHandles.contains(parentHandle))
			{
				allHandles << parentHandle;
				ConnectionHandle * connection = ConnectionHandle::cast(parentHandle);
				if (connection)
				{
					QList<NodeHandle*> nodes = connection->nodes();
					for (int i=0; i < nodes.size(); ++i)
						allHandles << nodes[i];
				}
			}

			TextDataTable newParticipantsTable = parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName);
			parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName) = oldParticipantsData;
			
			for (int i=0; i < parentHandle->children.size(); ++i)
			{
				h = findCorrespondingHandle(parentHandle->children[i]->name,ConnectionHandle::cast(parentHandle));
				
				if (h && (h->isA(parentHandle->children[i]->family()) || parentHandle->children[i]->isA(h->family())))
				{
					nodes.removeAll(NodeHandle::cast(h));

					commands << new MergeHandlesCommand(
												tr("merge"), allHandles, QList<ItemHandle*>() << h << parentHandle->children[i]);

					for (int k=0; k < textTablesToBeReplaced.size(); ++k)
						if (h->hasTextData(textTablesToBeReplaced[k]) && 
							parentHandle->children[i]->hasTextData(textTablesToBeReplaced[k]))
						{
							TextDataTable * table1 = &(h->textDataTable(textTablesToBeReplaced[k]));
							TextDataTable * table2 = &(parentHandle->children[i]->textDataTable(textTablesToBeReplaced[k]));
							TextDataTable table3(*table1);
							for (int j1=0; j1 < table2->rows(); ++j1)
								for (int j2=0; j2 < table2->columns(); ++j2)
									table3.value(table2->rowName(j1), table2->columnName(j2)) = table2->value(j1,j2);

							commands << new ChangeTextDataCommand( tr("replace text table"), table1, &(table3));
						}

				for (int k=0; k < textTablesToBeRemoved.size(); ++k)
						if (parentHandle->children[i]->hasTextData(textTablesToBeRemoved[k]))
						{
							TextDataTable * table2 = &(parentHandle->children[i]->textDataTable(textTablesToBeRemoved[k]));
							TextDataTable table3;
							commands << new ChangeTextDataCommand( tr("remove text table"), table2, &(table3));
						}

					for (int k=0; k < numericalTablesToBeReplaced.size(); ++k)
						if (h->hasNumericalData(numericalTablesToBeReplaced[k]) && 
							parentHandle->children[i]->hasNumericalData(numericalTablesToBeReplaced[k]))
						{
							NumericalDataTable * table1 = &(h->numericalDataTable(numericalTablesToBeReplaced[k]));
							NumericalDataTable * table2 = &(parentHandle->children[i]->numericalDataTable(numericalTablesToBeReplaced[k]));
							NumericalDataTable table3(*table1);
							for (int j1=0; j1 < table2->rows(); ++j1)
								for (int j2=0; j2 < table2->columns(); ++j2)
									table3.value(table2->rowName(j1), table2->columnName(j2)) = table2->value(j1,j2);
							commands << new ChangeNumericalDataCommand( tr("replace num table"), table1, &(table3));
						}

					for (int k=0; k < numericalTablesToBeRemoved.size(); ++k)
						if (parentHandle->children[i]->hasNumericalData(numericalTablesToBeRemoved[k]))
						{
							NumericalDataTable * table2 = &(parentHandle->children[i]->numericalDataTable(numericalTablesToBeRemoved[k]));
							NumericalDataTable table3;
							commands << new ChangeNumericalDataCommand( tr("remove num table"), table2, &(table3));
						}
				}
			}
			parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName) = newParticipantsTable;

			for (int i=0; i < nodes.size(); ++i)
			{
				h = nodes[i];
				
				TextDataTable textTable;
				NumericalDataTable numericalTable;
				
				/*for (int k=0; k < textTablesToBeReplaced.size(); ++k)
					if (h->hasTextData(textTablesToBeReplaced[k]))
						commands << new ChangeTextDataCommand(
												tr("replace text table"), 
												&(h->textDataTable(textTablesToBeReplaced[k])), 
												&(textTable));
				
				for (int k=0; k < numericalTablesToBeReplaced.size(); ++k)
					if (h->hasNumericalData(numericalTablesToBeReplaced[k]))
						commands << new ChangeNumericalDataCommand(
												tr("replace num. table"), 
												&(h->numericalDataTable(numericalTablesToBeReplaced[k])), 
												&(numericalTable));*/
			}

			if (missing)
				(*missing) = checkValidityOfModule(ConnectionHandle::cast(parentHandle));
			if (!commands.isEmpty())
			{
				commands << new ChangeTextDataCommand(
										ConnectionHandle::ParticipantsTableName, &(parentHandle->textDataTable(ConnectionHandle::ParticipantsTableName)), &(oldParticipantsData));
				network->push( new CompositeCommand(tr("merged models by roles"),commands) );
			}
		}
	}

	bool ModuleTool::setMainWindow(MainWindow * main)
    {
		Tool::setMainWindow(main);
        if (mainWindow != 0)
        {
			connect(mainWindow,SIGNAL(mouseDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)),
                    this,SLOT(mouseDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)));
            
            connect(mainWindow,SIGNAL(keyPressed(GraphicsScene*,QKeyEvent *)),
				this ,SLOT(keyPressed(GraphicsScene*,QKeyEvent *)));

			connect(mainWindow,SIGNAL(mouseMoved(GraphicsScene* , QGraphicsItem*, QPointF , Qt::MouseButton, Qt::KeyboardModifiers, QList<QGraphicsItem*>&)),
                    this,SLOT(mouseMoved(GraphicsScene* , QGraphicsItem*, QPointF , Qt::MouseButton, Qt::KeyboardModifiers, QList<QGraphicsItem*>&)));

            connect(mainWindow,SIGNAL(itemsAboutToBeInserted(GraphicsScene*,QList<QGraphicsItem *>&, QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)),
					this, SLOT(itemsAboutToBeInserted(GraphicsScene*,QList<QGraphicsItem *>&, QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)));

			connect(this,SIGNAL(itemsInserted(NetworkHandle*, const QList<ItemHandle*>&)),
                    mainWindow, SIGNAL(itemsInserted(NetworkHandle*, const QList<ItemHandle*>&)));
                    
             connect(this,SIGNAL(saveModel(const QString&)), mainWindow, SIGNAL(saveNetwork(const QString&)));
			 
			 connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));

			//connect(mainWindow, SIGNAL(itemsAboutToBeRemoved(GraphicsScene *, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>& )),
			//		this, SLOT(itemsAboutToBeRemoved(GraphicsScene *, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>& )));
			
			//connect(mainWindow, SIGNAL(itemsRenamed(NetworkHandle * , const QList<ItemHandle*>& , const QList<QString>& , const QList<QString>& )),
			//		this, SLOT(itemsRenamed(NetworkHandle * , const QList<ItemHandle*>& , const QList<QString>& , const QList<QString>& )));

			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(toolLoaded(Tool*)));

			toolLoaded(mainWindow->tool(tr("")));
			
			//export module
			QList<QAction*> actions = mainWindow->fileMenu->actions();
			QMenu * exportmenu = 0;
		
			for (int i=0; i < actions.size(); ++i)
				if (actions[i] && actions[i]->menu() && actions[i]->text() == tr("&Export"))
				{
					exportmenu = actions[i]->menu();
				}
		
			if (!exportmenu)
			{
				for (int i=0; i < actions.size(); ++i)
					if (actions[i] && actions[i]->text() == tr("&Close page"))
					{
						exportmenu = new QMenu(tr("&Export"));
						mainWindow->fileMenu->insertMenu(actions[i],exportmenu);
					}
			}

			if (!exportmenu)
			{
				exportmenu = new QMenu(tr("&Export"));
				mainWindow->fileMenu->addMenu(exportmenu);
			}
		
			if (exportmenu)
			{
				exportmenu->addAction(tr("Share model"),this,SLOT(exportModule()));
			}
			
			//module snapshot window
			snapshotToolTip = new QDialog(mainWindow);
			//snapshotToolTip->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
			snapshotToolTip->setPalette(QPalette(Qt::white));
			snapshotToolTip->setFixedSize(WINDOW_WIDTH,WINDOW_WIDTH);
			QRect rect = mainWindow->geometry();
			snapshotToolTip->setGeometry (rect.right() - WINDOW_WIDTH, rect.bottom() - WINDOW_WIDTH, WINDOW_WIDTH, WINDOW_WIDTH );
			QHBoxLayout * layout = new QHBoxLayout;
			layout->setContentsMargins(1,1,1,1);
			snapshotIcon = new QToolButton;
			connect(snapshotIcon, SIGNAL(pressed()), this, SLOT(clickedModuleImage()));
			layout->addWidget(snapshotIcon);
			snapshotToolTip->setLayout(layout);
			snapshotToolTip->hide();
        }

        return true;
    }
    
    void ModuleTool::exportModule()
	{
    	if (!exportModuleDialog || !modulesComboBox || !moduleNameEdit)
		{
			QMessageBox::information(mainWindow, tr("Cannot export"), tr("Some necessary files or plug-ins for exporting sub-models are missing"));
			return;
		}
		
		NetworkHandle * network = currentNetwork();
		if (!network || network->handles().isEmpty())
		{
			QMessageBox::information(mainWindow, tr("Cannot export"), tr("You cannot export an empty as a module"));
			return;
		}

		exportModuleDialog->exec();
		
		if (exportModuleDialog->result() == QDialog::Accepted)
		{
			QString name = moduleNameEdit->text();
			if (name.isEmpty())
			{
				QMessageBox::information(mainWindow, tr("Cannot export"), tr("Please enter a module name"));
				exportModuleDialog->exec();
				return;
			}
			
			name.replace(QRegExp(tr("[^A-Za-z0-9_]")),tr(""));
			moduleNameEdit->setText(name);
			name.replace(" ","_");

			if (GlobalSettings::SAVE_FILE_EXTENSIONS.isEmpty())
				GlobalSettings::SAVE_FILE_EXTENSIONS << "tic";

			name += tr(".") + GlobalSettings::SAVE_FILE_EXTENSIONS[0];
			QString s = modulesComboBox->currentText();
			
			if (s.isEmpty() || s.contains(tr("Custom...")) || !Ontology::connectionFamily(s))
			{
				QMessageBox::information(mainWindow, tr("Cannot export"), 
					tr("Sorry, custom model families cannot be added through this interface just yet. An alternative is to edit the Modules.nt in the TinkerCell home folder to add new Module types."));
				return;
			}

			ConnectionFamily * connectionFamily = Ontology::connectionFamily(s);
			QStringList roles = connectionFamily->participantRoles();
			QStringList missingRoles;

			for (int i=0; i < roles.size(); ++i)
			{
				QList<ItemHandle*> items = network->findItem(roles[i]);
				if (items.isEmpty() || items[0]->fullName() != roles[i])
					missingRoles << roles[i];
			}

			if (!missingRoles.isEmpty())
			{
				QMessageBox::information(mainWindow, tr("Cannot export"), 
					tr("You must have the following components for a module of type ") + s + tr(": \n") + missingRoles.join(", "));
				return;
			}
			
			QString appDir = QCoreApplication::applicationDirPath();
			QString home = homeDir();
			
			s.replace(tr(" "),tr("_"));
			QString dirname = home + tr("/Modules/") + s;
			QDir dir(dirname);
	
			if (!dir.exists())
				dir.setPath(home + tr("/Modules/") + s.toLower());
			
			if (!dir.exists())
					dir.setPath(appDir + tr("/Modules/") + s);
				
			if (!dir.exists())
					dir.setPath(appDir + tr("/Modules/") + s.toLower());

			if (dir.exists())
			{
				QString filename = dir.absoluteFilePath(name);
				emit saveModel(filename);

				QPair<QString,QString> login = CodingWindow::requestLoginInfo();
				QString username = login.first;
				QString password = login.second;

				if (!username.isEmpty() && !password.isEmpty())
				{
				#ifdef Q_WS_WIN
					home.replace(tr("/"),tr("\\"));
					home = tr("\"") + home + tr("\"");
					QString s = QObject::tr("cd ") + home + QObject::tr("& svn add ") + filename + QObject::tr("& svn commit -m\"new module added\" --username ") + username + QObject::tr(" --password ") + password + QObject::tr(" --no-auth-cache --non-interactive");
				#else
					QString s = QObject::tr("cd ") + home + QObject::tr("; svn add ") + filename + QObject::tr("; svn commit -m\"new module added\" --username ") + username + QObject::tr(" --password ") + password + QObject::tr(" --no-auth-cache --non-interactive");
				#endif
					system(s.toAscii().data());
				}
		
				QMessageBox::information(this,tr("Saved"),tr("The file ") + filename + tr(" has been saved and uploaded"));
			}
			else
			{
				QMessageBox::information(mainWindow, tr("Cannot export"), 
					tr("The modules folder seems to be missing in TinkerCell home folder. Try installing Subversion."));
			}
		}
    }
    
    void ModuleTool::initializeExportDialog()
    {
    	if (!connectionsTree) return;
    	
		QStringList allNames = connectionsTree->getAllFamilyNames();
		QStringList moduleNames;

		QString appDir = QCoreApplication::applicationDirPath();
		QString home = homeDir();

		for (int i=0; i < allNames.size(); ++i)
		{
			QString s = allNames[i];
			
			s.replace(tr(" "),tr("_"));
		
			QString dirname = home + tr("/Modules/") + s;
			QDir dir(dirname);
	
			if (!dir.exists())
				dir.setPath(home + tr("/Modules/") + s.toLower());
			
			if (!dir.exists())
					dir.setPath(appDir + tr("/Modules/") + s);
				
			if (!dir.exists())
					dir.setPath(appDir + tr("/Modules/") + s.toLower());
				
			if (dir.exists())
				moduleNames << allNames[i];
		}
		
		exportModuleDialog = new QDialog(mainWindow);
		QHBoxLayout * layout = new QHBoxLayout;

		QGroupBox * nameGroupBox = new QGroupBox(tr(" name your model "));
		QVBoxLayout * nameGroupLayout = new QVBoxLayout;
		moduleNameEdit = new QLineEdit;
		//nameGroupLayout->addWidget(new QLabel(tr(" model name : ")),0);
		nameGroupLayout->addWidget(moduleNameEdit,1);
		nameGroupBox->setLayout(nameGroupLayout);
		
		QGroupBox * familyGroupBox = new QGroupBox(tr(" set type of model "));
		QVBoxLayout * familyGroupLayout = new QVBoxLayout;
		modulesComboBox = new QComboBox;		
		modulesComboBox->addItems(moduleNames);
		//familyGroupLayout->addWidget(new QLabel(tr("model family : ")),0);
		familyGroupLayout->addWidget(modulesComboBox,1);
    	familyGroupBox->setLayout(familyGroupLayout);
    	
    	QVBoxLayout * okCancel = new QVBoxLayout;
    	QPushButton * okButton = new QPushButton(tr("&Save"));
    	QPushButton * cancelButton = new QPushButton(tr("&Cancel"));
		okCancel->addStretch(2);
    	okCancel->addWidget(okButton,1);
    	okCancel->addWidget(cancelButton,1);
    	okCancel->addStretch(2);
    	
    	layout->addWidget(nameGroupBox,1);
    	layout->addWidget(familyGroupBox,1);
    	layout->addLayout(okCancel,0);
    	
    	QVBoxLayout * layout2 = new QVBoxLayout;
    	layout2->addLayout(layout,1);
    	layout2->addWidget(moduleSavingStatus = new QLabel(tr("")),0);
    	
    	connect(cancelButton,SIGNAL(pressed()),exportModuleDialog,SLOT(reject()));
    	connect(okButton,SIGNAL(pressed()),exportModuleDialog,SLOT(accept()));
    	
    	exportModuleDialog->setLayout(layout2);
    	exportModuleDialog->hide();
    }

	void ModuleTool::toolLoaded(Tool*)
	{
		static bool connected1 = false;
		QString appDir = QCoreApplication::applicationDirPath();
		QString home = homeDir();
		
		if (mainWindow->tool(tr("Nodes Tree")) && !nodesTree)
		{
			Tool * tool = static_cast<Tool*>(mainWindow->tool(tr("Nodes Tree")));
			nodesTree = static_cast<NodesTree*>(tool);
		}
		
		QStringList moduleFamilyNames;
		if (mainWindow->tool(tr("Connections Tree")) && !connectionsTree)
		{
			Tool * tool = static_cast<Tool*>(mainWindow->tool(tr("Connections Tree")));
			connectionsTree = static_cast<ConnectionsTree*>(tool);
		}
		
		if (connectionsTree && 
				(QFile::exists(home + tr("/Modules/modules.nt")) || QFile::exists(appDir + tr("/Modules/modules.nt")))
			)
		{
			ConnectionFamily * moduleFamily = connectionsTree->getFamily(tr("Module"));
			if (!moduleFamily)
			{
				moduleFamily = new ConnectionFamily(tr("Module"));
				moduleFamily->pixmap = QPixmap(tr(":/images/module.png"));
				moduleFamily->description = tr("Self-contained subsystem that can be used to build larger systems");
				moduleFamily->textAttributes[tr("Functional description")] = tr("");
				moduleFamily->graphicsItems << new ArrowHeadItem(moduleFileName);
				connectionsTree->insertFamily(moduleFamily,0);

				if (QFile::exists(home + tr("/Modules/modules.nt")))
					connectionsTree->readTreeFile(home + tr("/Modules/modules.nt"));
				else
				if (QFile::exists(appDir + tr("/Modules/modules.nt")))				
					connectionsTree->readTreeFile(appDir + tr("/Modules/modules.nt"));

				moduleFamily = connectionsTree->getFamily(tr("Module"));

				if (moduleFamily)
				{
					QList<ItemFamily*> childFamilies = moduleFamily->allChildren();

					if (childFamilies.isEmpty())
						QMessageBox::information(this, tr("Cannot connect to repository"), 
						#ifdef Q_WS_WIN	
							tr("One of these is missing: internet or subversion. \nTinkerCell uses subversion to download some files the first time it runs.\n\n Try rebooting or re-installing SVN located in ") 
								+ appDir + tr("\\installation") 
								+ tr("\n\n You can still use TinkerCell without modular modeling and some plugins")
						#else
							 tr("One of these is missing: internet or subversion. \n TinkerCell uses subversion to download some files the first time it runs. \n\n Please install subversion. \n\n You can still use TinkerCell without modular modeling and some plugins")
						#endif
							 );
			
					for (int i=0; i < childFamilies.size(); ++i)
					{
						QString s = childFamilies[i]->name();
						s.replace(tr(" "),tr("_"));
						QString dirname = home + tr("/Modules/") + s;
						QDir dir(dirname);

						if (!dir.exists())
							dir.setPath(home + tr("/Modules/") + s.toLower());
				
						if (!dir.exists())
							dir.setPath(appDir + tr("/Modules/") + s);
					
						if (!dir.exists())
							dir.setPath(appDir + tr("/Modules/") + s.toLower());
				
						if (dir.exists())
							moduleFamilyNames << childFamilies[i]->name();
					}
			
					if (!exportModuleDialog)
						initializeExportDialog();
				}
			}
		}

		if (mainWindow->tool(tr("Parts and Connections Catalog")) && !connected1 && connectionsTree && connectionsTree->getFamily(tr("Module")))
		{
			Tool * tool = static_cast<Tool*>(mainWindow->tool(tr("Parts and Connections Catalog")));
			catalogWidget = static_cast<CatalogWidget*>(tool);

			connect(catalogWidget,SIGNAL(buttonPressed(const QString&)),
					this,SLOT(moduleButtonPressed(const QString&)));

			QList<QToolButton*> newButtons = catalogWidget->addNewButtons(
				tr("Modules"),
				QStringList() 	<< tr("New module"),
				QList<QIcon>() 	<< QIcon(QPixmap(tr(":/images/module.png"))),
				QStringList() 	<< tr("Modules are sub-models that can be used to substitute parts of a larger model. Modules can be shared between multiple TinkerCell users automatically.")
				);
			
			if (!moduleFamilyNames.isEmpty())
				catalogWidget->showButtons(moduleFamilyNames);

			connected1 = true;
		}
	}
/*	
	void ModuleTool::itemsRenamed(NetworkHandle * network, const QList<ItemHandle*>& items, const QList<QString>& oldnames, const QList<QString>& newnames)
	{
		QList<GraphicsScene*> scenes = network->scenes();
		QRegExp regex("\\.([^\\.]+)$");
		ConnectionHandle * c;
		
		for (int i=0; i < items.size(); ++i)
			if (items[i] 
				&& (c = ConnectionHandle::cast(items[i]))
				&& !items[i]->children.isEmpty() 
				&& newnames.contains(items[i]->fullName()))
			{
				int k = newnames.indexOf(items[i]->fullName());
				if (k < 0 || k > oldnames.size()) continue;
				
				QString oldname = oldnames[i], newname = newnames[i];
				
				if (regex.indexIn(oldname) && !regex.cap(1).isEmpty())
					oldname = regex.cap(1);
				if (regex.indexIn(newname) && !regex.cap(1).isEmpty())
					newname = regex.cap(1);

				QList<ItemHandle*> children = items[i]->children;
				QList<NodeHandle*> nodes = c->nodes();
				for (int j=0; j < nodes.size(); ++j)
					children << nodes[j];

				QList<QGraphicsItem*> gitems;

				for (int j=0; j < children.size(); ++j)
					gitems << children[j]->graphicsItems;

				NodeGraphicsItem * node;
				ConnectionGraphicsItem * connection;
				TextGraphicsItem * text;
				for (int j=0; j < gitems.size(); ++j)
					if ((node = NodeGraphicsItem::cast(gitems[j])) && node->groupID == oldname)
						node->groupID = newname;
					else
					if ((connection = ConnectionGraphicsItem::cast(gitems[j])) && connection->groupID == oldname)
						connection->groupID = newname;
					else
					if ((text = TextGraphicsItem::cast(gitems[j])) && text->groupID == oldname)
						text->groupID = newname;
			}
	}
*/
	void ModuleTool::itemsAboutToBeInserted(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands,GraphicsScene::InsertType type)
	{
		if (!scene || !scene->network) return;

		bool loadedItems = (type != GraphicsScene::NEW); //loaded or pasted items
		
		int count = 0;
		QList<ItemHandle*> visited;
		
		if (loadedItems)
		{
			for (int i=0; i < handles.size(); ++i)
				if (ConnectionHandle::cast(handles[i]) && !visited.contains(handles[i]))
				{
					visited << handles[i];
					if (!handles[i]->children.isEmpty())
						createNewWindow(ConnectionHandle::cast(handles[i]), scene->network);
				}
			return;
		}
		
		NodeGraphicsItem * node;
		ConnectionGraphicsItem * connection;
		TextGraphicsItem * text;
		
		if (scene->localHandle())
		{
			QString groupName = scene->localHandle()->name;
			GraphicsScene::setItemGroup(items,groupName);
		}
		
		visited.clear();
		for (int i=0; i < handles.size(); ++i)
			if (handles[i] && handles[i]->children.isEmpty() && !visited.contains(handles[i]) && ConnectionFamily::cast(handles[i]->family()))
			{
				visited << handles[i];
				QStringList list = listOfModels(handles[i]->family());
				if (!list.isEmpty())
					substituteModel(handles[i], list[0]);
				QString filename;
				if (handles[i]->isA(tr("Module")))
				{
					QString family = handles[i]->family()->name();
					filename = homeDir() + tr("/Graphics/") + NodesTree::themeDirectory + tr("/Decorators/") + ConnectionsTree::decoratorImageFile(family);
					if (!QFile(filename).exists())
						filename = tr(":/images/") + ConnectionsTree::decoratorImageFile(family);
					if (!QFile(filename).exists())
						filename = QCoreApplication::applicationDirPath() + tr("/Graphics/") + NodesTree::themeDirectory + tr("/Decorators/") + ConnectionsTree::decoratorImageFile(family);
					if (!QFile(filename).exists())
						filename = tr(":/images/Module.xml");
				}
				else
					if (!list.isEmpty())
					{
						filename = tr(":/images/expand.xml");
					}
				
				if (!filename.isEmpty() && QFile::exists(filename))
				{
					QList<ConnectionGraphicsItem*> connections = ConnectionGraphicsItem::cast(handles[i]->graphicsItems);
					for (int i2=0; i2 < connections.size(); ++i2)
					{
						ConnectionGraphicsItem * connection = connections[i2];
						ArrowHeadItem * newDecorator = new ArrowHeadItem(filename,connection);
						if (connection->centerRegionItem)
						{
							QList<ConnectionGraphicsItem*> connectionsOnDecorator = connection->centerRegionItem->connections();
							for (int j=0; j < connectionsOnDecorator.size(); ++j)
							{
								connectionsOnDecorator[j]->replaceNode(connection->centerRegionItem, newDecorator);
							}
							items.removeAll(connection->centerRegionItem);
							delete connection->centerRegionItem;
						}
						connection->centerRegionItem = newDecorator;
						connection->refresh();
						items += newDecorator;
					}
				}
			}
	}

/*
	void ModuleTool::itemsAboutToBeRemoved(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands)
	{
		if (!scene || !scene->network) return;
		
		ItemHandle * h1 = 0, * h2 = 0;
		
		ConnectionGraphicsItem * connection = 0;
		QList<NodeGraphicsItem*> nodes;
		QStringList oldNames, newNames;
		
		for (int i=0; i < items.size(); ++i)
			if ((connection = ConnectionGraphicsItem::cast(items[i])) && connection->className == connectionClassName)
			{
				nodes = connection->nodes();
				if (nodes.size() == 2)
				{
					h1 = getHandle(nodes[0]);
					h2 = getHandle(nodes[1]);
					if (h1 && h2 && h1->parent && h1 != h2)
					{
						QList<ItemHandle*> moduleHandles;
						moduleHandles << h1->parent;
						moduleHandles << h1->parent->allChildren();
						commands << new RenameCommand(tr("module connection removed"),scene->network,moduleHandles,h2->fullName(),h1->fullName());
					}
				}
			}
	}
*/

	NodeHandle * ModuleTool::findCorrespondingHandle(const QString& name0, ConnectionHandle * connection)
	{
		if (name0.isNull() || name0.isEmpty() || !connection || !connection->hasTextData(ConnectionHandle::ParticipantsTableName))
			return 0;

		QList<NodeHandle*> nodes = connection->nodes();

		TextDataTable & participants = connection->textDataTable(ConnectionHandle::ParticipantsTableName);
		ConnectionFamily * family = ConnectionFamily::cast(connection->family());
		
		if (!family) return 0;

		QString s, name = name0;

		QString parentNameWithDot = connection->fullName() + tr("\\.");
		QString parentNameWithUnderscore = connection->fullName("_") + tr("_");

		if (name.startsWith(parentNameWithDot))
			name.replace(QRegExp( tr("^") + parentNameWithDot ), tr(""));
		else
		if (name.startsWith(parentNameWithUnderscore))
			name.replace(QRegExp( tr("^") + parentNameWithUnderscore ), tr(""));
		
		QStringList candidates = family->synonymsForRole(name);
		
		for (int i=0; i < nodes.size(); ++i)
		{
			for (int j=0; j < candidates.size(); ++j)
			{
				if (participants.hasRow(candidates[j]))
				{
					s = participants.value(candidates[j],0);
					if (nodes[i]->fullName().compare(s,Qt::CaseSensitive) == 0)
						return nodes[i];
				}
			}
		}
		return 0;
	}
/*
    void ModuleTool::itemsInserted(NetworkHandle * network, const QList<ItemHandle*>& handles)
	{
		GraphicsScene * scene = network->currentScene();
		if (scene)
		{
			ConnectionGraphicsItem * c;
			ConnectionHandle * ch;
			QList<QUndoCommand*> commands;
			QList<ItemHandle*> modules;
			ItemHandle * h;
			
			for (int i=0; i < handles.size(); ++i)
				if ((ch = ConnectionHandle::cast(handles[i])) && !handles[i]->children.isEmpty())
				{
					modules << handles[i];
				}

			for (int i=0; i < modules.size(); ++i)
			{
				TextDataTable oldParticipantsData (modules[i]->textDataTable(ConnectionHandle::ParticipantsTableName));
				for (int j=0; j < modules[i]->children.size(); ++j)
				{
					h = findCorrespondingHandle(NodeHandle::cast(modules[i]->children[j]),ConnectionHandle::cast(modules[i]));
					if (h)
					{
						commands << new MergeHandlesCommand(
													tr("merge"), network, QList<ItemHandle*>() << h << modules[i]->children[j]);
						
						for (int k=0; k < textTablesToBeReplaced.size(); ++k)
							if (h->hasTextData(textTablesToBeReplaced[k]) && 
								modules[i]->children[j]->hasTextData(textTablesToBeReplaced[k]))
								commands << new ChangeTextDataCommand(
														tr("replace text table"), 
														&(h->textDataTable(textTablesToBeReplaced[k])), 
														&(modules[i]->children[j]->textDataTable(textTablesToBeReplaced[k])));
						
						for (int k=0; k < numericalTablesToBeReplaced.size(); ++k)
							if (h->hasNumericalData(numericalTablesToBeReplaced[k]) && 
								modules[i]->children[j]->hasNumericalData(numericalTablesToBeReplaced[k]))
								commands << new ChangeNumericalDataCommand(
														tr("replace num. table"), 
														&(h->numericalDataTable(numericalTablesToBeReplaced[k])), 
														&(modules[i]->children[j]->numericalDataTable(numericalTablesToBeReplaced[k])));
					}
				}
				commands << new ChangeTextDataCommand(
													ConnectionHandle::ParticipantsTableName, &(modules[i]->textDataTable(ConnectionHandle::ParticipantsTableName)), &(oldParticipantsData));
			}

			if (!commands.isEmpty())
			{
				network->push( new CompositeCommand(tr("merged models by roles"),commands) );
			}
			
			for (int i=0; i < modules.size(); ++i)
				if (ch = ConnectionHandle::cast(modules[i]))
				{
					createNewWindow(ch,scene->network);					
				}
	    }
    }
*/
    void ModuleTool::mouseMoved(GraphicsScene* scene, QGraphicsItem * hoverOverItem, QPointF point, Qt::MouseButton, Qt::KeyboardModifiers, QList<QGraphicsItem*>& items)
    {
		if (!GlobalSettings::PROGRAM_MODE.isEmpty()) return;

		if (mainWindow && scene && scene->useDefaultBehavior())
		{
			/*if (!hoverOverItem && scene->selected().size() == 1)
			{
				if (ArrowHeadItem::cast(scene->selected()[0]))
					hoverOverItem = scene->selected()[0];
			}*/

			if (hoverOverItem && !TextGraphicsItem::cast(hoverOverItem) && snapshotToolTip)
			{
				ItemHandle * h = 0;
				ConnectionHandle * ch = 0;
				ArrowHeadItem * arrowHead = ArrowHeadItem::cast(hoverOverItem);

				if (arrowHead && arrowHead->connectionItem && 
					arrowHead == arrowHead->connectionItem->centerRegionItem)
					h = getHandle(arrowHead->connectionItem);

				if (h && (ch = ConnectionHandle::cast(h)) && moduleSnapshots.contains(ch))
				{
					if (!snapshotToolTip->isVisible())
					{
						QRect rect = scene->mapToWidget( hoverOverItem->sceneBoundingRect() );
						snapshotToolTip->setGeometry (rect.right() + 100, rect.top() - 100, WINDOW_WIDTH, WINDOW_WIDTH );
						snapshotIcon->setIcon(QIcon(moduleSnapshots[ch]));
						snapshotIcon->setIconSize(QSize(WINDOW_WIDTH,WINDOW_WIDTH));
						snapshotToolTip->show();
						snapshotToolTip->raise();
					}
					return;
				}
			}
		}
		
		if (snapshotToolTip && snapshotToolTip->isVisible())
			snapshotToolTip->hide();
    }

	
	void ModuleTool::modelButtonClicked ( QAbstractButton * button )
	{
		if (button)
		{
			QString filename = button->toolTip();
			NetworkHandle * network = currentNetwork();
			
			if (QFile::exists(filename) && network)
			{
				NetworkWindow * window = network->currentWindow();
				
				if (!window || !window->handle || !window->handle->family()) return;
				ItemHandle * parentHandle = window->handle;
				
				QStringList missing;
				substituteModel(parentHandle, filename, window, &missing);
				if (currentScene())
					currentScene()->fitAll();

				if (!missing.isEmpty())
					QMessageBox::information(mainWindow, tr("Invalid model"), tr("Please note that this model is invalid because it is missing the following items: \n\n") + missing.join("\n"));
			}
		}
	}
	
	QString ModuleTool::emptyModel()
	{
		QString emptyModelFile = homeDir() + tr("/Modules/Empty_Model.tic");
		
		if (!QFile(emptyModelFile).exists())
			emptyModelFile = homeDir() + tr("/Modules/Empty_Model.TIC");
		
		if (!QFile(emptyModelFile).exists())
			emptyModelFile = QCoreApplication::applicationDirPath() + tr("/Modules/Empty_Model.tic");
		
		if (!QFile(emptyModelFile).exists())
			emptyModelFile = QCoreApplication::applicationDirPath() + tr("/Modules/Empty_Model.TIC");
		
		if (!QFile(emptyModelFile).exists())
			emptyModelFile = tr("");
		
		return emptyModelFile;
	}
	
	QDockWidget * ModuleTool::makeDockWidget(const QStringList & families)
	{
		QDockWidget * dock = 0;
		QWidget * widget = 0;
		QHBoxLayout * layout = 0;
		QScrollArea * scrollArea = 0;
		QButtonGroup * group = 0;
		
		QString emptyModelFile = emptyModel();
		
		for (int i=0; i < families.size(); ++i)
		{
			QString s = families[i];
			s.replace(tr(" "),tr("_"));

			QString dirname = homeDir() + tr("/Modules/") + s;
			QDir dir(dirname);

			if (!dir.exists())		
				dir.setPath(homeDir() + tr("/Modules/") + s.toLower());

			if (!dir.exists())
				dir.setPath(QCoreApplication::applicationDirPath() + tr("/Modules/") + s);

			if (!dir.exists())
				dir.setPath(QCoreApplication::applicationDirPath() + tr("/Modules/") + s.toLower());

			if (!dir.exists())
				continue;
				
			if (!dock)
			{
				dock = new QDockWidget;
				dock->setMaximumHeight(150);
				widget = new QWidget;
				layout = new QHBoxLayout;
				scrollArea = new QScrollArea;

				group = new QButtonGroup(this);
				connect(group,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(modelButtonClicked(QAbstractButton*)));
			}
			
			dir.setFilter(QDir::Files);
			dir.setSorting(QDir::Size);
			QFileInfoList list = dir.entryInfoList();
			
			if (!emptyModelFile.isEmpty())
			{
				list += QFileInfo(emptyModelFile);
				emptyModelFile = tr("");
			}

			for (int i = 0; i < list.size(); ++i)
			{
				QFileInfo fileInfo = list.at(i);
				if (fileInfo.suffix().contains(tr("~"))) continue;
				QToolButton * button = new QToolButton;
				button->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
				QString base = fileInfo.baseName();
				QString base2 = base;
				base2.replace(tr("_"),tr(" "));
				button->setText(base2);
				button->setToolTip(fileInfo.absoluteFilePath());

				if (QFile::exists(dirname + base + tr(".png")))
					button->setIcon(QIcon(dirname + base + tr(".png")));
				else
					button->setIcon(QIcon(tr(":/images/module.png")));
				group->addButton(button,i);
				layout->addWidget(button,0,Qt::AlignTop);
			}
		}
		
		if (!dock)
			return dock;

		layout->setContentsMargins(5,8,5,5);
		layout->setSpacing(12);
		widget->setLayout(layout);
		widget->setPalette(QPalette(QColor(255,255,255)));
		widget->setAutoFillBackground (true);

		scrollArea->setWidget(widget);
		scrollArea->setPalette(QPalette(QColor(255,255,255)));
		scrollArea->setAutoFillBackground (true);
		
		widget = new QWidget;
		QVBoxLayout * vlayout = new QVBoxLayout;
		vlayout->addWidget(scrollArea);
		vlayout->setContentsMargins(0,0,0,0);
		vlayout->setSpacing(0);
		widget->setLayout(vlayout);
		
		dock->setWidget(widget);
		dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		dock->setMaximumHeight(120);
		
		return dock;
	}
	
	NetworkWindow * ModuleTool::createNewWindow(ConnectionHandle * chandle, NetworkHandle * network)
	{
		if (chandle && chandle->family() && network)
		{
			QList<TextEditor*> editors = network->editors();
			QList<GraphicsScene*> scenes = network->scenes();

			for (int i=0; i < editors.size(); ++i)
				if (editors[i]->localHandle() == chandle && !editors[i]->text().isEmpty())
				{
					//editors[i]->popOut();
					return 0;
				}

			for (int i=0; i < scenes.size(); ++i)
				if (scenes[i]->localHandle() == chandle && !scenes[i]->items().isEmpty())
				{
					//scenes[i]->popOut();
					return 0;
				}

			ItemFamily * family = chandle->family();
			QList<ItemFamily*> children = family->children();
			QStringList familynames;
			familynames << family->name();
			for (int i=0; i < children.size(); ++i)
				familynames << children[i]->name();
			
			QDockWidget * dock = makeDockWidget(familynames);
			if (dock)
			{
				ConnectionGraphicsItem * connection;
				NodeGraphicsItem * node;
				TextGraphicsItem * text;
				QString groupID = chandle->name;
				QList<QGraphicsItem*> items, items2;
				for (int i=0; i < chandle->children.size(); ++i)
					if (chandle->children[i])
						items2 << chandle->children[i]->graphicsItems;
				
				for (int j=0; j < items2.size(); ++j)
					if (!items2[j]->scene() &&
						(
							( (connection = ConnectionGraphicsItem::cast(items2[j])) &&
								(groupID == connection->groupID || connection->groupID.isEmpty()))
							||
							( (node = NodeGraphicsItem::cast(items2[j]))  &&
								(groupID  ==  node->groupID || node->groupID.isEmpty()))
							||	
							( (text = TextGraphicsItem::cast(items2[j]))  &&
								(groupID == text->groupID || text->groupID.isEmpty()))
						))
						{
							items << items2[j];
						}
				
				items2.clear();
				QList<NodeHandle*> nodes = chandle->nodes();
				for (int i=0; i < nodes.size(); ++i)
					if (nodes[i])
						items2 << nodes[i]->graphicsItems;
				
				for (int j=0; j < items2.size(); ++j)
					if (!items2[j]->scene() &&
						(
							( (connection = ConnectionGraphicsItem::cast(items2[j])) &&
								(groupID == connection->groupID))
							||
							( (node = NodeGraphicsItem::cast(items2[j]))  &&
								(groupID  ==  node->groupID))
							||	
							( (text = TextGraphicsItem::cast(items2[j]))  &&
								(groupID == text->groupID))
						))
						{
							items << items2[j];
						}

				NetworkWindow * window = 0;
				if (!items.isEmpty() || chandle->children.isEmpty())
				{
					GraphicsScene * newScene = network->createScene();
					window = newScene->networkWindow;
					if (window)
					{
						window->addDockWidget(Qt::TopDockWidgetArea,dock);
						window->handle = chandle;	
					}
					newScene->insert(chandle->name + tr(" visible"),items, GraphicsScene::LOADED);
					QPixmap printer(WINDOW_WIDTH, WINDOW_WIDTH);
					printer.fill();
					newScene->print(&printer);
					moduleSnapshots[chandle] = printer;
				}
				else
				{
					QString modelText;
					TextEditor * newEditor = network->createTextEditor();	
					window = newEditor->networkWindow;
					if (window)
					{
						window->addDockWidget(Qt::TopDockWidgetArea,dock);
						window->handle = chandle;	
					}
					newEditor->insert(chandle->children);
					emit getTextVersion(chandle->children, &modelText);
					newEditor->setText(modelText);
				}
				
				if (!window)
					delete dock;
				
				return window;
			}
		}
		return 0;
	}

	void ModuleTool::clickedModuleImage()
	{
		GraphicsScene * scene = currentScene();
		QList<QGraphicsItem*> & selected = scene->selected();
		
		if (selected.size() == 1)
		{
			mouseDoubleClicked(scene,QPointF(),selected[0],Qt::LeftButton,(Qt::KeyboardModifiers)0);
		}
	}
	
	void ModuleTool::keyPressed(GraphicsScene* scene,QKeyEvent * keyEvent)
	{
		if (!keyEvent || keyEvent->modifiers() || !scene || !scene->useDefaultBehavior()) return;

		QList<QGraphicsItem*> & selected = scene->selected();
		
		if (selected.size() == 1 && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Space))
		{
			mouseDoubleClicked(scene,QPointF(),selected[0],Qt::LeftButton,(Qt::KeyboardModifiers)0);
		}
	}

	void ModuleTool::mouseDoubleClicked (GraphicsScene * scene, QPointF , QGraphicsItem * item, Qt::MouseButton, Qt::KeyboardModifiers modifiers)
    {
		if (!scene || !scene->network || !item || !mainWindow || modifiers || !(ArrowHeadItem::cast(item) || ConnectionGraphicsItem::cast(item))) return;

		ItemHandle * handle = getHandle(item);
		
		if (!handle)
		{
			if (qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(item))
				handle = getHandle(qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(item)->connectionItem);
			else
			{
				ArrowHeadItem * arrow = ArrowHeadItem::cast(item);
				if (arrow)
				{
					if (arrow->connectionItem && arrow->connectionItem->centerRegionItem == arrow)
						handle = getHandle(arrow->connectionItem);
				}
			}
		}

		ConnectionHandle * chandle = ConnectionHandle::cast(handle);

		if (chandle && handle->family() && !handle->children.isEmpty())
		{
			QList<TextEditor*> editors = scene->network->editors();
			QList<GraphicsScene*> scenes = scene->network->scenes();

			for (int i=0; i < editors.size(); ++i)
				if (editors[i]->localHandle() == handle)
				{
					editors[i]->popOut();
					return;
				}

			for (int i=0; i < scenes.size(); ++i)
				if (scenes[i]->localHandle() == handle && !scenes[i]->items().isEmpty())
				{
					scenes[i]->popOut();
					scenes[i]->fitAll();
					return;
				}
		}
    }

	void ModuleTool::moduleButtonPressed(const QString& name)
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		if (name == tr("New module"))
		{
			mainWindow->setCursor(Qt::ArrowCursor);
			//showNewModuleDialog();
			exportModule();
		}
	}
	
	void ModuleTool::updateNumberForNewModule(int n)
	{
		if (!newModuleTable || !nodesTree) return;
		
		for (int i=0; i < newModuleTable->rowCount(); ++i)
		{
			QWidget * widget = newModuleTable->cellWidget(i,1);
			delete widget;
		}
		newModuleTable->setRowCount(n);
		QStringList names(nodesTree->getAllFamilyNames());
		
		int k = names.indexOf(tr("Molecule"));
		if (k < 0)
			k = 0;
		QLineEdit * lineEdit;
		QComboBox * comboBox;
		
		for (int i=0; i < newModuleTable->rowCount(); ++i)
		{
			lineEdit = new QLineEdit;
			comboBox = new QComboBox;
			
			comboBox->addItems(names);
			comboBox->setCurrentIndex(k);
			
			newModuleTable->setCellWidget(i,0,lineEdit);
			newModuleTable->setCellWidget(i,1,comboBox);
		}
	}

	void ModuleTool::showNewModuleDialog()
	{
		if (!nodesTree || !connectionsTree)
		{
			QMessageBox::information(mainWindow, tr("No catalog"), tr("Cannot create new modules because no catalog of components is available"));
			return;
		}
		
		if (!newModuleDialog)
		{
			newModuleDialog = new QDialog(mainWindow);
			QVBoxLayout * layout = new QVBoxLayout;
			
			QGroupBox * group1 = new QGroupBox(tr(""));
			QVBoxLayout * layout1 = new QVBoxLayout;
			QHBoxLayout * layout1a = new QHBoxLayout, * layout1b = new QHBoxLayout;
			newModuleName = new QLineEdit;
			layout1a->addStretch(1);
			layout1a->addWidget(new QLabel(tr(" Module name : ")),0);
			layout1a->addWidget(newModuleName,0);
			layout1a->addStretch(1);

			layout1b->addStretch(1);
			layout1b->addWidget(new QLabel(tr(" Number of inputs/outputs : ")),0);
			QSpinBox * spinBox = new QSpinBox;
			spinBox->setRange(2,20);
			connect(spinBox,SIGNAL(valueChanged(int)),this,SLOT(updateNumberForNewModule(int)));
			layout1b->addWidget(spinBox,0);
			layout1b->addStretch(1);

			layout1->addLayout(layout1a);
			layout1->addLayout(layout1b);

			group1->setLayout(layout1);
			layout->addWidget(group1);

			newModuleTable = new QTableWidget;
			newModuleTable->setColumnCount(2);
			newModuleTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Family" );
			QGroupBox * group2 = new QGroupBox(tr(""));
			QHBoxLayout * layout2 = new QHBoxLayout;
			layout2->addWidget(newModuleTable,1,Qt::AlignCenter);
			group2->setLayout(layout2);
			layout->addWidget(group2);

			QPushButton * okButton = new QPushButton("&Make Module");
			QPushButton * cancelButton = new QPushButton("&Cancel");
			connect(okButton,SIGNAL(pressed()),newModuleDialog,SLOT(accept()));
			connect(cancelButton,SIGNAL(pressed()),newModuleDialog,SLOT(reject()));
			if (mainWindow)
				connect(cancelButton,SIGNAL(pressed()),mainWindow,SLOT(sendEscapeSignal()));
			QGroupBox * group3 = new QGroupBox(tr(""));
			QHBoxLayout * layout3 = new QHBoxLayout;
			layout3->addStretch(1);
			layout3->addWidget(okButton);
			layout3->addWidget(cancelButton);
			layout3->addStretch(1);
			group3->setLayout(layout3);
			layout->addWidget(group3);

			newModuleDialog->setLayout(layout);
			spinBox->setValue(3);
		}

		newModuleDialog->exec();
		
		if (newModuleDialog->result() == QDialog::Accepted)		
			makeNewModule();
	}
	
	void ModuleTool::makeNewModule()
	{
		if (!catalogWidget || !nodesTree || !connectionsTree || !newModuleName || !newModuleTable || 
			!connectionsTree->getFamily(tr("Module"))) 
			return;

		QString name = newModuleName->text();
		if (name.isNull() || name.isEmpty()) return;
		
		QString appDir = QCoreApplication::applicationDirPath();

		ConnectionFamily * moduleFamily = connectionsTree->getFamily(tr("Module"));
		ConnectionFamily * newModuleFamily = new ConnectionFamily(name);
		newModuleFamily->setParent(moduleFamily);
		newModuleFamily->pixmap = moduleFamily->pixmap;
		newModuleFamily->description = moduleFamily->description;
		newModuleFamily->graphicsItems << new ArrowHeadItem(appDir + tr("/Graphics/") + NodesTree::themeDirectory + tr("/Arrows/default.xml"))
										<< new ArrowHeadItem(moduleFileName);

		
		FamilyTreeButton * button = new FamilyTreeButton(newModuleFamily);
		connectionsTree->insertFamily(newModuleFamily,button);
		connect(button,SIGNAL(connectionSelected(ConnectionFamily*)),connectionsTree,SLOT(buttonPressed(ConnectionFamily*)));
				
		QLineEdit * lineEdit;
		QComboBox * comboBox;

		for (int i=0; i < newModuleTable->rowCount(); ++i)
		{
			lineEdit = static_cast<QLineEdit*>(newModuleTable->cellWidget(i,0));
			comboBox = static_cast<QComboBox*>(newModuleTable->cellWidget(i,1));			
			newModuleFamily->addParticipant(lineEdit->text(), comboBox->currentText());
		}
		
		QStringList newModuleNames;
		newModuleNames << newModuleFamily->name();
		catalogWidget->showButtons(newModuleNames);
	}
	
	void ModuleTool::doSubstituteModel(QSemaphore * sem, ItemHandle * parent, const QString& filename)
	{
		substituteModel(parent, filename);
		if (sem)
			sem->release();
	}
	
	typedef void (*tc_ModuleTool_api)(
		void (*substituteModel)(long, const char*),
		tc_strings (*listOfModels)(long));

	void ModuleTool::setupFunctionPointers( QLibrary * library )
	{
		tc_ModuleTool_api f = (tc_ModuleTool_api)library->resolve("tc_ModuleTool_api");
		if (f)
		{
			f(
				&(_substituteModel),
				&(_listOfModels));
		}
	}
	
	tc_strings ModuleTool::_listOfModels(long o)
	{
		ItemHandle * handle = ConvertValue(o);
		QStringList list;
		if (handle && handle->family())
			list = listOfModels(handle->family());
		return ConvertValue(list);
	}

	void ModuleTool::_substituteModel(long o, const char * s)
	{
		fToS->substituteModel(o, s);
	}
	
	void ModuleTool_FToS::substituteModel(long o, const char * s)
	{
		QSemaphore * sem = new QSemaphore(1);
		sem->acquire();
		emit doSubstituteModel(sem, ConvertValue(o), ConvertValue(s));
		sem->acquire();
		sem->release();
		delete sem;
	}
	
	ModuleTool_FToS * ModuleTool::fToS;
	QStringList ModuleTool::numericalTablesToBeReplaced;
	QStringList ModuleTool::textTablesToBeReplaced;
	QStringList ModuleTool::numericalTablesToBeRemoved;
	QStringList ModuleTool::textTablesToBeRemoved;
}

/*
extern "C" TINKERCELLEXPORT void loadTCTool(Tinkercell::MainWindow * main)
{
    if (!main) return;

    main->addTool(new Tinkercell::ModuleTool);

}*/

