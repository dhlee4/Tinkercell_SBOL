<<<<<<< HEAD
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- "Parameters" and "Text Attributes".
Attributes are essentially <name,value> pairs (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();rs that are used to characterize an item.

The ParametersTool contains two tools, one for text attributes and one
for parameters. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include "NetworkHandle.h"
#include "GraphicsScene.h"
#include "UndoCommands.h"
#include "ConsoleWindow.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "NodeGraphicsReader.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ParametersTool.h"
#include "NodesTree.h"
#include "GlobalSettings.h"

namespace Tinkercell
{	
	QHash<QString,double> ParametersTool::initialValues;
	bool ParametersTool::EnforceDefaultParameters=false;

	void ParametersTool::select(int)
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		itemHandles = getHandle(scene->selected(),false);
		if (itemHandles.isEmpty())
				if (scene->localHandle())
					itemHandles += scene->localHandle();
				else
					if (scene->globalHandle())
						itemHandles += scene->globalHandle();

		if (itemHandles.isEmpty()) return;

		if (dockWidget && dockWidget->widget() != this)
			dockWidget->setWidget(this);

		openedByUser = true;
		updateTable();
		closeButton->setVisible(true);

		if (dockWidget != 0)
		{
			if (dockWidget->isVisible())
				openedByUser = false;
			else
				dockWidget->show();
		}
		else
		{
			if (isVisible())
				openedByUser = false;
			else
				show();
		}
		this->setFocus();
	}

	void ParametersTool::deselect(int)
	{
		/*if (openedByUser && (!dockWidget || dockWidget->isFloating()))
		{
			openedByUser = false;

			if (dockWidget != 0)
				dockWidget->hide();
			else
				hide();
		}*/
	}

	void ParametersTool::loadInitialValues()
	{
		Tool * tool = mainWindow->tool(tr("Nodes Tree"));
		
		if (!tool) return;
		
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);
		settings.beginGroup("ParametersTool");
		
		NodesTree * nodesTree = static_cast<NodesTree*>(tool);

		QStringList families(nodesTree->getAllFamilyNames());
		QStringList keys, rownames;
		NodeFamily * family;
		QString key;
		
		for (int i=0; i < families.size(); ++i)
		{
			family = nodesTree->getFamily( families[i] );
			if (family && !family->measurementUnit.property.isEmpty())
			{
				key = family->measurementUnit.property;
				initialValues[key] = 1.0;
			}
		}
		
		keys = initialValues.keys();		
		QStringList names = settings.value(tr("InitialValueNames"),QStringList()).toStringList();
		QStringList values = settings.value(tr("InitialValues"),QStringList()).toStringList();
			
		bool ok;

		for (int i=0; i < names.size() && i < values.size(); ++i)
		{
			double d = values[i].toDouble(&ok);
			if (ok && !names[i].isEmpty())
				initialValues[ names[i] ] = d;
		}

		settings.endGroup();
	}

	bool ParametersTool::setMainWindow(MainWindow * main)
	{
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

		Tool::setMainWindow(main);

		if (mainWindow)
		{
			loadInitialValues();
			
			connect(this,SIGNAL(dataChanged(const QList<ItemHandle*>&)),
					mainWindow,SIGNAL(dataChanged(const QList<ItemHandle*>&)));
			
			connect(mainWindow,SIGNAL(networkClosing(NetworkHandle * , bool *)),this,SLOT(windowClosing(NetworkHandle * , bool *)));

			connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle *, const QList<ItemHandle*>&)),
				this,SLOT(itemsInserted(NetworkHandle *, const QList<ItemHandle*>&)));

			connect(mainWindow,SIGNAL(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,SLOT(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)));

			connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));

			connect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));

			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(pluginLoaded(Tool*)));

			setWindowTitle(name);
			setWindowFlags(Qt::Dialog);
			dockWidget = mainWindow->addToolWindow(this,MainWindow::DockWidget,Qt::BottomDockWidgetArea,Qt::NoDockWidgetArea);

			if (dockWidget)
			{
				if (type == text)
					dockWidget->move(mainWindow->geometry().bottomRight() - QPoint(sizeHint().width()*2,sizeHint().height()*3));
				else
					dockWidget->move(mainWindow->geometry().bottomRight() - QPoint(sizeHint().width()*2,sizeHint().height()*2));

				dockWidget->setWindowFlags(Qt::Dialog);
				dockWidget->setAttribute(Qt::WA_ContentsPropagated);
				dockWidget->setPalette(QPalette(QColor(255,255,255,255)));
				dockWidget->setAutoFillBackground(true);
				if (closeButton)
					connect(closeButton,SIGNAL(pressed()),dockWidget,SLOT(hide()));
				//dockWidget->setWindowOpacity(0.8);
	
				QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

				if (type == both || type == numerical)
				{
					settings.beginGroup("ParametersTool");
					//dockWidget->resize(settings.value("size", sizeHint()).toSize());
					//dockWidget->move(settings.value("pos", dockWidget->pos()).toPoint());
				}
				else
				{
					settings.beginGroup("ParametersToolText");
					//dockWidget->resize(settings.value("size", sizeHint()).toSize());
					//dockWidget->move(settings.value("pos", dockWidget->pos()).toPoint());
				}
				
				settings.endGroup();
				dockWidget->hide();
				dockWidget->setFloating(true);
			}

			pluginLoaded(0);
		}
		return (mainWindow != 0);
	}

	void ParametersTool::pluginLoaded(Tool*)
	{
		static bool connected = false;
		if (connected) return;

		if (!connected && type != text && mainWindow && mainWindow->tool(tr("Model Summary")))
		{
			QWidget * modelSummary = mainWindow->tool(tr("Model Summary"));

			connect(modelSummary,SIGNAL(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)),
				this,SLOT(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)));
			connected = true;
		}
	}

	void ParametersTool::windowClosing(NetworkHandle * , bool *)
	{
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

		if (dockWidget)
		{
			if (type == text)
			{
				settings.beginGroup("ParametersToolText");
			}
			else
			{
				settings.beginGroup("ParametersTool");
			}
			//settings.setValue("floating", dockWidget && dockWidget->isFloating());
			settings.setValue("size", dockWidget->size());
			settings.setValue("pos", dockWidget->pos());
			
			QStringList values;
			QStringList names = initialValues.keys();
			QList<double> doubles = initialValues.values();
			
			for (int i=0; i < doubles.size(); ++i)	values << QString::number(doubles[i]);
			
			settings.setValue(tr("InitialValueNames"),names);
			settings.setValue(tr("InitialValues"),values);
			
			settings.endGroup();
		}
	}

	void ParametersTool::aboutToDisplayModel(const QList<ItemHandle*>& , QHash<QString,qreal>& , QHash<QString,QString>& )
	{
	}

	void ParametersTool::displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equationsList)
	{
		if (type == text) return;

		itemHandles.clear();
		tableItems.clear();
		tableWidget.clear();
		
		//QStringList eqns(equationsList.values());
		//QString equations = QStringList(equationsList.values()).join(" ");
		closeButton->setVisible(false);

		for (int i=0; i < items.size(); ++i)
		{
			if (!itemHandles.contains(items[i]) && items[i])
				itemHandles += items[i];
		}

		QStringList names, values, min, max;
		QStringList headers;
		ignoredVarNames.clear();

		DataTable<qreal> * nDataTable = 0;

		for (int i=0; i < itemHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			//bool remove = true;
			if (itemHandles[i] != 0)
			{
				if (itemHandles[i]->hasNumericalData(this->name))
				{
					nDataTable = &(itemHandles[i]->numericalDataTable(this->name));
					for (int j=0; j < nDataTable->rows(); ++j)
					{
						QString str = itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j);
						//if (equations.isEmpty() || equations.contains(str))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							headers << (itemHandles[i]->fullName() + tr("."));
							names += nDataTable->rowName(j);
							values += QString::number(nDataTable->value(j,0));
							min += QString::number(nDataTable->value(j,1));
							max += QString::number(nDataTable->value(j,2));
							constants.insert(str,nDataTable->value(j,0));
							//remove = false;
						}
						/*else
						{
							ignoredVarNames << (itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j));
						}*/
					}
				}
			}
			/*if (remove)
			{
				itemHandles.removeAt(i);
				--i;
			}*/
		}

		if (names.size() > 0)
		{
			if (dockWidget && dockWidget->isVisible())
				dockWidget->hide();

			widgets.insertTab(0,this,tr("Parameters"));

			tableWidget.clear();
			tableWidget.setRowCount(names.size());
			tableWidget.setColumnCount(4);
			tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" << "min" << "max");
			tableWidget.setVerticalHeaderLabels(headers);

			disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

			for (int i=0; i < tableWidget.rowCount(); ++i)
			{
				tableWidget.setItem(i,0,new QTableWidgetItem(names[i]));
				tableWidget.setItem(i,1,new QTableWidgetItem(values[i]));
				tableWidget.setItem(i,2,new QTableWidgetItem(min[i]));
				tableWidget.setItem(i,3,new QTableWidgetItem(max[i]));
			}

			connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		}
		else
			if (dockWidget && dockWidget->widget() != this)
				dockWidget->setWidget(this);
	}

	void ParametersTool::historyUpdate(int i)
	{
		if (parentWidget() && parentWidget()->isVisible())
			updateTable();
		NetworkHandle * win = currentNetwork();
		if (mainWindow && win && mainWindow->statusBar())
			mainWindow->statusBar()->showMessage(win->history.text(i-1));
	}

	void ParametersTool::itemsInserted(NetworkHandle * , const QList<ItemHandle*>& handles)
	{
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->family() && !handles[i]->tools.contains(this))
			{
				handles[i]->tools += this;	
			}

			if (handles[i] && handles[i]->family())
			{
				insertDataMatrix(handles[i]);
			}
		}
	}

	void ParametersTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& list, QPointF , Qt::KeyboardModifiers )
	{
		if (scene && (parentWidget() && parentWidget() != mainWindow && parentWidget()->isVisible()))
		{
			itemHandles.clear();
			ItemHandle * handle = 0;
			for (int i=0; i < list.size(); ++i)
			{
				if ((handle = getHandle(list[i])))
					itemHandles += handle;
			}

			if (itemHandles.isEmpty())
				if (scene->localHandle())
					itemHandles += scene->localHandle();
				else
					if (scene->globalHandle())
						itemHandles += scene->globalHandle();
			
			updateTable();
		}
	}

	void ParametersTool::setValue(int row,int col)
	{
		static bool recursive = false;

		if (recursive)
		{
			recursive = false;
			return;
		}
		recursive = false;

		NetworkHandle * win = currentNetwork();
		if (!win) return;

		if (col > 3  || row >= tableItems.size() || !tableWidget.item(row,col)) return;

		ItemHandle * handle = tableItems[row].first;
		int rowNumber = tableItems[row].second;

		if (!handle) return;

		if ((type == both || type == numerical) && handle->hasNumericalData(this->name))
		{
			DataTable<qreal> nDat(handle->numericalDataTable(this->name));

			if (col == 0)
			{
				if (rowNumber < nDat.rows())
				{
						QString name = tableWidget.item(row,col)->text();
						name.replace(QRegExp(tr("[^a-zA-Z_0-9]")),tr(""));
						if (name.isEmpty())
						{
							QString s = nDat.rowName(rowNumber);
							QString err = removeParameterFromModel(win, handle, s);
							if (!err.isEmpty())
							{
								QMessageBox::information(this,tr("Cannot remove"), err);
								recursive = true;
								tableWidget.item(row,col)->setText(s);
								return;
							}
						}
						else
						{
							QString oldname = nDat.rowName(rowNumber);
							if (handle->name.isEmpty())
								win->rename(oldname, name);
							else
								win->rename(handle->fullName() + tr(".") + oldname, handle->fullName() + tr(".") + name);
						}
				}
			}
			else
			{
				bool ok;
				double value = tableWidget.item(row,col)->text().toDouble(&ok);
				if (!ok)
				{
					recursive = true;
					tableWidget.item(row,col)->setText( QString::number(nDat.value(rowNumber,col-1)) );
					return;
				}
				nDat.value(rowNumber,col-1) = value;
				if (handle->name.isEmpty())
					win->changeData(nDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&nDat);
				else
					win->changeData(handle->fullName() + tr(".") + nDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&nDat);
			}
		}

		if ((type == both || type == text) && handle->hasTextData(this->name))
		{
			DataTable<QString> sDat(handle->textDataTable(this->name));

			if (col == 0)
			{
				if (rowNumber < sDat.rows())
				{
					if (handle->family() && handle->family()->textAttributes.contains(sDat.rowName(rowNumber)))
					{
						recursive = true;
						tableWidget.item(row,col)->setText(sDat.rowName(rowNumber));
						if (console())
                            console()->message(sDat.rowName(rowNumber) + tr(" cannot be removed because it is a family attribute"));
					}
					else
					{
						QString name = tableWidget.item(row,col)->text();
						name.replace(QRegExp(tr("[^a-zA-Z_]")),tr(""));
						if (name.isEmpty())
						{
							QString s = sDat.rowName(rowNumber);
							sDat.removeRow(rowNumber);
							if (handle->name.isEmpty())
								win->changeData(s + tr(" removed"),handle, this->name,&sDat);
							else
								win->changeData(handle->fullName() + tr(".") + s + tr(" removed"),handle, this->name,&sDat);
						}
						else
						{
							QString oldname = sDat.rowName(rowNumber);
							if (handle->name.isEmpty())
								win->rename(oldname, name);
							else
								win->rename(handle->fullName() + tr(".") + oldname, handle->fullName() + tr(".") + name);
						}
					}
				}
			}
			else
			if (col == 1)
			{
				QString value = tableWidget.item(row,col)->text();
				sDat.value(rowNumber,0) = value;
				if (handle->name.isEmpty())
					win->changeData(sDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&sDat);
				else
					win->changeData(handle->fullName() + tr(".") + sDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&sDat);
			}
		}
	}

	ParametersTool::ParametersTool(const QString& typ) : Tool(tr("Attributes"),tr("Modeling")), delegate(QStringList())
	{
		QString appDir = QCoreApplication::applicationDirPath();
		delegate.textColumn = 0;
		if (typ == tr("both"))
		{
			type = ParametersTool::both;
			groupBox = new QGroupBox(tr(" Attributes "),this);
		}
		else
			if (typ == tr("text"))
			{
				type = ParametersTool::text;
				name = tr("Text Attributes");
				groupBox = new QGroupBox(tr(" Strings "),this);
			}
			else
			{
				type = ParametersTool::numerical;
				name = tr("Parameters");
				groupBox = new QGroupBox(tr(" Constants "),this);
			}

			NodeGraphicsReader reader;
			ToolGraphicsItem * toolGraphicsItem = new ToolGraphicsItem(this);
			addGraphicsItem(toolGraphicsItem);
			
			if (type == ParametersTool::both)
			{
				reader.readXml(&item,tr(":/images/textsheet.xml"));
				item.setToolTip(tr("Attributes"));
				toolGraphicsItem->setToolTip(tr("Attributes"));
			}
			else
				if (type == ParametersTool::numerical)
				{
					reader.readXml(&item,tr(":/images/datasheet.xml"));
					item.setToolTip(tr("Parameters"));
					toolGraphicsItem->setToolTip(tr("Parameters"));
				}
				else
				{
					reader.readXml(&item,tr(":/images/textsheet.xml"));
					item.setToolTip(tr("Text attributes"));
					toolGraphicsItem->setToolTip(tr("Text attributes"));
				}

			item.normalize();
			item.scale(30.0/item.sceneBoundingRect().width(),40.0/item.sceneBoundingRect().height());
			toolGraphicsItem->addToGroup(&item);

			openedByUser = false;

			tableWidget.setEditTriggers ( QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed );
			connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

			QFont font = this->font();
			font.setPointSize(12);
			tableWidget.setFont(font);

			QHBoxLayout * actionsLayout = new QHBoxLayout;

			QToolButton * addAttribAction = new QToolButton(this);
			addAttribAction->setIcon(QIcon(":/images/plus.png"));

			QToolButton * removeAttribAction = new QToolButton(this);
			removeAttribAction->setIcon(QIcon(":/images/minus.png"));

			closeButton = new QPushButton("Done", this);
			
			QString message;
			if (type == ParametersTool::numerical)
			{
				addAttribAction->setToolTip(tr("Add constant"));
				removeAttribAction->setToolTip(tr("Remove constant"));
				//tableWidget.setItemDelegate(&delegate);
				//tableWidget.setFixedWidth(160);
				groupBox->setMinimumWidth(100);
				message = tr("This table shows the set of parameters belonging with the selected objects. Parameters can be used inside rate equations or other equations. For example, if the name of the parameter is k0 and it belongs with an object name J0, then you can use J0.k0 inside questions in order to address this value. You may change the name or value of the parameter using this table. Changing the name will automatically update all the equations where it is used.");
			}
			else
				if (type == ParametersTool::text)
				{
					addAttribAction->setToolTip(tr("Add string"));
					removeAttribAction->setToolTip(tr("Remove string"));
					message = tr("This table shows the text attributes belonging with the selected objects. Text attributes are any properties of an object that is represented as a string. Examples include DNA sequence, database IDs, etc.");
				}
				else
				{
					addAttribAction->setToolTip(tr("Add attribute"));
					removeAttribAction->setToolTip(tr("Remove attribute"));
					message = tr("This table shows the set of parameters belonging with the selected objects. Parameters can be used inside rate equations or other equations. For example, if the name of the parameter is k0 and it belongs with an object name J0, then you can use J0.k0 inside questions in order to address this value. You may change the name or value of the parameter using this table. Changing the name will automatically update all the equations where it is used.");
					message += tr("\nThis table also shows the text attributes belonging with the selected objects. Text attributes are any properties of an object that is represented as a string. Examples include DNA sequence, database IDs, etc.");
				}

		connect(addAttribAction,SIGNAL(pressed()),this,SLOT(addAttribute()));
		connect(removeAttribAction,SIGNAL(pressed()),this,SLOT(removeSelectedAttributes()));
		
		QToolButton * question = new QToolButton(this);
		question->setIcon(QIcon(":/images/question.png"));

		QMessageBox * messageBox = new QMessageBox(QMessageBox::Information,tr("About Attributes Table"),message,QMessageBox::StandardButtons(QMessageBox::Close), const_cast<QWidget*>((QWidget*)this), Qt::WindowFlags (Qt::Dialog));
		connect(question,SIGNAL(pressed()),messageBox,SLOT(exec()));

		actionsLayout->addWidget(addAttribAction);
		actionsLayout->addWidget(removeAttribAction);
		actionsLayout->addStretch(1);
		actionsLayout->addWidget(closeButton);
		actionsLayout->addStretch(1);
		actionsLayout->addWidget(question);

		QVBoxLayout * boxLayout = new QVBoxLayout;
		boxLayout->addWidget(&tableWidget,1);

		boxLayout->addLayout(actionsLayout);
		groupBox->setLayout(boxLayout);

		dockWidget = 0;

		QVBoxLayout * layout = new QVBoxLayout;
		layout->addWidget(groupBox);
		setLayout(layout);

		connectTCFunctions();
	}

	QSize ParametersTool::sizeHint() const
	{
		return QSize(300, 200);
	}


	void ParametersTool::insertDataMatrix(ItemHandle * handle)
	{
		if (handle == 0 || handle->family() == 0) return;

		QStringList columnNames;
		
		if (type == ParametersTool::text)
			columnNames << "value";
		else
			columnNames << "value" << "min" << "max";

		ItemFamily* family = handle->family();

		if (!family->measurementUnit.property.isEmpty() && !handle->hasNumericalData(QString("Initial Value")))
		{
			DataTable<qreal> table;
			table.resize(1,1);

			table.setRowName(0, family->measurementUnit.property);

			table.setColumnName(0,family->measurementUnit.property);
			table.value(0,0) = 0.0;
			
			if (initialValues.contains(family->measurementUnit.property))
			{
				table.value(0,0) = initialValues[family->measurementUnit.property];
			}
			
			table.description() = tr("Initial value: stores measurement value of an item. See each family's measurement unit for detail.");

			handle->numericalDataTable(QString("Initial Value")) = table;
		}

		if ((type == both || type == numerical) && !(handle->hasNumericalData(name)))
		{
			QStringList nKeys;
			if (ParametersTool::EnforceDefaultParameters)
				nKeys = family->numericalAttributes.keys();
				
			DataTable<qreal> numericalAttributes;
			numericalAttributes.resize(nKeys.size(),3);
			numericalAttributes.description() = tr("Parameters: an Nx3 table storing all the real attributes for this item. Row names are the attribute names. First column holds the values. Second and third columns hold the upper and lower bounds.");

			for (int i=0; i < nKeys.size(); ++i)
			{
				numericalAttributes.value(i,0) = family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.value(i,1) = 0.1 * family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.value(i,2) = 10.0 * family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.setRowName(i, nKeys[i]);
			}

			numericalAttributes.setColumnNames(columnNames);
			handle->numericalDataTable(this->name) = numericalAttributes;
		}

		if ((type == both || type == text) && !(handle->hasTextData(name)))
		{
			QList<QString> sKeys = family->textAttributes.keys();
			//if (ParametersTool::EnforceDefaultParameters)
				//sKeys = family->textAttributes.keys();

			DataTable<QString> textAttributes;
			textAttributes.description() = tr("Text Attributes: an Nx1 table storing all the string attributes for this item. Row names are the attribute names, and first column holds the values.");
			textAttributes.resize(sKeys.size(),1);

			for (int i=0; i < textAttributes.rows() && i < sKeys.size(); ++i)
			{
				textAttributes.value(i,0) = family->textAttributes.value(sKeys[i]);
				textAttributes.setRowName(i, sKeys[i]);
			}

			textAttributes.setColumnNames(columnNames);
			handle->textDataTable(this->name) = textAttributes;
		}
	}

	void ParametersTool::updateTable()
	{
		tableItems.clear();
		tableWidget.clear();

		if (itemHandles.size() < 1)
		{
			tableWidget.clearContents();
			tableWidget.setRowCount(0);
			return;
		}

		QStringList names, values, lowerBounds, upperBounds;
		QStringList headers;

		DataTable<qreal> * nDataTable = 0;
		DataTable<QString> * sDataTable = 0;

		for (int i=0; i < itemHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			if (itemHandles[i] != 0)
			{
				if ((type == both || type == numerical) && itemHandles[i]->hasNumericalData(this->name))
				{
					nDataTable = &(itemHandles[i]->numericalDataTable(this->name));
					for (int j=0; j < nDataTable->rows(); ++j)
					{
						if (parentWidget() == dockWidget || !ignoredVarNames.contains(itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j)))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							
							if (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();
							names += nDataTable->rowName(j);
							values += QString::number(nDataTable->value(j,0));
							lowerBounds += QString::number(nDataTable->value(j,1));
							upperBounds += QString::number(nDataTable->value(j,2));
						}
					}
				}
				if ((type == both || type == text) && itemHandles[i]->hasTextData(this->name))
				{
					sDataTable = &(itemHandles[i]->textDataTable(this->name));
					for (int j=0; j < sDataTable->rows(); ++j)
					{
						if (parentWidget() == dockWidget || !ignoredVarNames.contains(itemHandles[i]->fullName() + tr(".") + sDataTable->rowName(j)))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							if (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();
							names += sDataTable->rowName(j);
							values += (sDataTable->value(j,0));
						}
					}
				}
			}
		}

		tableWidget.setRowCount(names.size());
		if (lowerBounds.isEmpty() || upperBounds.isEmpty())
		{
				tableWidget.setColumnCount(2);
				tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" );
		}
		else
		{
				tableWidget.setColumnCount(4);
				tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" << "min" << "max");
		}
		tableWidget.setVerticalHeaderLabels(headers);

		disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

		for (int i=0; i < tableWidget.rowCount(); ++i)
		{
			tableWidget.setItem(i,0,new QTableWidgetItem(names[i]));
			tableWidget.setItem(i,1,new QTableWidgetItem(values[i]));
			if (!(lowerBounds.isEmpty() || upperBounds.isEmpty()))
			{
				tableWidget.setItem(i,2,new QTableWidgetItem(lowerBounds[i]));
				tableWidget.setItem(i,3,new QTableWidgetItem(upperBounds[i]));
			}
		}

		connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
	}

	void ParametersTool::addAttribute()
	{
		NetworkHandle * win = currentNetwork();
		GraphicsScene * scene = currentScene();
		if (!win || !scene) return;
		QString name;

		int i=0;
		ItemHandle * lastItem = 0;

		if (tableItems.size() < 1)
		{
			if (!scene->selected().isEmpty()) 
				lastItem = getHandle(scene->selected())[0];
			else
			if (scene->localHandle())
				lastItem = scene->localHandle();
			else
			if (scene->globalHandle())
				lastItem = scene->globalHandle();
		}
		else
		{
			lastItem = tableItems[i].first;
			while (!lastItem && (i+1) < tableItems.size()) lastItem = tableItems[++i].first;
		}

		if (lastItem == 0) return;

		disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		disconnect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));

		int n = tableWidget.rowCount();
		tableWidget.insertRow(n);
		
		if (!lastItem->name.isEmpty())
			tableWidget.setVerticalHeaderItem(n, new QTableWidgetItem(lastItem->fullName()));
		else
			tableWidget.setVerticalHeaderItem(n, new QTableWidgetItem(tr("")));

		if (type == both || type == numerical)
		{
			if (lastItem->hasNumericalData(this->name))
			{
				DataTable<qreal> nDat(lastItem->numericalDataTable(this->name));
				i = 0;
				name = tr("k0");
				
				if (!lastItem->name.isEmpty())
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(lastItem->fullName() + tr(".") + name))
						name = tr("k") + QString::number(++i);
				}
				else
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(name))
						name = tr("k") + QString::number(++i);
				}
				tableWidget.setItem(n,1,new QTableWidgetItem(tr("1.0")));
				tableWidget.setItem(n,0,new QTableWidgetItem(name));
				tableItems << QPair<ItemHandle*,int>(lastItem,nDat.rowNames().size());

				nDat.resize(nDat.rows()+1,nDat.columns());
				nDat.value(nDat.rows()-1,0) = 1.0;
				nDat.value(nDat.rows()-1,1) = 1e-8;
				nDat.value(nDat.rows()-1,2) = 100.0;
				nDat.setRowName(nDat.rows()-1,name);

				if (!lastItem->name.isEmpty())
					win->changeData(lastItem->fullName() + tr(".") + name + tr(" added"), lastItem,this->name,&nDat);
				else
					win->changeData(name + tr(" added"), lastItem,this->name,&nDat);
			}
		}

		if (type == both || type == text)
		{
			if (lastItem->hasTextData(this->name))
			{
				DataTable<QString> sDat(lastItem->textDataTable(this->name));

				i = 0;
				name = tr("s0");
				
				if (!lastItem->name.isEmpty())
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(lastItem->fullName() + tr(".") + name))
						name = tr("s") + QString::number(++i);
				}
				else
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(name))
						name = tr("s") + QString::number(++i);
				}

				tableWidget.setItem(n,1,new QTableWidgetItem(tr("1.0")));
				tableWidget.setItem(n,0,new QTableWidgetItem(name));
				tableItems << QPair<ItemHandle*,int>(lastItem,sDat.rowNames().size());

				sDat.resize(sDat.rows()+1,sDat.columns());
				sDat.value(sDat.rows()-1,0) = tr("hello world");
				sDat.setRowName(sDat.rows()-1 , name);

				if (!lastItem->name.isEmpty())
					win->changeData(lastItem->fullName() + tr(".") + name + tr(" added"),lastItem,this->name,&sDat);
				else
					win->changeData(name + tr(" added"),lastItem,this->name,&sDat);
			}
		}

		connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		connect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));
	}

	void ParametersTool::removeSelectedAttributes()
	{
		QList<QTableWidgetItem*> selectedItems = tableWidget.selectedItems();
		NetworkHandle * win = currentNetwork();
		if (!win) return;

		QList<ItemHandle*> handles1, handles2;
		QList<DataTable<qreal>*> nDats;
		QList<DataTable<QString>*> sDats;
		QList<ItemHandle*> allHandles = win->handles();

		for (int i=0; i < selectedItems.size(); ++i)
		{
			int row = selectedItems[i]->row();

			if (row >= tableItems.size()) break;

			ItemHandle * handle = tableItems[row].first;
			int rowNumber = tableItems[row].second;

			if (!handle) continue;

			if ((type == both || type == numerical) && handle->hasNumericalData(this->name))
			{
				DataTable<qreal> & nDat = handle->numericalDataTable(this->name);

				if (rowNumber < nDat.rows())
				{
					QString s = nDat.rowName(rowNumber);
					QRegExp regex1(tr("[^a-zA-Z_\\.]") + handle->fullName() + tr(".") + s);
					QRegExp regex2(tr("^") + handle->fullName() + tr(".") + s);
					
					bool beingUsed = false;
					QString s3;

					for (int j=0; j < allHandles.size(); ++j)
					{
						QStringList textKeys = allHandles[j]->textDataNames();
						for (int k=0; k < textKeys.size(); ++k)
						{
							TextDataTable & textData = allHandles[j]->textDataTable(textKeys[k]);
							for (int k1=0; k1 < textData.rows(); ++k1)
							{
								for (int k2=0; k2 < textData.columns(); ++k2)
									if (textData.value(k1,k2).contains(regex1) || textData.value(k1,k2).contains(regex2))
									{
										s3 = allHandles[j]->fullName() + tr("'s ") + textKeys[k];
										beingUsed = true;
										break;
									}
								if (beingUsed) break;
							}
							if (beingUsed) break;
						}

					}
					
					if (beingUsed)
					{
					    QMessageBox::information(this, tr("Cannot remove"), s + tr(" cannot be removed because it is being used inside ") + s3);	
					}
					else
					{
						DataTable<qreal> * nDat2 = new DataTable<qreal>(nDat);
						nDat2->removeRow(rowNumber);
						nDats << nDat2;
						handles1 << handle;
					}
				}
			}

			if ((type == both || type == text) && handle->hasTextData(this->name))
			{
				DataTable<QString> * sDat = new DataTable<QString>(handle->textDataTable(this->name));

				if (rowNumber < sDat->rows())
				{
					sDat->removeRow(rowNumber);
					sDats << sDat;
					handles2 << handle;
				}
				else
					delete sDat;
			}
		}

		if (nDats.size() > 0)
			win->changeData(tr("selected parameters removed"), handles1,this->name,nDats);

		if (sDats.size() > 0)
			win->changeData(tr("selected text attributes removed"),handles2,this->name,sDats);

		for (int i=0; i < nDats.size(); ++i)
			delete nDats[i];

		for (int i=0; i < sDats.size(); ++i)
			delete sDats[i];
	}

	typedef void (*tc_ParametersTool_Text_api)(
		char* (*getTextData)(long ,const char* ),
		tc_strings (*getAllTextDataNamed)(tc_items, tc_strings),
		void (*setTextData)(long ,const char* ,const char* ));

	typedef void (*tc_ParametersTool_Numeric_api)(
		tc_matrix (*getInitialValues)(tc_items ),
		void (*setInitialValues)(tc_items,tc_matrix),
		tc_matrix (*getParameters)(tc_items ),
		tc_matrix (*getFixedVars)(tc_items),
		tc_matrix (*getFixedAndParameters)(tc_items),
		double (*getNumericalData)(long ,const char* ),
		tc_matrix (*getParametersNamed)(tc_items, tc_strings),
		tc_matrix (*getParametersExcept)(tc_items, tc_strings),
		void (*setNumericalData)(long ,const char* ,double ));

	void ParametersTool::setupFunctionPointers( QLibrary * library )
	{
		if (type == both || type == numerical)
		{
			tc_ParametersTool_Numeric_api f = (tc_ParametersTool_Numeric_api)library->resolve("tc_ParametersTool_Numeric_api");
			if (f)
			{
				//qDebug() << "tc_ParametersTool_Numeric_api resolved";
				f(
					&(_getInitialValues),
					&(_setInitialValues),
					&(_getParameters),
					&(_getFixedVars),
					&(_getFixedAndParameters),
					&(_getNumericalData),
					&(_getParametersNamed),
					&(_getParametersExcept),
					&(_setNumericalData)
					);
			}
		}
		if (type == both || type == text)
		{
			tc_ParametersTool_Text_api f = (tc_ParametersTool_Text_api)library->resolve("tc_ParametersTool_Text_api");
			if (f)
			{
				//qDebug() << "tc_ParametersTool_Text_api resolved";
				f(
					&(_getTextData),
					&(_getAllTextDataNamed),
					&(_setTextData)
					);

			}
		}
	}

	void ParametersTool::connectTCFunctions()
	{
		if (type == both || type == text)
		{
			connect(&fToS,SIGNAL(getTextData(QSemaphore*,QString*,ItemHandle*,const QString&)),this,SLOT(getTextData(QSemaphore*,QString*,ItemHandle*,const QString&)));
			connect(&fToS,SIGNAL(setTextData(QSemaphore*,ItemHandle*,const QString&,const QString&)),this,SLOT(setTextData(QSemaphore*,ItemHandle*,const QString&,const QString&)));
			connect(&fToS,SIGNAL(getAllTextDataNamed(QSemaphore*,QStringList*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getAllTextDataNamed(QSemaphore*,QStringList*,const QList<ItemHandle*>&,const QStringList&)));
		}
		if (type == both || type == numerical)
		{
			connect(&fToS,SIGNAL(getInitialValues(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getInitialValues(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(setInitialValues(QSemaphore*,const QList<ItemHandle*>&,const DataTable<qreal>&)),this,SLOT(setInitialValues(QSemaphore*,const QList<ItemHandle*>&,const DataTable<qreal>&)));
			connect(&fToS,SIGNAL(getParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getFixedVars(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getFixedVars(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getFixedAndParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getFixedAndParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&)),this,SLOT(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&)));
			connect(&fToS,SIGNAL(setNumericalData(QSemaphore*,ItemHandle*,const QString&,qreal)),this,SLOT(setNumericalData(QSemaphore*,ItemHandle*,const QString&,qreal)));
			connect(&fToS,SIGNAL(getParametersNamed(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getParametersNamed(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)));
			connect(&fToS,SIGNAL(getParametersExcept(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getParametersExcept(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)));
		}
	}

	DataTable<qreal> ParametersTool::getParameters(const QList<QGraphicsItem*>& items, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QList<ItemHandle*> handles;

		ItemHandle* handle = 0;

		for (int i=0; i < items.size(); ++i)
		{
			handle = getHandle(items.at(i));
			if (handle && !handles.contains(handle))
				handles += handle;
		}

		return getParameters(handles,mustHave,exclude,sep);
	}

	DataTable<qreal> ParametersTool::getParameters(const QList<ItemHandle*>& handles, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QStringList rownames;
        QList<qreal> values, minv, maxv;
        QString s;
        QString name("Parameters");

        ItemHandle * handle = 0;
        DataTable<qreal> * dataTable = 0;

        for (int i=0; i < handles.size(); ++i)
        {
            handle = handles.at(i);

            if (handle && handle->hasNumericalData(name))
            {
                dataTable = &(handle->numericalDataTable(name));
                if (dataTable && dataTable->columns() > 0)
                    for (int j=0; j < dataTable->rows(); ++j)
                    {
		                if ((mustHave.isEmpty() || mustHave.contains(dataTable->rowName(j).toLower()) || mustHave.contains(dataTable->rowName(j)))
		                    && (exclude.isEmpty() || !(exclude.contains(dataTable->rowName(j).toLower()) || exclude.contains(dataTable->rowName(j)))))
							{
								if (handle->name.isEmpty())
									s = dataTable->rowName(j);
								else							
									s = handle->fullName(sep) + sep + dataTable->rowName(j);

								rownames += s;
								values += dataTable->at(j,0);
								minv += dataTable->at(j,1);
								maxv += dataTable->at(j,2);
							}
		            }
            }
        }

        DataTable<qreal> combinedTable;
        combinedTable.resize(values.size(),1);
        for (int i=0; i < values.size() && i < rownames.size(); ++i)
        {
            combinedTable.setRowName(i,rownames[i]);
            combinedTable.value(i,0) = values[i];
            combinedTable.value(i,1) = minv[i];
            combinedTable.value(i,2) = maxv[i];
        }
        combinedTable.setColumnName(0,tr("values"));
        combinedTable.setColumnName(1,tr("min"));
        combinedTable.setColumnName(2,tr("max"));

        return combinedTable;
	}

	DataTable<QString> ParametersTool::getTextData(const QList<ItemHandle*>& handles, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QStringList rownames;
		QList<QString> values;

		QString name("Text Attributes");

		ItemHandle * handle = 0;
		DataTable<QString> * dataTable = 0;

		for (int i=0; i < handles.size(); ++i)
		{
			handle = handles.at(i);

			if (handle && handle->hasTextData(name))
			{				dataTable = &(handle->textDataTable(name));
			if (dataTable && dataTable->columns() > 0)
				for (int j=0; j < dataTable->rows(); ++j)
				{
					if ((mustHave.isEmpty() || mustHave.contains(dataTable->rowName(j).toLower()) || mustHave.contains(dataTable->rowName(j)))
						&& (exclude.isEmpty() || !(exclude.contains(dataTable->rowName(j).toLower()) || exclude.contains(dataTable->rowName(j)))))						
					{
						if (handle->name.isEmpty())
							rownames += dataTable->rowName(j);
						else
                            rownames += handle->fullName(sep) + sep + dataTable->rowName(j);

                        values += dataTable->at(j,0);
					}
				}
			}
		}

		QRegExp regex(tr("\\.(?!\\d)"));
		for (int i=0; i < values.size(); ++i)
		{
			while (regex.indexIn(values[i]) != -1)
			{
				values[i].replace(regex,sep);
			}
		}

		DataTable<QString> combinedTable;
		combinedTable.resize(values.size(),1);
		for (int i=0; i < values.size() && i < rownames.size(); ++i)
		{
			combinedTable.setRowName(i, rownames[i]);
			combinedTable.value(i,0) = values[i];
		}

		return combinedTable;
	}

	void ParametersTool::getInitialValues(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles)
	{
		if (ptr)
		{
			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;


			for (int i=0; i < handles.size(); ++i)

			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Initial Value")))
				{
					dataTable = &(handle->numericalDataTable(tr("Initial Value")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0)
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = (*ptr).rows();
			(*ptr).resize( rows + names.size(), (*ptr).columns() );

			for (int i=0; i < names.size(); ++i)
			{
				(*ptr).setRowName(rows + i , names[i]);
				(*ptr).value(rows + i, 0) = values[i];
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::setInitialValues(QSemaphore* s,const QList<ItemHandle*>& handles,const DataTable<qreal>& dat)
	{
		ItemHandle * handle = 0;
		DataTable<qreal> * dataTable = 0;

		QStringList names;
		QList<qreal> values;

		if (dat.columns() == 1)
		{
			QList< DataTable<qreal>* > newData;
			QList< ItemHandle* > handles2;
			for (int i=0; i < handles.size() && i < dat.rows(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Initial Value")))
				{
					dataTable = new DataTable<qreal>(handle->numericalDataTable(tr("Initial Value")));
					dataTable->value(0,0) = dat.at(i,0);
					newData << dataTable;
					handles2 << handle;
				}
			}
			if (newData.size() > 0)
			{
				if (currentNetwork())
					currentNetwork()->changeData(tr("Initial values changed"),handles2,tr("Initial Value"),newData);
				for (int i=0; i < newData.size(); ++i)
					if (newData[i])
						delete newData[i];
			}
		}

		if (s)
			s->release();
	}

	void ParametersTool::getFixedVars(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles)
	{
		if (ptr)
		{
			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;

			for (int i=0; i < handles.size(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Fixed")))
				{
					dataTable = &(handle->numericalDataTable(tr("Fixed")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0 && (dataTable->value(0,0) > 0))
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = (*ptr).rows();
			(*ptr).resize( rows + names.size(), (*ptr).columns() );

			for (int i=0; i < names.size(); ++i)
			{
				(*ptr).setRowName(rows + i , names[i]);
				(*ptr).value(rows + i, 0) = values[i];
			}

		}
		if (s)
			s->release();
	}

	void ParametersTool::getFixedAndParameters(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles0)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles = handles0;

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			int i,j;
			QString replaceDot("_");
			//QStringList rates = StoichiometryTool::getRates(handles, replaceDot);
			//params.insertColumn(1,tr("used"));
			DataTable<qreal> params = ParametersTool::getParameters(handles,QStringList(), QStringList(), replaceDot);

			/*bool used = false;
			for (i=0; i < params.rows(); ++i)
			{
				used = false;
				for (j=0; j < rates.size(); ++j)
				{
					if (rates[j].contains(params.rowName(i)))
					{
						used = true;
						break;
					}
				}
				if (used)
					params.value(i,1) = 1.0;
				else
					params.value(i,1) = 0.0;
			}*/

			QRegExp regex(tr("\\.(?!\\d)"));
			QString s1,s2;

			/*for (i=0; i < handles.size(); ++i)
			{
				if (!handles[i])
					continue;

				if (handles[i]->hasTextData(tr("Events")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Events"));
					if (dat.columns() == 1)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);

							s2 =  dat.value(j,0);

							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}

				if (handles[i]->hasTextData(tr("Assignments")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Assignments"));
					if (dat.columns() > 0 && dat.rows() > 0)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);
							s2 =  dat.value(j,0);
							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}

				if (handles[i]->hasTextData(tr("Functions")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Functions"));
					if (dat.columns() == 1)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);
							s2 =  dat.value(j,0);
							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}
			}*/

			int count = params.rows();
			/*for (int i=0; i < params.rows(); ++i)
			{
				if (params.value(i,1) > 0.0) ++count;
			}*/

			DataTable<qreal> params2;
			params2.resize(count,1);
			params2.setColumnName(0,params.columnName(0));

			for (int i=0, j=0; i < params.rows() && j < count; ++i)
			{
				if (params.value(i,1) > 0.0)
				{
					params2.setRowName(j, params.rowName(i));
					params2.value(j,0) = params.value(i,0);
					++j;
				}
			}

			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;

			for (int i=0; i < handles.size(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Fixed")))
				{
					dataTable = &(handle->numericalDataTable(tr("Fixed")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0 && (dataTable->value(0,0) > 0))
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = params2.rows();
			params2.resize( rows + names.size(), 1 );

			for (int i=0; i < names.size(); ++i)
			{
				params2.setRowName(rows + i, names[i]);
				params2.value(rows + i, 0) = values[i];
			}

			(*ptr) = params2;
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParameters(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getUsedParameters(currentNetwork(), handles);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParametersNamed(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getParameters(handles,text);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParametersExcept(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getParameters(handles,QStringList(),text);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getAllTextDataNamed(QSemaphore* s,QStringList* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			DataTable<QString> dat = getTextData(handles,text);
			if (dat.columns() > 0)
			{
				for (int i=0; i < dat.rows(); ++i)
					(*ptr) << dat.at(i,0);
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::getTextData(QSemaphore* s,QString* ptr,ItemHandle* handle,const QString& text)
	{
		if (ptr)
		{
			if (handle && handle->hasTextData(this->name))
			{
				if (handle->textDataTable(name).hasRow(text))
				{
					(*ptr) = handle->textDataTable(name).at(text,0);
				}
				
				QRegExp regex(tr("\\.(?!\\d)"));
				(*ptr).replace(regex,tr("_"));
			}
			else
			{
				(*ptr) = tr("");
			}
		}
		if (s)
			s->release();
	}
	void ParametersTool::getNumericalData(QSemaphore* s,qreal* ptr,ItemHandle* handle,const QString& text)
	{
		if (ptr)
		{
			if (handle && handle->hasNumericalData(name))
			{
				(*ptr) = 0.0;
				if (handle->numericalDataTable(name).hasRow(text))
				{
					(*ptr) = handle->numericalDataTable(name).at(text,0);
				}
			}
			else
			{
				(*ptr) = 0.0;
			}
		}
		if (s)
			s->release();
	}
	void ParametersTool::setTextData(QSemaphore* s,ItemHandle* handle,const QString& text,const QString& value)
	{
		if (handle)
		{
			if (handle->hasTextData(name))
			{
				NetworkHandle * win = mainWindow->currentNetwork();
				if (win)
				{
					DataTable<QString> * newData = new DataTable<QString>(handle->textDataTable(name));
					newData->value(text,0) = value;
					win->changeData(handle->fullName() + tr(".") + text + tr(" = ") + value, handle,name,newData);
					delete newData;
				}
				else
				{
					handle->textDataTable(name).value(text,0) = value;
				}
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::setNumericalData(QSemaphore* s,ItemHandle* handle,const QString& text,qreal value)
	{
		if (handle)
		{
			if (handle->hasNumericalData(name))
			{
				NetworkHandle * win = mainWindow->currentNetwork();
				if (win)
				{
					DataTable<qreal> * newData = new DataTable<qreal>(handle->numericalDataTable(name));
					newData->value(text,0) = value;
					win->changeData(handle->fullName() + tr(".") + text + tr(" = ") + QString::number(value), handle,this->name,newData);
					delete newData;
				}
				else
				{
					handle->numericalDataTable(name).value(text,0) = value;
				}
			}
		}
		if (s)
			s->release();
	}

	/********************************************************/

	ParametersTool_FToS ParametersTool::fToS;

	tc_matrix ParametersTool::_getParameters(tc_items A)
	{
		return fToS.getParameters(A);
	}

	tc_matrix ParametersTool::_getInitialValues(tc_items A)
	{
		return fToS.getInitialValues(A);
	}

	void ParametersTool::_setInitialValues(tc_items A, tc_matrix M)
	{
		fToS.setInitialValues(A,M);
	}

	tc_matrix ParametersTool::_getFixedVars(tc_items A)
	{
		return fToS.getFixedVars(A);
	}

	tc_matrix ParametersTool::_getFixedAndParameters(tc_items A)
	{
		return fToS.getFixedAndParameters(A);
	}

	char* ParametersTool::_getTextData(long o,const char* c)
	{
		return fToS.getTextData(o,c);
	}

	double ParametersTool::_getNumericalData(long o,const char* c)
	{
		return fToS.getNumericalData(o,c);
	}

	tc_matrix ParametersTool::_getParametersNamed(tc_items A, tc_strings c)
	{
		return fToS.getParametersNamed(A,c);
	}

	tc_matrix ParametersTool::_getParametersExcept(tc_items A, tc_strings c)
	{
		return fToS.getParametersExcept(A,c);
	}

	tc_strings ParametersTool::_getAllTextDataNamed(tc_items A, tc_strings c)
	{
		return fToS.getAllTextDataNamed(A,c);
	}

	void ParametersTool::_setTextData(long o,const char* a,const char* b)
	{
		return fToS.setTextData(o,a,b);
	}

	void ParametersTool::_setNumericalData(long o,const char* a,double b)
	{
		return fToS.setNumericalData(o,a,b);
	}

	tc_matrix ParametersTool_FToS::getParameters(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParameters(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getInitialValues(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getInitialValues(s,p,*list);
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

	void ParametersTool_FToS::setInitialValues(tc_items a0, tc_matrix M)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = ConvertValue(M);
		s->acquire();


		emit setInitialValues(s,*list,*p);
		s->acquire();
		s->release();
		delete s;
		delete list;
		if (p)
		{
			delete p;
		}
	}

	tc_matrix ParametersTool_FToS::getFixedVars(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getFixedVars(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getFixedAndParameters(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getFixedAndParameters(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getParametersNamed(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParametersNamed(s,p,*list,ConvertValue(name));
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

	tc_matrix ParametersTool_FToS::getParametersExcept(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParametersExcept(s,p,*list,ConvertValue(name));
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

	tc_strings ParametersTool_FToS::getAllTextDataNamed(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getAllTextDataNamed(s,&p,*list,ConvertValue(name));
		s->acquire();
		s->release();
		delete s;
		delete list;
		return ConvertValue(p);
	}

	char* ParametersTool_FToS::getTextData(long a0,const char* a1)
	{
		QSemaphore * s = new QSemaphore(1);
		QString p;
		s->acquire();
		emit getTextData(s,&p,ConvertValue(a0),ConvertValue(a1));
		s->acquire();
		s->release();
		delete s;
		return (char*)ConvertValue(p);
	}

	double ParametersTool_FToS::getNumericalData(long a0,const char* a1)
	{
		QSemaphore * s = new QSemaphore(1);
		qreal p;
		s->acquire();
		emit getNumericalData(s,&p,ConvertValue(a0),ConvertValue(a1));
		s->acquire();
		s->release();
		delete s;
		return (double)p;
	}

	void ParametersTool_FToS::setTextData(long a0,const char* a1,const char* a2)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setTextData(s,ConvertValue(a0),ConvertValue(a1),ConvertValue(a2));
		s->acquire();
		s->release();
		delete s;
	}

	void ParametersTool_FToS::setNumericalData(long a0,const char* a1,double a2)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setNumericalData(s,ConvertValue(a0),a1,a2);
		s->acquire();
		s->release();
		delete s;
	}

	void ParametersTool::keyPressEvent ( QKeyEvent * event )
	{
		if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
		{
			removeSelectedAttributes();
		}
	}
	
	DataTable<qreal> ParametersTool::getUsedParameters(NetworkHandle * network, QList<ItemHandle*>& handles, const QString& replaceDot)
	{
		int i,j;
		DataTable<qreal> params = ParametersTool::getParameters(handles,QStringList(), QStringList(), replaceDot);
		
		params.insertColumn(3,tr("used"));

		QRegExp regex(tr("\\.(?!\\d)"));
		QString s1,s2;

		QList<ItemHandle*> allhandles;
		if (network)
			allhandles = network->handles();
		else
			allhandles = handles;

		for (i=0; i < allhandles.size(); ++i)
		{
			if (!allhandles[i])
				continue;

			if (allhandles[i]->children.isEmpty() && allhandles[i]->hasTextData(tr("Rate equations")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Rate equations"));
				if (dat.columns() == 1)
				{
					for (j=0; j < dat.rows(); ++j)
					{
						s2 =  dat.value(j,0);
						s2.replace(regex,replaceDot);
						
						if (s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
				}
			}

			if (allhandles[i]->hasTextData(tr("Events")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Events"));
				if (dat.columns() == 1)
					for (j=0; j < dat.rows(); ++j)
					{
						s1 =  dat.rowName(j);

						s1.replace(regex,replaceDot);
						s2 =  dat.value(j,0);

						s2.replace(regex,replaceDot);

						if (s1.isEmpty() || s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
			}

			if (allhandles[i]->hasTextData(tr("Assignments")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Assignments"));
				if (dat.columns() > 0 && dat.rows() > 0)
					for (j=0; j < dat.rows(); ++j)
					{
						s1 =  dat.rowName(j);
						s1.replace(regex,replaceDot);
						s2 =  dat.value(j,0);
						s2.replace(regex,replaceDot);

						if (s1.isEmpty() || s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
			}
		}
		
		int count = 0;
		for (int i=0; i < params.rows(); ++i)
		{
			if (params.value(i,3) > 0.0) ++count;
		}

		DataTable<qreal> params2;
		params2.resize(count,3);
		params2.setColumnName(0,params.columnName(0));
		params2.setColumnName(1,params.columnName(1));
		params2.setColumnName(2,params.columnName(2));

		for (int i=0, j=0; i < params.rows() && j < count; ++i)
		{
			if (params.value(i,3) > 0.0)
			{
				params2.setRowName(j , params.rowName(i));
				params2.value(j,0) = params.value(i,0);
				params2.value(j,1) = params.value(i,1);
				params2.value(j,2) = params.value(i,2);
				++j;
			}
		}
		return params2;
	}
	
	QString ParametersTool::removeParameterFromModel(NetworkHandle * win, ItemHandle * handle, const QString& s)
	{
		if (!handle) return QString("No item");		
		if (!handle->hasNumericalData("Parameters")) return QString("No parameters inside this item");
		
		NumericalDataTable nDat(handle->numericalDataTable("Parameters"));
		if (!nDat.hasRow(s)) return QString("No such parameter inside this item");
		
		bool beingUsed = false;
		QRegExp regex1, regex2;
		
		if (handle->name.isEmpty())
		{
			regex1 = QRegExp(QObject::tr("[^a-zA-Z_\\.]") + s);
			regex2 = QRegExp(QObject::tr("^") + s);
		}
		else
		{
			regex1 = QRegExp(QObject::tr("[^a-zA-Z_\\.]") + handle->fullName() + tr(".") + s);
			regex2 = QRegExp(QObject::tr("^") + handle->fullName() + tr(".") + s);
		}
		
		QString s3;
						
		QList<ItemHandle*> allHandles = win->handles();
		for (int i=0; i < allHandles.size(); ++i)
		{
			QStringList textKeys = allHandles[i]->textDataNames();
			for (int j=0; j < textKeys.size(); ++j)
			{
				TextDataTable & textData = allHandles[i]->textDataTable(textKeys[j]);
				for (int k1=0; k1 < textData.rows(); ++k1)
				{
					for (int k2=0; k2 < textData.columns(); ++k2)
						if (textData.value(k1,k2).contains(regex1) || textData.value(k1,k2).contains(regex2))
						{
							s3 = allHandles[i]->fullName() + tr("'s ") + textKeys[j];
							beingUsed = true;
							break;
						}
					if (beingUsed) break;
				}
				if (beingUsed) break;
			}
		}

		if (beingUsed)
		{
			return s + tr(" cannot be removed because it is being used inside ") + s3;
		}
		else
		{
			nDat.removeRow(s);
			if (handle->name.isEmpty())
				win->changeData(s + QObject::tr(" removed"),handle, QObject::tr("Parameters"), &nDat);
			else
				win->changeData(handle->fullName() + QObject::tr(".") + s + QObject::tr(" removed"),handle, QObject::tr("Parameters"), &nDat);
		}
		return QString();
	}
	
	QString ParametersTool::removeUnusedParametersInModel(NetworkHandle * win)
	{
		return QString();

		QString allFormulas;
						
		QList<ItemHandle*> allHandles = win->handles();
		for (int i=0; i < allHandles.size(); ++i)
		{
			QStringList textKeys = allHandles[i]->textDataNames();
			for (int j=0; j < textKeys.size(); ++j)
			{
				TextDataTable & textData = allHandles[i]->textDataTable(textKeys[j]);
				for (int k1=0; k1 < textData.rows(); ++k1)
				{
					for (int k2=0; k2 < textData.columns(); ++k2)
					{
						allFormulas += textData.value(k1,k2);
						allFormulas += QObject::tr(";");
					}
				}
			}
		}
		
		QList<ItemHandle*> changedHandles, handles = win->handles();
		QList<NumericalDataTable*> oldData, newData;
		
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->hasNumericalData(QObject::tr("Parameters")))
			{
				QString s;
				if (!handles[i]->name.isEmpty())
					s = handles[i]->fullName() + QObject::tr(".");
				bool changes = false;
				NumericalDataTable * oldDat = &(handles[i]->numericalDataTable(QObject::tr("Parameters")));
				NumericalDataTable * newDat = new NumericalDataTable(*oldDat);

				for (int j=0; j < newDat->rows(); ++j)
				{
					if (!allFormulas.contains(s + newDat->rowName(j)))
					{
						changes = true;
						newDat->removeRow(j);
						--j;
					}
				}
				if (changes)
				{
					changedHandles << handles[i];
					oldData << oldDat;
					newData << newDat;
				}
			}
		}
		
		if (!oldData.isEmpty())
		{
			win->changeData(QObject::tr("Unused parameters removed"), changedHandles, oldData, newData);
			for (int i=0; i < newData.size(); ++i)
				delete newData[i];
		}
		
	}
}

=======
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- "Parameters" and "Text Attributes".
Attributes are essentially <name,value> pairs (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();rs that are used to characterize an item.

The ParametersTool contains two tools, one for text attributes and one
for parameters. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include "NetworkHandle.h"
#include "GraphicsScene.h"
#include "UndoCommands.h"
#include "ConsoleWindow.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "NodeGraphicsReader.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ParametersTool.h"
#include "NodesTree.h"
#include "GlobalSettings.h"

namespace Tinkercell
{	
	QHash<QString,double> ParametersTool::initialValues;
	bool ParametersTool::EnforceDefaultParameters=false;

	void ParametersTool::select(int)
	{
		GraphicsScene * scene = currentScene();
		if (!scene) return;

		itemHandles = getHandle(scene->selected(),false);
		if (itemHandles.isEmpty())
				if (scene->localHandle())
					itemHandles += scene->localHandle();
				else
					if (scene->globalHandle())
						itemHandles += scene->globalHandle();

		if (itemHandles.isEmpty()) return;

		if (dockWidget && dockWidget->widget() != this)
			dockWidget->setWidget(this);

		openedByUser = true;
		updateTable();
		closeButton->setVisible(true);

		if (dockWidget != 0)
		{
			if (dockWidget->isVisible())
				openedByUser = false;
			else
				dockWidget->show();
		}
		else
		{
			if (isVisible())
				openedByUser = false;
			else
				show();
		}
		this->setFocus();
	}

	void ParametersTool::deselect(int)
	{
		/*if (openedByUser && (!dockWidget || dockWidget->isFloating()))
		{
			openedByUser = false;

			if (dockWidget != 0)
				dockWidget->hide();
			else
				hide();
		}*/
	}

	void ParametersTool::loadInitialValues()
	{
		Tool * tool = mainWindow->tool(tr("Nodes Tree"));
		
		if (!tool) return;
		
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);
		settings.beginGroup("ParametersTool");
		
		NodesTree * nodesTree = static_cast<NodesTree*>(tool);

		QStringList families(nodesTree->getAllFamilyNames());
		QStringList keys, rownames;
		NodeFamily * family;
		QString key;
		
		for (int i=0; i < families.size(); ++i)
		{
			family = nodesTree->getFamily( families[i] );
			if (family && !family->measurementUnit.property.isEmpty())
			{
				key = family->measurementUnit.property;
				initialValues[key] = 1.0;
			}
		}
		
		keys = initialValues.keys();		
		QStringList names = settings.value(tr("InitialValueNames"),QStringList()).toStringList();
		QStringList values = settings.value(tr("InitialValues"),QStringList()).toStringList();
			
		bool ok;

		for (int i=0; i < names.size() && i < values.size(); ++i)
		{
			double d = values[i].toDouble(&ok);
			if (ok && !names[i].isEmpty())
				initialValues[ names[i] ] = d;
		}

		settings.endGroup();
	}

	bool ParametersTool::setMainWindow(MainWindow * main)
	{
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

		Tool::setMainWindow(main);

		if (mainWindow)
		{
			loadInitialValues();
			
			connect(this,SIGNAL(dataChanged(const QList<ItemHandle*>&)),
					mainWindow,SIGNAL(dataChanged(const QList<ItemHandle*>&)));
			
			connect(mainWindow,SIGNAL(networkClosing(NetworkHandle * , bool *)),this,SLOT(windowClosing(NetworkHandle * , bool *)));

			connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle *, const QList<ItemHandle*>&)),
				this,SLOT(itemsInserted(NetworkHandle *, const QList<ItemHandle*>&)));

			connect(mainWindow,SIGNAL(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,SLOT(itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)));

			connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));

			connect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));

			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(pluginLoaded(Tool*)));

			setWindowTitle(name);
			setWindowFlags(Qt::Dialog);
			dockWidget = mainWindow->addToolWindow(this,MainWindow::DockWidget,Qt::BottomDockWidgetArea,Qt::NoDockWidgetArea);

			if (dockWidget)
			{
				if (type == text)
					dockWidget->move(mainWindow->geometry().bottomRight() - QPoint(sizeHint().width()*2,sizeHint().height()*3));
				else
					dockWidget->move(mainWindow->geometry().bottomRight() - QPoint(sizeHint().width()*2,sizeHint().height()*2));

				dockWidget->setWindowFlags(Qt::Dialog);
				dockWidget->setAttribute(Qt::WA_ContentsPropagated);
				dockWidget->setPalette(QPalette(QColor(255,255,255,255)));
				dockWidget->setAutoFillBackground(true);
				if (closeButton)
					connect(closeButton,SIGNAL(pressed()),dockWidget,SLOT(hide()));
				//dockWidget->setWindowOpacity(0.8);
	
				QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

				if (type == both || type == numerical)
				{
					settings.beginGroup("ParametersTool");
					//dockWidget->resize(settings.value("size", sizeHint()).toSize());
					//dockWidget->move(settings.value("pos", dockWidget->pos()).toPoint());
				}
				else
				{
					settings.beginGroup("ParametersToolText");
					//dockWidget->resize(settings.value("size", sizeHint()).toSize());
					//dockWidget->move(settings.value("pos", dockWidget->pos()).toPoint());
				}
				
				settings.endGroup();
				dockWidget->hide();
				dockWidget->setFloating(true);
			}

			pluginLoaded(0);
		}
		return (mainWindow != 0);
	}

	void ParametersTool::pluginLoaded(Tool*)
	{
		static bool connected = false;
		if (connected) return;

		if (!connected && type != text && mainWindow && mainWindow->tool(tr("Model Summary")))
		{
			QWidget * modelSummary = mainWindow->tool(tr("Model Summary"));

			connect(modelSummary,SIGNAL(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)),
				this,SLOT(displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&)));
			connected = true;
		}
	}

	void ParametersTool::windowClosing(NetworkHandle * , bool *)
	{
		QSettings settings(GlobalSettings::ORGANIZATIONNAME, GlobalSettings::ORGANIZATIONNAME);

		if (dockWidget)
		{
			if (type == text)
			{
				settings.beginGroup("ParametersToolText");
			}
			else
			{
				settings.beginGroup("ParametersTool");
			}
			//settings.setValue("floating", dockWidget && dockWidget->isFloating());
			settings.setValue("size", dockWidget->size());
			settings.setValue("pos", dockWidget->pos());
			
			QStringList values;
			QStringList names = initialValues.keys();
			QList<double> doubles = initialValues.values();
			
			for (int i=0; i < doubles.size(); ++i)	values << QString::number(doubles[i]);
			
			settings.setValue(tr("InitialValueNames"),names);
			settings.setValue(tr("InitialValues"),values);
			
			settings.endGroup();
		}
	}

	void ParametersTool::aboutToDisplayModel(const QList<ItemHandle*>& , QHash<QString,qreal>& , QHash<QString,QString>& )
	{
	}

	void ParametersTool::displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equationsList)
	{
		if (type == text) return;

		itemHandles.clear();
		tableItems.clear();
		tableWidget.clear();
		
		//QStringList eqns(equationsList.values());
		//QString equations = QStringList(equationsList.values()).join(" ");
		closeButton->setVisible(false);

		for (int i=0; i < items.size(); ++i)
		{
			if (!itemHandles.contains(items[i]) && items[i])
				itemHandles += items[i];
		}

		QStringList names, values, min, max;
		QStringList headers;
		ignoredVarNames.clear();

		DataTable<qreal> * nDataTable = 0;

		for (int i=0; i < itemHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			//bool remove = true;
			if (itemHandles[i] != 0)
			{
				if (itemHandles[i]->hasNumericalData(this->name))
				{
					nDataTable = &(itemHandles[i]->numericalDataTable(this->name));
					for (int j=0; j < nDataTable->rows(); ++j)
					{
						QString str = itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j);
						//if (equations.isEmpty() || equations.contains(str))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							headers << (itemHandles[i]->fullName() + tr("."));
							names += nDataTable->rowName(j);
							values += QString::number(nDataTable->value(j,0));
							min += QString::number(nDataTable->value(j,1));
							max += QString::number(nDataTable->value(j,2));
							constants.insert(str,nDataTable->value(j,0));
							//remove = false;
						}
						/*else
						{
							ignoredVarNames << (itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j));
						}*/
					}
				}
			}
			/*if (remove)
			{
				itemHandles.removeAt(i);
				--i;
			}*/
		}

		if (names.size() > 0)
		{
			if (dockWidget && dockWidget->isVisible())
				dockWidget->hide();

			widgets.insertTab(0,this,tr("Parameters"));

			tableWidget.clear();
			tableWidget.setRowCount(names.size());
			tableWidget.setColumnCount(4);
			tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" << "min" << "max");
			tableWidget.setVerticalHeaderLabels(headers);

			disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

			for (int i=0; i < tableWidget.rowCount(); ++i)
			{
				tableWidget.setItem(i,0,new QTableWidgetItem(names[i]));
				tableWidget.setItem(i,1,new QTableWidgetItem(values[i]));
				tableWidget.setItem(i,2,new QTableWidgetItem(min[i]));
				tableWidget.setItem(i,3,new QTableWidgetItem(max[i]));
			}

			connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		}
		else
			if (dockWidget && dockWidget->widget() != this)
				dockWidget->setWidget(this);
	}

	void ParametersTool::historyUpdate(int i)
	{
		if (parentWidget() && parentWidget()->isVisible())
			updateTable();
		NetworkHandle * win = currentNetwork();
		if (mainWindow && win && mainWindow->statusBar())
			mainWindow->statusBar()->showMessage(win->history.text(i-1));
	}

	void ParametersTool::itemsInserted(NetworkHandle * , const QList<ItemHandle*>& handles)
	{
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->family() && !handles[i]->tools.contains(this))
			{
				handles[i]->tools += this;	
			}

			if (handles[i] && handles[i]->family())
			{
				insertDataMatrix(handles[i]);
			}
		}
	}

	void ParametersTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& list, QPointF , Qt::KeyboardModifiers )
	{
		if (scene && (parentWidget() && parentWidget() != mainWindow && parentWidget()->isVisible()))
		{
			itemHandles.clear();
			ItemHandle * handle = 0;
			for (int i=0; i < list.size(); ++i)
			{
				if ((handle = getHandle(list[i])))
					itemHandles += handle;
			}

			if (itemHandles.isEmpty())
				if (scene->localHandle())
					itemHandles += scene->localHandle();
				else
					if (scene->globalHandle())
						itemHandles += scene->globalHandle();
			
			updateTable();
		}
	}

	void ParametersTool::setValue(int row,int col)
	{
		static bool recursive = false;

		if (recursive)
		{
			recursive = false;
			return;
		}
		recursive = false;

		NetworkHandle * win = currentNetwork();
		if (!win) return;

		if (col > 3  || row >= tableItems.size() || !tableWidget.item(row,col)) return;

		ItemHandle * handle = tableItems[row].first;
		int rowNumber = tableItems[row].second;

		if (!handle) return;

		if ((type == both || type == numerical) && handle->hasNumericalData(this->name))
		{
			DataTable<qreal> nDat(handle->numericalDataTable(this->name));

			if (col == 0)
			{
				if (rowNumber < nDat.rows())
				{
						QString name = tableWidget.item(row,col)->text();
						name.replace(QRegExp(tr("[^a-zA-Z_0-9]")),tr(""));
						if (name.isEmpty())
						{
							QString s = nDat.rowName(rowNumber);
							QString err = removeParameterFromModel(win, handle, s);
							if (!err.isEmpty())
							{
								QMessageBox::information(this,tr("Cannot remove"), err);
								recursive = true;
								tableWidget.item(row,col)->setText(s);
								return;
							}
						}
						else
						{
							QString oldname = nDat.rowName(rowNumber);
							if (handle->name.isEmpty())
								win->rename(oldname, name);
							else
								win->rename(handle->fullName() + tr(".") + oldname, handle->fullName() + tr(".") + name);
						}
				}
			}
			else
			{
				bool ok;
				double value = tableWidget.item(row,col)->text().toDouble(&ok);
				if (!ok)
				{
					recursive = true;
					tableWidget.item(row,col)->setText( QString::number(nDat.value(rowNumber,col-1)) );
					return;
				}
				nDat.value(rowNumber,col-1) = value;
				if (handle->name.isEmpty())
					win->changeData(nDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&nDat);
				else
					win->changeData(handle->fullName() + tr(".") + nDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&nDat);
			}
		}

		if ((type == both || type == text) && handle->hasTextData(this->name))
		{
			DataTable<QString> sDat(handle->textDataTable(this->name));

			if (col == 0)
			{
				if (rowNumber < sDat.rows())
				{
					if (handle->family() && handle->family()->textAttributes.contains(sDat.rowName(rowNumber)))
					{
						recursive = true;
						tableWidget.item(row,col)->setText(sDat.rowName(rowNumber));
						if (console())
                            console()->message(sDat.rowName(rowNumber) + tr(" cannot be removed because it is a family attribute"));
					}
					else
					{
						QString name = tableWidget.item(row,col)->text();
						name.replace(QRegExp(tr("[^a-zA-Z_]")),tr(""));
						if (name.isEmpty())
						{
							QString s = sDat.rowName(rowNumber);
							sDat.removeRow(rowNumber);
							if (handle->name.isEmpty())
								win->changeData(s + tr(" removed"),handle, this->name,&sDat);
							else
								win->changeData(handle->fullName() + tr(".") + s + tr(" removed"),handle, this->name,&sDat);
						}
						else
						{
							QString oldname = sDat.rowName(rowNumber);
							if (handle->name.isEmpty())
								win->rename(oldname, name);
							else
								win->rename(handle->fullName() + tr(".") + oldname, handle->fullName() + tr(".") + name);
						}
					}
				}
			}
			else
			if (col == 1)
			{
				QString value = tableWidget.item(row,col)->text();
				sDat.value(rowNumber,0) = value;
				if (handle->name.isEmpty())
					win->changeData(sDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&sDat);
				else
					win->changeData(handle->fullName() + tr(".") + sDat.rowName(rowNumber) + tr(" changed"),handle,this->name,&sDat);
			}
		}
	}

	ParametersTool::ParametersTool(const QString& typ) : Tool(tr("Attributes"),tr("Modeling")), delegate(QStringList())
	{
		QString appDir = QCoreApplication::applicationDirPath();
		delegate.textColumn = 0;
		if (typ == tr("both"))
		{
			type = ParametersTool::both;
			groupBox = new QGroupBox(tr(" Attributes "),this);
		}
		else
			if (typ == tr("text"))
			{
				type = ParametersTool::text;
				name = tr("Text Attributes");
				groupBox = new QGroupBox(tr(" Strings "),this);
			}
			else
			{
				type = ParametersTool::numerical;
				name = tr("Parameters");
				groupBox = new QGroupBox(tr(" Constants "),this);
			}

			NodeGraphicsReader reader;
			ToolGraphicsItem * toolGraphicsItem = new ToolGraphicsItem(this);
			addGraphicsItem(toolGraphicsItem);
			
			if (type == ParametersTool::both)
			{
				reader.readXml(&item,tr(":/images/textsheet.xml"));
				item.setToolTip(tr("Attributes"));
				toolGraphicsItem->setToolTip(tr("Attributes"));
			}
			else
				if (type == ParametersTool::numerical)
				{
					reader.readXml(&item,tr(":/images/datasheet.xml"));
					item.setToolTip(tr("Parameters"));
					toolGraphicsItem->setToolTip(tr("Parameters"));
				}
				else
				{
					reader.readXml(&item,tr(":/images/textsheet.xml"));
					item.setToolTip(tr("Text attributes"));
					toolGraphicsItem->setToolTip(tr("Text attributes"));
				}

			item.normalize();
			item.scale(30.0/item.sceneBoundingRect().width(),40.0/item.sceneBoundingRect().height());
			toolGraphicsItem->addToGroup(&item);

			openedByUser = false;

			tableWidget.setEditTriggers ( QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed );
			connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

			QFont font = this->font();
			font.setPointSize(12);
			tableWidget.setFont(font);

			QHBoxLayout * actionsLayout = new QHBoxLayout;

			QToolButton * addAttribAction = new QToolButton(this);
			addAttribAction->setIcon(QIcon(":/images/plus.png"));

			QToolButton * removeAttribAction = new QToolButton(this);
			removeAttribAction->setIcon(QIcon(":/images/minus.png"));

			closeButton = new QPushButton("Done", this);
			
			QString message;
			if (type == ParametersTool::numerical)
			{
				addAttribAction->setToolTip(tr("Add constant"));
				removeAttribAction->setToolTip(tr("Remove constant"));
				//tableWidget.setItemDelegate(&delegate);
				//tableWidget.setFixedWidth(160);
				groupBox->setMinimumWidth(100);
				message = tr("This table shows the set of parameters belonging with the selected objects. Parameters can be used inside rate equations or other equations. For example, if the name of the parameter is k0 and it belongs with an object name J0, then you can use J0.k0 inside questions in order to address this value. You may change the name or value of the parameter using this table. Changing the name will automatically update all the equations where it is used.");
			}
			else
				if (type == ParametersTool::text)
				{
					addAttribAction->setToolTip(tr("Add string"));
					removeAttribAction->setToolTip(tr("Remove string"));
					message = tr("This table shows the text attributes belonging with the selected objects. Text attributes are any properties of an object that is represented as a string. Examples include DNA sequence, database IDs, etc.");
				}
				else
				{
					addAttribAction->setToolTip(tr("Add attribute"));
					removeAttribAction->setToolTip(tr("Remove attribute"));
					message = tr("This table shows the set of parameters belonging with the selected objects. Parameters can be used inside rate equations or other equations. For example, if the name of the parameter is k0 and it belongs with an object name J0, then you can use J0.k0 inside questions in order to address this value. You may change the name or value of the parameter using this table. Changing the name will automatically update all the equations where it is used.");
					message += tr("\nThis table also shows the text attributes belonging with the selected objects. Text attributes are any properties of an object that is represented as a string. Examples include DNA sequence, database IDs, etc.");
				}

		connect(addAttribAction,SIGNAL(pressed()),this,SLOT(addAttribute()));
		connect(removeAttribAction,SIGNAL(pressed()),this,SLOT(removeSelectedAttributes()));
		
		QToolButton * question = new QToolButton(this);
		question->setIcon(QIcon(":/images/question.png"));

		QMessageBox * messageBox = new QMessageBox(QMessageBox::Information,tr("About Attributes Table"),message,QMessageBox::StandardButtons(QMessageBox::Close), const_cast<QWidget*>((QWidget*)this), Qt::WindowFlags (Qt::Dialog));
		connect(question,SIGNAL(pressed()),messageBox,SLOT(exec()));

		actionsLayout->addWidget(addAttribAction);
		actionsLayout->addWidget(removeAttribAction);
		actionsLayout->addStretch(1);
		actionsLayout->addWidget(closeButton);
		actionsLayout->addStretch(1);
		actionsLayout->addWidget(question);

		QVBoxLayout * boxLayout = new QVBoxLayout;
		boxLayout->addWidget(&tableWidget,1);

		boxLayout->addLayout(actionsLayout);
		groupBox->setLayout(boxLayout);

		dockWidget = 0;

		QVBoxLayout * layout = new QVBoxLayout;
		layout->addWidget(groupBox);
		setLayout(layout);

		connectTCFunctions();
	}

	QSize ParametersTool::sizeHint() const
	{
		return QSize(300, 200);
	}


	void ParametersTool::insertDataMatrix(ItemHandle * handle)
	{
		if (handle == 0 || handle->family() == 0) return;

		QStringList columnNames;
		
		if (type == ParametersTool::text)
			columnNames << "value";
		else
			columnNames << "value" << "min" << "max";

		ItemFamily* family = handle->family();

		if (!family->measurementUnit.property.isEmpty() && !handle->hasNumericalData(QString("Initial Value")))
		{
			DataTable<qreal> table;
			table.resize(1,1);

			table.setRowName(0, family->measurementUnit.property);

			table.setColumnName(0,family->measurementUnit.property);
			table.value(0,0) = 0.0;
			
			if (initialValues.contains(family->measurementUnit.property))
			{
				table.value(0,0) = initialValues[family->measurementUnit.property];
			}
			
			table.description() = tr("Initial value: stores measurement value of an item. See each family's measurement unit for detail.");

			handle->numericalDataTable(QString("Initial Value")) = table;
		}

		if ((type == both || type == numerical) && !(handle->hasNumericalData(name)))
		{
			QStringList nKeys;
			if (ParametersTool::EnforceDefaultParameters)
				nKeys = family->numericalAttributes.keys();
				
			DataTable<qreal> numericalAttributes;
			numericalAttributes.resize(nKeys.size(),3);
			numericalAttributes.description() = tr("Parameters: an Nx3 table storing all the real attributes for this item. Row names are the attribute names. First column holds the values. Second and third columns hold the upper and lower bounds.");

			for (int i=0; i < nKeys.size(); ++i)
			{
				numericalAttributes.value(i,0) = family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.value(i,1) = 0.1 * family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.value(i,2) = 10.0 * family->numericalAttributes.value(nKeys[i]);
				numericalAttributes.setRowName(i, nKeys[i]);
			}

			numericalAttributes.setColumnNames(columnNames);
			handle->numericalDataTable(this->name) = numericalAttributes;
		}

		if ((type == both || type == text) && !(handle->hasTextData(name)))
		{
			QList<QString> sKeys = family->textAttributes.keys();
			//if (ParametersTool::EnforceDefaultParameters)
				//sKeys = family->textAttributes.keys();

			DataTable<QString> textAttributes;
			textAttributes.description() = tr("Text Attributes: an Nx1 table storing all the string attributes for this item. Row names are the attribute names, and first column holds the values.");
			textAttributes.resize(sKeys.size(),1);

			for (int i=0; i < textAttributes.rows() && i < sKeys.size(); ++i)
			{
				textAttributes.value(i,0) = family->textAttributes.value(sKeys[i]);
				textAttributes.setRowName(i, sKeys[i]);
			}

			textAttributes.setColumnNames(columnNames);
			handle->textDataTable(this->name) = textAttributes;
		}
	}

	void ParametersTool::updateTable()
	{
		tableItems.clear();
		tableWidget.clear();

		if (itemHandles.size() < 1)
		{
			tableWidget.clearContents();
			tableWidget.setRowCount(0);
			return;
		}

		QStringList names, values, lowerBounds, upperBounds;
		QStringList headers;

		DataTable<qreal> * nDataTable = 0;
		DataTable<QString> * sDataTable = 0;

		for (int i=0; i < itemHandles.size(); ++i) //build combined matrix for all selected reactions
		{
			if (itemHandles[i] != 0)
			{
				if ((type == both || type == numerical) && itemHandles[i]->hasNumericalData(this->name))
				{
					nDataTable = &(itemHandles[i]->numericalDataTable(this->name));
					for (int j=0; j < nDataTable->rows(); ++j)
					{
						if (parentWidget() == dockWidget || !ignoredVarNames.contains(itemHandles[i]->fullName() + tr(".") + nDataTable->rowName(j)))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							
							if (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();
							names += nDataTable->rowName(j);
							values += QString::number(nDataTable->value(j,0));
							lowerBounds += QString::number(nDataTable->value(j,1));
							upperBounds += QString::number(nDataTable->value(j,2));
						}
					}
				}
				if ((type == both || type == text) && itemHandles[i]->hasTextData(this->name))
				{
					sDataTable = &(itemHandles[i]->textDataTable(this->name));
					for (int j=0; j < sDataTable->rows(); ++j)
					{
						if (parentWidget() == dockWidget || !ignoredVarNames.contains(itemHandles[i]->fullName() + tr(".") + sDataTable->rowName(j)))
						{
							tableItems << QPair<ItemHandle*,int>(itemHandles[i],j);
							if (!itemHandles[i]->name.isEmpty())
								headers << (itemHandles[i]->fullName() + tr("."));
							else
								headers << QString();
							names += sDataTable->rowName(j);
							values += (sDataTable->value(j,0));
						}
					}
				}
			}
		}

		tableWidget.setRowCount(names.size());
		if (lowerBounds.isEmpty() || upperBounds.isEmpty())
		{
				tableWidget.setColumnCount(2);
				tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" );
		}
		else
		{
				tableWidget.setColumnCount(4);
				tableWidget.setHorizontalHeaderLabels(QStringList() << "variable" << "value" << "min" << "max");
		}
		tableWidget.setVerticalHeaderLabels(headers);

		disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));

		for (int i=0; i < tableWidget.rowCount(); ++i)
		{
			tableWidget.setItem(i,0,new QTableWidgetItem(names[i]));
			tableWidget.setItem(i,1,new QTableWidgetItem(values[i]));
			if (!(lowerBounds.isEmpty() || upperBounds.isEmpty()))
			{
				tableWidget.setItem(i,2,new QTableWidgetItem(lowerBounds[i]));
				tableWidget.setItem(i,3,new QTableWidgetItem(upperBounds[i]));
			}
		}

		connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
	}

	void ParametersTool::addAttribute()
	{
		NetworkHandle * win = currentNetwork();
		GraphicsScene * scene = currentScene();
		if (!win || !scene) return;
		QString name;

		int i=0;
		ItemHandle * lastItem = 0;

		if (tableItems.size() < 1)
		{
			if (!scene->selected().isEmpty()) 
				lastItem = getHandle(scene->selected())[0];
			else
			if (scene->localHandle())
				lastItem = scene->localHandle();
			else
			if (scene->globalHandle())
				lastItem = scene->globalHandle();
		}
		else
		{
			lastItem = tableItems[i].first;
			while (!lastItem && (i+1) < tableItems.size()) lastItem = tableItems[++i].first;
		}

		if (lastItem == 0) return;

		disconnect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		disconnect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));

		int n = tableWidget.rowCount();
		tableWidget.insertRow(n);
		
		if (!lastItem->name.isEmpty())
			tableWidget.setVerticalHeaderItem(n, new QTableWidgetItem(lastItem->fullName()));
		else
			tableWidget.setVerticalHeaderItem(n, new QTableWidgetItem(tr("")));

		if (type == both || type == numerical)
		{
			if (lastItem->hasNumericalData(this->name))
			{
				DataTable<qreal> nDat(lastItem->numericalDataTable(this->name));
				i = 0;
				name = tr("k0");
				
				if (!lastItem->name.isEmpty())
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(lastItem->fullName() + tr(".") + name))
						name = tr("k") + QString::number(++i);
				}
				else
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(name))
						name = tr("k") + QString::number(++i);
				}
				tableWidget.setItem(n,1,new QTableWidgetItem(tr("1.0")));
				tableWidget.setItem(n,0,new QTableWidgetItem(name));
				tableItems << QPair<ItemHandle*,int>(lastItem,nDat.rowNames().size());

				nDat.resize(nDat.rows()+1,nDat.columns());
				nDat.value(nDat.rows()-1,0) = 1.0;
				nDat.value(nDat.rows()-1,1) = 1e-8;
				nDat.value(nDat.rows()-1,2) = 100.0;
				nDat.setRowName(nDat.rows()-1,name);

				if (!lastItem->name.isEmpty())
					win->changeData(lastItem->fullName() + tr(".") + name + tr(" added"), lastItem,this->name,&nDat);
				else
					win->changeData(name + tr(" added"), lastItem,this->name,&nDat);
			}
		}

		if (type == both || type == text)
		{
			if (lastItem->hasTextData(this->name))
			{
				DataTable<QString> sDat(lastItem->textDataTable(this->name));

				i = 0;
				name = tr("s0");
				
				if (!lastItem->name.isEmpty())
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(lastItem->fullName() + tr(".") + name))
						name = tr("s") + QString::number(++i);
				}
				else
				{
					while (win->symbolsTable.uniqueDataWithDot.contains(name))
						name = tr("s") + QString::number(++i);
				}

				tableWidget.setItem(n,1,new QTableWidgetItem(tr("1.0")));
				tableWidget.setItem(n,0,new QTableWidgetItem(name));
				tableItems << QPair<ItemHandle*,int>(lastItem,sDat.rowNames().size());

				sDat.resize(sDat.rows()+1,sDat.columns());
				sDat.value(sDat.rows()-1,0) = tr("hello world");
				sDat.setRowName(sDat.rows()-1 , name);

				if (!lastItem->name.isEmpty())
					win->changeData(lastItem->fullName() + tr(".") + name + tr(" added"),lastItem,this->name,&sDat);
				else
					win->changeData(name + tr(" added"),lastItem,this->name,&sDat);
			}
		}

		connect(&tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(setValue(int,int)));
		connect(mainWindow,SIGNAL(historyChanged(int)),this,SLOT(historyUpdate(int)));
	}

	void ParametersTool::removeSelectedAttributes()
	{
		QList<QTableWidgetItem*> selectedItems = tableWidget.selectedItems();
		NetworkHandle * win = currentNetwork();
		if (!win) return;

		QList<ItemHandle*> handles1, handles2;
		QList<DataTable<qreal>*> nDats;
		QList<DataTable<QString>*> sDats;
		QList<ItemHandle*> allHandles = win->handles();

		for (int i=0; i < selectedItems.size(); ++i)
		{
			int row = selectedItems[i]->row();

			if (row >= tableItems.size()) break;

			ItemHandle * handle = tableItems[row].first;
			int rowNumber = tableItems[row].second;

			if (!handle) continue;

			if ((type == both || type == numerical) && handle->hasNumericalData(this->name))
			{
				DataTable<qreal> & nDat = handle->numericalDataTable(this->name);

				if (rowNumber < nDat.rows())
				{
					QString s = nDat.rowName(rowNumber);
					QRegExp regex1(tr("[^a-zA-Z_\\.]") + handle->fullName() + tr(".") + s);
					QRegExp regex2(tr("^") + handle->fullName() + tr(".") + s);
					
					bool beingUsed = false;
					QString s3;

					for (int j=0; j < allHandles.size(); ++j)
					{
						QStringList textKeys = allHandles[j]->textDataNames();
						for (int k=0; k < textKeys.size(); ++k)
						{
							TextDataTable & textData = allHandles[j]->textDataTable(textKeys[k]);
							for (int k1=0; k1 < textData.rows(); ++k1)
							{
								for (int k2=0; k2 < textData.columns(); ++k2)
									if (textData.value(k1,k2).contains(regex1) || textData.value(k1,k2).contains(regex2))
									{
										s3 = allHandles[j]->fullName() + tr("'s ") + textKeys[k];
										beingUsed = true;
										break;
									}
								if (beingUsed) break;
							}
							if (beingUsed) break;
						}

					}
					
					if (beingUsed)
					{
					    QMessageBox::information(this, tr("Cannot remove"), s + tr(" cannot be removed because it is being used inside ") + s3);	
					}
					else
					{
						DataTable<qreal> * nDat2 = new DataTable<qreal>(nDat);
						nDat2->removeRow(rowNumber);
						nDats << nDat2;
						handles1 << handle;
					}
				}
			}

			if ((type == both || type == text) && handle->hasTextData(this->name))
			{
				DataTable<QString> * sDat = new DataTable<QString>(handle->textDataTable(this->name));

				if (rowNumber < sDat->rows())
				{
					sDat->removeRow(rowNumber);
					sDats << sDat;
					handles2 << handle;
				}
				else
					delete sDat;
			}
		}

		if (nDats.size() > 0)
			win->changeData(tr("selected parameters removed"), handles1,this->name,nDats);

		if (sDats.size() > 0)
			win->changeData(tr("selected text attributes removed"),handles2,this->name,sDats);

		for (int i=0; i < nDats.size(); ++i)
			delete nDats[i];

		for (int i=0; i < sDats.size(); ++i)
			delete sDats[i];
	}

	typedef void (*tc_ParametersTool_Text_api)(
		char* (*getTextData)(long ,const char* ),
		tc_strings (*getAllTextDataNamed)(tc_items, tc_strings),
		void (*setTextData)(long ,const char* ,const char* ));

	typedef void (*tc_ParametersTool_Numeric_api)(
		tc_matrix (*getInitialValues)(tc_items ),
		void (*setInitialValues)(tc_items,tc_matrix),
		tc_matrix (*getParameters)(tc_items ),
		tc_matrix (*getFixedVars)(tc_items),
		tc_matrix (*getFixedAndParameters)(tc_items),
		double (*getNumericalData)(long ,const char* ),
		tc_matrix (*getParametersNamed)(tc_items, tc_strings),
		tc_matrix (*getParametersExcept)(tc_items, tc_strings),
		void (*setNumericalData)(long ,const char* ,double ));

	void ParametersTool::setupFunctionPointers( QLibrary * library )
	{
		if (type == both || type == numerical)
		{
			tc_ParametersTool_Numeric_api f = (tc_ParametersTool_Numeric_api)library->resolve("tc_ParametersTool_Numeric_api");
			if (f)
			{
				//qDebug() << "tc_ParametersTool_Numeric_api resolved";
				f(
					&(_getInitialValues),
					&(_setInitialValues),
					&(_getParameters),
					&(_getFixedVars),
					&(_getFixedAndParameters),
					&(_getNumericalData),
					&(_getParametersNamed),
					&(_getParametersExcept),
					&(_setNumericalData)
					);
			}
		}
		if (type == both || type == text)
		{
			tc_ParametersTool_Text_api f = (tc_ParametersTool_Text_api)library->resolve("tc_ParametersTool_Text_api");
			if (f)
			{
				//qDebug() << "tc_ParametersTool_Text_api resolved";
				f(
					&(_getTextData),
					&(_getAllTextDataNamed),
					&(_setTextData)
					);

			}
		}
	}

	void ParametersTool::connectTCFunctions()
	{
		if (type == both || type == text)
		{
			connect(&fToS,SIGNAL(getTextData(QSemaphore*,QString*,ItemHandle*,const QString&)),this,SLOT(getTextData(QSemaphore*,QString*,ItemHandle*,const QString&)));
			connect(&fToS,SIGNAL(setTextData(QSemaphore*,ItemHandle*,const QString&,const QString&)),this,SLOT(setTextData(QSemaphore*,ItemHandle*,const QString&,const QString&)));
			connect(&fToS,SIGNAL(getAllTextDataNamed(QSemaphore*,QStringList*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getAllTextDataNamed(QSemaphore*,QStringList*,const QList<ItemHandle*>&,const QStringList&)));
		}
		if (type == both || type == numerical)
		{
			connect(&fToS,SIGNAL(getInitialValues(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getInitialValues(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(setInitialValues(QSemaphore*,const QList<ItemHandle*>&,const DataTable<qreal>&)),this,SLOT(setInitialValues(QSemaphore*,const QList<ItemHandle*>&,const DataTable<qreal>&)));
			connect(&fToS,SIGNAL(getParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getFixedVars(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getFixedVars(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getFixedAndParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)),this,SLOT(getFixedAndParameters(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&)));
			connect(&fToS,SIGNAL(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&)),this,SLOT(getNumericalData(QSemaphore*,qreal*,ItemHandle*,const QString&)));
			connect(&fToS,SIGNAL(setNumericalData(QSemaphore*,ItemHandle*,const QString&,qreal)),this,SLOT(setNumericalData(QSemaphore*,ItemHandle*,const QString&,qreal)));
			connect(&fToS,SIGNAL(getParametersNamed(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getParametersNamed(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)));
			connect(&fToS,SIGNAL(getParametersExcept(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)),this,SLOT(getParametersExcept(QSemaphore*,DataTable<qreal>*,const QList<ItemHandle*>&,const QStringList&)));
		}
	}

	DataTable<qreal> ParametersTool::getParameters(const QList<QGraphicsItem*>& items, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QList<ItemHandle*> handles;

		ItemHandle* handle = 0;

		for (int i=0; i < items.size(); ++i)
		{
			handle = getHandle(items.at(i));
			if (handle && !handles.contains(handle))
				handles += handle;
		}

		return getParameters(handles,mustHave,exclude,sep);
	}

	DataTable<qreal> ParametersTool::getParameters(const QList<ItemHandle*>& handles, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QStringList rownames;
        QList<qreal> values, minv, maxv;
        QString s;
        QString name("Parameters");

        ItemHandle * handle = 0;
        DataTable<qreal> * dataTable = 0;

        for (int i=0; i < handles.size(); ++i)
        {
            handle = handles.at(i);

            if (handle && handle->hasNumericalData(name))
            {
                dataTable = &(handle->numericalDataTable(name));
                if (dataTable && dataTable->columns() > 0)
                    for (int j=0; j < dataTable->rows(); ++j)
                    {
		                if ((mustHave.isEmpty() || mustHave.contains(dataTable->rowName(j).toLower()) || mustHave.contains(dataTable->rowName(j)))
		                    && (exclude.isEmpty() || !(exclude.contains(dataTable->rowName(j).toLower()) || exclude.contains(dataTable->rowName(j)))))
							{
								if (handle->name.isEmpty())
									s = dataTable->rowName(j);
								else							
									s = handle->fullName(sep) + sep + dataTable->rowName(j);

								rownames += s;
								values += dataTable->at(j,0);
								minv += dataTable->at(j,1);
								maxv += dataTable->at(j,2);
							}
		            }
            }
        }

        DataTable<qreal> combinedTable;
        combinedTable.resize(values.size(),1);
        for (int i=0; i < values.size() && i < rownames.size(); ++i)
        {
            combinedTable.setRowName(i,rownames[i]);
            combinedTable.value(i,0) = values[i];
            combinedTable.value(i,1) = minv[i];
            combinedTable.value(i,2) = maxv[i];
        }
        combinedTable.setColumnName(0,tr("values"));
        combinedTable.setColumnName(1,tr("min"));
        combinedTable.setColumnName(2,tr("max"));

        return combinedTable;
	}

	DataTable<QString> ParametersTool::getTextData(const QList<ItemHandle*>& handles, const QStringList& mustHave, const QStringList& exclude, const QString& sep)
	{
		QStringList rownames;
		QList<QString> values;

		QString name("Text Attributes");

		ItemHandle * handle = 0;
		DataTable<QString> * dataTable = 0;

		for (int i=0; i < handles.size(); ++i)
		{
			handle = handles.at(i);

			if (handle && handle->hasTextData(name))
			{				dataTable = &(handle->textDataTable(name));
			if (dataTable && dataTable->columns() > 0)
				for (int j=0; j < dataTable->rows(); ++j)
				{
					if ((mustHave.isEmpty() || mustHave.contains(dataTable->rowName(j).toLower()) || mustHave.contains(dataTable->rowName(j)))
						&& (exclude.isEmpty() || !(exclude.contains(dataTable->rowName(j).toLower()) || exclude.contains(dataTable->rowName(j)))))						
					{
						if (handle->name.isEmpty())
							rownames += dataTable->rowName(j);
						else
                            rownames += handle->fullName(sep) + sep + dataTable->rowName(j);

                        values += dataTable->at(j,0);
					}
				}
			}
		}

		QRegExp regex(tr("\\.(?!\\d)"));
		for (int i=0; i < values.size(); ++i)
		{
			while (regex.indexIn(values[i]) != -1)
			{
				values[i].replace(regex,sep);
			}
		}

		DataTable<QString> combinedTable;
		combinedTable.resize(values.size(),1);
		for (int i=0; i < values.size() && i < rownames.size(); ++i)
		{
			combinedTable.setRowName(i, rownames[i]);
			combinedTable.value(i,0) = values[i];
		}

		return combinedTable;
	}

	void ParametersTool::getInitialValues(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles)
	{
		if (ptr)
		{
			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;


			for (int i=0; i < handles.size(); ++i)

			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Initial Value")))
				{
					dataTable = &(handle->numericalDataTable(tr("Initial Value")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0)
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = (*ptr).rows();
			(*ptr).resize( rows + names.size(), (*ptr).columns() );

			for (int i=0; i < names.size(); ++i)
			{
				(*ptr).setRowName(rows + i , names[i]);
				(*ptr).value(rows + i, 0) = values[i];
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::setInitialValues(QSemaphore* s,const QList<ItemHandle*>& handles,const DataTable<qreal>& dat)
	{
		ItemHandle * handle = 0;
		DataTable<qreal> * dataTable = 0;

		QStringList names;
		QList<qreal> values;

		if (dat.columns() == 1)
		{
			QList< DataTable<qreal>* > newData;
			QList< ItemHandle* > handles2;
			for (int i=0; i < handles.size() && i < dat.rows(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Initial Value")))
				{
					dataTable = new DataTable<qreal>(handle->numericalDataTable(tr("Initial Value")));
					dataTable->value(0,0) = dat.at(i,0);
					newData << dataTable;
					handles2 << handle;
				}
			}
			if (newData.size() > 0)
			{
				if (currentNetwork())
					currentNetwork()->changeData(tr("Initial values changed"),handles2,tr("Initial Value"),newData);
				for (int i=0; i < newData.size(); ++i)
					if (newData[i])
						delete newData[i];
			}
		}

		if (s)
			s->release();
	}

	void ParametersTool::getFixedVars(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles)
	{
		if (ptr)
		{
			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;

			for (int i=0; i < handles.size(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Fixed")))
				{
					dataTable = &(handle->numericalDataTable(tr("Fixed")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0 && (dataTable->value(0,0) > 0))
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = (*ptr).rows();
			(*ptr).resize( rows + names.size(), (*ptr).columns() );

			for (int i=0; i < names.size(); ++i)
			{
				(*ptr).setRowName(rows + i , names[i]);
				(*ptr).value(rows + i, 0) = values[i];
			}

		}
		if (s)
			s->release();
	}

	void ParametersTool::getFixedAndParameters(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& handles0)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles = handles0;

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			int i,j;
			QString replaceDot("_");
			//QStringList rates = StoichiometryTool::getRates(handles, replaceDot);
			//params.insertColumn(1,tr("used"));
			DataTable<qreal> params = ParametersTool::getParameters(handles,QStringList(), QStringList(), replaceDot);

			/*bool used = false;
			for (i=0; i < params.rows(); ++i)
			{
				used = false;
				for (j=0; j < rates.size(); ++j)
				{
					if (rates[j].contains(params.rowName(i)))
					{
						used = true;
						break;
					}
				}
				if (used)
					params.value(i,1) = 1.0;
				else
					params.value(i,1) = 0.0;
			}*/

			QRegExp regex(tr("\\.(?!\\d)"));
			QString s1,s2;

			/*for (i=0; i < handles.size(); ++i)
			{
				if (!handles[i])
					continue;

				if (handles[i]->hasTextData(tr("Events")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Events"));
					if (dat.columns() == 1)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);

							s2 =  dat.value(j,0);

							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}

				if (handles[i]->hasTextData(tr("Assignments")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Assignments"));
					if (dat.columns() > 0 && dat.rows() > 0)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);
							s2 =  dat.value(j,0);
							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}

				if (handles[i]->hasTextData(tr("Functions")))
				{
					DataTable<QString>& dat = handles[i]->textDataTable(tr("Functions"));
					if (dat.columns() == 1)
						for (j=0; j < dat.rows(); ++j)
						{
							s1 =  dat.rowName(j);
							s1.replace(regex,replaceDot);
							s2 =  dat.value(j,0);
							s2.replace(regex,replaceDot);

							if (s1.isEmpty() || s2.isEmpty()) continue;

							for (int k=0; k < params.rows(); ++k)
								if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
									params.value(k,1) = 1.0;
						}
				}
			}*/

			int count = params.rows();
			/*for (int i=0; i < params.rows(); ++i)
			{
				if (params.value(i,1) > 0.0) ++count;
			}*/

			DataTable<qreal> params2;
			params2.resize(count,1);
			params2.setColumnName(0,params.columnName(0));

			for (int i=0, j=0; i < params.rows() && j < count; ++i)
			{
				if (params.value(i,1) > 0.0)
				{
					params2.setRowName(j, params.rowName(i));
					params2.value(j,0) = params.value(i,0);
					++j;
				}
			}

			ItemHandle * handle = 0;
			DataTable<qreal> * dataTable = 0;

			QStringList names;
			QList<qreal> values;

			for (int i=0; i < handles.size(); ++i)
			{
				handle = handles.at(i);
				if (handle && handle->hasNumericalData(tr("Fixed")))
				{
					dataTable = &(handle->numericalDataTable(tr("Fixed")));
					if (dataTable && dataTable->columns() > 0 && dataTable->rows() > 0 && (dataTable->value(0,0) > 0))
					{
						names << handle->fullName(tr("_"));
						values << dataTable->value(0,0);
					}
				}
			}

			int rows = params2.rows();
			params2.resize( rows + names.size(), 1 );

			for (int i=0; i < names.size(); ++i)
			{
				params2.setRowName(rows + i, names[i]);
				params2.value(rows + i, 0) = values[i];
			}

			(*ptr) = params2;
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParameters(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getUsedParameters(currentNetwork(), handles);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParametersNamed(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getParameters(handles,text);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getParametersExcept(QSemaphore* s,DataTable<qreal>* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			(*ptr) = getParameters(handles,QStringList(),text);
		}
		if (s)
			s->release();
	}

	void ParametersTool::getAllTextDataNamed(QSemaphore* s,QStringList* ptr,const QList<ItemHandle*>& list,const QStringList& text)
	{
		if (ptr)
		{
			QList<ItemHandle*> handles;
			for (int i=0; i < list.size(); ++i)
				if (list[i])
					handles << list[i];

			if (currentNetwork() && currentNetwork()->globalHandle())
				if (!handles.contains(currentNetwork()->globalHandle()))
					handles << currentNetwork()->globalHandle();

			DataTable<QString> dat = getTextData(handles,text);
			if (dat.columns() > 0)
			{
				for (int i=0; i < dat.rows(); ++i)
					(*ptr) << dat.at(i,0);
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::getTextData(QSemaphore* s,QString* ptr,ItemHandle* handle,const QString& text)
	{
		if (ptr)
		{
			if (handle && handle->hasTextData(this->name))
			{
				if (handle->textDataTable(name).hasRow(text))
				{
					(*ptr) = handle->textDataTable(name).at(text,0);
				}
				
				QRegExp regex(tr("\\.(?!\\d)"));
				(*ptr).replace(regex,tr("_"));
			}
			else
			{
				(*ptr) = tr("");
			}
		}
		if (s)
			s->release();
	}
	void ParametersTool::getNumericalData(QSemaphore* s,qreal* ptr,ItemHandle* handle,const QString& text)
	{
		if (ptr)
		{
			if (handle && handle->hasNumericalData(name))
			{
				(*ptr) = 0.0;
				if (handle->numericalDataTable(name).hasRow(text))
				{
					(*ptr) = handle->numericalDataTable(name).at(text,0);
				}
			}
			else
			{
				(*ptr) = 0.0;
			}
		}
		if (s)
			s->release();
	}
	void ParametersTool::setTextData(QSemaphore* s,ItemHandle* handle,const QString& text,const QString& value)
	{
		if (handle)
		{
			if (handle->hasTextData(name))
			{
				NetworkHandle * win = mainWindow->currentNetwork();
				if (win)
				{
					DataTable<QString> * newData = new DataTable<QString>(handle->textDataTable(name));
					newData->value(text,0) = value;
					win->changeData(handle->fullName() + tr(".") + text + tr(" = ") + value, handle,name,newData);
					delete newData;
				}
				else
				{
					handle->textDataTable(name).value(text,0) = value;
				}
			}
		}
		if (s)
			s->release();
	}

	void ParametersTool::setNumericalData(QSemaphore* s,ItemHandle* handle,const QString& text,qreal value)
	{
		if (handle)
		{
			if (handle->hasNumericalData(name))
			{
				NetworkHandle * win = mainWindow->currentNetwork();
				if (win)
				{
					DataTable<qreal> * newData = new DataTable<qreal>(handle->numericalDataTable(name));
					newData->value(text,0) = value;
					win->changeData(handle->fullName() + tr(".") + text + tr(" = ") + QString::number(value), handle,this->name,newData);
					delete newData;
				}
				else
				{
					handle->numericalDataTable(name).value(text,0) = value;
				}
			}
		}
		if (s)
			s->release();
	}

	/********************************************************/

	ParametersTool_FToS ParametersTool::fToS;

	tc_matrix ParametersTool::_getParameters(tc_items A)
	{
		return fToS.getParameters(A);
	}

	tc_matrix ParametersTool::_getInitialValues(tc_items A)
	{
		return fToS.getInitialValues(A);
	}

	void ParametersTool::_setInitialValues(tc_items A, tc_matrix M)
	{
		fToS.setInitialValues(A,M);
	}

	tc_matrix ParametersTool::_getFixedVars(tc_items A)
	{
		return fToS.getFixedVars(A);
	}

	tc_matrix ParametersTool::_getFixedAndParameters(tc_items A)
	{
		return fToS.getFixedAndParameters(A);
	}

	char* ParametersTool::_getTextData(long o,const char* c)
	{
		return fToS.getTextData(o,c);
	}

	double ParametersTool::_getNumericalData(long o,const char* c)
	{
		return fToS.getNumericalData(o,c);
	}

	tc_matrix ParametersTool::_getParametersNamed(tc_items A, tc_strings c)
	{
		return fToS.getParametersNamed(A,c);
	}

	tc_matrix ParametersTool::_getParametersExcept(tc_items A, tc_strings c)
	{
		return fToS.getParametersExcept(A,c);
	}

	tc_strings ParametersTool::_getAllTextDataNamed(tc_items A, tc_strings c)
	{
		return fToS.getAllTextDataNamed(A,c);
	}

	void ParametersTool::_setTextData(long o,const char* a,const char* b)
	{
		return fToS.setTextData(o,a,b);
	}

	void ParametersTool::_setNumericalData(long o,const char* a,double b)
	{
		return fToS.setNumericalData(o,a,b);
	}

	tc_matrix ParametersTool_FToS::getParameters(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParameters(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getInitialValues(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getInitialValues(s,p,*list);
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

	void ParametersTool_FToS::setInitialValues(tc_items a0, tc_matrix M)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = ConvertValue(M);
		s->acquire();


		emit setInitialValues(s,*list,*p);
		s->acquire();
		s->release();
		delete s;
		delete list;
		if (p)
		{
			delete p;
		}
	}

	tc_matrix ParametersTool_FToS::getFixedVars(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getFixedVars(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getFixedAndParameters(tc_items a0)
	{
		QSemaphore * s = new QSemaphore(1);
		QList<ItemHandle*> * list = ConvertValue(a0);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getFixedAndParameters(s,p,*list);
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

	tc_matrix ParametersTool_FToS::getParametersNamed(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParametersNamed(s,p,*list,ConvertValue(name));
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

	tc_matrix ParametersTool_FToS::getParametersExcept(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		DataTable<qreal> * p = new DataTable<qreal>;
		s->acquire();
		emit getParametersExcept(s,p,*list,ConvertValue(name));
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

	tc_strings ParametersTool_FToS::getAllTextDataNamed(tc_items a0,tc_strings name)
	{
		QList<ItemHandle*> * list = ConvertValue(a0);
		QSemaphore * s = new QSemaphore(1);
		QStringList p;
		s->acquire();
		emit getAllTextDataNamed(s,&p,*list,ConvertValue(name));
		s->acquire();
		s->release();
		delete s;
		delete list;
		return ConvertValue(p);
	}

	char* ParametersTool_FToS::getTextData(long a0,const char* a1)
	{
		QSemaphore * s = new QSemaphore(1);
		QString p;
		s->acquire();
		emit getTextData(s,&p,ConvertValue(a0),ConvertValue(a1));
		s->acquire();
		s->release();
		delete s;
		return (char*)ConvertValue(p);
	}

	double ParametersTool_FToS::getNumericalData(long a0,const char* a1)
	{
		QSemaphore * s = new QSemaphore(1);
		qreal p;
		s->acquire();
		emit getNumericalData(s,&p,ConvertValue(a0),ConvertValue(a1));
		s->acquire();
		s->release();
		delete s;
		return (double)p;
	}

	void ParametersTool_FToS::setTextData(long a0,const char* a1,const char* a2)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setTextData(s,ConvertValue(a0),ConvertValue(a1),ConvertValue(a2));
		s->acquire();
		s->release();
		delete s;
	}

	void ParametersTool_FToS::setNumericalData(long a0,const char* a1,double a2)
	{
		QSemaphore * s = new QSemaphore(1);
		s->acquire();
		emit setNumericalData(s,ConvertValue(a0),a1,a2);
		s->acquire();
		s->release();
		delete s;
	}

	void ParametersTool::keyPressEvent ( QKeyEvent * event )
	{
		if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
		{
			removeSelectedAttributes();
		}
	}
	
	DataTable<qreal> ParametersTool::getUsedParameters(NetworkHandle * network, QList<ItemHandle*>& handles, const QString& replaceDot)
	{
		int i,j;
		DataTable<qreal> params = ParametersTool::getParameters(handles,QStringList(), QStringList(), replaceDot);
		
		params.insertColumn(3,tr("used"));

		QRegExp regex(tr("\\.(?!\\d)"));
		QString s1,s2;

		QList<ItemHandle*> allhandles;
		if (network)
			allhandles = network->handles();
		else
			allhandles = handles;

		for (i=0; i < allhandles.size(); ++i)
		{
			if (!allhandles[i])
				continue;

			if (allhandles[i]->children.isEmpty() && allhandles[i]->hasTextData(tr("Rate equations")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Rate equations"));
				if (dat.columns() == 1)
				{
					for (j=0; j < dat.rows(); ++j)
					{
						s2 =  dat.value(j,0);
						s2.replace(regex,replaceDot);
						
						if (s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
				}
			}

			if (allhandles[i]->hasTextData(tr("Events")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Events"));
				if (dat.columns() == 1)
					for (j=0; j < dat.rows(); ++j)
					{
						s1 =  dat.rowName(j);

						s1.replace(regex,replaceDot);
						s2 =  dat.value(j,0);

						s2.replace(regex,replaceDot);

						if (s1.isEmpty() || s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
			}

			if (allhandles[i]->hasTextData(tr("Assignments")))
			{
				DataTable<QString>& dat = allhandles[i]->textDataTable(tr("Assignments"));
				if (dat.columns() > 0 && dat.rows() > 0)
					for (j=0; j < dat.rows(); ++j)
					{
						s1 =  dat.rowName(j);
						s1.replace(regex,replaceDot);
						s2 =  dat.value(j,0);
						s2.replace(regex,replaceDot);

						if (s1.isEmpty() || s2.isEmpty()) continue;

						for (int k=0; k < params.rows(); ++k)
							if (s2.contains(params.rowName(k)) || s1.contains(params.rowName(k)))
								params.value(k,3) = 1.0;
					}
			}
		}
		
		int count = 0;
		for (int i=0; i < params.rows(); ++i)
		{
			if (params.value(i,3) > 0.0) ++count;
		}

		DataTable<qreal> params2;
		params2.resize(count,3);
		params2.setColumnName(0,params.columnName(0));
		params2.setColumnName(1,params.columnName(1));
		params2.setColumnName(2,params.columnName(2));

		for (int i=0, j=0; i < params.rows() && j < count; ++i)
		{
			if (params.value(i,3) > 0.0)
			{
				params2.setRowName(j , params.rowName(i));
				params2.value(j,0) = params.value(i,0);
				params2.value(j,1) = params.value(i,1);
				params2.value(j,2) = params.value(i,2);
				++j;
			}
		}
		return params2;
	}
	
	QString ParametersTool::removeParameterFromModel(NetworkHandle * win, ItemHandle * handle, const QString& s)
	{
		if (!handle) return QString("No item");		
		if (!handle->hasNumericalData("Parameters")) return QString("No parameters inside this item");
		
		NumericalDataTable nDat(handle->numericalDataTable("Parameters"));
		if (!nDat.hasRow(s)) return QString("No such parameter inside this item");
		
		bool beingUsed = false;
		QRegExp regex1, regex2;
		
		if (handle->name.isEmpty())
		{
			regex1 = QRegExp(QObject::tr("[^a-zA-Z_\\.]") + s);
			regex2 = QRegExp(QObject::tr("^") + s);
		}
		else
		{
			regex1 = QRegExp(QObject::tr("[^a-zA-Z_\\.]") + handle->fullName() + tr(".") + s);
			regex2 = QRegExp(QObject::tr("^") + handle->fullName() + tr(".") + s);
		}
		
		QString s3;
						
		QList<ItemHandle*> allHandles = win->handles();
		for (int i=0; i < allHandles.size(); ++i)
		{
			QStringList textKeys = allHandles[i]->textDataNames();
			for (int j=0; j < textKeys.size(); ++j)
			{
				TextDataTable & textData = allHandles[i]->textDataTable(textKeys[j]);
				for (int k1=0; k1 < textData.rows(); ++k1)
				{
					for (int k2=0; k2 < textData.columns(); ++k2)
						if (textData.value(k1,k2).contains(regex1) || textData.value(k1,k2).contains(regex2))
						{
							s3 = allHandles[i]->fullName() + tr("'s ") + textKeys[j];
							beingUsed = true;
							break;
						}
					if (beingUsed) break;
				}
				if (beingUsed) break;
			}
		}

		if (beingUsed)
		{
			return s + tr(" cannot be removed because it is being used inside ") + s3;
		}
		else
		{
			nDat.removeRow(s);
			if (handle->name.isEmpty())
				win->changeData(s + QObject::tr(" removed"),handle, QObject::tr("Parameters"), &nDat);
			else
				win->changeData(handle->fullName() + QObject::tr(".") + s + QObject::tr(" removed"),handle, QObject::tr("Parameters"), &nDat);
		}
		return QString();
	}
	
	QString ParametersTool::removeUnusedParametersInModel(NetworkHandle * win)
	{
		return QString();

		QString allFormulas;
						
		QList<ItemHandle*> allHandles = win->handles();
		for (int i=0; i < allHandles.size(); ++i)
		{
			QStringList textKeys = allHandles[i]->textDataNames();
			for (int j=0; j < textKeys.size(); ++j)
			{
				TextDataTable & textData = allHandles[i]->textDataTable(textKeys[j]);
				for (int k1=0; k1 < textData.rows(); ++k1)
				{
					for (int k2=0; k2 < textData.columns(); ++k2)
					{
						allFormulas += textData.value(k1,k2);
						allFormulas += QObject::tr(";");
					}
				}
			}
		}
		
		QList<ItemHandle*> changedHandles, handles = win->handles();
		QList<NumericalDataTable*> oldData, newData;
		
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->hasNumericalData(QObject::tr("Parameters")))
			{
				QString s;
				if (!handles[i]->name.isEmpty())
					s = handles[i]->fullName() + QObject::tr(".");
				bool changes = false;
				NumericalDataTable * oldDat = &(handles[i]->numericalDataTable(QObject::tr("Parameters")));
				NumericalDataTable * newDat = new NumericalDataTable(*oldDat);

				for (int j=0; j < newDat->rows(); ++j)
				{
					if (!allFormulas.contains(s + newDat->rowName(j)))
					{
						changes = true;
						newDat->removeRow(j);
						--j;
					}
				}
				if (changes)
				{
					changedHandles << handles[i];
					oldData << oldDat;
					newData << newDat;
				}
			}
		}
		
		if (!oldData.isEmpty())
		{
			win->changeData(QObject::tr("Unused parameters removed"), changedHandles, oldData, newData);
			for (int i=0; i < newData.size(); ++i)
				delete newData[i];
		}
		
	}
}

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
