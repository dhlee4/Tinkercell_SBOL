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

using namespace Tinkercell;
using namespace std;

const int NODE_TYPE_DC = 3;
const int NODE_TYPE_SA = 4;
const int NODE_TYPE_DS = 5;

SimpleDesigner::SimpleDesigner(): Tool(tr("Simple Designer"))
{
	actionGroup = new QActionGroup(this);
	mode = 0;
	plotTool = 0;

	toolBar = new QToolBar(this);
	toolBar->setObjectName("Simple Designer Toolbar");
	actionGroup->setExclusive(true);

	arrowButton = new QAction(QIcon(":/images/arrow.png"),tr("arrow"),toolBar);
	QAction * nodeButton = new QAction(QIcon(":/images/blueRect.png"),tr("node"),toolBar);
	QAction * edgeButton = new QAction(QIcon(":/images/1to1.png"),tr("reaction"),toolBar);
	QAction * DNAComponent = new QAction(QIcon(":/images/blueRect.png"),tr("DNA Component"),toolBar);
	QAction * SequenceAnnotation = new QAction(QIcon(":/images/blueRect.png"),tr("Sequence Annotation"),toolBar);
	QAction * DNASequence = new QAction(QIcon(":/images/blueRect.png"),tr("DNA Sequence"),toolBar);

	arrowButton->setCheckable(true);
	nodeButton->setCheckable(true);
	edgeButton->setCheckable(true);
	DNAComponent->setCheckable(true);
	SequenceAnnotation->setCheckable(true);
	DNASequence->setCheckable(true);

	toolBar->addAction(arrowButton);
	toolBar->addAction(nodeButton);
	toolBar->addAction(edgeButton);
	toolBar->addAction(DNAComponent);
	toolBar->addAction(SequenceAnnotation);
	toolBar->addAction(DNASequence);

	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	actionGroup->addAction(arrowButton);
	actionGroup->addAction(nodeButton);
	actionGroup->addAction(edgeButton);
	actionGroup->addAction(DNAComponent);
	actionGroup->addAction(SequenceAnnotation);
	actionGroup->addAction(DNASequence);

	QGridLayout * layout1 = new QGridLayout;
	QGridLayout * layout2 = new QGridLayout;
	QGridLayout * dc_layout = new QGridLayout;
	QGridLayout * sa_layout = new QGridLayout;
	QGridLayout * ds_layout = new QGridLayout;
	QVBoxLayout * layout3 = new QVBoxLayout;

	layout1->addWidget(new QLabel(tr("name")),0,0);
	layout1->addWidget(new QLabel(tr("concentration")),1,0);
	layout1->addWidget(name1 = new QLineEdit,0,1);
	layout1->addWidget(conc  = new QLineEdit,1,1);

	layout2->addWidget(new QLabel(tr("name")),0,0);
	layout2->addWidget(new QLabel(tr("rate")),1,0);
	layout2->addWidget(name2 = new QLineEdit,0,1);
	layout2->addWidget(rate  = new QLineEdit,1,1);

//added
	dc_layout->addWidget(new QLabel(tr("uri")),0,0);
	dc_layout->addWidget(new QLabel(tr("displayId")),1,0);
	dc_layout->addWidget(new QLabel(tr("name")),2,0);
	dc_layout->addWidget(new QLabel(tr("description")),3,0);
	dc_layout->addWidget(new QLabel(tr("type")),4,0);
	dc_layout->addWidget(dc_uri = new QLineEdit,0,1);
	dc_layout->addWidget(dc_displayId = new QLineEdit,1,1);
	dc_layout->addWidget(dc_name = new QLineEdit,2,1);
	dc_layout->addWidget(dc_description = new QLineEdit,3,1);
	dc_layout->addWidget(dc_type = new QLineEdit,4,1);

	sa_layout->addWidget(new QLabel(tr("uri")),0,0);
	sa_layout->addWidget(new QLabel(tr("bioStart")),1,0);
	sa_layout->addWidget(new QLabel(tr("bioEnd")),2,0);
	sa_layout->addWidget(new QLabel(tr("strand")),3,0);
	sa_layout->addWidget(sa_uri = new QLineEdit,0,1);
	sa_layout->addWidget(sa_bioStart = new QLineEdit,1,1);
	sa_layout->addWidget(sa_bioEnd = new QLineEdit,2,1);
	sa_layout->addWidget(sa_strand = new QLineEdit,3,1);


	ds_layout->addWidget(new QLabel(tr("uri")),0,0);
	ds_layout->addWidget(new QLabel(tr("nucleotides")),1,0);
	ds_layout->addWidget(ds_uri = new QLineEdit,0,1);
	ds_layout->addWidget(ds_nucleotides = new QLineEdit, 1,1);

	//added end



	listWidget = new QListWidget;
	connect(listWidget,SIGNAL(itemActivated ( QListWidgetItem * )), this, SLOT(parameterItemActivated ( QListWidgetItem * )));
	layout3->addWidget(listWidget);

	groupBox1 = new QGroupBox(tr(" Species "));
	groupBox1->setLayout(layout1);

	groupBox2 = new QGroupBox(tr(" Reaction "));
	groupBox2->setLayout(layout2);

	groupBox3 = new QGroupBox(tr(" Parameters "));
	groupBox3->setLayout(layout3);

	dc_groupBox = new QGroupBox(tr(" DNA Component "));
	dc_groupBox->setLayout(dc_layout);

    sa_groupBox = new QGroupBox(tr(" Sequence Annotation "));
    sa_groupBox->setLayout(sa_layout);

    ds_groupBox = new QGroupBox(tr(" DNA Sequence "));
    ds_groupBox->setLayout(ds_layout);

	QVBoxLayout * layout4 = new QVBoxLayout;
	layout4->addWidget(groupBox1);
	layout4->addWidget(groupBox2);
	layout4->addWidget(dc_groupBox);
	layout4->addWidget(sa_groupBox);
	layout4->addWidget(ds_groupBox);
	layout4->addWidget(groupBox3);


	setLayout(layout4);

	groupBox1->hide();
	groupBox2->hide();
	dc_groupBox->hide();
	sa_groupBox->hide();
	ds_groupBox->hide();

	connect(name1,SIGNAL(editingFinished()),this,SLOT(nameChanged()));
	connect(name2,SIGNAL(editingFinished()),this,SLOT(nameChanged()));
	connect(conc,SIGNAL(editingFinished()),this,SLOT(concentrationChanged()));
	connect(rate,SIGNAL(editingFinished()),this,SLOT(rateChanged()));

	//for new
    connect(dc_displayId, SIGNAL(editingFinished()), this, SLOT(dc_displayIdChanged()));
	connect(dc_uri,SIGNAL(editingFinished()), this, SLOT(uriChanged()));
	connect(sa_uri,SIGNAL(editingFinished()), this, SLOT(uriChanged()));
	connect(ds_uri,SIGNAL(editingFinished()), this, SLOT(uriChanged()));
}

void SimpleDesigner::dc_displayIdChanged()
{

    GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

	if (dc_displayId->isVisible())
    {
        console()->message(dc_displayId->text());

        /*DNAComponent* current = getDNAComponent(doc, dc_uri->text().toStdString().c_str());
        setDNAComponentDisplayID(current, dc_displayId->text().toStdString().c_str());
        dc_uri->setText(QString::fromStdString(string(getDNAComponentURI(current))));
        handle->rename(QString::fromStdString(string(getDNAComponentDisplayID(current))));*/
        /*DNAComponent* current = (DNAComponent*)(global_entity[dc_uri->text().toStdString()].first);

        current->set_displayId(dc_displayId->text());
        global_entity[current->s_get_uri()] = std::make_pair((void*)current,NODE_TYPE_DC);
        dc_uri->setText(current->get_uri());
        handle->rename(current->get_displayId());*/
        //handle->rename(dc_displayId->text());
    }

	for (int i=0; i < handle->graphicsItems.size(); ++i)
		handle->graphicsItems[i]->update();

    uriChanged();

}


void SimpleDesigner::uriChanged()
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

    if(!handle || !handle->hasTextData("uri")) return;

    bool ok;

    DataTable<QString> table;

    int node_type = global_entity[handle->textData("uri").toStdString()].second;

//    if(isDNAComponentURI(doc,handle->textData("uri").toStdString().c_str()))
    if(node_type == NODE_TYPE_DC)
    {
        /*DNAComponent* current = getDNAComponent(doc, handle->textData("uri").toStdString().c_str());
        setDNAComponentURI(current,dc_uri->text().toStdString().c_str());*/

        /*DNAComponent* current = (DNAComponent*)global_entity[handle->textData("uri").toStdString()].first;
        global_entity.erase(current->s_get_uri());
        current->set_uri(dc_uri->text());
        std::pair<void*,int> po;
        po = std::make_pair((void*)current, NODE_TYPE_DC);
        global_entity[current->s_get_uri()] = po;

        table.value(0,0) = dc_uri->text();*/
    }
//    else if (isSequenceAnnotationURI(doc,handle->textData("uri").toStdString().c_str()))
    else if(node_type == NODE_TYPE_SA)
    {
        /*SequenceAnnotation* current = getSequenceAnnotation(doc, handle->textData("uri").toStdString().c_str());
        deleteSequenceAnnotation(current);
        SequenceAnnotation* new_sa = createSequenceAnnotation(doc, sa_uri->text().toStdString().c_str());*/

        /*SequenceAnnotation * current = (SequenceAnnotation*)global_entity[handle->textData("uri").toStdString()].first;
        global_entity.erase(current->s_get_uri());
        current->set_uri(sa_uri->text());
        std::pair<void*, int> po;
        po = std::make_pair((void*)current, NODE_TYPE_SA);
        global_entity[current->s_get_uri()] = po;
        table.value(0,0) = sa_uri->text();*/
    }
//    else if (isDNASequenceURI(doc,handle->textData("uri").toStdString().c_str()))
    else if (node_type == NODE_TYPE_DS)
    {
        //DNASequence* current = getDNASequence(doc, handle->textData("uri").toStdString().c_str());
        //deleteDNASequence(current);
        //DNASequence* new_ds = createDNASequence(doc, ds_uri()->text().toStdString().c_str() );
        /*DNASequence * current = (DNASequence*)global_entity[handle->textData("uri").toStdString()].first;
        global_entity.erase(current->s_get_uri());
        current->set_uri(ds_uri->text());
        std::pair<void*, int> po;
        po = std::make_pair((void*)current, NODE_TYPE_DS);
        global_entity[current->s_get_uri()] = po;
        table.value(0,0) = ds_uri->text();*/
    }
    handle->changeData("uri",&table);

}

void SimpleDesigner::nameChanged()
{
	GraphicsScene * scene = currentScene();
	if (!scene || scene->selected().size() != 1) return;

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

	if (name1->isVisible())
		handle->rename(name1->text());
	else
	if (name2->isVisible())
		handle->rename(name2->text());

	for (int i=0; i < handle->graphicsItems.size(); ++i)
		handle->graphicsItems[i]->update();
}

void SimpleDesigner::addParameters(QStringList& newVars)
{
	NetworkHandle * network = currentNetwork();
	if (!network || !network->globalHandle()) return;

	listWidget->clear();

	ItemHandle * globalHandle = network->globalHandle(); //handle for the entire model
	QStringList vars;
	DataTable<qreal> params;

	params = globalHandle->numericalDataTable("parameters"); //get existing set of parameters
	vars = params.rowNames();

	for (int i=0; i < newVars.size(); ++i)
		if (!vars.contains(newVars[i]))
		{
			vars << newVars[i];
			params.value(newVars[i],0) = 1.0;   //add new parameters to existing set
		}

	if (vars.isEmpty()) return;

	globalHandle->changeData("parameters", &params); //update with new set of parameters
	vars.clear();

	for (int i=0; i < params.rows(); ++i)
		vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"

	listWidget->addItems(vars);   //update list widget for viewing parameters
}

void SimpleDesigner::parameterItemActivated ( QListWidgetItem * item )
{
	NetworkHandle * network = currentNetwork();
	if (!network) return;  //no window open

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

	//find all the new variables in this equation
	QStringList newVars, oldVars;
	QList<ItemHandle*> handles;
	bool ok = net->parseMath(formula,newVars,oldVars,handles);

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

	QGraphicsItem * selectedItem = scene->selected()[0];
	ItemHandle * handle = getHandle(selectedItem);

	if (!handle || !handle->hasNumericalData("concentration")) return;

	DataTable<qreal> table;

	bool ok;

	table.value(0,0) = conc->text().toDouble(&ok);

	if (ok && handle->numericalData("concentration") != table.value(0,0))
		handle->changeData("concentration",&table);
}

void SimpleDesigner::actionTriggered(QAction* action)
{
	mode = 0;

	if (!action) return;

	if (action->text() == tr("node"))
		mode = 1;
	else
	if (action->text() == tr("reaction"))
		mode = 2;
    else
    if (action->text() == tr("DNA Component"))
        mode = NODE_TYPE_DC;
    else
    if (action->text() == tr("Sequence Annotation"))
        mode = NODE_TYPE_SA;
    else
    if (action->text() == tr("DNA Sequence"))
        mode = NODE_TYPE_DS;
}

bool SimpleDesigner::setMainWindow(MainWindow * main)
{
	Tool::setMainWindow(main);
	if (mainWindow)
	{
		QWidget * tool = mainWindow->tool("Default Plot Tool");
		plotTool = static_cast<PlotTool*>(tool);

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

		connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle*, const QList<ItemHandle*>&)),
				this, SLOT(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)));

		connect(mainWindow,
				SIGNAL(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,
				SLOT(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers))
				);

		connect(mainWindow,
				SIGNAL(escapeSignal(const QWidget * )),
				this,
				SLOT(escapeSignal(const QWidget * ))
				);

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
	if (!scene || !scene->network || mode != 1)
        if(mode != NODE_TYPE_DC && mode != NODE_TYPE_SA && mode !=NODE_TYPE_DS)
            {
                groupBox1->hide();
                groupBox2->hide();
                dc_groupBox->hide();
                sa_groupBox->hide();
                ds_groupBox->hide();
                return;
            }

	QGraphicsItem * item = new SimpleNode;
	NodeHandle * handle = new NodeHandle;

    if(mode == 1)
    {
        handle->name = scene->network->makeUnique(tr("s1"));
    }
    else if(mode == NODE_TYPE_DC)
    {
        handle->name = scene->network->makeUnique(tr("dc1"));
        handle->textData("displayId") = handle->name;
    }
    else if(mode == NODE_TYPE_SA)
    {
        handle->name = scene->network->makeUnique(tr("sa1"));
    }
    else if(mode == NODE_TYPE_DS)
    {
        handle->name = scene->network->makeUnique(tr("ds1"));
    }

	setHandle(item,handle);
	item->setPos(point);

	scene->insert(handle->name + tr(" inserted"),item);

}

void SimpleDesigner::setToolTip(ItemHandle* item)
{
	if (!item) return;

	if (NodeHandle::cast(item) && item->hasNumericalData("concentration")) //is a node
	{
		for (int i=0; i < item->graphicsItems.size(); ++i)
			item->graphicsItems[i]->setToolTip
			(
				item->name + tr(" : ") + QString::number(item->numericalData("concentration"))
			);
	}
	else
    if (NodeHandle::cast(item) && item->hasNumericalData("uri")) // covers DC, SA
    {
        for (int i=0; i< item->graphicsItems.size(); ++i)
            item->graphicsItems[i]->setToolTip
            (
                item->name+tr(":")+QString::number(item->numericalData("uri")));
    }
	else
	if (ConnectionHandle::cast(item) && item->hasTextData("rate")) //is a connection
	{
		for (int i=0; i < item->graphicsItems.size(); ++i)
        {
			item->graphicsItems[i]->setToolTip
			(
				item->name + tr(" : ") + item->textData("rate")
			);
        }

	}
}

void SimpleDesigner::itemsInserted(NetworkHandle * net,const QList<ItemHandle*>& items)
{
	for (int i=0; i < items.size(); ++i)
	{
	    if (NodeHandle::cast(items[i])) //is node?
		{
		    if(mode == 1)
                items[i]->numericalData("concentration") = 10.0;
            else if(mode == NODE_TYPE_DC)
            {
                /*string temp = "http://partregistry.org/Part:"+items[i]->name.toStdString();
                DNAComponent *dc1 = createDNAComponent(doc,  temp.c_str());
                setDNAComponentDisplayID(dc1,items[i]->name.toStdString().c_str());
                setDNAComponentName(dc1, items[i]->name.toStdString().c_str());
                setDNAComponentDescription(dc1,"Add Description");*/
                /*DNAComponent * current;
                current = new DNAComponent(NodeHandle::cast(items[i])->name); // garbage collection needed
                std::pair<void*,int> po;
                po = std::make_pair((void*)current, NODE_TYPE_DC);
                global_entity[current->s_get_uri()] = po;
                items[i]->textData("uri") = current->get_uri();
                items[i]->textData("displayId") = current->get_displayId();*/
            }
            else if(mode == NODE_TYPE_DS)
            {
                /*
                string temp = "http://sbols.org/data#"+items[i]->name.toStdString();
                DNASequence *ds = createDNASequence(doc, temp.c_str());
                setDNASequenceNucleotides(ds,"aaaa");*/
                /*DNASequence * current;
                current = new DNASequence(NodeHandle::cast(items[i])->name);
                std::pair<void*,int> po;
                po = std::make_pair((void*)current, NODE_TYPE_DS);
                global_entity[current->s_get_uri()] = po;
                items[i]->textData("uri") = current->get_uri();
                items[i]->textData("nucleotides") = current->get_nucleotides();*/
            }
            else if(mode == NODE_TYPE_SA)
            {
                /*
                string temp = "http://sbols.org/annot#"+items[i]->name.toStdString();
                SequenceAnnotation *sa = createSequenceAnnotation(doc, temp.c_str());
*/
                /*SequenceAnnotation * current;
                current = new SequenceAnnotation(NodeHandle::cast(items[i])->name);
                std::pair<void*,int> po;
                po = std::make_pair((void*)current, NODE_TYPE_SA);
                global_entity[current->s_get_uri()] = po;
                items[i]->textData("uri") = current->get_uri();*/

            }

		}
		if (ConnectionHandle::cast(items[i])) //is reaction?
		{
			ConnectionHandle * connection = ConnectionHandle::cast(items[i]);
			QString rate;

			if (connection->hasTextData("rate"))  //rate already exists
			{
				QStringList newVars,oldVars;
				QList<ItemHandle*> handles;
				rate = connection->textData("rate");
				bool ok = net->parseMath(rate,newVars,oldVars,handles);

				if (ok)
				{
					addParameters(newVars);
				}
			}
			else
			{
				QList<NodeHandle*> nodes = connection->nodes("reactant");
				//QList<NodeHandle*> nodes = connection->nodes("product");

				rate = tr("1.0");
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
be in connect-mode, i.e. connecting two items together
*/
void SimpleDesigner::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
	if (!scene) return;

	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;

	//go through all the previously selected items and un-color them
	for (int i=0; i < selectedItems.size(); ++i)
	{
		deselectItem(scene,selectedItems[i]);

		if (mode == 2) //if connect-mode, then store the previously selected nodes
		{
			node = NodeGraphicsItem::cast(selectedItems[i]);
			if (node && !nodeItems.contains(node))
				nodeItems << node;
		}
	}

	selectedItems.clear();

	if (items.size() != 1 || !items[0]) //scene selected
	{
		groupBox1->hide();
		groupBox2->hide();
		listWidget->clear();

		NetworkHandle * network = scene->network;
		NumericalDataTable & params = network->globalHandle()->numericalDataTable("parameters");
		QStringList vars;

		for (int i=0; i < params.rows(); ++i)
			vars << params.rowName(i) + tr(" = ") + QString::number(params.value(i,0));  //"param = value"

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
            dc_groupBox->hide();
            ds_groupBox->hide();
            sa_groupBox->hide();
		}
		else
		if (NodeHandle::cast(handle) && handle->hasTextData("uri"))
        {
            if(global_entity.count(handle->textData("uri").toStdString()))
            {
                int node_type = global_entity[handle->textData("uri").toStdString()].second;

                sa_groupBox->hide();
                dc_groupBox->hide();
                ds_groupBox->hide();
                groupBox1->hide();
                groupBox2->hide();

                if(node_type == NODE_TYPE_DC)
                {
                    dc_displayId->setText(handle->name);
                    dc_uri->setText( handle->textData("uri") );
                    dc_groupBox->show();
                }
                else if (node_type == NODE_TYPE_DS)
                {
                    ds_nucleotides->setText(handle->textData("nucleotides"));
                    ds_uri->setText(handle->textData("uri"));
                    ds_groupBox->show();
                }
                else if (node_type == NODE_TYPE_SA)
                {
                    sa_uri->setText(handle->textData("uri"));
                    sa_groupBox->show();
                }
            }
        }
		else
		if (ConnectionHandle::cast(handle) && handle->hasTextData("rate"))
		{
			name2->setText(  handle->name  );
			rate->setText(  handle->textData("rate") );
			groupBox1->hide();
			groupBox2->show();
			ds_groupBox->hide();
			dc_groupBox->hide();
			sa_groupBox->hide();
		}
		else
		{
			groupBox1->hide();
			groupBox2->hide();
			sa_groupBox->hide();
			dc_groupBox->hide();
			ds_groupBox->hide();
		}
	}

	selectedItems = items;

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

	if (mode == 2 && nodeItems.size() == 2) //insert connection mode
	{
		NodeGraphicsItem * node1 = NodeGraphicsItem::cast(nodeItems[0]),  //nodes to connect
									 * node2 = NodeGraphicsItem::cast(nodeItems[1]);

		ConnectionGraphicsItem * item = new ConnectionGraphicsItem(node1, node2); //create new connection

		ConnectionHandle * handle = new ConnectionHandle;  //create handle for the connection
		setHandle(item,handle);

		NodeHandle * nodeHandle1 = NodeHandle::cast(node1->handle()),
						   * nodeHandle2 = NodeHandle::cast(node2->handle());
		handle->setNodeRole(nodeHandle1, "reactant");  //assign roles for each node (will be useful later)
		handle->setNodeRole(nodeHandle2, "product");
		handle->name = scene->network->makeUnique(tr("J1"));  //find unique name, with J1 being the default

		scene->insert(tr("connection inserted"),item); //insert the new connection

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

	SBMLDocument_t * doc = SBMLDocument_create();
	Model_t * model = SBMLDocument_createModel(doc);

	QList<ItemHandle*> handles = network->handles();
	NumericalDataTable params = network->globalHandle()->numericalDataTable("parameters");

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

	//create list of reactions
	for (int i=0; i < handles.size(); ++i)
		if (ConnectionHandle::cast(handles[i]))  //if reaction
		{

			ConnectionHandle * reactionHandle = ConnectionHandle::cast(handles[i]);

			QString name = reactionHandle->name;
			QString rate = reactionHandle->textData("rate");

			QList<NodeHandle*> reactants = reactionHandle->nodes("reactant");
			QList<NodeHandle*> products = reactionHandle->nodes("product");

			Reaction_t * reac = Model_createReaction(model);
			Reaction_setId(reac, ConvertValue(name));
			Reaction_setName(reac, ConvertValue(name));
			Reaction_setId(reac, ConvertValue(name));
			KineticLaw_t  * kinetic = Reaction_createKineticLaw(reac);
			KineticLaw_setFormula( kinetic, ConvertValue( rate ));

			for (int j=0; j < reactants.size(); ++j)
			{
				SpeciesReference_t * sref = Reaction_createReactant(reac);
				SpeciesReference_setId(sref, ConvertValue(reactants[j]->name));
				SpeciesReference_setName(sref, ConvertValue(reactants[j]->name));
				SpeciesReference_setSpecies(sref, ConvertValue(reactants[j]->name));
				//SpeciesReference_setStoichiometry( sref, -stoictc_matrix.value(j,i) );
			}
			for (int j=0; j < products.size(); ++j)
			{
				SpeciesReference_t * sref = Reaction_createProduct(reac);
				SpeciesReference_setId(sref, ConvertValue(products[j]->name));
				SpeciesReference_setName(sref, ConvertValue(products[j]->name));
				SpeciesReference_setSpecies(sref, ConvertValue(products[j]->name));
			}
		}

	//create list of parameters
	for (int i=0; i < params.rows(); ++i)
	{
		Parameter_t * p = Model_createParameter(model);
		Parameter_setId(p, ConvertValue(params.rowName(i)));
		Parameter_setName(p, ConvertValue(params.rowName(i)));
		Parameter_setValue(p, params.value(i,0));
	}

	SBML_sim sim("C:\\Users\\Deepak\\Desktop\\temp.txt");

	vector<string> names = sim.getVariableNames();
	vector< vector<double> > output;
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

	if (plotTool)
		plotTool->plot(results,"Simulation");

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

	//MUST DO
	QApplication app(argc, argv);
	QString appDir = QCoreApplication::applicationDirPath();

	//enable features -- must do before creating MainWindow
	GlobalSettings::ENABLE_HISTORY_WINDOW = false;
	GlobalSettings::ENABLE_CONSOLE_WINDOW = true;
	GlobalSettings::ENABLE_GRAPHING_TOOLS = true;
	GlobalSettings::ENABLE_CODING_TOOLS = false;
	GlobalSettings::ENABLE_PYTHON = false;
	GlobalSettings::ENABLE_OCTAVE = true;
	GlobalSettings::ENABLE_LOADSAVE_TOOL = true;

	MainWindow mainWindow(true,false,false);  //@args: enable scene, text, allow pop-out windows
	mainWindow.readSettings();   //load settings such as window positions

	//optional
	mainWindow.setWindowTitle("Simple Designer");
    mainWindow.statusBar()->showMessage("Welcome to Simple Designer");

	//This is our main tool
	mainWindow.addTool(new SimpleDesigner);

	/*  optional  GUI configurations */
	GraphicsScene::SelectionRectangleBrush = QBrush(QColor(5,5,5,40));
	ConnectionGraphicsItem::DefaultMiddleItemFile = "";
	ConnectionGraphicsItem::DefaultArrowHeadFile = ":/images/arrow.xml";

	//Ontology::readNodes("NodesTree.nt");
	//Ontology::readConnections("ConnectionsTree.nt");

    GraphicsScene * scene = mainWindow.newScene();
	GraphicsView::DEFAULT_ZOOM = 0.5;

    mainWindow.show();

    int output = app.exec();

    return output;
}

