<<<<<<< HEAD
/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 A tool that allows users to construct models using Antimony scripts in the TextEditor

****************************************************************************/
#include <iostream>
#include <QClipboard>
#include "TextEditor.h"
#include "NetworkHandle.h"
#include "NetworkWindow.h"
#include "UndoCommands.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ConsoleWindow.h"
#include "LoadSaveTool.h"
#include "AntimonyEditor.h"
#include "ModelSummaryTool.h"
#include "ModuleTool.h"
#include "SBMLImportExport.h"
#include "CloneItems.h"
#include <QToolButton>
#include <QRegExp>
#include <QFile>
#include <QPair>
#include <QRegExp>
#include <QtDebug>
#include <QSemaphore>
#include <QFileDialog>
#include "antimony_api.h"

namespace Tinkercell
{
	AntimonyEditor::AntimonyEditor() : TextParser(tr("Antimony Parser"))
	{
		scriptDisplayWindow = new CodeEditor(this);
		icon = QPixmap(tr(":/images/antimony.png"));
	}

	bool AntimonyEditor::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);
		if (mainWindow)
		{
			mainWindow->addParser(this);

			connect(mainWindow,SIGNAL(copyItems(GraphicsScene *, QList<QGraphicsItem*>&, QList<ItemHandle*>&)),
						this,SLOT(copyItems(GraphicsScene *, QList<QGraphicsItem*>&, QList<ItemHandle*>&)));

			connect(mainWindow,SIGNAL(windowChanged(NetworkWindow*,NetworkWindow*)),
						this,SLOT(windowChanged(NetworkWindow*,NetworkWindow*)));
			
			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),
						this,SLOT(toolLoaded(Tool*)));

			connect(mainWindow,SIGNAL(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)),
						this,SLOT(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)));

			toolLoaded(0);

		}
		return false;
	}
	
	void AntimonyEditor::loadNetwork(const QString& filename, bool * b)
	{
		if (b && (*b)) return;

		QFile file(filename);
		QString modelString;

		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			modelString = file.readAll();
			file.close();
		}
		else
		{
			modelString = filename;	
		}

		ItemHandle root;
		QList<ItemHandle*> itemsToInsert = parse(modelString, &root);

		//if (!itemsToInsert.isEmpty())
		{
			TextEditor * editor = mainWindow->newTextEditor();
			(*editor->globalHandle()) = root;
			editor->setText(modelString);
			editor->setItems(itemsToInsert);
		}
	}

	void AntimonyEditor::windowChanged(NetworkWindow*,NetworkWindow * window)
	{
		if (window && window->editor && !visitedWindows.contains(window) && TextParser::currentParser() == this)
		{
			visitedWindows[window] = true;

			AntimonySyntaxHighlighter * as = new AntimonySyntaxHighlighter(window->editor->document());
			connect(this,SIGNAL(validSyntax(bool)),as,SLOT(setValid(bool)));

			QToolButton * button = new QToolButton(this);
			button->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
			button->setIcon(QIcon(":/images/antimony.png"));
			button->setIconSize(QSize(30,30));
			button->setText(tr("Compile script"));
			button->setToolTip(tr("interpret script using Antimony language"));
			connect(button,SIGNAL(pressed()),this,SLOT(parse()));
			
			QDockWidget * dock = new QDockWidget;
			dock->setMaximumWidth(150);
			QVBoxLayout * layout = new QVBoxLayout;
			QWidget * widget = new QWidget;
			layout->setContentsMargins(5,8,5,5);
			layout->setSpacing(12);
			widget->setLayout(layout);
			layout->addWidget(button,1,Qt::AlignTop);
			widget->setPalette(QPalette(QColor(255,255,255)));
			widget->setAutoFillBackground (true);
			dock->setWidget(widget);
			window->addDockWidget(Qt::RightDockWidgetArea,dock);
			/*
			button = new QToolButton;
			button->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
			button->setIcon(QIcon(":/images/module.png"));
			button->setIconSize(QSize(30,30));
			button->setText(tr("Export modules"));
			button->setToolTip(tr("export module(s) to the last graphics window"));*/
		}
	}

	void AntimonyEditor::parse()
	{
		parse(currentTextEditor());
	}

	void AntimonyEditor::parse(TextEditor * editor)
	{
		if (!editor) return;

		QString modelString = editor->toPlainText() + tr("\n");

		ItemHandle * root = editor->localHandle();
		if (!root)
			root = editor->globalHandle();

		QList<ItemHandle*> itemsToInsert = parse(modelString, root);

		if (!itemsToInsert.isEmpty())
		{
			editor->setItems(itemsToInsert);
		}
	}

	static void substituteToEqualitySymbols(QString& s)
	{
		s.replace(QRegExp("gt\\((.*),(.*)\\)"), "\\1>\\2");
		s.replace(QRegExp("lt\\((.*),(.*)\\)"), "\\1<\\2");
		s.replace(QRegExp("ge\\((.*),(.*)\\)"), "\\1>=\\2");
		s.replace(QRegExp("le\\((.*),(.*)\\)"), "\\1<=\\2");
		s.replace(QRegExp("ne\\((.*),(.*)\\)"), "\\1!=\\2");
		s.replace(QRegExp("eq\\((.*),(.*)\\)"), "\\1=\\2");
	}

	QList<ItemHandle*> AntimonyEditor::parse(QString& modelString, ItemHandle * moduleHandle)
	{
		long ok = loadString(modelString.toAscii().data());
		
		if (ok < 0)
		{
			if (console() && !moduleHandle && currentTextEditor())
				console()->error(tr(getLastError()));
			emit validSyntax(false);
			return QList<ItemHandle*>();
		}
		else
		{
			modelString = tr(getAntimonyString(NULL));
			emit validSyntax(true);
		}

		QString appDir = QCoreApplication::applicationDirPath();

		ConnectionFamily * biochemicalFamily = LoadSaveTool::getConnectionFamily(tr("biochemical reaction"));
		NodeFamily * speciesFamily = LoadSaveTool::getNodeFamily(tr("molecule"));

		if (!biochemicalFamily || !speciesFamily)
		{
			return QList<ItemHandle*>();
		}

		//load

		int nummods = (int)getNumModules();
		char** modnames = getModuleNames();

		QList<ItemHandle*> itemsToInsert;

		//for (int i=0; i < nummods; ++i)
		{
			QStringList symbolsInModule;
			
			/*ItemHandle * moduleHandle = 0;
			moduleHandle = new NodeHandle(moduleFamily);
			moduleHandle->name = QString(moduleName);
			itemsToInsert += moduleHandle;

			if (QString(modnames[i]) == tr("__main"))
				moduleHandle->name = tr("main");

			symbolsInModule << moduleHandle->name;

			QList<ItemHandle*> handlesInModule;*/
			QHash<QString,NodeHandle*> speciesItems;
			QHash<QString,ConnectionHandle*> reactionItems;

			char ***leftrxnnames = getReactantNames("__main");
			char ***rightrxnnames = getProductNames("__main");
			char **rxnnames = getSymbolNamesOfType("__main",allReactions);
			char **rxnrates = getReactionRates("__main");

			double **leftrxnstoichs = getReactantStoichiometries("__main");
			double **rightrxnstoichs = getProductStoichiometries("__main");

			int numrxn = (int)getNumReactions("__main");

			for (int rxn=0; rxn < numrxn; ++rxn)
				if (!reactionItems.contains(tr(rxnnames[rxn])))
				{
					int numReactants = getNumReactants("__main",rxn);
					int numProducts = getNumProducts("__main",rxn);

					QList<NodeHandle*> nodesIn, nodesOut;
					NumericalDataTable reactants, products;
					TextDataTable rate;
					QStringList columnNames;

					for (int var=0; var<numReactants; ++var)
					{
						if (!columnNames.contains(tr(leftrxnnames[rxn][var])))
						{
							columnNames << tr(leftrxnnames[rxn][var]);
						}
					}
				
					reactants.resize(1,columnNames.size());
					reactants.setColumnNames(columnNames);
					columnNames.clear();
				
					for (int var=0; var<numProducts; ++var)
					{
						if (!columnNames.contains(tr(rightrxnnames[rxn][var])))
						{
							columnNames << tr(rightrxnnames[rxn][var]);
						}
					}

					products.resize(1,columnNames.size());
					products.setColumnNames(columnNames);

					for (int j=0; j < columnNames.size(); ++j)
					{
						products.value(0,j) = 0;
					}

					rate.resize(1,1);
					ConnectionHandle * reactionHandle = new ConnectionHandle(biochemicalFamily);

					reactionHandle->name = rxnnames[rxn];
					reactants.setRowName(0, reactionHandle->name);
					products.setRowName(0, reactionHandle->name);
					symbolsInModule << reactionHandle->name;

					for (int var=0; var<numReactants; ++var)
					{
						NodeHandle * handle = 0;
						QString name = tr(leftrxnnames[rxn][var]);

						if (!speciesItems.contains(name))
						{
							handle = new NodeHandle(speciesFamily);
							handle->name = name;
							itemsToInsert  += handle;
						
							symbolsInModule << name;
							speciesItems[name] = handle;
							nodesIn << handle;
						}
						else
						{
							handle = speciesItems[tr(leftrxnnames[rxn][var])];
						}
						reactants.value(0,tr(leftrxnnames[rxn][var])) += leftrxnstoichs[rxn][var];
					}

					for (int var=0; var<numProducts; var++)
					{
						NodeHandle * handle = 0;
						QString name = tr(rightrxnnames[rxn][var]);						
						if (!speciesItems.contains(name))
						{
							handle = new NodeHandle(speciesFamily);
							handle->name = name;
							itemsToInsert += handle;

							speciesItems[name] = handle;
							symbolsInModule << name;
							nodesOut << handle;
						}
						else
						{
							handle = speciesItems[tr(rightrxnnames[rxn][var])];
						}

						products.value(0,tr(rightrxnnames[rxn][var])) += rightrxnstoichs[rxn][var];
					}

					QString srate = tr(rxnrates[rxn]);
					rate.setRowName(0, reactionHandle->name);
					rate.setColumnName(0, tr("rate"));
					rate.value(0,0) = srate;
					reactionHandle->textDataTable(tr("Rate equations")) = rate;
					reactionHandle->numericalDataTable(tr("Reactant stoichiometries")) = reactants;
					reactionHandle->numericalDataTable(tr("Product stoichiometries")) = products;

					for (int var=0; var < nodesIn.size(); ++var)
						reactionHandle->setNodeRole(nodesIn[var],"reactant");
				
					for (int var=0; var < nodesOut.size(); ++var)
						reactionHandle->setNodeRole(nodesOut[var],"product");

					itemsToInsert += reactionHandle;
				}
			
			int numSpecies = (int)getNumSymbolsOfType("__main",varSpecies);
			char ** speciesNames = getSymbolNamesOfType("__main",varSpecies);
			char ** speciesValues = getSymbolEquationsOfType("__main",varSpecies);
			for (int j=0; j < numSpecies; ++j)
			{
				bool ok;
				qreal x = QString(speciesValues[j]).toDouble(&ok);
				QString s(speciesNames[j]);
				
				if (!speciesItems.contains(s))
				{
					NodeHandle * handle = new NodeHandle(speciesFamily);
					handle->name = s;
					itemsToInsert << handle;

					speciesItems[s] = handle;
					symbolsInModule << handle->name;
				}

				if (ok)
				{
					speciesItems[s]->numericalData(tr("Initial Value"),speciesFamily->measurementUnit.name,speciesFamily->measurementUnit.property) = x;
					speciesItems[s]->numericalData(tr("Fixed")) = 0;
				}
			}
			
			int numConstSpecies = (int)getNumSymbolsOfType("__main",constSpecies);
			char ** constSpeciesNames = getSymbolNamesOfType("__main",constSpecies);
			char ** constSpeciesValues = getSymbolEquationsOfType("__main",constSpecies);
			for (int j=0; j < numConstSpecies; ++j)
			{
				bool ok;
				qreal x = QString(constSpeciesValues[j]).toDouble(&ok);
				QString s(constSpeciesNames[j]);
				if (!speciesItems.contains(s))
				{
					NodeHandle * handle = new NodeHandle(speciesFamily);
					handle->name = s;
					itemsToInsert << handle;

					symbolsInModule << handle->name;
					speciesItems[s] = handle;
				}
				if (ok)
				{
					speciesItems[s]->numericalData(tr("Initial Value")) = x;
					speciesItems[s]->numericalDataTable(tr("Initial Value")).setRowName(0, tr("concentration"));
					speciesItems[s]->numericalDataTable(tr("Initial Value")).setColumnName(0, tr("uM"));
					speciesItems[s]->numericalData(tr("Fixed")) = 1;
					speciesItems[s]->numericalDataTable(tr("Fixed")).setRowName(0, tr("fix"));
					speciesItems[s]->numericalDataTable(tr("Fixed")).setColumnName(0, tr("value"));
				}
			}
			
			QList<ItemHandle*> allHandles = itemsToInsert;
			
			if (moduleHandle)
				allHandles += moduleHandle;

			int numAssignments = (int)getNumSymbolsOfType("__main",varFormulas);
			char ** assignmentNames = getSymbolNamesOfType("__main",varFormulas);
			char ** assignmentValues = getSymbolEquationsOfType("__main",varFormulas);

			TextDataTable assgnsTable;
			
			for (int j=0; j < numAssignments; ++j)
			{
				QString x(assignmentValues[j]);
				assgnsTable.value(tr(assignmentNames[j]),0) = x;
				symbolsInModule << tr(assignmentNames[j]);
				if (moduleHandle && !moduleHandle->name.isEmpty())
					RenameCommand::findReplaceAllHandleData(allHandles,tr(assignmentNames[j]),moduleHandle->name + tr(".") + tr(assignmentNames[j]));
			}

			if (moduleHandle)
				moduleHandle->textDataTable(tr("Assignments")) = assgnsTable;

			int numEvents = (int)getNumEvents("__main");
			char ** eventNames = getEventNames("__main");

			TextDataTable eventsTable;
			for (int j=0; j < numEvents; ++j)
			{
				QString trigger(getTriggerForEvent("__main",j));
				substituteToEqualitySymbols(trigger);

				int n = (int)getNumAssignmentsForEvent("__main",j);

				for (int k=0; k < n; ++k)
				{
					QString x(getNthAssignmentVariableForEvent("__main",j,k));
					QString f(getNthAssignmentEquationForEvent("__main",j,k));
					eventsTable.value(trigger,0) = x + tr(" = ") + f;
				}
			}

			if (moduleHandle)
				moduleHandle->textDataTable(tr("Events")) = eventsTable;

			int numParams = (int)getNumSymbolsOfType("__main",constFormulas);
			char ** paramNames = getSymbolNamesOfType("__main",constFormulas);
			char ** paramValues = getSymbolEquationsOfType("__main",constFormulas);

			NumericalDataTable paramsTable;
			for (int j=0; j < numParams; ++j)
			{
				bool ok;
				qreal x = QString(paramValues[j]).toDouble(&ok);
				symbolsInModule << tr(paramNames[j]);
				if (ok)
				{
					paramsTable.value(tr(paramNames[j]),0) = x;
					if (moduleHandle && !moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
					{						
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					}
				}
				else
				if (moduleHandle)
				{
					if (!moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					moduleHandle->textDataTable(tr("Assignments")).value(tr(paramNames[j]),0) = paramValues[j];
				}
			}

			numParams = (int)getNumSymbolsOfType("__main",allSymbols);
			paramNames = getSymbolNamesOfType("__main",allSymbols);
			paramValues = getSymbolEquationsOfType("__main",allSymbols);

			for (int j=0; j < numParams; ++j)
			{
				if (!symbolsInModule.contains(tr(paramNames[j])))
				{
					bool ok;
					qreal x = QString(paramValues[j]).toDouble(&ok);
					if (!ok)
						x = 1.0;
					symbolsInModule << tr(paramNames[j]);
					paramsTable.value(tr(paramNames[j]),0) = x;
					if (moduleHandle && !moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
					{
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					}
				}
			}

			if (moduleHandle)
			{
				moduleHandle->numericalDataTable(tr("Parameters")) = paramsTable;

				if (!moduleHandle->name.isEmpty())	
					for (int j=0; j < itemsToInsert.size(); ++j)
						if (itemsToInsert[j] && !itemsToInsert[j]->parent && !itemsToInsert[j]->name.startsWith(moduleHandle->fullName()))
						{
							RenameCommand::findReplaceAllHandleData(allHandles,itemsToInsert[j]->name,moduleHandle->fullName() + tr(".") + itemsToInsert[j]->name);
						}
			}
		}

		freeAll();

		return itemsToInsert;
	}


	void AntimonyEditor::textChanged(TextEditor * editor, const QString&, const QString&, const QString&)
	{
		if (editor && editor->toPlainText().size() < 1000)
			parse(editor);
	}

	void AntimonyEditor::lineChanged(TextEditor *, int, const QString&)
	{
	}

	QList<ItemHandle*> AntimonyEditor::clone(const QList<ItemHandle*>& items)
	{
		return cloneHandles(items);
	}

	void AntimonyEditor::toolLoaded(Tool*)
	{
		static bool connected1 = false, connected2 = false;
		if (connected1 && connected2) return;

		if (!connected1 && mainWindow && mainWindow->tool(tr("SBML Tool")))
		{
			connected1 = true;
			QWidget * widget = mainWindow->tool(tr("SBML Tool"));
			SBMLImportExport * sbmlTool = static_cast<SBMLImportExport*>(widget);
			connect(sbmlTool,SIGNAL(getTextVersion(const QList<ItemHandle*>&, QString*)),
					this,SLOT(getTextVersion(const QList<ItemHandle*>&, QString*)));
		}
		if (!connected2 && mainWindow && mainWindow->tool(tr("Module Connection Tool")))
		{
			connected2 = true;
			QWidget * widget = mainWindow->tool(tr("Module Connection Tool"));
			ModuleTool * moduleTool = static_cast<ModuleTool*>(widget);
			connect(moduleTool,SIGNAL(getTextVersion(const QList<ItemHandle*>&, QString*)),
					this,SLOT(getTextVersion(const QList<ItemHandle*>&, QString*)));
		}
	}

	void AntimonyEditor::copyItems(GraphicsScene * scene, QList<QGraphicsItem*>& , QList<ItemHandle*>& handles)
	{
		if (scene && handles.size() > 0)
		{
			QClipboard * clipboard = QApplication::clipboard();
			if (clipboard)
			{
				clipboard->setText( getAntimonyScript(handles) );
			}
		}
	}

	void AntimonyEditor::appendScript(QString& s, const QList<ItemHandle*>& childHandles)
	{
		QRegExp regex(tr("\\.(?!\\d)"));
		QString s2,s3;
		QString allEqns;
		QStringList usedSymbols;
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && childHandles[j]->isA(tr("Module")))
			{
				s += tr("    ");
				s += childHandles[j]->name;
				s += tr(": ");
				s += childHandles[j]->name;
				s += tr(";\n\n");
			}

		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && !childHandles[j]->children.isEmpty() && childHandles[j]->isA(tr("compartment")))
			{
				if (s2.isEmpty())
					s2 += tr("compartment ");
				else
					s2 += tr(", ");
				s2 += childHandles[j]->fullName(tr("_"));

				if (!s2.isEmpty())
				{
					s3 = QString();
					for (int j=0; j < childHandles.size(); ++j)
						if (childHandles[j] && childHandles[j]->children.isEmpty())
						{
							if (s3.isEmpty())
								s3 += tr("species ");
							else
								s3 += tr(", ");
							s3 += childHandles[j]->fullName(tr("_"));
							if (childHandles[j]->parent && childHandles[j]->parent->isA(tr("compartment")))
								s3 += tr("in ") + childHandles[j]->parent->fullName(tr("_"));
						}

					s += s2;
					s += tr("\n");
					s += s3;
					s += tr("\n");
				}
			}
		
		s2 = QString();
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && childHandles[j]->isA(tr("molecule")) && !childHandles[j]->parentOfFamily("compartment"))
			{
				if (s2.isEmpty())
					s2 = tr("species ");
				else
					s2 += tr(", ");
				s2 += childHandles[j]->fullName(tr("_"));	
			}

		if (!s2.isEmpty())
		{
			s += s2;
			s += tr("\n");
		}
		
		QStringList assignmentRules, assignmentsCode;
		QString name;
		
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
            {
                name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasTextData(tr("Assignments")))
				{
					TextDataTable& assigns = childHandles[j]->textDataTable(tr("Assignments"));

					for (int r=0; r < assigns.rows(); ++r)
					{
						QString rule = assigns.value(r,0);
						rule.replace(regex,tr("_"));
						
						if (rule.size() < 2) continue;
						
						if (assigns.rowName(r).isEmpty() || assigns.rowName(r) == tr("self"))
						{
							assignmentsCode += name;
							assignmentRules += name;
						}
						else
						{
							if (!childHandles[j]->name.isEmpty())
							{
								assignmentsCode += name + tr("_") + assigns.rowName(r);
								assignmentRules += name + tr("_") + assigns.rowName(r);
							}
							else
							{
								assignmentsCode += assigns.rowName(r);
								assignmentRules += assigns.rowName(r);
							}
						}
						assignmentsCode += tr(" = ");
						assignmentsCode += rule;
						assignmentsCode += tr(";\n");

						allEqns += rule;
						usedSymbols << (name + tr(".") + assigns.rowName(r));
					}
				}
            }
		
		if (!assignmentRules.isEmpty())
		{
			s3 = tr("var    ") + assignmentRules.join(",");
			s += s3;
			s += tr("\n");
		}
		
		QString rate, species;
		QStringList lhs, rhs;
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
				name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->children.isEmpty() &&
					childHandles[j]->hasNumericalData(tr("Reactant Stoichiometries")) &&
					childHandles[j]->hasNumericalData(tr("Product Stoichiometries")) &&
					 childHandles[j]->hasTextData(tr("Rate equations")))
					{
						NumericalDataTable& reactants = childHandles[j]->numericalDataTable(tr("Reactant Stoichiometries"));
						NumericalDataTable& products = childHandles[j]->numericalDataTable(tr("Product Stoichiometries"));
						TextDataTable& rates = childHandles[j]->textDataTable(tr("Rate equations"));
						
						if (reactants.rows() < 1 && products.rows() < 1) continue;
						
						for (int r=0; r < rates.rows(); ++r)
						{
							lhs.clear();
							rhs.clear();
							rate = rates.value(r,0);
							
							for (int c=0; c < reactants.columns(); ++c)
								if (reactants.value(r,c) > 0)
								{
									species = reactants.columnName(c);
									lhs += species.replace(tr("."),tr("_"));
								}
							
							for (int c=0; c < products.columns(); ++c)
								if (products.value(r,c) > 0)
								{
									species = products.columnName(c);
									rhs += species.replace(tr("."),tr("_"));
								}

							if (rate.isEmpty() || (lhs.isEmpty() && rhs.isEmpty())) continue;

							rate.replace(regex,tr("_"));
                            allEqns += rate;
                            s += name;
							if (rates.rows() > 1)
								s += tr("_") + rates.rowName(r);
							s += tr(":    ");
							s += lhs.join(tr(" + "));
							s += tr(" -> ");
							s += rhs.join(tr(" + "));
							s += tr(";    ");
							s += rate + tr(";\n");
						}

					}
			}        

        for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
			    name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasNumericalData(tr("Parameters")))
				{
					NumericalDataTable& params = childHandles[j]->numericalDataTable(tr("Parameters"));
					
					QString pname;

					for (int r=0; r < params.rows(); ++r)
					{
						if (childHandles[j]->name.isEmpty())
							pname = params.rowName(r);
						else
							pname = name + tr("_") + params.rowName(r);

                        if (allEqns.contains(pname) && !usedSymbols.contains(pname))
                        {
                            s += pname;
                            s += tr(" = ");
                            s += QString::number(params.value(r,0));
                            s += tr(";\n");
                            usedSymbols << pname;
                        }
                     }
				}
			}

        for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
			    name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasNumericalData(tr("Initial Value")) && !name.isEmpty() && !assignmentRules.contains(name))
				{
					s += name;
					s += tr(" = ");
					s += QString::number(childHandles[j]->numericalData(tr("Initial Value")));
					s += tr(";\n");
				}
			}
		if (!assignmentsCode.isEmpty())
		{
			s += assignmentsCode.join("");
			s += tr("\n");
		}
	}

	QString AntimonyEditor::getAntimonyScript(const QList<ItemHandle*>& list)
	{
		QString s;

		QList<ItemHandle*> parentHandleChildren;		

		QList<ItemHandle*> visitedHandles, allHandles, childHandles, temp;
		ItemHandle * root;

		for (int i=0; i < list.size(); ++i)
		{	
			if (!allHandles.contains(list[i]))
				allHandles += list[i];
		}

		for (int i=0; i < allHandles.size(); ++i)
			if (allHandles[i]->isA(tr("Module")))
			{
				visitedHandles << allHandles[i];

				s += tr("Module ");
				s += allHandles[i]->name;
				s += tr("()\n");
				childHandles = allHandles[i]->children;

				visitedHandles << childHandles;

				appendScript(s,childHandles);

				s += tr("end;\n\n");
			}

		childHandles.clear();

		for (int i=0; i < allHandles.size(); ++i)
			if (allHandles[i] && !visitedHandles.contains(allHandles[i]))
				childHandles << allHandles[i];

		appendScript(s,childHandles);

		return s;
	}

	void AntimonyEditor::getTextVersion(const QList<ItemHandle*>& items, QString* text)
	{
		if (text)
			(*text) = getAntimonyScript(items);
	}
	
	void AntimonyEditor::getItemsFromFile(QList<ItemHandle*>& items, QList<QGraphicsItem*>& , const QString& filename, ItemHandle * root)
	{
		if (!items.isEmpty()) return;

		NetworkWindow * window = currentWindow();
		
		QString s;
		QFile file(filename);
		if (file.open(QFile::ReadOnly | QFile::Text))
	    {
	        s = file.readAll();
	        file.close();
	    }

		items = parse(s, root);
	}
}

/*
extern "C" TINKERCELLEXPORT void loadTCTool(Tinkercell::MainWindow * main)
{
	if (!main) return;

	Tinkercell::AntimonyEditor * antimonyEditor = new Tinkercell::AntimonyEditor;
	main->addTool(antimonyEditor);

}*/

=======
/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 A tool that allows users to construct models using Antimony scripts in the TextEditor

****************************************************************************/
#include <iostream>
#include <QClipboard>
#include "TextEditor.h"
#include "NetworkHandle.h"
#include "NetworkWindow.h"
#include "UndoCommands.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ConsoleWindow.h"
#include "LoadSaveTool.h"
#include "AntimonyEditor.h"
#include "ModelSummaryTool.h"
#include "ModuleTool.h"
#include "SBMLImportExport.h"
#include "CloneItems.h"
#include <QToolButton>
#include <QRegExp>
#include <QFile>
#include <QPair>
#include <QRegExp>
#include <QtDebug>
#include <QSemaphore>
#include <QFileDialog>
#include "antimony_api.h"

namespace Tinkercell
{
	AntimonyEditor::AntimonyEditor() : TextParser(tr("Antimony Parser"))
	{
		scriptDisplayWindow = new CodeEditor(this);
		icon = QPixmap(tr(":/images/antimony.png"));
	}

	bool AntimonyEditor::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);
		if (mainWindow)
		{
			mainWindow->addParser(this);

			connect(mainWindow,SIGNAL(copyItems(GraphicsScene *, QList<QGraphicsItem*>&, QList<ItemHandle*>&)),
						this,SLOT(copyItems(GraphicsScene *, QList<QGraphicsItem*>&, QList<ItemHandle*>&)));

			connect(mainWindow,SIGNAL(windowChanged(NetworkWindow*,NetworkWindow*)),
						this,SLOT(windowChanged(NetworkWindow*,NetworkWindow*)));
			
			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),
						this,SLOT(toolLoaded(Tool*)));

			connect(mainWindow,SIGNAL(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)),
						this,SLOT(getItemsFromFile(QList<ItemHandle*>&, QList<QGraphicsItem*>&, const QString&,ItemHandle*)));

			toolLoaded(0);

		}
		return false;
	}
	
	void AntimonyEditor::loadNetwork(const QString& filename, bool * b)
	{
		if (b && (*b)) return;

		QFile file(filename);
		QString modelString;

		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			modelString = file.readAll();
			file.close();
		}
		else
		{
			modelString = filename;	
		}

		ItemHandle root;
		QList<ItemHandle*> itemsToInsert = parse(modelString, &root);

		//if (!itemsToInsert.isEmpty())
		{
			TextEditor * editor = mainWindow->newTextEditor();
			(*editor->globalHandle()) = root;
			editor->setText(modelString);
			editor->setItems(itemsToInsert);
		}
	}

	void AntimonyEditor::windowChanged(NetworkWindow*,NetworkWindow * window)
	{
		if (window && window->editor && !visitedWindows.contains(window) && TextParser::currentParser() == this)
		{
			visitedWindows[window] = true;

			AntimonySyntaxHighlighter * as = new AntimonySyntaxHighlighter(window->editor->document());
			connect(this,SIGNAL(validSyntax(bool)),as,SLOT(setValid(bool)));

			QToolButton * button = new QToolButton(this);
			button->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
			button->setIcon(QIcon(":/images/antimony.png"));
			button->setIconSize(QSize(30,30));
			button->setText(tr("Compile script"));
			button->setToolTip(tr("interpret script using Antimony language"));
			connect(button,SIGNAL(pressed()),this,SLOT(parse()));
			
			QDockWidget * dock = new QDockWidget;
			dock->setMaximumWidth(150);
			QVBoxLayout * layout = new QVBoxLayout;
			QWidget * widget = new QWidget;
			layout->setContentsMargins(5,8,5,5);
			layout->setSpacing(12);
			widget->setLayout(layout);
			layout->addWidget(button,1,Qt::AlignTop);
			widget->setPalette(QPalette(QColor(255,255,255)));
			widget->setAutoFillBackground (true);
			dock->setWidget(widget);
			window->addDockWidget(Qt::RightDockWidgetArea,dock);
			/*
			button = new QToolButton;
			button->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
			button->setIcon(QIcon(":/images/module.png"));
			button->setIconSize(QSize(30,30));
			button->setText(tr("Export modules"));
			button->setToolTip(tr("export module(s) to the last graphics window"));*/
		}
	}

	void AntimonyEditor::parse()
	{
		parse(currentTextEditor());
	}

	void AntimonyEditor::parse(TextEditor * editor)
	{
		if (!editor) return;

		QString modelString = editor->toPlainText() + tr("\n");

		ItemHandle * root = editor->localHandle();
		if (!root)
			root = editor->globalHandle();

		QList<ItemHandle*> itemsToInsert = parse(modelString, root);

		if (!itemsToInsert.isEmpty())
		{
			editor->setItems(itemsToInsert);
		}
	}

	static void substituteToEqualitySymbols(QString& s)
	{
		s.replace(QRegExp("gt\\((.*),(.*)\\)"), "\\1>\\2");
		s.replace(QRegExp("lt\\((.*),(.*)\\)"), "\\1<\\2");
		s.replace(QRegExp("ge\\((.*),(.*)\\)"), "\\1>=\\2");
		s.replace(QRegExp("le\\((.*),(.*)\\)"), "\\1<=\\2");
		s.replace(QRegExp("ne\\((.*),(.*)\\)"), "\\1!=\\2");
		s.replace(QRegExp("eq\\((.*),(.*)\\)"), "\\1=\\2");
	}

	QList<ItemHandle*> AntimonyEditor::parse(QString& modelString, ItemHandle * moduleHandle)
	{
		long ok = loadString(modelString.toAscii().data());
		
		if (ok < 0)
		{
			if (console() && !moduleHandle && currentTextEditor())
				console()->error(tr(getLastError()));
			emit validSyntax(false);
			return QList<ItemHandle*>();
		}
		else
		{
			modelString = tr(getAntimonyString(NULL));
			emit validSyntax(true);
		}

		QString appDir = QCoreApplication::applicationDirPath();

		ConnectionFamily * biochemicalFamily = LoadSaveTool::getConnectionFamily(tr("biochemical reaction"));
		NodeFamily * speciesFamily = LoadSaveTool::getNodeFamily(tr("molecule"));

		if (!biochemicalFamily || !speciesFamily)
		{
			return QList<ItemHandle*>();
		}

		//load

		int nummods = (int)getNumModules();
		char** modnames = getModuleNames();

		QList<ItemHandle*> itemsToInsert;

		//for (int i=0; i < nummods; ++i)
		{
			QStringList symbolsInModule;
			
			/*ItemHandle * moduleHandle = 0;
			moduleHandle = new NodeHandle(moduleFamily);
			moduleHandle->name = QString(moduleName);
			itemsToInsert += moduleHandle;

			if (QString(modnames[i]) == tr("__main"))
				moduleHandle->name = tr("main");

			symbolsInModule << moduleHandle->name;

			QList<ItemHandle*> handlesInModule;*/
			QHash<QString,NodeHandle*> speciesItems;
			QHash<QString,ConnectionHandle*> reactionItems;

			char ***leftrxnnames = getReactantNames("__main");
			char ***rightrxnnames = getProductNames("__main");
			char **rxnnames = getSymbolNamesOfType("__main",allReactions);
			char **rxnrates = getReactionRates("__main");

			double **leftrxnstoichs = getReactantStoichiometries("__main");
			double **rightrxnstoichs = getProductStoichiometries("__main");

			int numrxn = (int)getNumReactions("__main");

			for (int rxn=0; rxn < numrxn; ++rxn)
				if (!reactionItems.contains(tr(rxnnames[rxn])))
				{
					int numReactants = getNumReactants("__main",rxn);
					int numProducts = getNumProducts("__main",rxn);

					QList<NodeHandle*> nodesIn, nodesOut;
					NumericalDataTable reactants, products;
					TextDataTable rate;
					QStringList columnNames;

					for (int var=0; var<numReactants; ++var)
					{
						if (!columnNames.contains(tr(leftrxnnames[rxn][var])))
						{
							columnNames << tr(leftrxnnames[rxn][var]);
						}
					}
				
					reactants.resize(1,columnNames.size());
					reactants.setColumnNames(columnNames);
					columnNames.clear();
				
					for (int var=0; var<numProducts; ++var)
					{
						if (!columnNames.contains(tr(rightrxnnames[rxn][var])))
						{
							columnNames << tr(rightrxnnames[rxn][var]);
						}
					}

					products.resize(1,columnNames.size());
					products.setColumnNames(columnNames);

					for (int j=0; j < columnNames.size(); ++j)
					{
						products.value(0,j) = 0;
					}

					rate.resize(1,1);
					ConnectionHandle * reactionHandle = new ConnectionHandle(biochemicalFamily);

					reactionHandle->name = rxnnames[rxn];
					reactants.setRowName(0, reactionHandle->name);
					products.setRowName(0, reactionHandle->name);
					symbolsInModule << reactionHandle->name;

					for (int var=0; var<numReactants; ++var)
					{
						NodeHandle * handle = 0;
						QString name = tr(leftrxnnames[rxn][var]);

						if (!speciesItems.contains(name))
						{
							handle = new NodeHandle(speciesFamily);
							handle->name = name;
							itemsToInsert  += handle;
						
							symbolsInModule << name;
							speciesItems[name] = handle;
							nodesIn << handle;
						}
						else
						{
							handle = speciesItems[tr(leftrxnnames[rxn][var])];
						}
						reactants.value(0,tr(leftrxnnames[rxn][var])) += leftrxnstoichs[rxn][var];
					}

					for (int var=0; var<numProducts; var++)
					{
						NodeHandle * handle = 0;
						QString name = tr(rightrxnnames[rxn][var]);						
						if (!speciesItems.contains(name))
						{
							handle = new NodeHandle(speciesFamily);
							handle->name = name;
							itemsToInsert += handle;

							speciesItems[name] = handle;
							symbolsInModule << name;
							nodesOut << handle;
						}
						else
						{
							handle = speciesItems[tr(rightrxnnames[rxn][var])];
						}

						products.value(0,tr(rightrxnnames[rxn][var])) += rightrxnstoichs[rxn][var];
					}

					QString srate = tr(rxnrates[rxn]);
					rate.setRowName(0, reactionHandle->name);
					rate.setColumnName(0, tr("rate"));
					rate.value(0,0) = srate;
					reactionHandle->textDataTable(tr("Rate equations")) = rate;
					reactionHandle->numericalDataTable(tr("Reactant stoichiometries")) = reactants;
					reactionHandle->numericalDataTable(tr("Product stoichiometries")) = products;

					for (int var=0; var < nodesIn.size(); ++var)
						reactionHandle->setNodeRole(nodesIn[var],"reactant");
				
					for (int var=0; var < nodesOut.size(); ++var)
						reactionHandle->setNodeRole(nodesOut[var],"product");

					itemsToInsert += reactionHandle;
				}
			
			int numSpecies = (int)getNumSymbolsOfType("__main",varSpecies);
			char ** speciesNames = getSymbolNamesOfType("__main",varSpecies);
			char ** speciesValues = getSymbolEquationsOfType("__main",varSpecies);
			for (int j=0; j < numSpecies; ++j)
			{
				bool ok;
				qreal x = QString(speciesValues[j]).toDouble(&ok);
				QString s(speciesNames[j]);
				
				if (!speciesItems.contains(s))
				{
					NodeHandle * handle = new NodeHandle(speciesFamily);
					handle->name = s;
					itemsToInsert << handle;

					speciesItems[s] = handle;
					symbolsInModule << handle->name;
				}

				if (ok)
				{
					speciesItems[s]->numericalData(tr("Initial Value"),speciesFamily->measurementUnit.name,speciesFamily->measurementUnit.property) = x;
					speciesItems[s]->numericalData(tr("Fixed")) = 0;
				}
			}
			
			int numConstSpecies = (int)getNumSymbolsOfType("__main",constSpecies);
			char ** constSpeciesNames = getSymbolNamesOfType("__main",constSpecies);
			char ** constSpeciesValues = getSymbolEquationsOfType("__main",constSpecies);
			for (int j=0; j < numConstSpecies; ++j)
			{
				bool ok;
				qreal x = QString(constSpeciesValues[j]).toDouble(&ok);
				QString s(constSpeciesNames[j]);
				if (!speciesItems.contains(s))
				{
					NodeHandle * handle = new NodeHandle(speciesFamily);
					handle->name = s;
					itemsToInsert << handle;

					symbolsInModule << handle->name;
					speciesItems[s] = handle;
				}
				if (ok)
				{
					speciesItems[s]->numericalData(tr("Initial Value")) = x;
					speciesItems[s]->numericalDataTable(tr("Initial Value")).setRowName(0, tr("concentration"));
					speciesItems[s]->numericalDataTable(tr("Initial Value")).setColumnName(0, tr("uM"));
					speciesItems[s]->numericalData(tr("Fixed")) = 1;
					speciesItems[s]->numericalDataTable(tr("Fixed")).setRowName(0, tr("fix"));
					speciesItems[s]->numericalDataTable(tr("Fixed")).setColumnName(0, tr("value"));
				}
			}
			
			QList<ItemHandle*> allHandles = itemsToInsert;
			
			if (moduleHandle)
				allHandles += moduleHandle;

			int numAssignments = (int)getNumSymbolsOfType("__main",varFormulas);
			char ** assignmentNames = getSymbolNamesOfType("__main",varFormulas);
			char ** assignmentValues = getSymbolEquationsOfType("__main",varFormulas);

			TextDataTable assgnsTable;
			
			for (int j=0; j < numAssignments; ++j)
			{
				QString x(assignmentValues[j]);
				assgnsTable.value(tr(assignmentNames[j]),0) = x;
				symbolsInModule << tr(assignmentNames[j]);
				if (moduleHandle && !moduleHandle->name.isEmpty())
					RenameCommand::findReplaceAllHandleData(allHandles,tr(assignmentNames[j]),moduleHandle->name + tr(".") + tr(assignmentNames[j]));
			}

			if (moduleHandle)
				moduleHandle->textDataTable(tr("Assignments")) = assgnsTable;

			int numEvents = (int)getNumEvents("__main");
			char ** eventNames = getEventNames("__main");

			TextDataTable eventsTable;
			for (int j=0; j < numEvents; ++j)
			{
				QString trigger(getTriggerForEvent("__main",j));
				substituteToEqualitySymbols(trigger);

				int n = (int)getNumAssignmentsForEvent("__main",j);

				for (int k=0; k < n; ++k)
				{
					QString x(getNthAssignmentVariableForEvent("__main",j,k));
					QString f(getNthAssignmentEquationForEvent("__main",j,k));
					eventsTable.value(trigger,0) = x + tr(" = ") + f;
				}
			}

			if (moduleHandle)
				moduleHandle->textDataTable(tr("Events")) = eventsTable;

			int numParams = (int)getNumSymbolsOfType("__main",constFormulas);
			char ** paramNames = getSymbolNamesOfType("__main",constFormulas);
			char ** paramValues = getSymbolEquationsOfType("__main",constFormulas);

			NumericalDataTable paramsTable;
			for (int j=0; j < numParams; ++j)
			{
				bool ok;
				qreal x = QString(paramValues[j]).toDouble(&ok);
				symbolsInModule << tr(paramNames[j]);
				if (ok)
				{
					paramsTable.value(tr(paramNames[j]),0) = x;
					if (moduleHandle && !moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
					{						
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					}
				}
				else
				if (moduleHandle)
				{
					if (!moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					moduleHandle->textDataTable(tr("Assignments")).value(tr(paramNames[j]),0) = paramValues[j];
				}
			}

			numParams = (int)getNumSymbolsOfType("__main",allSymbols);
			paramNames = getSymbolNamesOfType("__main",allSymbols);
			paramValues = getSymbolEquationsOfType("__main",allSymbols);

			for (int j=0; j < numParams; ++j)
			{
				if (!symbolsInModule.contains(tr(paramNames[j])))
				{
					bool ok;
					qreal x = QString(paramValues[j]).toDouble(&ok);
					if (!ok)
						x = 1.0;
					symbolsInModule << tr(paramNames[j]);
					paramsTable.value(tr(paramNames[j]),0) = x;
					if (moduleHandle && !moduleHandle->name.isEmpty() && !tr(paramNames[j]).startsWith(moduleHandle->fullName("_")))
					{
						RenameCommand::findReplaceAllHandleData(allHandles,tr(paramNames[j]),moduleHandle->fullName() + tr(".") + tr(paramNames[j]));
					}
				}
			}

			if (moduleHandle)
			{
				moduleHandle->numericalDataTable(tr("Parameters")) = paramsTable;

				if (!moduleHandle->name.isEmpty())	
					for (int j=0; j < itemsToInsert.size(); ++j)
						if (itemsToInsert[j] && !itemsToInsert[j]->parent && !itemsToInsert[j]->name.startsWith(moduleHandle->fullName()))
						{
							RenameCommand::findReplaceAllHandleData(allHandles,itemsToInsert[j]->name,moduleHandle->fullName() + tr(".") + itemsToInsert[j]->name);
						}
			}
		}

		freeAll();

		return itemsToInsert;
	}


	void AntimonyEditor::textChanged(TextEditor * editor, const QString&, const QString&, const QString&)
	{
		if (editor && editor->toPlainText().size() < 1000)
			parse(editor);
	}

	void AntimonyEditor::lineChanged(TextEditor *, int, const QString&)
	{
	}

	QList<ItemHandle*> AntimonyEditor::clone(const QList<ItemHandle*>& items)
	{
		return cloneHandles(items);
	}

	void AntimonyEditor::toolLoaded(Tool*)
	{
		static bool connected1 = false, connected2 = false;
		if (connected1 && connected2) return;

		if (!connected1 && mainWindow && mainWindow->tool(tr("SBML Tool")))
		{
			connected1 = true;
			QWidget * widget = mainWindow->tool(tr("SBML Tool"));
			SBMLImportExport * sbmlTool = static_cast<SBMLImportExport*>(widget);
			connect(sbmlTool,SIGNAL(getTextVersion(const QList<ItemHandle*>&, QString*)),
					this,SLOT(getTextVersion(const QList<ItemHandle*>&, QString*)));
		}
		if (!connected2 && mainWindow && mainWindow->tool(tr("Module Connection Tool")))
		{
			connected2 = true;
			QWidget * widget = mainWindow->tool(tr("Module Connection Tool"));
			ModuleTool * moduleTool = static_cast<ModuleTool*>(widget);
			connect(moduleTool,SIGNAL(getTextVersion(const QList<ItemHandle*>&, QString*)),
					this,SLOT(getTextVersion(const QList<ItemHandle*>&, QString*)));
		}
	}

	void AntimonyEditor::copyItems(GraphicsScene * scene, QList<QGraphicsItem*>& , QList<ItemHandle*>& handles)
	{
		if (scene && handles.size() > 0)
		{
			QClipboard * clipboard = QApplication::clipboard();
			if (clipboard)
			{
				clipboard->setText( getAntimonyScript(handles) );
			}
		}
	}

	void AntimonyEditor::appendScript(QString& s, const QList<ItemHandle*>& childHandles)
	{
		QRegExp regex(tr("\\.(?!\\d)"));
		QString s2,s3;
		QString allEqns;
		QStringList usedSymbols;
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && childHandles[j]->isA(tr("Module")))
			{
				s += tr("    ");
				s += childHandles[j]->name;
				s += tr(": ");
				s += childHandles[j]->name;
				s += tr(";\n\n");
			}

		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && !childHandles[j]->children.isEmpty() && childHandles[j]->isA(tr("compartment")))
			{
				if (s2.isEmpty())
					s2 += tr("compartment ");
				else
					s2 += tr(", ");
				s2 += childHandles[j]->fullName(tr("_"));

				if (!s2.isEmpty())
				{
					s3 = QString();
					for (int j=0; j < childHandles.size(); ++j)
						if (childHandles[j] && childHandles[j]->children.isEmpty())
						{
							if (s3.isEmpty())
								s3 += tr("species ");
							else
								s3 += tr(", ");
							s3 += childHandles[j]->fullName(tr("_"));
							if (childHandles[j]->parent && childHandles[j]->parent->isA(tr("compartment")))
								s3 += tr("in ") + childHandles[j]->parent->fullName(tr("_"));
						}

					s += s2;
					s += tr("\n");
					s += s3;
					s += tr("\n");
				}
			}
		
		s2 = QString();
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j] && childHandles[j]->isA(tr("molecule")) && !childHandles[j]->parentOfFamily("compartment"))
			{
				if (s2.isEmpty())
					s2 = tr("species ");
				else
					s2 += tr(", ");
				s2 += childHandles[j]->fullName(tr("_"));	
			}

		if (!s2.isEmpty())
		{
			s += s2;
			s += tr("\n");
		}
		
		QStringList assignmentRules, assignmentsCode;
		QString name;
		
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
            {
                name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasTextData(tr("Assignments")))
				{
					TextDataTable& assigns = childHandles[j]->textDataTable(tr("Assignments"));

					for (int r=0; r < assigns.rows(); ++r)
					{
						QString rule = assigns.value(r,0);
						rule.replace(regex,tr("_"));
						
						if (rule.size() < 2) continue;
						
						if (assigns.rowName(r).isEmpty() || assigns.rowName(r) == tr("self"))
						{
							assignmentsCode += name;
							assignmentRules += name;
						}
						else
						{
							if (!childHandles[j]->name.isEmpty())
							{
								assignmentsCode += name + tr("_") + assigns.rowName(r);
								assignmentRules += name + tr("_") + assigns.rowName(r);
							}
							else
							{
								assignmentsCode += assigns.rowName(r);
								assignmentRules += assigns.rowName(r);
							}
						}
						assignmentsCode += tr(" = ");
						assignmentsCode += rule;
						assignmentsCode += tr(";\n");

						allEqns += rule;
						usedSymbols << (name + tr(".") + assigns.rowName(r));
					}
				}
            }
		
		if (!assignmentRules.isEmpty())
		{
			s3 = tr("var    ") + assignmentRules.join(",");
			s += s3;
			s += tr("\n");
		}
		
		QString rate, species;
		QStringList lhs, rhs;
		for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
				name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->children.isEmpty() &&
					childHandles[j]->hasNumericalData(tr("Reactant Stoichiometries")) &&
					childHandles[j]->hasNumericalData(tr("Product Stoichiometries")) &&
					 childHandles[j]->hasTextData(tr("Rate equations")))
					{
						NumericalDataTable& reactants = childHandles[j]->numericalDataTable(tr("Reactant Stoichiometries"));
						NumericalDataTable& products = childHandles[j]->numericalDataTable(tr("Product Stoichiometries"));
						TextDataTable& rates = childHandles[j]->textDataTable(tr("Rate equations"));
						
						if (reactants.rows() < 1 && products.rows() < 1) continue;
						
						for (int r=0; r < rates.rows(); ++r)
						{
							lhs.clear();
							rhs.clear();
							rate = rates.value(r,0);
							
							for (int c=0; c < reactants.columns(); ++c)
								if (reactants.value(r,c) > 0)
								{
									species = reactants.columnName(c);
									lhs += species.replace(tr("."),tr("_"));
								}
							
							for (int c=0; c < products.columns(); ++c)
								if (products.value(r,c) > 0)
								{
									species = products.columnName(c);
									rhs += species.replace(tr("."),tr("_"));
								}

							if (rate.isEmpty() || (lhs.isEmpty() && rhs.isEmpty())) continue;

							rate.replace(regex,tr("_"));
                            allEqns += rate;
                            s += name;
							if (rates.rows() > 1)
								s += tr("_") + rates.rowName(r);
							s += tr(":    ");
							s += lhs.join(tr(" + "));
							s += tr(" -> ");
							s += rhs.join(tr(" + "));
							s += tr(";    ");
							s += rate + tr(";\n");
						}

					}
			}        

        for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
			    name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasNumericalData(tr("Parameters")))
				{
					NumericalDataTable& params = childHandles[j]->numericalDataTable(tr("Parameters"));
					
					QString pname;

					for (int r=0; r < params.rows(); ++r)
					{
						if (childHandles[j]->name.isEmpty())
							pname = params.rowName(r);
						else
							pname = name + tr("_") + params.rowName(r);

                        if (allEqns.contains(pname) && !usedSymbols.contains(pname))
                        {
                            s += pname;
                            s += tr(" = ");
                            s += QString::number(params.value(r,0));
                            s += tr(";\n");
                            usedSymbols << pname;
                        }
                     }
				}
			}

        for (int j=0; j < childHandles.size(); ++j)
			if (childHandles[j])
			{
			    name = childHandles[j]->fullName(tr("_"));
				if (childHandles[j]->hasNumericalData(tr("Initial Value")) && !name.isEmpty() && !assignmentRules.contains(name))
				{
					s += name;
					s += tr(" = ");
					s += QString::number(childHandles[j]->numericalData(tr("Initial Value")));
					s += tr(";\n");
				}
			}
		if (!assignmentsCode.isEmpty())
		{
			s += assignmentsCode.join("");
			s += tr("\n");
		}
	}

	QString AntimonyEditor::getAntimonyScript(const QList<ItemHandle*>& list)
	{
		QString s;

		QList<ItemHandle*> parentHandleChildren;		

		QList<ItemHandle*> visitedHandles, allHandles, childHandles, temp;
		ItemHandle * root;

		for (int i=0; i < list.size(); ++i)
		{	
			if (!allHandles.contains(list[i]))
				allHandles += list[i];
		}

		for (int i=0; i < allHandles.size(); ++i)
			if (allHandles[i]->isA(tr("Module")))
			{
				visitedHandles << allHandles[i];

				s += tr("Module ");
				s += allHandles[i]->name;
				s += tr("()\n");
				childHandles = allHandles[i]->children;

				visitedHandles << childHandles;

				appendScript(s,childHandles);

				s += tr("end;\n\n");
			}

		childHandles.clear();

		for (int i=0; i < allHandles.size(); ++i)
			if (allHandles[i] && !visitedHandles.contains(allHandles[i]))
				childHandles << allHandles[i];

		appendScript(s,childHandles);

		return s;
	}

	void AntimonyEditor::getTextVersion(const QList<ItemHandle*>& items, QString* text)
	{
		if (text)
			(*text) = getAntimonyScript(items);
	}
	
	void AntimonyEditor::getItemsFromFile(QList<ItemHandle*>& items, QList<QGraphicsItem*>& , const QString& filename, ItemHandle * root)
	{
		if (!items.isEmpty()) return;

		NetworkWindow * window = currentWindow();
		
		QString s;
		QFile file(filename);
		if (file.open(QFile::ReadOnly | QFile::Text))
	    {
	        s = file.readAll();
	        file.close();
	    }

		items = parse(s, root);
	}
}

/*
extern "C" TINKERCELLEXPORT void loadTCTool(Tinkercell::MainWindow * main)
{
	if (!main) return;

	Tinkercell::AntimonyEditor * antimonyEditor = new Tinkercell::AntimonyEditor;
	main->addTool(antimonyEditor);

}*/

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
