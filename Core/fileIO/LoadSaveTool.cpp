/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This tool allows the loading and saving of Networks.

****************************************************************************/
#include "LoadSaveTool.h"
#include "CThread.h"
#include "CloneItems.h"
#include "ConsoleWindow.h"
#include "UndoCommands.h"
#include "GlobalSettings.h"
#include "Ontology.h"
#include <QtDebug>
#include <QRegExp>
#include <QMessageBox>

namespace Tinkercell
{
	LoadSaveTool::LoadSaveTool() : Tool(tr("Save and Load"),tr("Basic GUI"))
	{	
	    /*******  save and open extensions ***********/	   	
   		GlobalSettings::OPEN_FILE_EXTENSIONS << "TIC" << "tic";
		GlobalSettings::SAVE_FILE_EXTENSIONS << "TIC" << "tic";
		countHistory = 0;
		restoreDialog = 0;
		restoreButton = 0;
	}

	void LoadSaveTool::historyChangedSlot(int i)
	{
		if (i==-1) return;
		GraphicsScene * scene = currentScene();
		if (scene && scene->network && scene->networkWindow)
		{
			savedNetworks[scene->network] = false;
			if (!scene->networkWindow->windowTitle().contains(tr("*")))
				scene->networkWindow->setWindowTitle(
					tr("*") + scene->networkWindow->windowTitle()
				);
		}
		++countHistory;
		if (countHistory > 10)
		{
			saveNetwork(GlobalSettings::tempDir(tr("backup.xml")));
			countHistory = 0;
		}
	}

	void LoadSaveTool::networkClosing(NetworkHandle * win, bool * close)
	{
		if (mainWindow && win && close && (*close) && savedNetworks.contains(win) && !savedNetworks.value(win))
		{
			QString title = tr("");
			if (win->currentWindow())
				title = win->currentWindow()->windowTitle();
			QMessageBox::StandardButton button = QMessageBox::question(
				mainWindow,
				tr("Save?"),
				tr("Save ") + title + tr("\nbefore closing?"),
				QMessageBox::Ok | QMessageBox::No | QMessageBox::Cancel,
				QMessageBox::Ok);
			if (button == QMessageBox::Ok)
			{
				mainWindow->saveWindow();
			}
			else
				if (button == QMessageBox::Cancel)
					(*close) = false;

			if (*close)
				savedNetworks.remove(win);
		}
	}

	bool LoadSaveTool::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);
		if (mainWindow)
		{
			connect(mainWindow,SIGNAL(saveNetwork(const QString&)),this,SLOT(saveNetwork(const QString&)));
			connect(mainWindow,SIGNAL(loadNetwork(const QString&, bool*)),this,SLOT(loadNetwork(const QString&, bool*)));
			
			connect(this, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)));

			connect(this, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&, GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&,GraphicsScene::InsertType)));
			
			connect(mainWindow,SIGNAL(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)),this,SLOT(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)));
			connect(mainWindow,SIGNAL(networkClosing(NetworkHandle * , bool *)),this,SLOT(networkClosing(NetworkHandle * , bool *)));
			connect(mainWindow,SIGNAL(historyChanged( int )),this,SLOT(historyChangedSlot( int )));

			connect(mainWindow,SIGNAL(prepareNetworkForSaving(NetworkHandle*,bool*)),this,SLOT(prepareNetworkForSaving(NetworkHandle*,bool*)));
			connect(this,SIGNAL(networkSaved(NetworkHandle*)),mainWindow,SIGNAL(networkSaved(NetworkHandle*)));
			connect(this,SIGNAL(networkLoaded(NetworkHandle*)),mainWindow,SIGNAL(networkLoaded(NetworkHandle*)));

			QString filename = GlobalSettings::tempDir(tr("backup.xml"));
			QFile file(filename);
			
			if (file.open(QFile::ReadOnly | QFile::Text))
			{
				restoreDialog = new QMessageBox(QMessageBox::Question,tr("Restore"), tr("TinkerCell closed unexpectedly.\nRestore previous network?"), 0, mainWindow, Qt::Dialog);
				restoreDialog->addButton(tr("&No"), QMessageBox::NoRole);
				restoreButton = restoreDialog->addButton(tr("&Yes"), QMessageBox::YesRole);
				restoreDialog->show();
				connect(restoreDialog,SIGNAL(finished(int)),this,SLOT(restore(int)));
			}
			
			return true;
		}
		return false;
	}
	
	void LoadSaveTool::restore(int role)
	{
		if (restoreDialog->clickedButton() == restoreButton)
		{
			QString filename = GlobalSettings::tempDir() + tr("/backup.xml");
			QFile file(filename);
		
			if (file.open(QFile::ReadOnly | QFile::Text))
				loadNetwork(filename,0);
		}
	}

	void LoadSaveTool::prepareNetworkForSaving(NetworkHandle * net, bool* b)
	{
		if (net && net->currentScene() && b)
			(*b) = true;
	}

	void LoadSaveTool::writeNode(NodeGraphicsItem* node, QXmlStreamWriter& modelWriter, int sceneNumber)
	{
		if (!node) return;

		ItemHandle * handle = getHandle(node);
		if (ArrowHeadItem::cast(node)) return; //don't write arrow heads and decorators

		modelWriter.writeStartElement("NodeItem");
		modelWriter.writeAttribute("className",node->className);
		modelWriter.writeAttribute(tr("scene"), QString::number(sceneNumber));

		if (handle)
			modelWriter.writeAttribute("handle",handle->fullName(tr(".")));
		else
			modelWriter.writeAttribute("handle","");

		modelWriter.writeAttribute("z",QString::number(node->zValue()));

		QTransform t1 = node->sceneTransform();
		QPointF pos = node->scenePos();

		modelWriter.writeStartElement("pos");
		modelWriter.writeAttribute("x",QString::number(pos.x()));
		modelWriter.writeAttribute("y",QString::number(pos.y()));
		modelWriter.writeEndElement();

		modelWriter.writeStartElement("transform");
		modelWriter.writeAttribute("m11",QString::number(t1.m11()));
		modelWriter.writeAttribute("m12",QString::number(t1.m12()));
		modelWriter.writeAttribute("m21",QString::number(t1.m21()));
		modelWriter.writeAttribute("m22",QString::number(t1.m22()));
		modelWriter.writeEndElement();

		t1 = node->transform();
		node->resetTransform();
		NodeGraphicsWriter::writeNodeGraphics(node,&modelWriter);
		node->setTransform(t1);
		modelWriter.writeEndElement();
	}

	void LoadSaveTool::writeConnection(ConnectionGraphicsItem* connection, QXmlStreamWriter& modelWriter, int sceneNumber)
	{
		if (!connection) return;
		ItemHandle * handle = getHandle(connection);

		modelWriter.writeStartElement(tr("ConnectionItem"));
		modelWriter.writeAttribute(tr("className"),connection->className);
		modelWriter.writeAttribute(tr("scene"), QString::number(sceneNumber));

		if (handle)
			modelWriter.writeAttribute(tr("handle"),handle->fullName(tr(".")));
		else
			modelWriter.writeAttribute(tr("handle"),tr(""));
		modelWriter.writeAttribute("z",QString::number(connection->zValue()));

		ConnectionGraphicsWriter::writeConnectionGraphics(connection,&modelWriter);
		modelWriter.writeEndElement();
	}

	void LoadSaveTool::writeText(TextGraphicsItem* text, QXmlStreamWriter& modelWriter,int sceneNumber)
	{
		if (!text) return;

		modelWriter.writeStartElement(tr("TextItem"));
		modelWriter.writeAttribute(tr("text"),text->toPlainText());
		modelWriter.writeAttribute(tr("scene"), QString::number(sceneNumber));
		
		ItemHandle * handle = getHandle(text);
		if (handle)
			modelWriter.writeAttribute(tr("handle"),handle->fullName());
		else
			modelWriter.writeAttribute(tr("handle"),tr(""));

		QPointF pos = text->scenePos();
		modelWriter.writeAttribute(tr("x"),QString::number(pos.x()));
		modelWriter.writeAttribute(tr("y"),QString::number(pos.y()));
		modelWriter.writeAttribute(tr("color"),text->defaultTextColor().name());

		QTransform transform = text->transform();
		modelWriter.writeAttribute(tr("m11"),QString::number(transform.m11()));
		modelWriter.writeAttribute(tr("m12"),QString::number(transform.m12()));
		modelWriter.writeAttribute(tr("m21"),QString::number(transform.m21()));
		modelWriter.writeAttribute(tr("m22"),QString::number(transform.m22()));

		modelWriter.writeAttribute(tr("z"),QString::number(text->zValue()));

		QFont font = text->font();
		modelWriter.writeAttribute(tr("font"),font.family());
		modelWriter.writeAttribute(tr("size"),QString::number(font.pointSize()));
		modelWriter.writeAttribute(tr("bold"),QString::number((int)font.bold()));
		modelWriter.writeAttribute(tr("italics"),QString::number((int)font.italic()));
		modelWriter.writeAttribute(tr("underline"),QString::number((int)font.underline()));
		if (!text->groupID.isEmpty())
			modelWriter.writeAttribute(tr("group"),text->groupID);

		modelWriter.writeEndElement();
	}

	void LoadSaveTool::saveItems(NetworkHandle * network, const QString& filename)
	{
		if (!network || filename.isEmpty()) return;
		
		QList<GraphicsScene*> scenes = network->scenes();
		QList<QGraphicsItem*> allitems, handleitems;

		for (int i=0; i < scenes.size(); ++i)
			if (scenes[i])
				allitems << scenes[i]->items();
		
		QList<ItemHandle*> allhandles = network->handles();
		
		if (network->globalHandle())
			saveUnitsToTable(network->globalHandle()->textDataTable("Units"));
		
		for (int i=0; i < allhandles.size(); ++i)
			if (allhandles[i])
			{
				handleitems = allhandles[i]->allGraphicsItems();
				for (int j=0; j < handleitems.size(); ++j)
					if (!allitems.contains(handleitems[j]))
						allitems += handleitems[j];
			}

		NodeGraphicsItem * node = 0;
		ConnectionGraphicsItem * connection = 0;
		TextGraphicsItem * text = 0;

		QFile file (filename);

		if (!file.open(QFile::WriteOnly | QFile::Text))
		{
			mainWindow->statusBar()->showMessage(tr("file cannot be opened : ") + filename);
			if (console())
                console()->error(tr("file cannot be opened : ") + filename);
			//qDebug() << "file cannot be opened : " << filename;
			return;
		}

		ModelWriter modelWriter;
		modelWriter.setDevice(&file);
		modelWriter.setAutoFormatting(true);

		modelWriter.writeStartDocument();
		modelWriter.writeDTD("<!DOCTYPE TinkerCell>");

		modelWriter.writeStartElement("Model");

		modelWriter.writeStartElement("Handles");
		modelWriter.writeModel(network,&file);
		modelWriter.writeEndElement();

		QList<NodeGraphicsItem*> nodeItems;
		QList<TextGraphicsItem*> textItems;
		QList<ConnectionGraphicsItem*> connectionItems;

		for (int i=0; i < allitems.size(); ++i)
		{
			node = NodeGraphicsItem::topLevelNodeItem(allitems[i]);
			if (node && !nodeItems.contains(node))
			{
				nodeItems << node;
			}
			else
			{
				connection = ConnectionGraphicsItem::topLevelConnectionItem(allitems[i]);
				if (connection && !connectionItems.contains(connection))
				{
					connectionItems << connection;
				}
				else
				{
					text = TextGraphicsItem::cast(allitems[i]);
					if (text && !textItems.contains(text))
					{
						textItems << text;
					}
				}
			}
		}

		modelWriter.writeStartElement(tr("Nodes"));
		for (int i=0; i < nodeItems.size(); ++i)
		{
			node = nodeItems[i];
			writeNode(node,modelWriter,scenes.indexOf(static_cast<GraphicsScene*>(node->scene())));
		}
		modelWriter.writeEndElement();

		modelWriter.writeStartElement(tr("Connections"));
		QList<ConnectionGraphicsItem*> firstSetofConnections;
		for (int i=0; i < connectionItems.size(); ++i)
		{
			if (connectionItems[i] && connectionItems[i]->centerRegionItem &&
				connectionItems[i]->centerRegionItem->scene() &&
				!connectionItems[i]->centerRegionItem->connections().isEmpty())
			{
				firstSetofConnections += connectionItems[i];
				connectionItems.removeAt(i);
				--i;
			}
		}
		for (int i=0; i < firstSetofConnections.size(); ++i)
		{
			connection = firstSetofConnections[i];
			writeConnection(connection,modelWriter,scenes.indexOf(static_cast<GraphicsScene*>(connection->scene())));
		}

		for (int i=0; i < connectionItems.size(); ++i)
		{
			connection = connectionItems[i];
			writeConnection(connection,modelWriter,scenes.indexOf(static_cast<GraphicsScene*>(connection->scene())));
		}

		modelWriter.writeEndElement();

		modelWriter.writeStartElement(tr("Texts"));

		for (int i=0; i < textItems.size(); ++i)
		{
			text = textItems[i];
			writeText(text,modelWriter,scenes.indexOf(static_cast<GraphicsScene*>(text->scene())));
		}

		modelWriter.writeEndElement();

		modelWriter.writeEndElement();
		modelWriter.writeEndDocument();

		savedNetworks[network] = true;

		emit networkSaved(network);

		mainWindow->statusBar()->showMessage(tr("model saved in ") + filename);
	}

	void LoadSaveTool::saveNetwork(const QString& filename)
	{
		NetworkHandle * network = currentNetwork();
		if (network)
			saveItems(network,filename);
	}

	void LoadSaveTool::loadNetwork(const QString& filename, bool * b)
	{
		if (b && (*b)) return;

		QList<QGraphicsItem*> items;
		ItemHandle globalHandle;
		loadItems(items,filename, &globalHandle);

		if (globalHandle.hasTextData("Units"))
		{
			readUnitsFromTable(globalHandle.textDataTable("Units"));
			globalHandle.textDataTable("Units").resize(0,0);
		}

		if (items.size() > 0)
		{
			if (b)
				(*b) = true;
			GraphicsScene * scene = currentScene();
			if (!scene || !scene->network || !scene->network->handles().isEmpty() || (scene->network->history.count() > 0))
			{
				scene = mainWindow->newScene();
			}

			if (!scene)
			{
				QList<ItemHandle*> visited;
				ItemHandle * handle;
				for (int i=0; i < items.size(); ++i)
				{
					if ((handle = getHandle(items[i])) && !handle->parent && !visited.contains(handle))
						delete handle;
					else
						delete items[i];					
				}
				return;
			}
			
			if (scene->globalHandle())
				(*scene->globalHandle()) = globalHandle;
		    //scene->insert(tr("load"),items);
		    QList<ItemHandle*> handles = getHandle(items);
		    QList<QUndoCommand*> commands;
		    
		    ConnectionGraphicsItem * connection = 0;
		    
			emit itemsAboutToBeInserted(scene, items , handles, commands, GraphicsScene::LOADED);

			commands << new InsertGraphicsCommand(tr("insert"),scene,items);
			QUndoCommand * command = new CompositeCommand(tr("load"), commands);
			command->redo();
			scene->network->updateSymbolsTable();
			
			emit itemsInserted(scene, items, handles, GraphicsScene::LOADED);
			loadCommands << command;
			//

			for (int i=0; i < items.size(); ++i)
			{
				if ((connection = ConnectionGraphicsItem::cast(items[i])))
				{
					connection->refresh();
					connection->setControlPointsVisible(false);
				}
			}

            scene->fitAll();

//			if (scene->historyStack)
	//			scene->historyStack->clear();

			savedNetworks[scene->network] = true;

			emit networkLoaded(scene->network);
		}
	}
	
	QHash< QString, LoadSaveTool::CachedModel* > LoadSaveTool::cachedModels;
	
	void LoadSaveTool::getItemsFromFile(QList<ItemHandle*>& handles, QList<QGraphicsItem*>& items, const QString& filename,ItemHandle * root)
	{
		if (!handles.isEmpty()) return;
		QFileInfo fileInfo(filename);
		
		if (!fileInfo.exists()) return;
		CachedModel * cache;

		ItemHandle * globalHandle = 0;

		bool cacheExists = false;
		if (cachedModels.contains(filename) )
		{
			cache = cachedModels[ filename ];
			if (cache && fileInfo.lastModified() == cache->time)
			{
				items = cache->items;
				globalHandle = cache->globalHandle;
				cacheExists = true;
			}
		}
		
		ItemHandle * h;
		if (!cacheExists)
		{
			globalHandle = new ItemHandle;
			loadItems(items,filename, globalHandle);
		}
		
		cache = new CachedModel;
		cache->time = fileInfo.lastModified();
		cache->globalHandle = globalHandle->clone();
		QList<ItemHandle*> newHandles;
		cache->items = cloneGraphicsItems(items, newHandles);
		cachedModels[filename] = cache;

		if (!root)
			handles += globalHandle;

		for (int i=0; i < items.size(); ++i)
			if ((h = getHandle(items[i])) && !handles.contains(h))
			{
				handles += h;
			}

		if (root)
		{
			//mapDataTables(root->children, handles);
			if (globalHandle)
			{
				h = globalHandle;
				QStringList keys = h->numericalDataNames();
				QString s;
				for (int i=0; i < keys.size(); ++i)
				{
					NumericalDataTable & dat1 = (h->numericalDataTable(keys[i]));
					
					if (!root->hasNumericalData(keys[i]))
						root->numericalDataTable(keys[i]) = dat1;
					else
					{
						NumericalDataTable & dat2 = (root->numericalDataTable(keys[i]));
						for (int j=0; j < dat1.rows(); ++j)
						{
							s = dat1.rowName(j);
							if (!dat2.hasRow(s))
								for (int k=0; k < dat1.columns(); ++k)
									dat2.value(s,k) = dat1.value(s,k);
						}
					}
					for (int j=0; j < dat1.rows(); ++j)
						RenameCommand::findReplaceAllHandleData(handles,dat1.rowName(j), tr("XXX...XXX.") + dat1.rowName(j));
				}

				keys = h->textDataNames();
				for (int i=0; i < keys.size(); ++i)
				{
					TextDataTable & dat1 = (h->textDataTable(keys[i]));
					
					if (!root->hasTextData(keys[i]))
						root->textDataTable(keys[i]) = dat1;
					else
					{
						TextDataTable & dat2 = (root->textDataTable(keys[i]));					
						for (int j=0; j < dat1.rows(); ++j)
						{
							s = dat1.rowName(j);
							if (!dat2.hasRow(s))
								for (int k=0; k < dat1.columns(); ++k)
									dat2.value(s,k) = dat1.value(s,k);
						}
					}
					for (int j=0; j < dat1.rows(); ++j)
						RenameCommand::findReplaceAllHandleData(handles,dat1.rowName(j), tr("XXX...XXX.") + dat1.rowName(j));
				}
			}
			
			QList<ItemHandle*> visited;
			for (int i=0; i < handles.size(); ++i)
				if ((h = handles[i]) && !h->parent && h->name == root->name)  //problem case -- do first
				{
					h->setParent(root,false);
					RenameCommand::findReplaceAllHandleData(handles,h->name, tr("XXX...XXX.") + h->name);
					visited += h;
				}
			
			for (int i=0; i < handles.size(); ++i)
				if ((h = handles[i]) && !h->parent && !h->name.isEmpty() && !visited.contains(h))
				{
					h->setParent(root,false);
					RenameCommand::findReplaceAllHandleData(handles,h->name,tr("XXX...XXX.") + h->name);
					visited += h;
				}
			RenameCommand::findReplaceAllHandleData(handles,tr("XXX...XXX"), root->fullName());
		}
	}

	void LoadSaveTool::loadItems(QList<QGraphicsItem*>& itemsToInsert, const QString& filename, ItemHandle * globalHandle)
	{
		QFile file1 (filename);
		
		if (!file1.open(QFile::ReadOnly | QFile::Text))
		{
			//mainWindow->statusBar()->showMessage(tr("file cannot be opened : ") + filename);
			QMessageBox::information(this,tr("Error"),tr("file cannot be opened : ") + filename);
			return;
		}

		//find starting point for the handles
		ModelReader modelReader;
		modelReader.setDevice(&file1);

		while (!modelReader.atEnd() && !(modelReader.isStartElement() && modelReader.name() == "Model"))
		{
			modelReader.readNext();
		}

		if (modelReader.atEnd()) //not a TinkerCell file
		{
			file1.close();
			return;
		}

		while (!modelReader.atEnd() && !(modelReader.isStartElement() && modelReader.name() == "Handles"))
		{
			modelReader.readNext();
		}

		if (modelReader.atEnd()) //not a TinkerCell file
		{
			file1.close();
			return;
		}

		modelReader.readNext();

		//read all the handles
		QList<QPair<QString,ItemHandle*> > handlesList = modelReader.readHandles(&file1);

		QHash<QString,ItemHandle*> handlesHash;  //hash name->handle
		NodeHandle * nodeHandle;
		ConnectionHandle * connectionHandle;
		QList<ItemHandle*> deleted;

		for (int i=0; i < handlesList.size(); ++i)
			if (handlesList[i].second && !deleted.contains(handlesList[i].second))
			{
				if (handlesList[i].first.isEmpty() && handlesList[i].second->name.isEmpty())
				{
					if (globalHandle)
						(*globalHandle) = (*handlesList[i].second);
				}
				else
				{
					if (handlesList[i].second->type == NodeHandle::TYPE)
					{
						nodeHandle = static_cast<NodeHandle*>(handlesList[i].second);
						if (getNodeFamily(handlesList[i].first))
							nodeHandle->setFamily(getNodeFamily(handlesList[i].first));
					}
					else
						if (handlesList[i].second->type == ConnectionHandle::TYPE)
						{
							connectionHandle = static_cast<ConnectionHandle*>(handlesList[i].second);
							if (getConnectionFamily(handlesList[i].first))
								connectionHandle->setFamily( getConnectionFamily(handlesList[i].first));
						}
					
					if (!handlesList[i].second->name.isEmpty())
						handlesHash[handlesList[i].second->fullName()] = handlesList[i].second;
					else
					{
						deleted << handlesList[i].second;
						delete handlesList[i].second;
					}
				}
			}

		file1.close();

		//find starting point for nodes
		QFile file2 (filename);

		if (!file2.open(QFile::ReadOnly | QFile::Text))
		{
			mainWindow->statusBar()->showMessage(tr("file cannot be opened") + filename);
			if (console())
                console()->error(tr("file cannot be opened : ") + filename);

			//qDebug() << "file cannot be opened";
			return;
		}

		QList<NodeGraphicsItem*> nodes;
		QList<QTransform> transforms;
		QList<QPointF> points;
		QList<qreal> zValues;
		QList<int> sceneNumbers;
		QList<QGraphicsItem*> items;

		//find starting point for nodes
		NodeGraphicsReader nodeReader;
		nodeReader.setDevice(&file2);

		while (!nodeReader.atEnd() && !(nodeReader.isStartElement() && nodeReader.name() == "Nodes"))
		{
			nodeReader.readNext();
		}

		//read all the nodes
		while (!nodeReader.atEnd() && !(nodeReader.isEndElement() && nodeReader.name() == "Nodes"))
		{
			QString s;
			QTransform t;
			QPointF p;
			qreal z;
			int sn=-1;
			NodeGraphicsItem * node = readNode(nodeReader,s,t,p,z,sn);
			if (node)
			{
				if (!s.isEmpty() && handlesHash.contains(s))
					setHandle(node,handlesHash[s]);  //very important
				transforms << t;
				points << p;
				items << node;
				nodes << node;
				zValues << z;
				sceneNumbers << sn;
			}
			nodeReader.readNext();
		}
		//read all connections
		while (!nodeReader.atEnd() && !(nodeReader.isStartElement() && nodeReader.name() == "Connections"))
		{
			nodeReader.readNext();
		}
		QList<ConnectionGraphicsItem*> connections;
		while (!nodeReader.atEnd() && !(nodeReader.isEndElement() && nodeReader.name() == "Connections"))
		{
			QString s;
			qreal z;
			int sn=-1;
			ConnectionGraphicsItem * connection = readConnection(nodeReader,nodes,connections,s,z,sn);
			if (connection)
			{
				if (!s.isEmpty() && handlesHash.contains(s))
					setHandle(connection,handlesHash[s]);  //very important
				transforms << QTransform();
				items << connection;
				connections << connection;
				points << QPointF(0,0);
				zValues << z;
				sceneNumbers << sn;
				QList<ConnectionGraphicsItem::ControlPoint*> controlPoints = connection->controlPoints(true);
				for (int i=0; i < controlPoints.size(); ++i)
				{
					if (controlPoints[i])
					{
						transforms << QTransform();
						items << controlPoints[i];
						points << controlPoints[i]->pos();
						zValues << (z+0.1);
						sceneNumbers << sn;
					}
				}
				QList<ArrowHeadItem*> arrowHeads;
				arrowHeads	<< connection->arrowHeads()
							<< connection->modifierArrowHeads();

				if (connection->centerRegionItem)
					arrowHeads << connection->centerRegionItem;

				for (int i=0; i < arrowHeads.size(); ++i)
				{
					if (arrowHeads[i] && (arrowHeads.indexOf(arrowHeads[i]) == i))
					{
						transforms << arrowHeads[i]->transform();
						points << arrowHeads[i]->pos();
						items << arrowHeads[i];
						zValues << (z+0.1);
						sceneNumbers << sn;
					}
				}
			}
			nodeReader.readNext();
		}

		//read all texts
		while (!nodeReader.atEnd() && !(nodeReader.isStartElement() && nodeReader.name() == "Texts"))
		{
			nodeReader.readNext();
		}
		while (!nodeReader.atEnd() && !(nodeReader.isEndElement() && nodeReader.name() == "Texts"))
		{
			QString s;
			QTransform t;
			QPointF p;
			qreal z;
			int sn=-1;
			TextGraphicsItem * text = readText(nodeReader,s,t,p,z,sn);
			if (text)
			{
				if (!s.isEmpty() && handlesHash.contains(s))
					setHandle(text,handlesHash[s]);  //very important
				transforms << t;
				points << p;
				items << text;
				zValues << z;
				sceneNumbers << sn;
			}
			nodeReader.readNext();
		}
		file2.close();

		if (items.size() > 0)
		{
			int minSceneNumber = -1;
			for (int i=0; i < items.size(); ++i)
			{
				items[i]->resetTransform();
				items[i]->setPos(QPointF(0,0));
				items[i]->setTransform(transforms[i]);
				items[i]->setPos(points[i]);
				items[i]->setZValue(zValues[i]);
				if (sceneNumbers[i] >= 0 && (sceneNumbers[i] < minSceneNumber || minSceneNumber < 0))
					minSceneNumber = sceneNumbers[i];
			}
			for (int i=0; i < items.size(); ++i)
			{
				if (sceneNumbers[i] == minSceneNumber)
					itemsToInsert << items[i];
			}
		}
	}

	TextGraphicsItem * LoadSaveTool::readText(QXmlStreamReader & nodeReader,QString& handle, QTransform& transform,QPointF& pos, qreal& z, int & sceneNumber)
	{
		if (nodeReader.isStartElement() && nodeReader.name() == "TextItem")
		{
			bool ok;
			TextGraphicsItem * text = new TextGraphicsItem;
			qreal m11=0,m21=0,m12=0,m22=0;
			QFont font;
			QXmlStreamAttributes attribs = nodeReader.attributes();
			for (int i=0; i < attribs.size(); ++i)
			{
				if (attribs[i].name().toString() == tr("text"))
					text->setPlainText(attribs[i].value().toString());
				else
				if (attribs[i].name().toString() == tr("scene"))
				{
					sceneNumber = attribs[i].value().toString().toInt(&ok);
				}
				else
					if (attribs[i].name().toString() == tr("handle"))
						handle = attribs[i].value().toString();
					else
						if (attribs[i].name().toString() == tr("x"))
							pos.rx() = attribs[i].value().toString().toDouble(&ok);
						else
							if (attribs[i].name().toString() == tr("y"))
								pos.ry() = attribs[i].value().toString().toDouble(&ok);
							else
								if (attribs[i].name().toString() == tr("color"))
									text->setDefaultTextColor(QColor(attribs[i].value().toString()));
								else
									if (attribs[i].name().toString() == tr("m11"))
										m11 = attribs[i].value().toString().toDouble(&ok);
									else
										if (attribs[i].name().toString() == tr("m12"))
											m12 = attribs[i].value().toString().toDouble(&ok);
										else
											if (attribs[i].name().toString() == tr("m21"))
												m21 = attribs[i].value().toString().toDouble(&ok);
											else
												if (attribs[i].name().toString() == tr("m22"))
													m22 = attribs[i].value().toString().toDouble(&ok);
												else
													if (attribs[i].name().toString() == tr("z"))
														z = attribs[i].value().toString().toDouble(&ok);
													else
														if (attribs[i].name().toString() == tr("font"))
															font.setFamily(attribs[i].value().toString());
														else
															if (attribs[i].name().toString() == tr("size"))
																font.setPointSize(attribs[i].value().toString().toInt(&ok));
															else
																if (attribs[i].name().toString() == tr("bold"))
																	font.setBold(attribs[i].value().toString().toInt(&ok)==1);
																else
																	if (attribs[i].name().toString() == tr("italics"))
																		font.setItalic(attribs[i].value().toString().toInt(&ok)==1);
																	else
																		if (attribs[i].name().toString() == tr("underline"))
																			font.setUnderline(attribs[i].value().toString().toInt(&ok)==1);
																		else
																			if (attribs[i].name().toString() == tr("group"))
																				text->groupID = attribs[i].value().toString();
			}
			text->setFont(font);
			transform.setMatrix(m11,m12,0.0, m21, m22, 0.0, 0.0, 0.0, 1.0);

			return text;
		}
		return 0;
	}

	ConnectionGraphicsItem * LoadSaveTool::readConnection(NodeGraphicsReader & nodeReader,QList<NodeGraphicsItem*>& nodes, QList<ConnectionGraphicsItem*>& connections, QString& handle, qreal& z, int & sceneNumber)
	{
		if (nodeReader.isStartElement() && nodeReader.name() == "ConnectionItem")
		{
			QFont font;
			QXmlStreamAttributes attribs = nodeReader.attributes();
			ConnectionGraphicsItem * connection = 0;
			QString type;
			bool ok;

			for (int i=0; i < attribs.size(); ++i)
			{
				if (attribs[i].name().toString() == tr("className"))
				{
					type = attribs[i].value().toString();
				}
				else
				if (attribs[i].name().toString() == tr("scene"))
				{
					sceneNumber = attribs[i].value().toString().toInt(&ok);
				}
				else
					if (attribs[i].name().toString() == tr("handle"))
					{
						handle = attribs[i].value().toString();
					}
					else
						if (attribs[i].name().toString() == tr("z"))
						{
							z = attribs[i].value().toString().toDouble(&ok);
							if (!ok) z = 1.0;
						}
			}

			while (!nodeReader.atEnd() && !(nodeReader.isEndElement() && nodeReader.name() == QObject::tr("ConnectionItem")))
			{
				if (nodeReader.name() == "ConnectionGraphicsItem")
				{
					connection = ConnectionGraphicsReader::readConnectionGraphics(nodes,connections, &nodeReader);
				}
				nodeReader.readNext();
			}
			if (connection && !type.isEmpty())
			{
				connection->className = type;
			}
			return connection;
		}
		return 0;
	}

	NodeGraphicsItem * LoadSaveTool::readNode(NodeGraphicsReader & reader,QString& handle, QTransform& transform,QPointF& pos, qreal& z, int & sceneNumber)
	{
		if (!(reader.isStartElement() && reader.name() == QObject::tr("NodeItem"))) return 0;

		QXmlStreamAttributes attribs = reader.attributes();
		QString type;
		bool ok;
		for (int i=0; i < attribs.size(); ++i)
		{
			if (attribs[i].name().toString() == tr("className"))
			{
				type = attribs[i].value().toString();
			}
			else
			if (attribs[i].name().toString() == tr("scene"))
			{
				sceneNumber = attribs[i].value().toString().toInt(&ok);
			}
			else
				if (attribs.at(i).name().toString() == tr("handle"))
				{
					handle = attribs[i].value().toString();
				}
				else
					if (attribs[i].name().toString() == tr("z"))
					{
						z = attribs[i].value().toString().toDouble(&ok);
						if (!ok) z = 1.0;
					}
		}

		NodeGraphicsItem * node = 0;

		if (type == ArrowHeadItem::CLASSNAME)
			node = new ArrowHeadItem;
		else
			node = new NodeGraphicsItem;

		if (!type.isEmpty()) node->className = type;

		qreal n=0,m11=0,m12=0,m21=0,m22=0;

		while (!reader.atEnd() && !(reader.isEndElement() && reader.name() == QObject::tr("NodeItem")))
		{
			if (reader.isStartElement())
			{
				if (reader.name() == "listOfRenderInformation")
				{
					reader.readNodeGraphics(node,reader.device());
				}
				else
					if (reader.name() == "pos")
					{
						attribs = reader.attributes();
						if (attribs.size() == 2)
						{
							n = attribs.at(0).value().toString().toDouble(&ok);
							if (ok)
								pos.rx() = n;

							n = attribs.at(1).value().toString().toDouble(&ok);
							if (ok)
								pos.ry() = n;
						}
					}
					else
						if (reader.name() == "transform")
						{
							QXmlStreamAttributes attribs = reader.attributes();
							if (attribs.size() == 4)
							{
								n = attribs.at(0).value().toString().toDouble(&ok);
								if (ok) m11 = n;

								n = attribs.at(1).value().toString().toDouble(&ok);
								if (ok) m12 = n;

								n = attribs.at(2).value().toString().toDouble(&ok);
								if (ok) m21 = n;

								n = attribs.at(3).value().toString().toDouble(&ok);
								if (ok) m22 = n;
							}
						}
			}
			reader.readNext();
		}
		if (node)
		{
			transform.setMatrix(m11,m12,0.0, m21, m22, 0.0, 0.0, 0.0, 1.0);
			node->refresh();
			node->setPos(pos);
		}
		return node;
	}

	LoadSaveTool::~LoadSaveTool()
	{
		for (int i=0; i < loadCommands.size(); ++i)
			delete loadCommands[i];
	
		//cleanup cached models
		QList<CachedModel*> cached = cachedModels.values();
		cachedModels.clear();
		
		QList<ItemHandle*> handles;
		
		for (int i=0; i < cached.size(); ++i)
			if (cached[i])
			{
				if (cached[i]->globalHandle)
					if (!MainWindow::invalidPointers.contains((void*)cached[i]->globalHandle))
					{
						MainWindow::invalidPointers[ cached[i]->globalHandle ] = true;
						delete cached[i]->globalHandle;
					}
				
				handles = getHandle(cached[i]->items,true);
				
				for (int j=0; j < handles.size(); ++j)
					if (!MainWindow::invalidPointers.contains((void*)handles[j]))
					{
						MainWindow::invalidPointers[ (void*)handles[j] ] = true;
						delete handles[j];
					}
				cached[i]->items.clear();
				cached[i]->globalHandle= 0;
				delete cached[i];
			}
	}
	
	NodeFamily * LoadSaveTool::getNodeFamily(const QString& name)
	{
		return Ontology::nodeFamily(name);
	}
	
	ConnectionFamily * LoadSaveTool::getConnectionFamily(const QString& name)
	{
		return Ontology::connectionFamily(name);
	}

	void LoadSaveTool::readUnitsFromTable(const TextDataTable & units)
	{
		if (units.columns() < 2) return;

		for (int i=0; i < units.rows(); ++i)
		{
			QString s = units.rowName(i).toLower();
			if (s.startsWith("_"))
				s = s.right(s.length() - 1);
			NodeFamily * node;
			ConnectionFamily * conn;
			if (node = Ontology::nodeFamily(s))
				node->measurementUnit = Unit( units(i,0), units(i,1) );
			else
			if (conn = Ontology::connectionFamily(s))
				conn->measurementUnit = Unit( units(i,0), units(i,1) );
		}
	}
	
	void LoadSaveTool::saveUnitsToTable(TextDataTable & units)
	{
		units.description() = tr("Measurement units");
		QList<NodeFamily*> nodes = Ontology::allNodeFamilies();
		units.resize(0,0);
		
		for (int i=0; i < nodes.size(); ++i)
			if (!nodes[i]->measurementUnit.name.isEmpty() && 
				!nodes[i]->measurementUnit.property.isEmpty())
			{
				units(tr("_") + nodes[i]->name(), "property") = nodes[i]->measurementUnit.property;
				units(tr("_") + nodes[i]->name(), "name") = nodes[i]->measurementUnit.name;
			}
		
		QList<ConnectionFamily*> connections = Ontology::allConnectionFamilies();
		
		for (int i=0; i < connections.size(); ++i)
			if (!connections[i]->measurementUnit.name.isEmpty() && 
				!connections[i]->measurementUnit.property.isEmpty())
			{
				units(tr("_") + connections[i]->name(), "property") = connections[i]->measurementUnit.property;
				units(tr("_") + connections[i]->name(), "name") = connections[i]->measurementUnit.name;
			}
	}

	void LoadSaveTool::mapDataTables(const QList<ItemHandle*> fromSet, const QList<ItemHandle*> toSet)
	{
		for (int i=0; i < fromSet.size(); ++i)
			for (int j=0; j < toSet.size(); ++j)
				if (fromSet[i] && toSet[j] && !fromSet[i]->name.isEmpty() &&
					fromSet[i]->name == toSet[j]->name) //exact match
					{
						toSet[j]->copyDataTablesFrom(fromSet[i]);
						if (!toSet[j]->children.isEmpty() && !fromSet[i]->children.isEmpty())
							mapDataTables(fromSet[i]->children, toSet[j]->children);
					}
	}
}

