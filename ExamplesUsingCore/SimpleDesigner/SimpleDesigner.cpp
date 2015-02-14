/****************************************************************************

 Copyright (C) 2009 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYWRITE.TXT

This is an example application that uses the TinkerCell Core library
****************************************************************************/

#include <vector>
#include <iostream>
#include "sbml/SBMLReader.h"
#include "sbml/SBMLWriter.h"
#include "sbml/SBMLDocument.h"
#include "sbml/Species.h"
#include "sbml/SpeciesReference.h"
#include "sbml/ListOf.h"
#include "sbml/Model.h"
#include "sbml/Rule.h"
#include "sbml_sim.h"
#include "BasicGraphicsToolbar.h"
#include "ConsoleWindow.h"
#include "interpreters/OctaveInterpreterThread.h"
#include "interpreters/PythonInterpreterThread.h"
#include "fileIO/LoadSaveTool.h"
#include "SimpleDesigner.h"
#include "GlobalSettings.h"
#include "GraphicsView.h"
#include "Ontology.h"
<<<<<<< HEAD
//#include "sbol.h"


=======
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

using namespace Tinkercell;
using namespace std;

SimpleDesigner::SimpleDesigner(): Tool(tr("Simple Designer"))
{
	actionGroup = new QActionGroup(this);
	mode = 0;
	plotTool = 0;
<<<<<<< HEAD

	toolBar = new QToolBar(this);
	toolBar->setObjectName("Simple Designer Toolbar");
	actionGroup->setExclusive(true);

	arrowButton = new QAction(QIcon(":/images/arrow.png"),tr("arrow"),toolBar);
	QAction * nodeButton = new QAction(QIcon(":/images/blueRect.png"),tr("node"),toolBar);
	QAction * edgeButton = new QAction(QIcon(":/images/1to1.png"),tr("reaction"),toolBar);

=======
	
	toolBar = new QToolBar(this);
	toolBar->setObjectName("Simple Designer Toolbar");
	actionGroup->setExclusive(true);
	
	arrowButton = new QAction(QIcon(":/images/arrow.png"),tr("arrow"),toolBar);
	QAction * nodeButton = new QAction(QIcon(":/images/blueRect.png"),tr("node"),toolBar);
	QAction * edgeButton = new QAction(QIcon(":/images/1to1.png"),tr("reaction"),toolBar);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	arrowButton->setCheckable(true);
	nodeButton->setCheckable(true);
	edgeButton->setCheckable(true);
	toolBar->addAction(arrowButton);
	toolBar->addAction(nodeButton);
	toolBar->addAction(edgeButton);

	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	actionGroup->addAction(arrowButton);
	actionGroup->addAction(nodeButton);
	actionGroup->addAction(edgeButton);
<<<<<<< HEAD

	QGridLayout * layout1 = new QGridLayout;
	QGridLayout * layout2 = new QGridLayout;
	QVBoxLayout * layout3 = new QVBoxLayout;

=======
	
	QGridLayout * layout1 = new QGridLayout;
	QGridLayout * layout2 = new QGridLayout;
	QVBoxLayout * layout3 = new QVBoxLayout;
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	layout1->addWidget(new QLabel(tr("name")),0,0);
	layout1->addWidget(new QLabel(tr("concentration")),1,0);
	layout1->addWidget(name1 = new QLineEdit,0,1);
	layout1->addWidget(conc  = new QLineEdit,1,1);
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	layout2->addWidget(new QLabel(tr("name")),0,0);
	layout2->addWidget(new QLabel(tr("rate")),1,0);
	layout2->addWidget(name2 = new QLineEdit,0,1);
	layout2->addWidget(rate  = new QLineEdit,1,1);
<<<<<<< HEAD

	listWidget = new QListWidget;
	connect(listWidget,SIGNAL(itemActivated ( QListWidgetItem * )), this, SLOT(parameterItemActivated ( QListWidgetItem * )));
	layout3->addWidget(listWidget);

	groupBox1 = new QGroupBox(tr(" Species "));
	groupBox1->setLayout(layout1);

	groupBox2 = new QGroupBox(tr(" Reaction "));
	groupBox2->setLayout(layout2);

	groupBox3 = new QGroupBox(tr(" Parameters "));
	groupBox3->setLayout(layout3);

=======
	
	listWidget = new QListWidget;
	connect(listWidget,SIGNAL(itemActivated ( QListWidgetItem * )), this, SLOT(parameterItemActivated ( QListWidgetItem * )));
	layout3->addWidget(listWidget);
	
	groupBox1 = new QGroupBox(tr(" Species "));
	groupBox1->setLayout(layout1);
	
	groupBox2 = new QGroupBox(tr(" Reaction "));
	groupBox2->setLayout(layout2);
	
	groupBox3 = new QGroupBox(tr(" Parameters "));
	groupBox3->setLayout(layout3);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	QVBoxLayout * layout4 = new QVBoxLayout;
	layout4->addWidget(groupBox1);
	layout4->addWidget(groupBox2);
	layout4->addWidget(groupBox3);
<<<<<<< HEAD

	setLayout(layout4);

	groupBox1->hide();
	groupBox2->hide();

=======
	
	setLayout(layout4);
	
	groupBox1->hide();
	groupBox2->hide();
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	connect(name1,SIGNAL(editingFinished()),this,SLOT(nameChanged()));
	connect(name2,SIGNAL(editingFinished()),this,SLOT(nameChanged()));
	connect(conc,SIGNAL(editingFinished()),this,SLOT(concentrationChanged()));
	connect(rate,SIGNAL(editingFinished()),this,SLOT(rateChanged()));
<<<<<<< HEAD

    //Document* doc = createDocument();
    // components
    //DNAComponent *dc1 = createDNAComponent(doc, "http://example.com/dc1");
    //DNAComponent *dc2 = createDNAComponent(doc, "http://example.com/dc2");
}

void SimpleDesigner::nameChanged()
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

=======
}

void SimpleDesigner::nameChanged() 
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;
	
	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (name1->isVisible())
		handle->rename(name1->text());
	else
	if (name2->isVisible())
		handle->rename(name2->text());
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	for (int i=0; i < handle->graphicsItems.size(); ++i)
		handle->graphicsItems[i]->update();
}

void SimpleDesigner::addParameters(QStringList& newVars)
{
	NetworkHandle * network = currentNetwork();
	if (!network || !network->globalHandle()) return;
<<<<<<< HEAD

	listWidget->clear();

	ItemHandle * globalHandle = network->globalHandle(); //handle for the entire model
	QStringList vars;
	DataTable<qreal> params;

	params = globalHandle->numericalDataTable("parameters"); //get existing set of parameters
	vars = params.rowNames();

=======
	
	listWidget->clear();
	
	ItemHandle * globalHandle = network->globalHandle(); //handle for the entire model	
	QStringList vars;
	DataTable<qreal> params;
	
	params = globalHandle->numericalDataTable("parameters"); //get existing set of parameters
	vars = params.rowNames();
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	for (int i=0; i < newVars.size(); ++i)
		if (!vars.contains(newVars[i]))
		{
			vars << newVars[i];
			params.value(newVars[i],0) = 1.0;   //add new parameters to existing set
		}
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (vars.isEmpty()) return;

	globalHandle->changeData("parameters", &params); //update with new set of parameters
	vars.clear();
<<<<<<< HEAD

	for (int i=0; i < params.rows(); ++i)
		vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"

=======
	
	for (int i=0; i < params.rows(); ++i)
		vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	listWidget->addItems(vars);   //update list widget for viewing parameters
}

void SimpleDesigner::parameterItemActivated ( QListWidgetItem * item )
{
	NetworkHandle * network = currentNetwork();
	if (!network) return;  //no window open
<<<<<<< HEAD

	QString s = item->text();  //get string "name = value"
	QStringList parts = s.split("=");
	QString param = parts[0].trimmed(); //parse to get parameter name

	ItemHandle * globalHandle = network->globalHandle(); //handle for the model
	NumericalDataTable & oldTable = globalHandle->numericalDataTable("parameters");

	double d = QInputDialog::getDouble(this,"Change parameter", param, 	oldTable.value(param,0)); //get value from user

	//update parameter value using to the history window
	NumericalDataTable newTable(oldTable); //new parameter table
	newTable.value(param,0) = d;
	QString message = tr("parameter ") + param + tr(" changed"); //message for the history stack
	network->changeData(message, globalHandle, "parameters", &newTable); //adds undo command

	item->setText(param + tr(" = ") + QString::number(d));
}

void SimpleDesigner::rateChanged()
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;

	NetworkHandle * net = scene->network;
	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

	if (!handle || !handle->hasTextData("rate")) return;

	QString formula = rate->text();

=======
	
	QString s = item->text();  //get string "name = value"
	QStringList parts = s.split("=");
	QString param = parts[0].trimmed(); //parse to get parameter name
	
	ItemHandle * globalHandle = network->globalHandle(); //handle for the model
	NumericalDataTable & oldTable = globalHandle->numericalDataTable("parameters");
	
	double d = QInputDialog::getDouble(this,"Change parameter", param, 	oldTable.value(param,0)); //get value from user
	
	//update parameter value using to the history window
	NumericalDataTable newTable(oldTable); //new parameter table
	newTable.value(param,0) = d;	
	QString message = tr("parameter ") + param + tr(" changed"); //message for the history stack
	network->changeData(message, globalHandle, "parameters", &newTable); //adds undo command
	
	item->setText(param + tr(" = ") + QString::number(d));
}

void SimpleDesigner::rateChanged() 
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;
	
	NetworkHandle * net = scene->network;	
	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);
	
	if (!handle || !handle->hasTextData("rate")) return;	
	
	QString formula = rate->text();
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//find all the new variables in this equation
	QStringList newVars, oldVars;
	QList<ItemHandle*> handles;
	bool ok = net->parseMath(formula,newVars,oldVars,handles);
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (ok && handle->textData("rate") != formula)
	{
		DataTable<QString> table;
		table.value(0,0) = formula;
		handle->changeData("rate",&table);
		addParameters(newVars); //insert new variables into the globalHandle
		setToolTip(handle);
	}
	else
		if (!ok)
			mainWindow->statusBar()->showMessage(tr("error in formula : ") + formula);
}

void SimpleDesigner::concentrationChanged()
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;
<<<<<<< HEAD

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

	if (!handle || !handle->hasNumericalData("concentration")) return;

	DataTable<qreal> table;

	bool ok;

	table.value(0,0) = conc->text().toDouble(&ok);

=======
	
	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);
	
	if (!handle || !handle->hasNumericalData("concentration")) return;
	
	DataTable<qreal> table;
	
	bool ok;
	
	table.value(0,0) = conc->text().toDouble(&ok);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (ok && handle->numericalData("concentration") != table.value(0,0))
		handle->changeData("concentration",&table);
}

void SimpleDesigner::actionTriggered(QAction* action)
{
	mode = 0;
<<<<<<< HEAD

	if (!action) return;

=======
	
	if (!action) return;
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (action->text() == tr("node"))
		mode = 1;
	else
	if (action->text() == tr("reaction"))
		mode = 2;
}
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
bool SimpleDesigner::setMainWindow(MainWindow * main)
{
	Tool::setMainWindow(main);
	if (mainWindow)
	{
		QWidget * tool = mainWindow->tool("Default Plot Tool");
		plotTool = static_cast<PlotTool*>(tool);
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		if (plotTool && plotTool->parentWidget())
		{
			QDockWidget * dockWidget = static_cast<QDockWidget*>(plotTool->parentWidget());
			dockWidget->setFloating(false);
		}

		mainWindow->addToolBar(Qt::LeftToolBarArea,toolBar);

		setWindowTitle(tr("Information Box"));
		setWindowIcon(QIcon(tr(":/images/about.png")));
		QDockWidget * dockWidget = mainWindow->addToolWindow(this, MainWindow::DockWidget, Qt::BottomDockWidgetArea, Qt::NoDockWidgetArea);
		if (dockWidget)
			dockWidget->setFloating(true);

		mainWindow->toolBarForTools->addAction(QIcon(tr(":/images/play.png")), tr("Simulate"), this, SLOT(ode()));
		mainWindow->toolBarForTools->addAction(QIcon(tr(":/images/graph.png")), tr("Stochastic"), this, SLOT(ssa()));
<<<<<<< HEAD

		connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle*, const QList<ItemHandle*>&)),
				this, SLOT(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)));

=======
		
		connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle*, const QList<ItemHandle*>&)),
				this, SLOT(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)));
				
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		connect(mainWindow,
				SIGNAL(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,
				SLOT(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers))
				);
<<<<<<< HEAD

=======
				
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		connect(mainWindow,
				SIGNAL(escapeSignal(const QWidget * )),
				this,
				SLOT(escapeSignal(const QWidget * ))
				);
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		connect(mainWindow,
				SIGNAL(mousePressed(GraphicsScene * , QPointF , Qt::MouseButton, Qt::KeyboardModifiers)),
				this,
				SLOT(mousePressed(GraphicsScene * , QPointF , Qt::MouseButton, Qt::KeyboardModifiers))
				);
		return true;
	}
	return false;
}

void SimpleDesigner::mousePressed(GraphicsScene * scene, QPointF point, Qt::MouseButton, Qt::KeyboardModifiers modifiers)
{
	if (!scene || !scene->network || mode != 1) return;
<<<<<<< HEAD

	QGraphicsItem * item = new SimpleNode;
	NodeHandle * handle = new NodeHandle;

	handle->name = scene->network->makeUnique(tr("s1"));
	setHandle(item,handle);
	item->setPos(point);

	scene->insert(handle->name + tr(" inserted"),item);

=======
	
	QGraphicsItem * item = new SimpleNode;
	NodeHandle * handle = new NodeHandle;
	
	handle->name = scene->network->makeUnique(tr("s1"));
	setHandle(item,handle);
	item->setPos(point);
	
	scene->insert(handle->name + tr(" inserted"),item);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
}

void SimpleDesigner::setToolTip(ItemHandle* item)
{
	if (!item) return;
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (NodeHandle::cast(item) && item->hasNumericalData("concentration")) //is a node
	{
		for (int i=0; i < item->graphicsItems.size(); ++i)
			item->graphicsItems[i]->setToolTip
			(
				item->name + tr(" : ") + QString::number(item->numericalData("concentration"))
			);
	}
	else
	if (ConnectionHandle::cast(item) && item->hasTextData("rate")) //is a connection
	{
		for (int i=0; i < item->graphicsItems.size(); ++i)
			item->graphicsItems[i]->setToolTip
			(
				item->name + tr(" : ") + item->textData("rate")
			);
	}
}

void SimpleDesigner::itemsInserted(NetworkHandle * net,const QList<ItemHandle*>& items)
{
	for (int i=0; i < items.size(); ++i)
	{
		if (NodeHandle::cast(items[i])) //is node?
		{
			items[i]->numericalData("concentration") = 10.0;
		}
		if (ConnectionHandle::cast(items[i])) //is reaction?
		{
			ConnectionHandle * connection = ConnectionHandle::cast(items[i]);
			QString rate;
<<<<<<< HEAD

=======
			
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
			if (connection->hasTextData("rate"))  //rate already exists
			{
				QStringList newVars,oldVars;
				QList<ItemHandle*> handles;
				rate = connection->textData("rate");
				bool ok = net->parseMath(rate,newVars,oldVars,handles);
<<<<<<< HEAD

=======
					
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
				if (ok)
				{
					addParameters(newVars);
				}
			}
			else
<<<<<<< HEAD
			{
				QList<NodeHandle*> nodes = connection->nodes("reactant");

				rate = tr("1.0");

=======
			{	
				QList<NodeHandle*> nodes = connection->nodes("reactant");
			
				rate = tr("1.0");
				
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
				for (int j=0; j < nodes.size(); ++j)
					rate += tr(" * ") + nodes[j]->name;   //default mass-action rate
			}

			connection->textData("rate") = rate;
		}

		setToolTip(items[i]);
	}
}

void SimpleDesigner::deselectItem(GraphicsScene * scene, QGraphicsItem * item)
{
	selectItem(scene, item, false);
}

void SimpleDesigner::selectItem(GraphicsScene * scene, QGraphicsItem * item, bool select)
{
	if (NodeGraphicsItem::cast(item))
	{
		NodeGraphicsItem * node = NodeGraphicsItem::cast(item);
		node->setBoundingBoxVisible(select);
		if (select)
			for (int j=0; j < node->boundaryControlPoints.size(); ++j)
				scene->moving() += node->boundaryControlPoints[j];
	}
	else
	if (ConnectionGraphicsItem::cast(item))
	{
		ConnectionGraphicsItem * connection = ConnectionGraphicsItem::cast(item);
		if (select)
		{
			connection->setPen(QPen(QColor(255,0,0,255)));
			scene->moving().removeAll(item);
			scene->moving() += connection->controlPointsAsGraphicsItems();
		}
		else
		{
			connection->setPen(connection->defaultPen);
			connection->update();
		}
		connection->setControlPointsVisible(select);     //show the red box
	}
	else
	if (qgraphicsitem_cast<NodeGraphicsItem::ControlPoint*>(item))
		qgraphicsitem_cast<NodeGraphicsItem::ControlPoint*>(item)->nodeItem->setBoundingBoxVisible(select);
	else
	if (qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(item))
		qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(item)->connectionItem->setControlPointsVisible(select);
}

/*
When items are selected, the program might be in regular mode or it might
<<<<<<< HEAD
be in connect-mode, i.e. connecting two items together
=======
be in connect-mode, i.e. connecting two items together 
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
*/
void SimpleDesigner::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
	if (!scene) return;
<<<<<<< HEAD

	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;

=======
	
	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//go through all the previously selected items and un-color them
	for (int i=0; i < selectedItems.size(); ++i)
	{
		deselectItem(scene,selectedItems[i]);
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		if (mode == 2) //if connect-mode, then store the previously selected nodes
		{
			node = NodeGraphicsItem::cast(selectedItems[i]);
			if (node && !nodeItems.contains(node))
				nodeItems << node;
		}
	}
<<<<<<< HEAD

	selectedItems.clear();

=======
	
	selectedItems.clear();
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	if (items.size() != 1 || !items[0]) //scene selected
	{
		groupBox1->hide();
		groupBox2->hide();
		listWidget->clear();
<<<<<<< HEAD

		NetworkHandle * network = scene->network;
		NumericalDataTable & params = network->globalHandle()->numericalDataTable("parameters");
		QStringList vars;

		for (int i=0; i < params.rows(); ++i)
			vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"

=======
		
		NetworkHandle * network = scene->network;
		NumericalDataTable & params = network->globalHandle()->numericalDataTable("parameters");
		QStringList vars;
	
		for (int i=0; i < params.rows(); ++i)
			vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		listWidget->addItems(vars);   //update list widget for viewing parameters
	}
	else //show concentration or rates
	{
		ItemHandle * handle = getHandle(items[0]);
		if (NodeHandle::cast(handle) && handle->hasNumericalData("concentration"))
		{
			name1->setText(  handle->name  );
			conc->setText(  QString::number( handle->numericalData("concentration")  ));
			groupBox1->show();
			groupBox2->hide();
		}
		else
		if (ConnectionHandle::cast(handle) && handle->hasTextData("rate"))
		{
			name2->setText(  handle->name  );
			rate->setText(  handle->textData("rate") );
			groupBox1->hide();
			groupBox2->show();
		}
		else
		{
			groupBox1->hide();
			groupBox2->hide();
		}
	}
<<<<<<< HEAD

	selectedItems = items;

=======
	
	selectedItems = items;
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	for (int i=0; i < selectedItems.size(); ++i)
	{
		selectItem(scene,selectedItems[i]);
		if (mode == 2)
		{
			node = NodeGraphicsItem::cast(selectedItems[i]);
			if (node && !nodeItems.contains(node))
				nodeItems << node;
		}
	}
<<<<<<< HEAD

	if (mode == 2 && nodeItems.size() == 2) //insert connection mode
	{
=======
	
	if (mode == 2 && nodeItems.size() == 2) //insert connection mode
	{	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		NodeGraphicsItem * node1 = NodeGraphicsItem::cast(nodeItems[0]),  //nodes to connect
									 * node2 = NodeGraphicsItem::cast(nodeItems[1]);

		ConnectionGraphicsItem * item = new ConnectionGraphicsItem(node1, node2); //create new connection

		ConnectionHandle * handle = new ConnectionHandle;  //create handle for the connection
		setHandle(item,handle);

<<<<<<< HEAD
		NodeHandle * nodeHandle1 = NodeHandle::cast(node1->handle()),
						   * nodeHandle2 = NodeHandle::cast(node2->handle());
=======
		NodeHandle * nodeHandle1 = NodeHandle::cast(node1->handle()), 
						   * nodeHandle2 = NodeHandle::cast(node2->handle()); 		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		handle->setNodeRole(nodeHandle1, "reactant");  //assign roles for each node (will be useful later)
		handle->setNodeRole(nodeHandle2, "product");
		handle->name = scene->network->makeUnique(tr("J1"));  //find unique name, with J1 being the default

		scene->insert(tr("connection inserted"),item); //insert the new connection
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
		scene->deselect(); //deselect everything
	}
}

void SimpleDesigner::escapeSignal(const QWidget * sender)
{
	arrowButton->trigger();
	if (currentScene())
		currentScene()->deselect();
}

void SimpleDesigner::ode()
{
	simulate(false);
}

void SimpleDesigner::ssa()
{
	simulate(true);
}

void SimpleDesigner::simulate(bool stochastic)
{
	NetworkHandle * network = currentNetwork();
	if (!network) return;
<<<<<<< HEAD

	SBMLDocument_t * doc = SBMLDocument_create();
	Model_t * model = SBMLDocument_createModel(doc);

	QList<ItemHandle*> handles = network->handles();
	NumericalDataTable params = network->globalHandle()->numericalDataTable("parameters");

=======
	
	SBMLDocument_t * doc = SBMLDocument_create();
	Model_t * model = SBMLDocument_createModel(doc);
	
	QList<ItemHandle*> handles = network->handles();
	NumericalDataTable params = network->globalHandle()->numericalDataTable("parameters");	
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//create compartment
	Compartment_t * comp = Model_createCompartment (model);
	Compartment_setId(comp, "DefaultCompartment");
	Compartment_setName(comp, "DefaultCompartment");
	Compartment_setVolume(comp, 1.0);
	Compartment_setUnits(comp, "uL");

	//create list of species
	for (int i=0; i < handles.size(); ++i)
		if (NodeHandle::cast(handles[i]))  //if node
		{
			double d = handles[i]->numericalData("concentration");
			QString name = handles[i]->name;

			Species_t * s = Model_createSpecies(model);
			Species_setId(s,ConvertValue(name));
			Species_setName(s,ConvertValue(name));
			Species_setConstant(s,0);
			Species_setInitialConcentration(s, d);
			Species_setInitialAmount(s, d);
			Species_setCompartment(s, "DefaultCompartment");
		}
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//create list of reactions
	for (int i=0; i < handles.size(); ++i)
		if (ConnectionHandle::cast(handles[i]))  //if reaction
		{

			ConnectionHandle * reactionHandle = ConnectionHandle::cast(handles[i]);
<<<<<<< HEAD

			QString name = reactionHandle->name;
			QString rate = reactionHandle->textData("rate");

			QList<NodeHandle*> reactants = reactionHandle->nodes("reactant");
			QList<NodeHandle*> products = reactionHandle->nodes("product");

=======
			
			QString name = reactionHandle->name;
			QString rate = reactionHandle->textData("rate");
			
			QList<NodeHandle*> reactants = reactionHandle->nodes("reactant");
			QList<NodeHandle*> products = reactionHandle->nodes("product");
			
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
			Reaction_t * reac = Model_createReaction(model);
			Reaction_setId(reac, ConvertValue(name));
			Reaction_setName(reac, ConvertValue(name));
			Reaction_setId(reac, ConvertValue(name));
			KineticLaw_t  * kinetic = Reaction_createKineticLaw(reac);
			KineticLaw_setFormula( kinetic, ConvertValue( rate ));

			for (int j=0; j < reactants.size(); ++j)
<<<<<<< HEAD
			{
=======
			{ 
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
				SpeciesReference_t * sref = Reaction_createReactant(reac);
				SpeciesReference_setId(sref, ConvertValue(reactants[j]->name));
				SpeciesReference_setName(sref, ConvertValue(reactants[j]->name));
				SpeciesReference_setSpecies(sref, ConvertValue(reactants[j]->name));
				//SpeciesReference_setStoichiometry( sref, -stoictc_matrix.value(j,i) );
			}
			for (int j=0; j < products.size(); ++j)
<<<<<<< HEAD
			{
=======
			{ 
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
				SpeciesReference_t * sref = Reaction_createProduct(reac);
				SpeciesReference_setId(sref, ConvertValue(products[j]->name));
				SpeciesReference_setName(sref, ConvertValue(products[j]->name));
				SpeciesReference_setSpecies(sref, ConvertValue(products[j]->name));
			}
		}
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//create list of parameters
	for (int i=0; i < params.rows(); ++i)
	{
		Parameter_t * p = Model_createParameter(model);
		Parameter_setId(p, ConvertValue(params.rowName(i)));
		Parameter_setName(p, ConvertValue(params.rowName(i)));
		Parameter_setValue(p, params.value(i,0));
<<<<<<< HEAD
	}

	SBML_sim sim("C:\\Users\\Deepak\\Desktop\\temp.txt");

	vector<string> names = sim.getVariableNames();
	vector< vector<double> > output;
=======
	}	
	
	SBML_sim sim("C:\\Users\\Deepak\\Desktop\\temp.txt");

	vector<string> names = sim.getVariableNames();	
	vector< vector<double> > output;	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	NumericalDataTable results;

	try
	{
		if (stochastic)
			output = sim.ssa(100.0);
		else
			output = sim.simulate(100.0,0.1);
	}
	catch(...)
	{
<<<<<<< HEAD

=======
		
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	}

	if (output.size() > 0)
	{
		int sz = output[0].size();

		results.resize(sz,output.size());
		for (int i=0; i < names.size(); ++i)
			results.setColumnName(i+1, QString(names[i].c_str()));
		results.setColumnName(0, tr("time"));

		for (int i=0; i < output.size(); ++i)
			for (int j=0; j < sz; ++j)
				results.value(j,i) = output[i][j];
	}
<<<<<<< HEAD

	if (plotTool)
		plotTool->plot(results,"Simulation");

=======
	
	if (plotTool)
		plotTool->plot(results,"Simulation");
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	delete doc;
}

/************ MAIN ******************/

#if defined(Q_WS_WIN) && !defined(MINGW)
int WinMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	//setup project name
    GlobalSettings::PROJECTWEBSITE = "www.tinkercell.com";
    GlobalSettings::ORGANIZATIONNAME = "Simple Designer";
    GlobalSettings::PROJECTNAME = "Simple Designer";
<<<<<<< HEAD

	//MUST DO
	QApplication app(argc, argv);
	QString appDir = QCoreApplication::applicationDirPath();

=======
	
	//MUST DO
	QApplication app(argc, argv);
	QString appDir = QCoreApplication::applicationDirPath();    
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	//enable features -- must do before creating MainWindow
	GlobalSettings::ENABLE_HISTORY_WINDOW = false;
	GlobalSettings::ENABLE_CONSOLE_WINDOW = true;
	GlobalSettings::ENABLE_GRAPHING_TOOLS = true;
	GlobalSettings::ENABLE_CODING_TOOLS = false;
	GlobalSettings::ENABLE_PYTHON = false;
	GlobalSettings::ENABLE_OCTAVE = true;
	GlobalSettings::ENABLE_LOADSAVE_TOOL = true;
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	MainWindow mainWindow(true,false,false);  //@args: enable scene, text, allow pop-out windows
	mainWindow.readSettings();   //load settings such as window positions

	//optional
<<<<<<< HEAD
	mainWindow.setWindowTitle("Simple Designer");
    mainWindow.statusBar()->showMessage("Welcome to Simple Designer");

	//This is our main tool
	mainWindow.addTool(new SimpleDesigner);

=======
	mainWindow.setWindowTitle("Simple Designer"); 
    mainWindow.statusBar()->showMessage("Welcome to Simple Designer");  
	
	//This is our main tool
	mainWindow.addTool(new SimpleDesigner);
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
	/*  optional  GUI configurations */
	GraphicsScene::SelectionRectangleBrush = QBrush(QColor(5,5,5,40));
	ConnectionGraphicsItem::DefaultMiddleItemFile = "";
	ConnectionGraphicsItem::DefaultArrowHeadFile = ":/images/arrow.xml";

	//Ontology::readNodes("NodesTree.nt");
	//Ontology::readConnections("ConnectionsTree.nt");
<<<<<<< HEAD

=======
	
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
    GraphicsScene * scene = mainWindow.newScene();
	GraphicsView::DEFAULT_ZOOM = 0.5;

    mainWindow.show();

    int output = app.exec();

    return output;
}

