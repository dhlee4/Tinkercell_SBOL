/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This tool places a stoichiometry table and a table of rates inside all connection handles.
An associated GraphicsTool is also defined. This allow brings up a table for editting
the stoichiometry and rates tables.

****************************************************************************/

#include <QSettings>
#include <QToolBox>
#include <QInputDialog>
#include "GraphicsScene.h"
#include "NetworkHandle.h"
#include "UndoCommands.h"
#include "MainWindow.h"
#include "NodeGraphicsReader.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ConsoleWindow.h"
#include "ConnectionSelection.h"
#include "StoichiometryTool.h"
#include "ModelSummaryTool.h"
#include "ParametersTool.h"
#include "CThread.h"
#include "DefaultReactionRates.h"
#include "ConnectionsTree.h"
#include "EquationParser.h"
#include "muParserDef.h"
#include "muParser.h"
#include "muParserInt.h"
#include <QtDebug>

namespace Tinkercell
{

	void StoichiometryTool::select(int)
	{
	}

	void StoichiometryTool::deselect(int)
	{
	}

	bool StoichiometryTool::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);

		if (mainWindow)
		{
			connect(this,SIGNAL(dataChanged(const QList<ItemHandle*>&)),
					mainWindow,SIGNAL(dataChanged(const QList<ItemHandle*>&)));

			connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle * , const QList<ItemHandle*>&)),
				this, SLOT(itemsInserted(NetworkHandle * , const QList<ItemHandle*>& )));

			connect(mainWindow,SIGNAL(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,SLOT(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)));
			
			connect(mainWindow, SIGNAL(itemsAboutToBeRemoved(GraphicsScene *, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>& )),
				this, SLOT(itemsAboutToBeRemoved(GraphicsScene *, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>& )));
			
			connect(mainWindow, SIGNAL(handleFamilyChanged(NetworkHandle*, const QList<ItemHandle*>&, const QList<ItemFamily*>&)),
				this, SLOT(handleFamilyChanged(NetworkHandle*, const QList<ItemHandle*>&, const QList<ItemFamily*>&)));

			connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));

			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(toolLoaded(Tool*)));

			connect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));

			setWindowTitle(name);
			setWindowFlags(Qt::Dialog);
			
		}
		return (mainWindow != 0);
	}

	void StoichiometryTool::toolLoaded(Tool*)
	{
		static bool connected1 = false;
		static bool connected2 = false;

		if (connected1 && connected2) return;

		if (!connected1 && mainWindow && mainWindow->tool(tr("Model Summary")))
		{
			QWidget * widget = mainWindow->tool(tr("Model Summary"));
			ModelSummaryTool * modelSummary = static_cast<ModelSummaryTool*>(widget);
			connect(modelSummary,SIGNAL(aboutToDisplayModel(const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)),
				this,SLOT(aboutToDisplayModel(const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)));
			connect(modelSummary,SIGNAL(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)),
				this,SLOT(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)));
			connected1 = true;
		}

		if (!connected2 && mainWindow->tool(tr("Connection Selection")))
		{
			QWidget * widget = mainWindow->tool(tr("Connection Selection"));
			ConnectionSelection * connectionSelection = static_cast<ConnectionSelection*>(widget);
			if (connectionSelection)
			{
				connected2 = true;
				connect(this,SIGNAL(setMiddleBox(int,const QString&)),connectionSelection, SLOT(showMiddleBox(int,const QString&)));
			}
		}
	}

	void StoichiometryTool::historyUpdate(int )
	{
		if (connectionHandle && ratePlotWidget && stoichiometryWidget && 
				(ratePlotWidget->isVisible() || stoichiometryWidget->isVisible() ||
					(ratePlotWidget->parentWidget() && ratePlotWidget->parentWidget()->isVisible()) ||
					(stoichiometryWidget->parentWidget() && stoichiometryWidget->parentWidget()->isVisible()))
			)
		{
			updateWidgets();
		}
	}

	void StoichiometryTool::connectCFuntions()
	{
		connect(fToS,SIGNAL(getStoichiometry(QSemaphore*,NumericalDataTable*,QList<ItemHandle*>&)),this,SLOT(getStoichiometrySlot(QSemaphore*,NumericalDataTable*,QList<ItemHandle*>&)));
		connect(fToS,SIGNAL(setStoichiometry(QSemaphore*,QList<ItemHandle*>&,const NumericalDataTable&)),this,SLOT(setStoichiometrySlot(QSemaphore*,QList<ItemHandle*>&,const NumericalDataTable&)));
		connect(fToS,SIGNAL(getRates(QSemaphore*,QStringList*,QList<ItemHandle*>&)),this,SLOT(getRatesSlot(QSemaphore*,QStringList*,QList<ItemHandle*>&)));
		connect(fToS,SIGNAL(setRates(QSemaphore*,QList<ItemHandle*>&,const QStringList&)),this,SLOT(setRatesSlot(QSemaphore*,QList<ItemHandle*>&,const QStringList&)));
	}

	typedef void (*tc_StoichiometryTool_api)(
		tc_matrix (*getStoichiometry)(tc_items ),
		void (*setStoichiometry)(tc_items ,tc_matrix ),
		tc_strings (*getRates)(tc_items ),
		void (*setRates)(tc_items ,tc_strings )
		);

	void StoichiometryTool::setupFunctionPointers( QLibrary * library)
	{
		tc_StoichiometryTool_api f = (tc_StoichiometryTool_api)library->resolve("tc_StoichiometryTool_api");
		if (f)
		{
			f(
				&(_getStoichiometry),
				&(_setStoichiometry),
				&(_getRates),
				&(_setRates)
			);
		}
	}

	void StoichiometryTool::getStoichiometrySlot(QSemaphore * s, NumericalDataTable* p, QList<ItemHandle*>& items)
	{
		if (p)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < items.size(); ++i)
				if (items[i])
					handles << items[i];
			(*p) = (getStoichiometry(handles));
		}
		if (s)
			s->release();
	}

	void StoichiometryTool::getRatesSlot(QSemaphore * s, QStringList* p, QList<ItemHandle*>& items)
	{
		if (p)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < items.size(); ++i)
				if (items[i])
					handles << items[i];
			(*p) = getRates(handles);
		}
		if (s)
			s->release();
	}


	void StoichiometryTool::setStoichiometrySlot(QSemaphore * s, QList<ItemHandle*>& items, const NumericalDataTable& table)
	{
		if (mainWindow && mainWindow->currentNetwork())
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < items.size(); ++i)
				if (items[i])
					handles << items[i];
			setStoichiometry(mainWindow->currentNetwork(),handles,table);
		}
		if (s)
			s->release();
	}

	void StoichiometryTool::setRatesSlot(QSemaphore * s, QList<ItemHandle*>& items, const QStringList& rates)
	{
		if (mainWindow && mainWindow->currentNetwork())
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < items.size(); ++i)
				if (items[i])
					handles << items[i];
			setRates(mainWindow->currentNetwork(),handles,rates);
		}
		if (s)
			s->release();
	}

	void StoichiometryTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& list, QPointF , Qt::KeyboardModifiers )
	{
		/*connectionHandle = 0;
		if (scene)
		{
			ConnectionGraphicsItem * connection = 0;
			for (int i=0; i < list.size(); ++i)
			{
				connection = ConnectionGraphicsItem::cast(list[i]);
				if (connection && connection->handle() && connection->handle()->type == ConnectionHandle::TYPE)
				{
					connectionHandle = static_cast<ConnectionHandle*>(connection->handle());
					break;
				}
			}
		}*/

		bool reactions = false, species = false;
		ItemHandle * handle;

		for (int i=0; i < list.size(); ++i)
		{
			handle = getHandle(list[i]);
			
			if (ConnectionHandle::cast(handle))
				reactions = true;
			
			if (NodeHandle::cast(handle) && handle->isA(tr("Molecule")))
				species = true;
				
			if (reactions && species)
				break;
		}
		
		if (reactions || species)
		{
			if (separator)
				mainWindow->contextItemsMenu.addAction(separator);
			else
				separator = mainWindow->contextItemsMenu.addSeparator();













			if (reactions)
				mainWindow->contextItemsMenu.addAction(autoReverse);
			else
				mainWindow->contextItemsMenu.removeAction(autoReverse);
			
			if (species)
				mainWindow->contextItemsMenu.addAction(autoDimer);
			else
				mainWindow->contextItemsMenu.removeAction(autoDimer);
		}
		else
		{
			if (separator)
				mainWindow->contextItemsMenu.removeAction(separator);
			mainWindow->contextItemsMenu.removeAction(autoReverse);
			mainWindow->contextItemsMenu.removeAction(autoDimer);
		}
	}
	
	void StoichiometryTool::aboutToDisplayModel(const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations)
	{
		connectionHandle = 0;
		ConnectionHandle * connection = 0;
		for (int i=0; i < items.size(); ++i)
			if (connection = ConnectionHandle::cast(items[i]))
			{			
				if (!connectionHandle)
				{
					connectionHandle = connection;
					updateWidgets();
				}
				
				if (connectionHandle && connectionHandle->hasTextData(tr("Rate equations")))
				{
					TextDataTable & eqns = connectionHandle->textDataTable(tr("Rate equations"));
					for (int j=0; j < eqns.rows(); ++j)
						equations[ connectionHandle->fullName() + tr(".row") + QString::number(j)  ] = eqns(j,0);
				}
			}
	}
	
	void StoichiometryTool::displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations)
	{
		if (connectionHandle && ratePlotWidget && stoichiometryWidget)
		{
			widgets.insertTab(0,ratePlotWidget,tr("Rate equation"));
			widgets.insertTab(1,stoichiometryWidget,tr("Reaction stoichiometry"));
		}
	}
	
	void StoichiometryTool::handleFamilyChanged(NetworkHandle * network, const QList<ItemHandle*>& handles, const QList<ItemFamily*>& )
	{
		ConnectionFamily * connectionFamily;
		ConnectionHandle * connectionHandle;
		
		QList<NumericalDataTable*> oldparameters, newparameters;
		QList<TextDataTable*> oldequations, newequations;
		
		QList<ItemHandle*> changedHandles;

		for (int i=0; i < handles.size(); ++i)
		{
			if ((connectionHandle = ConnectionHandle::cast(handles[i])) &&
				(connectionFamily = ConnectionFamily::cast(connectionHandle->family())) &&
				connectionFamily->participantRoles().contains(tr("catalyst"),Qt::CaseInsensitive) &&
				connectionHandle->hasNumericalData(tr("Parameters")) &&
				connectionHandle->hasTextData(tr("Rate equations")) &&
				connectionHandle->hasTextData(tr("Participants")) &&
				!changedHandles.contains(connectionHandle))
				{					
					NumericalDataTable * oldparams = &connectionHandle->numericalDataTable(tr("Parameters"));
					TextDataTable * oldeqns = &connectionHandle->textDataTable(tr("Rate equations"));
					TextDataTable & participants = connectionHandle->textDataTable(tr("Participants"));

					bool validEquation = true;
					for (int j=0; j < participants.rows(); ++j)
						if (!oldeqns->value(0,0).contains(participants.at(j,0)))
						{
							validEquation = false;
							break;
						}
					
					if (validEquation) continue;

					changedHandles << connectionHandle;
					if (!oldparams->rowNames().contains(tr("Kcat")) ||
						!oldparams->rowNames().contains(tr("Km")))
					{
						oldparameters << oldparams;
						NumericalDataTable * newparams = new NumericalDataTable(*oldparams);
						newparams->value(tr("Kcat"),0) = 1.0;
						newparams->value(tr("Km"),0) = 1.0;
						newparameters << newparams;
					}
					
					QString enz;
					QStringList reacs;
					
					for (int j=0; j < participants.rows(); ++j)
					{
						if (participants.rowName(j).toLower() == tr("catalyst"))
							enz = participants(j,0);
						else
						if (participants.rowName(j).toLower().contains(tr("substrate")) || participants.rowName(j).toLower().contains(tr("reactant")))
							reacs += participants(j,0);
					}
					
					if (!enz.isEmpty() && !reacs.isEmpty())
					{
						QString s = connectionHandle->fullName();
						QString r = reacs.join(tr("*"));
						s += tr(".Kcat * ");
						s += enz;
						s += tr(" * ");
						s += r;
						s += tr("/( ");
						s += connectionHandle->fullName();
						s += tr(".Km + ");
						s += r;
						s += tr(")");
						TextDataTable * neweqns = new TextDataTable(*oldeqns);
						neweqns->value(0,0) = s;
						oldequations << oldeqns;
						newequations << neweqns;
					}
				}
		}
		
		if (!newequations.isEmpty())
		{
			network->push(new Change2DataCommand<double,QString>(
								tr("Rate equations updated"),
								oldparameters, newparameters,
								oldequations, newequations));

			for (int i=0; i < newparameters.size(); ++i) delete newparameters[i];
			for (int i=0; i < newequations.size(); ++i) delete newequations[i];

			emit dataChanged(changedHandles);
		}
	}

	void StoichiometryTool::itemsInserted(NetworkHandle * network, const QList<ItemHandle*>& handles)
	{
		ConnectionHandle * connectionHandle = 0;
		for (int i=0; i < handles.size(); ++i)
		{
			if ((connectionHandle = ConnectionHandle::cast(handles[i])) && handles[i]->isA(tr("Biological process")))
			{
				if (connectionHandle &&
					(
					!(connectionHandle->hasNumericalData(tr("Reactant stoichiometries"))) ||
					!(connectionHandle->hasNumericalData(tr("Product stoichiometries"))) ||
					!(connectionHandle->hasTextData(tr("Rate equations")))
					))
				{
					if (handles[i]->isA(tr("Multimerization")))
					{
						double n = QInputDialog::getDouble(this,tr("Reaction stoichiometry"),tr("Numer of monomers in ") + handles[i]->fullName() + tr(" : "),2.0);
						if (n < 2.0) 
							n = 2.0;
						handles[i]->numericalData(tr("Parameters"),tr("monomers"),tr("value")) = n;
					}
					insertDataMatrix(connectionHandle);
					handleFamilyChanged(network, QList<ItemHandle*>() << handles[i],  QList<ItemFamily*>() );
				}
			}
		}
	}

	bool StoichiometryTool::parseRateString(NetworkHandle * win, ItemHandle * handle, QString& s)
	{
		return EquationParser::validate(win, handle, s, QStringList() << "time" << "Time" << "TIME");
	}

	StoichiometryTool::StoichiometryTool() : Tool(tr("Stoichiometry and Rates"),tr("Modeling")),
		autoReverse(new QAction("Make reversible",this)),
		autoDimer(new QAction("Make multimer",this)),
		separator(0),
		currentRow(0),
		pickRow1(0), pickRow2(0)
	{
		StoichiometryTool::fToS = new StoichiometryTool_FToS;
		StoichiometryTool::fToS->setParent(this);
		
		QString appDir = QCoreApplication::applicationDirPath();
	
		setPalette(QPalette(QColor(255,255,255,255)));
		setAutoFillBackground(true);

		connectCFuntions();

		connect(autoReverse,SIGNAL(triggered()),this,SLOT(addReverseReaction()));
		autoReverse->setIcon(QIcon(":/images/horizontalFlip.png"));
		
		connect(autoDimer,SIGNAL(triggered()),this,SLOT(addDimer()));
		autoDimer->setIcon(QIcon(":/images/plus.png"));
		
		ratePlotWidget = new QWidget(this);
		QHBoxLayout * rateEquationLayout = new QHBoxLayout;
		rateEquationLayout->addWidget(plotLineEdit = new QLineEdit);
		rateEquationLayout->addWidget(pickRow1 = new QComboBox);
		connect(pickRow1,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
		
		QVBoxLayout * plotLayout = new QVBoxLayout;
		plotLayout->addLayout(rateEquationLayout);		
		plotLayout->addSpacing(20);		
		plotLayout->addWidget(graphWidget = new EquationGraph);
		ratePlotWidget->setLayout(plotLayout);
		connect(plotLineEdit,SIGNAL(editingFinished()),this,SLOT(rateEquationChanged()));

		stoichiometryWidget = new QWidget(this);
		stoichiometryLayout = new QHBoxLayout;
		
		QHBoxLayout * stoicSuperLayout = new QHBoxLayout;
		stoicSuperLayout->addWidget(pickRow2 = new QComboBox);
		connect(pickRow2,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
		stoicSuperLayout->addStretch(2);
		stoicSuperLayout->addLayout(stoichiometryLayout,1);
		stoicSuperLayout->addStretch(2);
		stoichiometryWidget->setLayout(stoicSuperLayout);
	}
	
	void StoichiometryTool::selectedRowChanged(int i)
	{
		if (!pickRow1 || !pickRow2) return;
		
		disconnect(pickRow1,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
		disconnect(pickRow2,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
		pickRow1->setCurrentIndex(i);
		pickRow2->setCurrentIndex(i);
		currentRow = i;
		updatePlotWidget();
		updateStoichiometryWidget();
		connect(pickRow1,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
		connect(pickRow2,SIGNAL(currentIndexChanged(int)),this,SLOT(selectedRowChanged(int)));
	}

	void StoichiometryTool::addReverseReaction()
	{
		GraphicsScene * scene = currentScene();
		if (!scene || !scene->network) return;

		QList<ItemHandle*> items;
		QList<NumericalDataTable*> nDataNew, nDataOld;
		QList<TextDataTable*> sDataNew, sDataOld;
		NumericalDataTable* nDat1, *nDat2;
		TextDataTable* sDat;

		QList<QGraphicsItem*>& selected = scene->selected();
		QList<ConnectionGraphicsItem*> connections;
		QList<QPointF> points;
		ItemHandle* handle;
		QStringList cannots;

		for (int i=0; i < selected.size(); ++i)
		{
			ConnectionGraphicsItem * connection = ConnectionGraphicsItem::cast(selected[i]);
			if (
				connection &&	(handle = connection->handle())
				&& handle->hasNumericalData(tr("Reactant stoichiometries"))
				&& handle->hasNumericalData(tr("Product stoichiometries"))
				&& handle->hasTextData(tr("Rate equations")))
			{
				items << handle;

				if (handle->numericalDataTable(tr("Reactant stoichiometries")).rows() != 1 ||
					handle->numericalDataTable(tr("Product stoichiometries")).rows() != 1 ||
					handle->textDataTable(tr("Rate equations")).rows() != 1)
				{
					cannots << handle->fullName();
				}
				else
				{
					nDat1 = new NumericalDataTable(handle->numericalDataTable(tr("Reactant stoichiometries")));
					nDat2 = new NumericalDataTable(handle->numericalDataTable(tr("Product stoichiometries")));
					sDat = new TextDataTable(handle->textDataTable(tr("Rate equations")));

					nDat1->setRowName(0,tr("forward")); 
					nDat2->setRowName(0,tr("forward"));
					sDat->setRowName(0,tr("forward"));
					nDat1->insertRow(1,tr("reverse"));
					nDat2->insertRow(1,tr("reverse"));
					sDat->insertRow(1,tr("reverse"));

					QStringList reactants;
					for (int j=0; j < nDat1->columns(); ++j)
						nDat2->value(1,nDat1->columnName(j)) = nDat1->value(0,j);

					for (int j=0; j < nDat2->columns(); ++j)
					{
						nDat1->value(1,nDat2->columnName(j)) = nDat2->value(0,j);
						if (nDat2->value(0,j) > 0)
						{
							if (nDat2->value(0,j) == 1)
								reactants << nDat2->columnName(j);
							else
								reactants << ( nDat2->columnName(j) + tr("^") + QString::number(nDat2->value(0,j)) );
						}
					}

					if (reactants.isEmpty())
						sDat->value(1,0) = handle->fullName() + tr(".k0");
					else
						sDat->value(1,0) = handle->fullName() + tr(".k0*") + reactants.join(tr("*"));

					nDataNew << nDat1 << nDat2;
					sDataNew << sDat;

					nDataOld << &(handle->numericalDataTable(tr("Reactant stoichiometries"))) 
							 << &(handle->numericalDataTable(tr("Product stoichiometries")));
					sDataOld << &(handle->textDataTable(tr("Rate equations")));

					QList<NodeGraphicsItem*> nodes = connection->nodes();
					for (int j=0; j < nodes.size(); ++j)
						if (nodes[j])
						{
							connections << connection;					
							points << nodes[j]->scenePos();
						}
				}
			}
		}

		if (nDataNew.size() > 0)
		{
			QList<QUndoCommand*> commands;
			commands  += new Change2DataCommand<qreal,QString>(tr("Reversible"),nDataOld,nDataNew,sDataOld,sDataNew);
			commands += new AddArrowHeadCommand(connections, points);

			scene->network->push(new CompositeCommand(tr("Reversible reactions added"), commands));
			
			emit dataChanged(items);

			for (int i=0; i < nDataNew.size(); ++i)
				delete nDataNew[i];

			for (int i=0; i < sDataNew.size(); ++i)
				delete sDataNew[i];
		}

		if (cannots.size() > 0)
		{
			if (console())
                console()->message(tr("Automatic reversibility can only be added to connections with one reaction, so the following were ignored: ") +
            cannots.join(tr(",")));
		}
		else
		{
			QString appDir = QCoreApplication::applicationDirPath();
			QString filename = tr(":/images/Reversible.xml");
			emit setMiddleBox(1,filename);
		}
	}
	
	void StoichiometryTool::addDimer()
	{
		GraphicsScene * scene = currentScene();
		if (!scene || !mainWindow || !scene->network) return;
		if (!mainWindow->tool(tr("Connections Tree"))) return;

		QWidget * treeWidget = mainWindow->tool(tr("Connections Tree"));
		ConnectionsTree * connectionsTree = static_cast<ConnectionsTree*>(treeWidget);

		if (!connectionsTree->getFamily("Multimerization")) return;

		NodeFamily * nodeFamily = 0;
		ConnectionFamily * connectionFamily = connectionsTree->getFamily("Multimerization");

		QList<QGraphicsItem*>& selected = scene->selected();
		ItemHandle * handle = 0;

		QList<QGraphicsItem*> list;

		QString appDir = QCoreApplication::applicationDirPath();

		QList<ItemHandle*> visitedHandles;		
		QStringList newNames;
		
		double n = QInputDialog::getDouble(this,tr("Reaction stoichiometry"),tr("Numer of monomers in complex : "),2.0);
		if (n < 2.0) 
			n = 2.0;

		for (int i=0; i < selected.size(); ++i)
		{
			handle = getHandle(selected[i]);
			if (NodeGraphicsItem::cast(selected[i]) && handle && handle->isA(tr("Molecule")) && !visitedHandles.contains(handle))
			{
				nodeFamily = NodeFamily::cast(handle->family());
				if (!nodeFamily) continue;

				visitedHandles += handle;
				NodeHandle * node = new NodeHandle(nodeFamily);
				node->name = handle->name + tr("_multi");
				node->name = scene->network->makeUnique(node->name,newNames);
				newNames << node->name;

				qreal xpos = (selected[i]->sceneBoundingRect().right() + 100.0),
					  ypos = (selected[i]->sceneBoundingRect().top() - 100.0),
					  height = 0.0;

				NodeGraphicsItem * image = 0;

				for (int k=0; k < 2; ++k)
					for (int j=0; j < nodeFamily->graphicsItems.size(); ++j)
					{
						image = (NodeGraphicsItem::topLevelNodeItem(nodeFamily->graphicsItems[j]));
						if (image)
						{
							image = image->clone();

							if (image->defaultSize.width() > 0 && image->defaultSize.height() > 0)
								image->scale(image->defaultSize.width()/image->sceneBoundingRect().width(),image->defaultSize.height()/image->sceneBoundingRect().height());

							qreal w = image->sceneBoundingRect().width();

							image->setPos(xpos, ypos);

							image->setBoundingBoxVisible(false);

							if (image->isValid())
							{
								xpos += w;
								setHandle(image,node);
								list += image;
							}
							if (image->sceneBoundingRect().height() > height)
								height = image->sceneBoundingRect().height();
						}
					}
				
				if (image)
				{
					TextGraphicsItem * nameItem;
					QFont font;

					ConnectionGraphicsItem * item = new ConnectionGraphicsItem;
					ConnectionHandle * connection = new ConnectionHandle(connectionFamily,item);

					item->curveSegments +=
						ConnectionGraphicsItem::CurveSegment(1,new ConnectionGraphicsItem::ControlPoint(item,selected[i]));

					item->curveSegments +=
						ConnectionGraphicsItem::CurveSegment(1,new ConnectionGraphicsItem::ControlPoint(item,image));

					if (!connectionFamily->graphicsItems.isEmpty())
					{
						NodeGraphicsItem * node = NodeGraphicsItem::cast(connectionFamily->graphicsItems[0]);
						if (node && ArrowHeadItem::cast(node))
						{
							ArrowHeadItem * arrow = static_cast<ArrowHeadItem*>(node->clone());
							arrow->connectionItem = item;
							item->curveSegments.last().arrowStart = arrow;
							list += item->curveSegments.last().arrowStart;
						}
					}

					connection->name = tr("J1");
					item->lineType = ConnectionGraphicsItem::line;
					connection->name = scene->network->makeUnique(connection->name,newNames);
					newNames << connection->name;

					nameItem = new TextGraphicsItem(connection,0);
					list += nameItem;
					nameItem->setPos( 0.5*(image->pos() + selected[i]->scenePos() ) );
					font = nameItem->font();
					font.setPointSize(22);
					nameItem->setFont(font);
					
					nameItem = new TextGraphicsItem(node,0);
					list += nameItem;
					nameItem->setPos( image->sceneBoundingRect().bottomRight() );
					font = nameItem->font();
					font.setPointSize(22);
					nameItem->setFont(font);



					//make the rates and stoichiometry table
					NumericalDataTable reactants, products;
					TextDataTable rates;
					
					rates.resize(3,1);
					reactants.resize(3,2);
					products.resize(3,2);
					reactants.description() = QString("Number of each reactant participating in this reaction");
					products.description() = QString("Number of each product participating in this reaction");
					rates.description() = QString("Rates: a set of rates, one for each reaction represented by this item. Row names correspond to reaction names. The number of rows in this table and the stoichiometry table will be the same.");
					
					rates.setColumnName(0,tr("rates"));
					
					reactants.setRowName(0,tr("forward"));
					products.setRowName(0,tr("forward"));
					rates.setRowName(0,tr("forward"));

					reactants.setRowName(1,tr("reverse"));
					products.setRowName(1,tr("reverse"));
					rates.setRowName(1,tr("reverse"));
					
					reactants.setRowName(2,tr("diffuse"));
					products.setRowName(2,tr("diffuse"));
					rates.setRowName(2,tr("diffuse"));

					reactants.setColumnName(0, handle->fullName());
					products.setColumnName(0,handle->fullName());
					reactants.setColumnName(1,node->fullName());
					products.setColumnName(1, node->fullName());

					reactants.value(0,0) = n;
					products.value(0,1) = 1.0;
					
					reactants.value(1,1) = 1.0;
					products.value(1,0) = n;
					
					reactants.value(2,1) = 1.0;
					
					rates.value(0,0) = connection->fullName() + tr(".kf * ") + handle->fullName() + tr("^") + QString::number(n);
					rates.value(1,0) = connection->fullName() + tr(".kb * ") + node->fullName();
					rates.value(2,0) = connection->fullName() + tr(".deg * ") + node->fullName();

					connection->numericalDataTable(tr("Reactant stoichiometries")) = reactants;
					connection->numericalDataTable(tr("Product stoichiometries")) = products;
					connection->textDataTable(tr("Rate equations")) = rates;
					connection->numericalData(tr("Parameters"),tr("monomers"),tr("value")) = n;
					connection->numericalData(tr("Parameters"),tr("monomers"),tr("min")) = n/2.0;
					connection->numericalData(tr("Parameters"),tr("monomers"),tr("max")) = n*2.0;
					connection->numericalData(tr("Parameters"),tr("kf"),tr("value")) = 1.0;
					connection->numericalData(tr("Parameters"),tr("kf"),tr("min")) = 0.0;
					connection->numericalData(tr("Parameters"),tr("kf"),tr("max")) = 100.0;
					connection->numericalData(tr("Parameters"),tr("kb"),tr("value")) = 0.1;
					connection->numericalData(tr("Parameters"),tr("kb"),tr("min")) = 0.0;
					connection->numericalData(tr("Parameters"),tr("kb"),tr("max")) = 100.0;
					connection->numericalData(tr("Parameters"),tr("deg"),tr("value")) = 0.1;
					connection->numericalData(tr("Parameters"),tr("deg"),tr("min")) = 0.0;
					connection->numericalData(tr("Parameters"),tr("deg"),tr("max")) = 1.0;

					list += item;
				}
			}
		}
		if (!list.isEmpty())
		{
			scene->insert(tr("dimer added"),list);
		}
	}	

	QSize StoichiometryTool::sizeHint() const
	{
		return QSize(600, 200);
	}

	void StoichiometryTool::insertDataMatrix(ConnectionHandle * handle)
	{
		if (!ConnectionHandle::cast(handle)) return;
		DefaultRateAndStoichiometry::setDefault(handle);
	}
	
	bool StoichiometryTool::updatePlotWidget()
	{
		if (!connectionHandle ||
			!plotLineEdit ||
			!graphWidget ||
			!pickRow1 ||
			!pickRow2 ||
			!connectionHandle->hasTextData(tr("Rate equations")) ||
			!connectionHandle->children.isEmpty())
			return false;
		
		TextDataTable& ratesTable = connectionHandle->textDataTable(tr("Rate equations"));
		
		int row = currentRow;
		
		QString rate = ratesTable.value(row,0);
		plotLineEdit->setText(rate);
		graphWidget->setFormula(rate, currentNetwork());
		if (ratesTable.rows() > 1)
			graphWidget->setTitle(connectionHandle->name + tr(" ") + ratesTable.rowName(row) + tr(" rate formula"));
		else
			graphWidget->setTitle(connectionHandle->name + tr(" rate formula"));
		graphWidget->setYLabel(connectionHandle->name);
		
		return true;
	}
	
	bool StoichiometryTool::updateStoichiometryWidget()
	{
		if (!connectionHandle || 
			!connectionHandle->hasNumericalData(tr("Reactant stoichiometries")) ||
			!connectionHandle->hasNumericalData(tr("Product stoichiometries")) ||
			!stoichiometryLayout ||
			!connectionHandle->children.isEmpty())
			{
				return false;
			}
		
		NumericalDataTable & reactants = connectionHandle->numericalDataTable(tr("Reactant stoichiometries")),
						   & products = connectionHandle->numericalDataTable(tr("Product stoichiometries"));

		int row = currentRow;

		QLineEdit * line;
		while (reactantCoeffs.size() < reactants.columns())
		{
			reactantCoeffs += (line = new QLineEdit(this));			
			line->setFixedWidth(50);
			connect(line,SIGNAL(editingFinished()),this,SLOT(stoichiometryChanged()));
		}
		
		while (reactantNames.size() < reactants.columns())
			reactantNames += (new QLabel(this));
			
		while (productNames.size() < products.columns())
			productNames += (new QLabel(this));
		
		while (productCoeffs.size() < products.columns())
		{
			productCoeffs += (line = new QLineEdit(this));
			line->setFixedWidth(50);
			connect(line,SIGNAL(editingFinished()),this,SLOT(stoichiometryChanged()));
		}
		
		for (int i=0; i < reactantCoeffs.size(); ++i)
			if (reactantCoeffs[i])
			{
				stoichiometryLayout->removeWidget(reactantCoeffs[i]);
				reactantCoeffs[i]->setParent(this);
				reactantCoeffs[i]->hide();
			}
		
		for (int i=0; i < productCoeffs.size(); ++i)
			if (productCoeffs[i])
			{
				stoichiometryLayout->removeWidget(productCoeffs[i]);
				productCoeffs[i]->setParent(this);
				productCoeffs[i]->hide();
			}
			
		for (int i=0; i < reactantNames.size(); ++i)
			if (reactantNames[i])
			{
				stoichiometryLayout->removeWidget(reactantNames[i]);
				reactantNames[i]->setParent(this);
				reactantNames[i]->hide();
			}
		
		for (int i=0; i < productNames.size(); ++i)
			if (productNames[i])
			{
				stoichiometryLayout->removeWidget(productNames[i]);
				productNames[i]->setParent(this);
				productNames[i]->hide();
			}
		
		for (int i=0; i < plusSigns.size(); ++i)
			if (plusSigns[i])
			{
				stoichiometryLayout->removeWidget(plusSigns[i]);
				plusSigns[i]->setParent(this);
				plusSigns[i]->hide();
			}

		int j=0;

		for (int i=0; i < reactants.columns(); ++i)
//			if (reactants.at(row,i) > 0)
			{
				reactantCoeffs[i]->setText(QString::number(reactants.at(row,i)));
				reactantNames[i]->setText(reactants.columnName(i));
				while (plusSigns.size() <= j)
					plusSigns += (new QLabel(this));
				plusSigns[j]->setText(tr("+"));
				if (j > 0)
				{
					plusSigns[j]->show();
					stoichiometryLayout->addWidget(plusSigns[j],0,Qt::AlignCenter);
				}
				else
					plusSigns[j]->hide();
				stoichiometryLayout->addWidget(reactantCoeffs[i],0,Qt::AlignRight);
				stoichiometryLayout->addWidget(reactantNames[i],0,Qt::AlignLeft);
				++j;
				reactantCoeffs[i]->show();
				reactantNames[i]->show();
			}

		while (plusSigns.size() <= j)
			plusSigns += (new QLabel(this));
			
		if ((reactants.columns() < 1) && (products.columns() < 1))
			plusSigns[j]->setText(tr(" No changes are happing in this reaction "));
		else
			plusSigns[j]->setText(tr("->"));
		plusSigns[j]->show();
		stoichiometryLayout->addWidget(plusSigns[j],0,Qt::AlignCenter);
		
		++j;
		int j2 = 0;
		for (int i=0; i < products.columns(); ++i)
//			if (products.at(row,i) > 0)
			{
				productCoeffs[i]->setText(QString::number(products.at(row,i)));
				productNames[i]->setText(products.columnName(i));
				while (plusSigns.size() <= j)
					plusSigns += (new QLabel(this));
				plusSigns[j]->setText(tr("+"));
				if (j2 > 0)
				{
					plusSigns[j]->show();
					stoichiometryLayout->addWidget(plusSigns[j],0,Qt::AlignCenter);
				}
				else
					plusSigns[j]->hide();
				stoichiometryLayout->addWidget(productCoeffs[i],0,Qt::AlignRight);
				stoichiometryLayout->addWidget(productNames[i],0,Qt::AlignLeft);
				++j;
				++j2;
				productCoeffs[i]->show();
				productNames[i]->show();
			}
		
		if (!plusSigns.size() >= j)
			plusSigns[j-1]->hide();
		
		return true;
	}
	
	void StoichiometryTool::rateEquationChanged()
	{
		NetworkHandle * network = currentNetwork();
		if (connectionHandle && plotLineEdit && network)
		{
			QString rate = plotLineEdit->text();
			if (parseRateString(network,connectionHandle,rate))
			{
				TextDataTable newTable(connectionHandle->textDataTable(tr("Rate equations")));
				int row = currentRow;
				
				if (row < 0) return;

				if (rate != newTable.value(row,0))
				{
					newTable.value(row,0) = rate;
					if (!StoichiometryTool::userModifiedRates.contains(connectionHandle))
						StoichiometryTool::userModifiedRates << connectionHandle;
					
					QString s;
					if (newTable.rows() > 1)
						s = connectionHandle->fullName() + tr(" ") + newTable.rowName(row) + tr(" rate changed");
					else
						s = connectionHandle->fullName() + tr(" rate changed");
					network->changeData(s,
										connectionHandle,
										tr("Rate equations"),
										&newTable);
					ParametersTool::removeUnusedParametersInModel(network);
					updatePlotWidget();
				}
			}
		}
	}
	
	void StoichiometryTool::stoichiometryChanged()
	{
		NetworkHandle * network = currentNetwork();
		if (connectionHandle && network)
		{
			NumericalDataTable reactants(connectionHandle->numericalDataTable(tr("Reactant stoichiometries"))),
							   products(connectionHandle->numericalDataTable(tr("Product stoichiometries")));
			
			QString s;
			double d;
			bool ok;
			bool changed = false;			
			int row = currentRow;
			
			if (row < 0) return;

			for (int i=0; i < reactantNames.size() && i < reactantCoeffs.size(); ++i)
				if (reactantNames[i]->isVisible() && reactantCoeffs[i]->isVisible())
				{
					s = reactantNames[i]->text();
					if (!s.isEmpty())
					{
						d = reactantCoeffs[i]->text().toDouble(&ok);
						if (ok && reactants.hasColumn(s) && reactants.value(row,s) != d)
						{
							changed = true;
							reactants.value(row,s) = d;
						}
					}
				}
			
			for (int i=0; i < productNames.size() && i < productCoeffs.size(); ++i)
				if (productNames[i]->isVisible() && productCoeffs[i]->isVisible())
				{
					s = productNames[i]->text();
					if (!s.isEmpty())
					{
						d = productCoeffs[i]->text().toDouble(&ok);
						if (ok && products.hasColumn(s) && products.value(row,s) != d)
						{
							changed = true;
							products.value(row,s) = d;
						}
					}
				}
			
			if (changed)
			{
				network->changeData(connectionHandle->fullName() + tr(" stoichiometry changed"),
									QList<ItemHandle*>() << connectionHandle << connectionHandle,
									QStringList() << tr("Reactant stoichiometries") << tr("Product stoichiometries"),
									QList<NumericalDataTable*>() << &reactants << &products);
				ParametersTool::removeUnusedParametersInModel(network);
				updateStoichiometryWidget();
			}
		}
	}	

	void StoichiometryTool::updateWidgets()
	{
		if (!connectionHandle ||
			!pickRow1 ||
			!pickRow2 ||
			!connectionHandle->hasTextData(tr("Rate equations")))
		{
			connectionHandle = 0;
			if (ratePlotWidget)
				ratePlotWidget->hide();
			if (stoichiometryWidget)
				stoichiometryWidget->hide();
			return;
		}

		TextDataTable& ratesTable = connectionHandle->textDataTable(tr("Rate equations"));
		NumericalDataTable & reactants = connectionHandle->numericalDataTable(tr("Reactant stoichiometries")),
						   & products = connectionHandle->numericalDataTable(tr("Product stoichiometries"));
		if (ratesTable.rows() < 1 ||
			((reactants.columns() < 1) && (products.columns() < 1)))
		{
			connectionHandle = 0;
			if (ratePlotWidget)
				ratePlotWidget->hide();
			if (stoichiometryWidget)
				stoichiometryWidget->hide();
			return;
		}
		QString s = pickRow1->currentText();
		pickRow1->clear();
		pickRow2->clear();
		currentRow = 0;
		QStringList list = ratesTable.rowNames();
		if (list.size() < 2)
		{
			pickRow1->hide();
			pickRow2->hide();
			updatePlotWidget();
			updateStoichiometryWidget();
		}
		else
		{
			pickRow1->show();
			pickRow2->show();
			pickRow1->addItems(list);
			pickRow2->addItems(list);			
			int i = list.indexOf(s);
			if (i < 0)
				i = 0;
			pickRow1->setCurrentIndex(i);
		}
	}

	//get the stoiciometry of the items and return the matrix
	NumericalDataTable StoichiometryTool::getStoichiometry(QList<ItemHandle*>& connectionHandles, const QString& replaceDot, bool includeFixed)
	{
		NumericalDataTable combinedTable;

		if (connectionHandles.size() < 1)
		{
			return combinedTable;
		}

		QStringList columnNames, rowNames, rates;
		NumericalDataTable * nDataTable1 = 0, * nDataTable2 = 0;
		TextDataTable * sDataTable = 0;
		ConnectionHandle * connection = 0;

		bool allZero = true;
		for (int i=0; i < connectionHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			if (connectionHandles[i])
			{
				if (connectionHandles[i]->hasNumericalData(QObject::tr("Initial Value")))
				{
					if (!columnNames.contains(connectionHandles[i]->fullName()))
						columnNames += connectionHandles[i]->fullName();
				}

				if (!connectionHandles[i]->children.isEmpty())
				{
					QList<ItemHandle*> children = connectionHandles[i]->children;
					for (int j=0; j < children.size(); ++j)
						if (children[j] && ConnectionHandle::cast(children[j]) && !connectionHandles.contains(children[j]))
							connectionHandles << children[j];
							
					continue;
				}
				
				QStringList fixedSpecies;
				if (!includeFixed && ConnectionHandle::cast(connectionHandles[i]))
				{
					QList<NodeHandle*> nodes = ConnectionHandle::cast(connectionHandles[i])->nodes();
					for (int j=0; j < nodes.size(); ++j)
						if (NodeHandle::cast(nodes[j])
							&& nodes[j]->hasNumericalData(tr("Fixed"))
							&& nodes[j]->numericalData(tr("Fixed")) > 0)
							fixedSpecies << nodes[j]->fullName();
				}
				if ((connection = ConnectionHandle::cast(connectionHandles[i])) &&
					connection->children.isEmpty() &&
					connection->hasNumericalData(QObject::tr("Reactant stoichiometries")) &&
					connection->hasNumericalData(QObject::tr("Product stoichiometries")) &&
					connection->hasTextData(QObject::tr("Rate equations")))
				{
					nDataTable1 = &(connection->numericalDataTable(QObject::tr("Reactant stoichiometries")));
					nDataTable2 = &(connection->numericalDataTable(QObject::tr("Product stoichiometries")));
					sDataTable = &(connection->textDataTable(QObject::tr("Rate equations")));
					
					if ((nDataTable1->rows() < 1 && nDataTable2->rows() < 1) || sDataTable->rows() < 1)
						continue;
					
					//get unique species names in the stoichiometry matrix
					for (int j=0; j < nDataTable1->columns(); ++j) 
					{
						QString s = nDataTable1->columnName(j);

						if (!columnNames.contains(s)
							&& !fixedSpecies.contains(s)
							&& !columnNames.contains(QString(s).replace(".",replaceDot))
							&& !columnNames.contains(QString(s).replace(replaceDot,".")))
						{
							columnNames += s;
						}
					}
					for (int j=0; j < nDataTable2->columns(); ++j) 
					{
						QString s = nDataTable2->columnName(j);

						if (!columnNames.contains(s)
							&& !fixedSpecies.contains(s)
							&& !columnNames.contains(QString(s).replace(".",replaceDot))
							&& !columnNames.contains(QString(s).replace(replaceDot,".")))
						{
							columnNames += s;
						}
					}
					//if any node does not appear in the stoichiometry matrix, add it anyway
					QList<NodeHandle*> connectedNodes = connection->nodes();
					for (int j=0; j < connectedNodes.size(); ++j)
						if (connectedNodes[j] && !connectedNodes[j]->isA(tr("empty")))
						{
							QString s = connectedNodes[j]->fullName();

							if (!columnNames.contains(s))
								columnNames << s;
						}
				
					for (int j=0; j < sDataTable->rows(); ++j) //get rates and reaction names
					{
						if (sDataTable->value(j,0).isEmpty()) continue;
						allZero = true;
						
						if (nDataTable1->rows() > j)
							for (int k=0; k < nDataTable1->columns(); ++k)
								if (nDataTable1->value(j, k) != 0)
								{
									allZero = false;
									break;
								}

						if (allZero && nDataTable2->rows() > j)
							for (int k=0; k < nDataTable2->columns(); ++k)
								if (nDataTable2->value(j, k) != 0)
								{	
									allZero = false;
									break;
								}

						if (allZero) continue;

						QString row;

						if (sDataTable->rows() > 1)
						{
							row = sDataTable->rowName(j);
							if (row.length() == 0) row = tr("_J0");
							row = connectionHandles[i]->fullName() + tr(".") + row;
						}
						else
							row = connectionHandles[i]->fullName();

						int i = 0;
						while (rowNames.contains(row))
							row = tr("_J") + QString::number(i++); //avoid duplicate rowname

						rowNames += row;
						rates += sDataTable->value(j,0);
					}
				}
			}
		}

		combinedTable.resize(rowNames.size(),columnNames.size());
		columnNames.sort();

		for (int i=0; i < columnNames.size(); ++i)
			combinedTable.setColumnName(i,columnNames[i]);

		for (int i=0; i < rowNames.size(); ++i)
			combinedTable.setRowName(i,rowNames[i]);

		int n = 0, j0;
		for (int i=0; i < connectionHandles.size(); ++i) //build combined matrix for all selected reactions
			if (connectionHandles[i] != 0 && 
					connectionHandles[i]->children.isEmpty() &&
					connectionHandles[i]->hasNumericalData(QObject::tr("Reactant stoichiometries")) &&
					connectionHandles[i]->hasNumericalData(QObject::tr("Product stoichiometries")) &&
					connectionHandles[i]->hasTextData(QObject::tr("Rate equations")))
				{
					nDataTable1 = &(connectionHandles[i]->numericalDataTable(QObject::tr("Reactant stoichiometries")));
					nDataTable2 = &(connectionHandles[i]->numericalDataTable(QObject::tr("Product stoichiometries")));
					sDataTable = &(connectionHandles[i]->textDataTable(QObject::tr("Rate equations")));
					
					if ((nDataTable1->rows() < 1 && nDataTable2->rows() < 1) || sDataTable->rows() < 1)
						continue;
					
					for (int k=0; k < sDataTable->rows(); ++k)
					{
						allZero = true;
						
						if (nDataTable1->rows() > k)
							for (int j=0; j < nDataTable1->columns(); ++j)
								if (nDataTable1->value(k, j) != 0)
								{	
									allZero = false;
									break;
								}

						if (allZero && nDataTable2->rows() > k)
							for (int j=0; j < nDataTable2->columns(); ++j)
								if (nDataTable2->value(k, j) != 0)
								{	
									allZero = false;
									break;
								}

						if (allZero) continue;

						if (nDataTable1->rows() > k)
							for (int j=0; j < nDataTable1->columns(); ++j)     //get unique species
							{
								QString s = nDataTable1->columnName(j);

								j0 = columnNames.indexOf(s);
								if (j0 < 0)
									j0 = columnNames.indexOf(QString(s).replace(".",replaceDot));
								if (j0 < 0)
									j0 = columnNames.indexOf(QString(s).replace(replaceDot,"."));

								if (j0 >= 0)
									combinedTable.value(n,j0) -= nDataTable1->value(k,j);
							}
						if (nDataTable2->rows() > k)
							for (int j=0; j < nDataTable2->columns(); ++j)     //get unique species
							{
								QString s = nDataTable2->columnName(j);

								j0 = columnNames.indexOf(s);
								if (j0 < 0)
									j0 = columnNames.indexOf(QString(s).replace(".",replaceDot));
								if (j0 < 0)
									j0 = columnNames.indexOf(QString(s).replace(replaceDot,"."));

								if (j0 >= 0)
									combinedTable.value(n,j0) += nDataTable2->value(k,j);
							}
						++n;
					}
				}

        /*this tool's matrix is actually the transpose of traditional stoichiometry matrix*/

		QString s;
		for (int i=0; i < columnNames.size(); ++i)
		{
			s = combinedTable.columnName(i);
			s.replace(QString("."),replaceDot);
			combinedTable.setColumnName(i,s);
		}

		for (int i=0; i < rowNames.size(); ++i)
		{
			s = combinedTable.rowName(i);
			s.replace(QString("."),replaceDot);
			combinedTable.setRowName(i,s);
		}

		return combinedTable.transpose();
	}

	//set the stoiciometry of the items from the matrix
	void StoichiometryTool::setStoichiometry(NetworkHandle * win, QList<ItemHandle*>& connectionHandles,const NumericalDataTable& N, const QString& replaceDot)
	{
		NumericalDataTable stoicMatrix = N.transpose();

		if (connectionHandles.size() < 1)
		{
			return;
		}

		NumericalDataTable * nDat1 = 0, * nDat2 = 0;

		QList< NumericalDataTable *> nDataTablesOld, nDataTablesNew;

		if (replaceDot != tr("."))
		{
			QRegExp regex(QString("([A-Za-z0-9])")+replaceDot+QString("([A-Za-z])"));

			QString s;
			for (int i=0; i < stoicMatrix.rows(); ++i)
			{
				s = stoicMatrix.rowName(i);
				s.replace(regex,QString("\\1.\\2"));
				stoicMatrix.setRowName(i,s);
			}

			for (int i=0; i < stoicMatrix.columns(); ++i)
			{
				s = stoicMatrix.columnName(i);
				s.replace(regex,QString("\\1.\\2"));
				stoicMatrix.setColumnName(i,s);
			}
		}

		int n=0;
		for (int i=0; i < connectionHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			if (connectionHandles[i] && connectionHandles[i]->children.isEmpty())
			{
				if (connectionHandles[i]->hasNumericalData(QObject::tr("Reactant stoichiometries")) &&
					connectionHandles[i]->hasNumericalData(QObject::tr("Product stoichiometries")))
				{
					nDat1 = new NumericalDataTable(connectionHandles[i]->numericalDataTable( QObject::tr("Reactant stoichiometries") ));
					nDat2 = new NumericalDataTable(connectionHandles[i]->numericalDataTable( QObject::tr("Product stoichiometries") ));

					bool last = i == connectionHandles.size() - 1;
					QList<bool> pickCol;
					int cols1 = 0, cols2 = 0, rows = 0;
					for (int j=0; j < stoicMatrix.columns(); ++j)
						pickCol << false;
					int n0 = n;
					int k = 0;

					//determine which cols to pick

					for (k=0; (last || k < nDat1->rows()) && k < nDat2->rows() && n0 < stoicMatrix.rows(); ++k, ++n0, ++rows)
					{
						for (int j=0; j < stoicMatrix.columns(); ++j)
						{
							if (stoicMatrix.at(n0,j) != 0)
							{
								if (!pickCol[j]) 
									if (stoicMatrix.at(n0,j) < 0)
										++cols1;
									else
										++cols2;
								pickCol[j] = true;
							}
						}
					}

					if (nDat1->rows() != rows || nDat1->columns() !=  cols1)
						nDat1->resize(rows,cols1);
					
					if (nDat2->rows() != rows || nDat2->columns() !=  cols2)
						nDat2->resize(rows,cols1);

					for (k=0; k < nDat1->rows() && n < stoicMatrix.rows(); ++k, ++n)
					{
						int j1 = 0, j2 = 0;
						for (int j=0; j < stoicMatrix.columns(); ++j)
							if (pickCol[j])
							{
								if (stoicMatrix.at(n,j) < 0)
								{
									nDat1->setColumnName(j1,stoicMatrix.columnName(j));
									nDat1->value(k,j1) = stoicMatrix.at(n,j);
									++j1;
								}
								else
								{
									nDat2->setColumnName(j2,stoicMatrix.columnName(j));
									nDat2->value(k,j2) = stoicMatrix.at(n,j);
									++j2;
								}
							}
					}

					nDataTablesOld << &(connectionHandles[i]->numericalDataTable(QObject::tr("Reactant stoichiometries") ))
								   << &(connectionHandles[i]->numericalDataTable( QObject::tr("Product stoichiometries") ));
					nDataTablesNew << nDat1 << nDat2;
				}
			}
		}

		if (win)
		{
			win->changeData(tr("stoichiometry changed"),connectionHandles,nDataTablesOld,nDataTablesNew);
			ParametersTool::removeUnusedParametersInModel(win);
		}

		for (int i=0; i < nDataTablesNew.size(); ++i)
			if (nDataTablesNew[i])
				delete nDataTablesNew[i];
	}

	//get the rates of the given items
	QStringList StoichiometryTool::getRates(QList<ItemHandle*>& connectionHandles, const QString& replaceDot)
	{
		QStringList rates;
		TextDataTable * sDataTable = 0;
		NumericalDataTable * nDataTable1, * nDataTable2;

		if (connectionHandles.size() < 1)
		{
			return rates;
		}

		for (int i=0; i < connectionHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			bool allZero;
			if (connectionHandles[i])
			{
				if (!connectionHandles[i]->children.isEmpty())
				{
					QList<ItemHandle*> children = connectionHandles[i]->children;
					for (int j=0; j < children.size(); ++j)
						if (children[j] && ConnectionHandle::cast(children[j]) && !connectionHandles.contains(children[j]))
							connectionHandles << children[j];
							
					continue;
				}
				
				if (connectionHandles[i]->hasNumericalData(QObject::tr("Reactant stoichiometries")) &&
					connectionHandles[i]->hasNumericalData(QObject::tr("Product stoichiometries")) &&
					connectionHandles[i]->hasTextData(QObject::tr("Rate equations")))
				{
					nDataTable1 = &(connectionHandles[i]->numericalDataTable(QObject::tr("Reactant stoichiometries")));
					nDataTable2 = &(connectionHandles[i]->numericalDataTable(QObject::tr("Product stoichiometries")));
					sDataTable = &(connectionHandles[i]->textDataTable(QObject::tr("Rate equations")));
					
					if ((nDataTable1->rows() < 1 && nDataTable2->rows() < 1) || sDataTable->rows() < 1)
						continue;

					for (int j=0; j < sDataTable->rows(); ++j) //get rates and reaction names
					{
						allZero = true;
						if (nDataTable1->rows() > j)
							for (int k=0; k < nDataTable1->columns(); ++k)
								if (nDataTable1->value(j,k) != 0)
								{	
									allZero = false;
									break;
								}

						if (allZero && nDataTable2->rows() > j)
							for (int k=0; k < nDataTable2->columns(); ++k)
								if (nDataTable2->value(j,k) != 0)
								{	
									allZero = false;
									break;
								}

						if (allZero) continue;
						rates += sDataTable->value(j,0);
					}
				}
			}
		}

		QRegExp regex(tr("\\.(?!\\d)"));
		for (int i=0; i<rates.size(); ++i)
		{
			rates[i].replace(regex,replaceDot);
		}
		return rates;
	}

	//set the rates of the given items
	void StoichiometryTool::setRates(NetworkHandle * win, QList<ItemHandle*>& connectionHandles,const QStringList& list, const QString& replaceDot)
	{
		if (connectionHandles.size() < 1)
		{
			return;
		}

		TextDataTable * sDataTable = 0;

		QList< TextDataTable *> sDataTablesNew;

		QList<ItemHandle*> handles;

		QRegExp regex(QString("([A-Za-z0-9])") + replaceDot + QString("([A-Za-z])"));

		int n=0;

		bool change = false;
		for (int i=0; i < connectionHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			if (connectionHandles[i] && connectionHandles[i]->children.isEmpty())
			{
				if (connectionHandles[i]->hasTextData(QObject::tr("Rate equations")))
				{
					sDataTable = new TextDataTable(connectionHandles[i]->textDataTable( QObject::tr("Rate equations") ));

					bool last = (i == (connectionHandles.size() - 1));
					int /*cols = 0,*/ rows = 0;

					int n0 = n;

					for (int k=0; (last || (k < sDataTable->rows() /*&& k < nDataTable->rows()*/)) && n0 < list.size(); ++k, ++n0, ++rows)
					{

					}

					if (sDataTable->rows() != rows)
						sDataTable->resize(rows,1);
					change = false;
					for (int k=0; k < sDataTable->rows() /*&& k < nDataTable->rows()*/ && n < list.size(); ++k, ++n)
					{
						sDataTable->value(k,0) = list.at(n);
						sDataTable->value(k,0).replace(regex,QString("\\1.\\2"));
						if (parseRateString(win,connectionHandles[i],sDataTable->value(k,0)))
						{
							change = true;
						}
					}
					if (change)
					{
						sDataTablesNew += sDataTable;
						handles += connectionHandles[i];
					}
				}
			}
		}
		if (win)
		{
			win->changeData(tr("Rate equations changed"),handles,QObject::tr("Rate equations"),sDataTablesNew);
			ParametersTool::removeUnusedParametersInModel(win);
		}

		for (int i=0; i < sDataTablesNew.size(); ++i)
			if (sDataTablesNew[i])
				delete sDataTablesNew[i];
	}


	/************************************************************/

	StoichiometryTool_FToS * StoichiometryTool::fToS;

	tc_matrix StoichiometryTool::_getStoichiometry(tc_items a0)
	{
		return fToS->getStoichiometry(a0);
	}

	tc_matrix StoichiometryTool_FToS::getStoichiometry(tc_items a0)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		NumericalDataTable * p = new NumericalDataTable;
		s->acquire();
		emit getStoichiometry(s,p,*list);
		s->acquire();
		s->release();
		delete s;
		delete list;
		if (p)
		{
			tc_matrix m = ConvertValue(*p);
			delete p;
			return m;
		}
		return emptyMatrix();
	}

	void StoichiometryTool::_setStoichiometry(tc_items a0,tc_matrix a1)
	{
		return fToS->setStoichiometry(a0,a1);
	}

	void StoichiometryTool_FToS::setStoichiometry(tc_items a0,tc_matrix a1)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		NumericalDataTable * dat = ConvertValue(a1);
		s->acquire();
		emit setStoichiometry(s,*list,*dat);
		s->acquire();
		s->release();
		delete s;
		delete dat;
		delete list;
	}

	tc_strings StoichiometryTool::_getRates(tc_items a0)
	{
		return fToS->getRates(a0);
	}

	tc_strings StoichiometryTool_FToS::getRates(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		QList<ItemHandle*> * list = ConvertValue(a0);
		emit getRates(s,&p,*list);
		s->acquire();
		s->release();
		delete s;
		delete list;
		return (tc_strings)ConvertValue(p);
	}

	void StoichiometryTool::_setRates(tc_items a0,tc_strings a1)
	{
		return fToS->setRates(a0,a1);
	}

	void StoichiometryTool_FToS::setRates(tc_items a0,tc_strings a1)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		s->acquire();
		emit setRates(s,*list,ConvertValue(a1));
		s->acquire();
		s->release();
		delete s;
		delete list;
	}

	
	void StoichiometryTool::itemsAboutToBeRemoved(GraphicsScene * , QList<QGraphicsItem*>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands)
	{
		NodeGraphicsItem * nodeItem;
		QList<ConnectionGraphicsItem*> tempList1;
		QList<NodeGraphicsItem*> tempList2;
		QHash<ConnectionGraphicsItem*,int> toBeRemoved;
		
		ItemHandle * connectionHandle, * nodeHandle;
		ConnectionGraphicsItem * connectionItem;
		ConnectionGraphicsItem::ControlPoint * controlPoint;
		ConnectionFamily * family;
		ItemFamily * nodeFamily;

		TextDataTable * oldTextTable, * newTextTable;
		QList<TextDataTable*> oldTextTables, newTextTables;
		
		NumericalDataTable * oldNumTable, * newNumTable;
		QList<NumericalDataTable*> oldNumTables, newNumTables;

		QList<ItemHandle*> changedHandles;
		int k;
		
		for (int i=0; i < items.size(); ++i)
		{
			if (nodeItem = NodeGraphicsItem::cast(items[i]))
			{
				tempList1 = nodeItem->connections();
				for (int j=0; j < tempList1.size(); ++j)
					if (tempList1[j] && (k = tempList1[j]->indexOf(nodeItem)) > -1)					
						toBeRemoved.insertMulti(tempList1[j],k);
			}
			
			if ((connectionItem = ConnectionGraphicsItem::cast(items[i])) &&
				(connectionHandle = connectionItem->handle()) &&
				(ConnectionGraphicsItem::cast(connectionHandle->graphicsItems)).size() > 1)
			{
				tempList2 = connectionItem->nodes();
				for (int j=0; j < tempList2.size(); ++j)
					if (tempList2[j] && (k = connectionItem->indexOf(tempList2[j])) > -1)
						toBeRemoved.insertMulti(connectionItem,k);
			}
			
			if ((controlPoint = qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(items[i])) &&
				controlPoint->connectionItem)
			{
				k = controlPoint->connectionItem->indexOf(controlPoint);
				if (controlPoint->connectionItem->curveSegments[k].size() < 5)
					toBeRemoved.insertMulti(controlPoint->connectionItem,k);
			}
		}

		QList<ConnectionGraphicsItem*> keys = toBeRemoved.keys();
		QList<int> indices;
		
		for (int i=0; i < keys.size(); ++i)
			if (connectionHandle = keys[i]->handle())
			{
				changedHandles << connectionHandle;
				
				oldTextTable = &(connectionHandle->textDataTable(tr("Rate equations")));
				newTextTable = new TextDataTable(*oldTextTable);
				
				QStringList removeRowNames;
				indices = toBeRemoved.values(keys[i]);

				for (int j=0; j < indices.size(); ++j)
				{
					nodeHandle = getHandle(keys[i]->nodeAt(indices[j]));
					if (nodeHandle)
						removeRowNames << nodeHandle->fullName();
				}
				
				if (!removeRowNames.isEmpty())
				{
					for (int j1=0; j1 < removeRowNames.size(); ++j1)
						for (int j2=0; j2 < newTextTable->rows(); ++j2)
							newTextTable->value(j2,0).replace(removeRowNames[j1],tr("1.0"));
					oldTextTables << oldTextTable;
					newTextTables << newTextTable;
				}
				
				oldNumTable = &(connectionHandle->numericalDataTable(tr("Reactant stoichiometries")));
				newNumTable = new NumericalDataTable(*oldNumTable);
				
				if (!removeRowNames.isEmpty())
				{
					for (int j=0; j < removeRowNames.size(); ++j)
						newNumTable->removeColumn(removeRowNames[j]);
					oldNumTables << oldNumTable;
					newNumTables << newNumTable;
				}
				
				oldNumTable = &(connectionHandle->numericalDataTable(tr("Product stoichiometries")));
				newNumTable = new NumericalDataTable(*oldNumTable);
				
				if (!removeRowNames.isEmpty())
				{
					for (int j=0; j < removeRowNames.size(); ++j)
						newNumTable->removeColumn(removeRowNames[j]);
					oldNumTables << oldNumTable;
					newNumTables << newNumTable;
				}
				
			}

		if (newTextTables.size() > 0)
		{
			commands << new ChangeTextDataCommand(tr("Add node roles"),oldTextTables,newTextTables);
			for (int i=0; i < newTextTables.size(); ++i)
				delete newTextTables[i];
		}
		
		if (newNumTables.size() > 0)
		{
			commands << new ChangeNumericalDataCommand(tr("Add node roles"),oldNumTables,newNumTables);
			for (int i=0; i < newNumTables.size(); ++i)
				delete newNumTables[i];
		}
		
		emit dataChanged(changedHandles);
	}
	
	QList<ItemHandle*> StoichiometryTool::userModifiedRates;
}

