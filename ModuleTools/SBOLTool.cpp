/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT

****************************************************************************/
#include <math.h>
#include <QRegExp>
#include <QProcess>
#include "ItemFamily.h"
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
#include "TreeButton.h"
#include "SBOLTool.h"
#include "GlobalSettings.h"
#include <sstream>
#include <functional>
#include <fstream>

#define SBOL_PROMOTER 101
#define SBOL_ASSEMBLY_SCAR 102
#define SBOL_BLUNT_RESTRICTION_SITE 103
#define SBOL_CDS 104
#define SBOL_FIVE_PRIME_OVERHANG 105
#define SBOL_FIVE_PRIME_STICKY_RESTRICTION_SITE 106
#define SBOL_INSULATOR 107
#define SBOL_OPERATOR 108
#define SBOL_PRIMER_BINDING_SITE 109
#define SBOL_PROTEASE_SITE 110
#define SBOL_PROTEIN_STABILITY_ELEMENT 111
#define SBOL_RESTRICTION_ENZYME_RECOGNITION_SITE 112
#define SBOL_RIBONUCLEASE_SITE 113
#define SBOL_RIBOSOME_ENTRY_SITE 114
#define SBOL_RNA_STABILITY_ELEMENT 115
#define SBOL_SIGNATURE 116
#define SBOL_TERMINATOR 117
#define SBOL_THREE_PRIME_OVERHANG 118
#define SBOL_THREE_PRIME_STICKY_RESTRICTION_SITE 119
#define SBOL_USER_DEFINED 120
#define SBOL_ORIGIN_OF_REPLICATION 121
#define SSTR( x ) dynamic_cast < std::ostringstream & > (( std::ostringstream() << std::dec << x ) ).str()

//need to do something about it from here
extern "C"{
#include "sbol.h"
}

static Document * sbol_doc;
static DNAComponent * head_dc;
//to here

namespace Tinkercell
{
    SBOLTool::SBOLTool() : Tool(tr("SBOL Visual Tool"),tr("Module tools"))
    {
        setPalette(QPalette(QColor(255,255,255,255)));
        setAutoFillBackground(true);

        QGridLayout * layout1 = new QGridLayout;
        QGridLayout * layout2 = new QGridLayout;
        QGridLayout * layout3 = new QGridLayout;

        layout1->addWidget(new QLabel(tr("uri")),0,0);
        layout1->addWidget(new QLabel(tr("displayId")),1,0);
        layout1->addWidget(new QLabel (tr("name")),2,0);
        layout1->addWidget(new QLabel(tr("description")),3,0);
        layout1->addWidget(new QLabel(tr("type")),4,0);

        layout1->addWidget(DC_uri = new QLineEdit,0,1);
        layout1->addWidget(DC_displayId = new QLineEdit,1,1);
        layout1->addWidget(DC_name = new QLineEdit,2,1);
        layout1->addWidget(DC_description = new QLineEdit,3,1);
        layout1->addWidget(DC_type = new QLineEdit, 4,1);

        layout2->addWidget(new QLabel(tr("uri")),0,0);
        layout2->addWidget(new QLabel(tr("nucleotides")),1,0);

        layout2->addWidget(DS_uri = new QLineEdit,0,1);
        layout2->addWidget(DS_nucleotides = new QLineEdit,1,1);

        layout3->addWidget(new QLabel(tr("uri")),0,0);
        layout3->addWidget(new QLabel(tr("bioStart")),1,0);
        layout3->addWidget(new QLabel(tr("bioEnd")),2,0);
        layout3->addWidget(new QLabel(tr("strand")),3,0);

        layout3->addWidget(SA_uri = new QLineEdit, 0,1);
        layout3->addWidget(SA_bioStart = new QLineEdit ,1,1);
        layout3->addWidget(SA_bioEnd = new QLineEdit, 2,1);
        layout3->addWidget(SA_strand = new QLineEdit,3,1);

        groupBox1 = new QGroupBox(tr("DNA Component"));
        groupBox1->setLayout(layout1);

        groupBox2 = new QGroupBox(tr("DNA Sequence"));
        groupBox2->setLayout(layout2);

        groupBox3 = new QGroupBox(tr("Sequence Annotation"));
        groupBox3->setLayout(layout3);

        QVBoxLayout * layout4 = new QVBoxLayout;
        layout4->addWidget(groupBox1);
        layout4->addWidget(groupBox2);
        layout4->addWidget(groupBox3);
        layout4->addWidget(addDS = new QPushButton("Add DNASequence", this));
        layout4->addWidget(delDS = new QPushButton("Delete DNASequence", this));
        layout4->addWidget(addSA = new QPushButton("Add SequenceAnnotation", this));
        layout4->addWidget(delSA = new QPushButton("Delete SequenceAnnotation", this));
        connect(addSA, SIGNAL(pressed()), this, SLOT(showSA()));
        connect(addDS, SIGNAL(pressed()), this, SLOT(showDS()));
        connect(delSA, SIGNAL(pressed()), this, SLOT(hideSA()));
        connect(delDS, SIGNAL(pressed()), this, SLOT(hideDS()));

        //for DNAComponent

        connect(DC_uri,SIGNAL(editingFinished()), this, SLOT(dc_uriChanged()));
        connect(DC_displayId,SIGNAL(editingFinished()), this, SLOT(dc_displayidChanged()));
        connect(DC_name,SIGNAL(editingFinished()), this, SLOT(dc_nameChanged()));
        connect(DC_description,SIGNAL(editingFinished()), this, SLOT(dc_descriptionChanged()));

        connect(DS_nucleotides,SIGNAL(editingFinished()),this,SLOT(ds_nucleotidesChanged()));
        connect(DS_uri,SIGNAL(editingFinished()),this,SLOT(ds_uriChanged()));

        connect(SA_uri,SIGNAL(editingFinished()),this,SLOT(sa_uriChanged()));
        connect(SA_bioStart,SIGNAL(editingFinished()),this,SLOT(sa_bioStartChanged()));
        connect(SA_bioEnd,SIGNAL(editingFinished()),this,SLOT(sa_bioEndChanged()));
        connect(SA_strand,SIGNAL(editingFinished()),this,SLOT(sa_strandChanged()));

        /*
        void sa_uriChanged();
        void sa_bioStartChanged();
        void sa_bioEndChanged();
        void sa_strandChanged();
*/

        groupBox1->show();
        groupBox2->hide();
        groupBox3->hide();
        delSA->hide();
        delDS->hide();
        setLayout(layout4);
        show();
        mode = 0;

    }

    template<typename T>
    void SBOLTool::dc_str_item_changed(T call_function, QLineEdit* cur_item)
    {
        DNAComponent * cur_dc;
        GraphicsScene * scene = currentScene();
        if (!scene || scene->selected().size() != 1)
            {

                if (cur_dc = getDNAComponent(sbol_doc, DC_uri->text().toAscii()))
                    {
                        (call_function)(cur_dc, cur_item->text().toAscii());
                    }
                return;
            }

        QGraphicsItem * selectedItem = scene->selected()[0];

        if(!selectedItem) return;


        NodeGraphicsItem *cur_node = NodeGraphicsItem::cast(selectedItem);

        cur_dc= getDNAComponent(sbol_doc, cur_node->name.toAscii());

        (call_function)(cur_dc, cur_item->text().toAscii());
        if(call_function == &setDNAComponentURI)
            {
                cur_node->name = DC_uri->text();
            }
    }

    template<typename T>
    void SBOLTool::ds_str_item_changed(T call_function, QLineEdit* cur_item)
    {
        DNAComponent * cur_dc;
        GraphicsScene * scene = currentScene();
        if (!scene || scene->selected().size() != 1) return;

        QGraphicsItem * selectedItem = scene->selected()[0];

        if(!selectedItem) return;

        NodeGraphicsItem *cur_node = NodeGraphicsItem::cast(selectedItem);

        cur_dc= getDNAComponent(sbol_doc, cur_node->name.toAscii());
        DNASequence *cur_ds;

        if(cur_ds = getDNAComponentSequence(cur_dc))
        {
            (call_function)(cur_ds, cur_item->text().toAscii());
        }
    }

    template<typename T>
    void SBOLTool::sa_str_item_changed(T call_function, QLineEdit* cur_item)
    {
        DNAComponent * cur_dc;
        GraphicsScene * scene = currentScene();
        if (!scene || scene->selected().size() != 1) return;

        QGraphicsItem * selectedItem = scene->selected()[0];

        if(!selectedItem) return;

        NodeGraphicsItem *cur_node = NodeGraphicsItem::cast(selectedItem);

        cur_dc= getDNAComponent(sbol_doc, cur_node->name.toAscii());
        SequenceAnnotation *cur_sa;

        if(cur_sa = getSequenceAnnotation(sbol_doc,SA_uri->text().toAscii()))
        {
            (call_function)(cur_sa, cur_item->text().toAscii());
        }
    }

    template<typename T>
    void SBOLTool::sa_int_item_changed(T call_function, QLineEdit* cur_item)
    {
        DNAComponent * cur_dc;
        GraphicsScene * scene = currentScene();
        if (!scene || scene->selected().size() != 1) return;

        QGraphicsItem * selectedItem = scene->selected()[0];

        if(!selectedItem) return;

        NodeGraphicsItem *cur_node = NodeGraphicsItem::cast(selectedItem);

        cur_dc= getDNAComponent(sbol_doc, cur_node->name.toAscii());
        SequenceAnnotation *cur_sa;

        if(cur_sa = getSequenceAnnotation(sbol_doc,SA_uri->text().toAscii()))
        {
            (call_function)(cur_sa, cur_item->text().toInt());
        }
    }

    void SBOLTool::dc_uriChanged(){dc_str_item_changed(&setDNAComponentURI, DC_uri);}
    void SBOLTool::dc_displayidChanged(){dc_str_item_changed(&setDNAComponentDisplayID, DC_displayId);}
    void SBOLTool::dc_nameChanged(){dc_str_item_changed(&setDNAComponentName, DC_name);}
    void SBOLTool::dc_descriptionChanged(){dc_str_item_changed(&setDNAComponentDescription,DC_description);}

    void SBOLTool::ds_uriChanged(){ds_str_item_changed(&setDNASequenceURI, DS_uri);}
    void SBOLTool::ds_nucleotidesChanged(){ds_str_item_changed(&setDNASequenceNucleotides,DS_nucleotides);}

    void SBOLTool::sa_uriChanged()
    {
        sa_str_item_changed(setSequenceAnnotationURI,SA_uri);
    }
    void SBOLTool::sa_bioStartChanged()
    {
        sa_int_item_changed(setSequenceAnnotationStart,SA_bioStart);
    }
    void SBOLTool::sa_bioEndChanged()
    {
        sa_int_item_changed(setSequenceAnnotationEnd,SA_bioEnd);
    }
    void SBOLTool::sa_strandChanged()
    {
        sa_int_item_changed(setSequenceAnnotationStrand,SA_strand);
    }


    void SBOLTool::hideSA()
    {
        groupBox3->hide();
        addSA->show();
        delSA->hide();
    }
    void SBOLTool::hideDS()
    {
        groupBox2->hide();
        addDS->show();
        delDS->hide();
    }

    void SBOLTool::showSA()
    {
        groupBox3->show();
        addSA->hide();
        delSA->show();

        DNAComponent * cur_dc;
        cur_dc = getDNAComponent(sbol_doc, DC_uri->text().toStdString().c_str());

        if(!cur_dc)
            {
                return;
            }
        SequenceAnnotation *cur_sa = 0;

        for(int i=0; i<getNumSequenceAnnotationsFor(head_dc); i++)
            {
                SequenceAnnotation * temp_sa;
                if (temp_sa = getNthSequenceAnnotationFor(head_dc,i))
                    {
                        if(getSequenceAnnotationSubComponent(temp_sa) == cur_dc)
                            {
                                cur_sa = temp_sa;
                                break;
                            }
                    }
            }
        if(cur_sa == 0)
            {
                cur_sa_cnt++;
                std::string temp = authority_sa+SSTR(cur_sa_cnt);
                cur_sa = createSequenceAnnotation(sbol_doc,temp.c_str());
                addSequenceAnnotation(head_dc,cur_sa);
                setSequenceAnnotationSubComponent(cur_sa,cur_dc);
            }
            SA_bioStart->setText(QString::number(getSequenceAnnotationStart(cur_sa)));
            SA_bioEnd->setText(QString::number(getSequenceAnnotationEnd(cur_sa)));
            SA_strand->setText(QString::number(getSequenceAnnotationStrand(cur_sa)));
            SA_uri->setText(QString::fromAscii(getSequenceAnnotationURI(cur_sa)));
    }

    void SBOLTool::showDS()
    {
        groupBox2->show();
        addDS->hide();
        delDS->show();


        DNAComponent * cur_dc;
        cur_dc = getDNAComponent(sbol_doc, DC_uri->text().toStdString().c_str());
        if(!cur_dc)
            {
                return;
            }

        DNASequence * cur_ds = getDNAComponentSequence(cur_dc);
        if(!cur_ds)
            {
                //NotExist
                cur_ds_cnt++;
                std::string temp_name = authority_ds+SSTR(cur_ds_cnt);
                cur_ds = createDNASequence(sbol_doc, temp_name.c_str());
                setDNAComponentSequence(cur_dc,cur_ds);
            }
        DS_uri->setText(QString::fromAscii(getDNASequenceURI(cur_ds)));
        DS_nucleotides->setText(QString::fromAscii(getDNASequenceNucleotides(cur_ds)));
        return;
    }

	bool SBOLTool::setMainWindow(MainWindow * main)
    {
		Tool::setMainWindow(main);
        if (mainWindow != 0)
        {
            cur_cnt++;

            setWindowTitle(tr("Information Box"));
            setWindowIcon(QIcon(tr(":/images/about.png")));
            setWindowFlags(Qt::Dialog);
            //QDockWidget * dockWidget = mainWindow->addToViewMenu(this);
            mainWindow->addToViewMenu(this);

            //QDockWidget * dockWidget = new QDockWidget(mainWindow);
            //if (dockWidget)
            //    dockWidget->setFloating(true);
/*			connect(mainWindow, SsIGNAL(itemsAboutToBeInserted(GraphicsScene*, QList<QGraphicsItem *>& , QList<ItemHandle*>& , QList<QUndoCommand*>& )),
						 this, SLOT(itemsAboutToBeInserted(GraphicsScene* , QList<QGraphicsItem *>&, QList<ItemHandle*>&, QList<QUndoCommand*>&)));
			connect(mainWindow,SIGNAL(mouseDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)),
                    this,SLOT(mouseDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)));

            connect(mainWindow,SIGNAL(keyPressed(GraphicsScene*,QKeyEvent *)),
				this ,SLOT(keyPressed(GraphicsScene*,QKeyEvent *)));
*/
            connect(this, SIGNAL(itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&)),
					mainWindow,SIGNAL(itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&)));
			connect(mainWindow,SIGNAL(itemsDropped(GraphicsScene *, const QString&, QPointF)),
				this,SLOT(itemsDropped(GraphicsScene *, const QString&, QPointF)));
            connect(mainWindow,SIGNAL(mousePressed(GraphicsScene *, QPointF, Qt::MouseButton, Qt::KeyboardModifiers)),
				this,SLOT(sceneClicked(GraphicsScene *, QPointF, Qt::MouseButton, Qt::KeyboardModifiers)));
            connect(mainWindow,
				SIGNAL(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,
				SLOT(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers))
				);
            connect(mainWindow,SIGNAL(loadNetwork(const QString&, bool*)),this,SLOT(loadNetwork(const QString&, bool*)));
            connect(mainWindow,SIGNAL(saveNetwork(const QString&)),this,SLOT(saveNetwork(const QString&)));

            //connect(mainWindow, SIGNAL(load))

            //GraphicsScene * scene, QPointF point, QGraphicsItem * item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers


			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(toolLoaded(Tool*))); // for loading tool. Initialization

            sbol_doc = createDocument();

            std::string temp = authority+SSTR(cur_cnt);
            head_dc = createDNAComponent(sbol_doc, temp.c_str());
            head_dc = getDNAComponent(sbol_doc, temp.c_str());
            setDNAComponentType(head_dc,"Main");

//Connecting Collision Detection
		static bool ac1 = false;
		if (!ac1 && mainWindow->tool(tr("Collision Detection")))
		{
			QWidget * widget = mainWindow->tool(tr("Collision Detection"));
			CollisionDetection * collisionDetection = static_cast<CollisionDetection*>(widget);
			if (collisionDetection)
			{
				ac1 = true;
				connect(collisionDetection,SIGNAL(nodeCollided(const QList<QGraphicsItem*>& , NodeGraphicsItem * , const QList<QPointF>&)),
						this, SLOT( nodeCollided(const QList<QGraphicsItem*>& , NodeGraphicsItem * , const QList<QPointF>& )));
			}
		}



// Export Menu
            GlobalSettings::OPEN_FILE_EXTENSIONS << "SBOL" << "sbol";

            if (mainWindow->fileMenu)
            {
                QList<QAction*> actions = mainWindow->fileMenu->actions();

                QAction * targetAction = 0;
                QMenu * exportmenu = 0;//, * importmenu = 0;

                for (int i=0; i < actions.size(); ++i)
                    if (actions[i] && actions[i]->menu())
                    {
                        if (actions[i]->text() == tr("&Export"))
                        {
                            exportmenu = actions[i]->menu();
                            targetAction = actions[i];
                        }
                        else
                            if (actions[i]->text() == tr("&Import"))
                            {
                                //importmenu = actions[i]->menu();
                                targetAction = actions[i];
                            }
                    }

                if (!exportmenu)
                {
                    for (int i=0; i < actions.size(); ++i)
                        if (actions[i] && actions[i]->text() == tr("&Close page"))
                        {
                            exportmenu = new QMenu(tr("&Export"));
                            //importmenu = new QMenu(tr("&Import"));
                            //mainWindow->fileMenu->insertMenu(actions[i],importmenu);
                            mainWindow->fileMenu->insertMenu(actions[i],exportmenu);
                        }
                }

                if (!exportmenu)
                {
                    exportmenu = new QMenu(tr("&Export"));
                    mainWindow->fileMenu->insertMenu(targetAction,exportmenu);
                }

                if (exportmenu)
                {
                    //importmenu->addAction(tr("load SBML file"),this,SLOT(loadSBMLFile()));
                    exportmenu->addAction(tr("SBOL"),this,SLOT(saveSBOLFile()));
                }
            }


            //setLayout(layout4);
//            dockWidget->setLayout(layout4);


			//toolLoaded(0);
        }

        return true;
    }

void SBOLTool::saveSBOLFile()
{
    QString file = QFileDialog::getSaveFileName (this, tr("Save SBOL file"), homeDir());
    if (file.isNull() || file.isEmpty()) return;

    exportSBOL(0, file);
    QDesktopServices::openUrl(QUrl(file));
    return;
}

void SBOLTool::exportSBOL(QSemaphore* sem, const QString &file)
{
    console()->message(writeDocumentToString(sbol_doc));
    NetworkHandle * network = currentNetwork();
    if (network)
        {
            network->globalHandle()->textData("sbol") = QString::fromAscii(writeDocumentToString(sbol_doc));
        }
    writeDocumentToFile(sbol_doc,file.toAscii());
    return;
}

/*void SBOLTool::getItemsFromFile(QList<ItemHandle*>& handles, QList<QGraphicsItem*>&, const QString& filename,ItemHandle * root)
	{
		if (!handles.isEmpty()) return;

		if (!root && currentWindow())
			root = currentWindow()->handle;

		//handles = importSBML(filename, root);
        console()->message("file loaded! SBOL");
	}*/
void SBOLTool::loadNetwork(const QString& filename, bool * b)
{

    NetworkHandle * network = currentNetwork();
    if (!network || filename.isEmpty()) return;
    QList<GraphicsScene*> scenes = network->scenes();

    console()->message("network loaded SBOL");
    //if (b && (*b)) return;
    console()->message("network loaded SBOL SUCCESS");
    if (network->globalHandle()->hasTextData("sbol"))
    {
        console()->message("SBOL Component exist");
        //console()->message(network->globalHandle()->textData("sbol"));
        if(sbol_doc)
        {
            deleteDocument(sbol_doc);
        }
        sbol_doc = createDocument();
        console()->message(network->globalHandle()->textData("sbol"));
        int *ret_val = new int;
        readSBOLString(sbol_doc,network->globalHandle()->textData("sbol").toStdString().c_str(),ret_val);
        if(*ret_val == 1)
            {
                console()->message("Not Valid XML");
            }
        else if (*ret_val == 2)
            {
                console()->message("not valid SBOL");
            }
        else if (*ret_val <=0)
            {
                console()->message(QString::fromStdString(SSTR(*ret_val)));
            }
        else if (*ret_val == 3)
            {
                console()->message(QString::fromAscii(writeDocumentToString(sbol_doc)));
            }
        delete ret_val;
    }
/*    std::ofstream oup;
    oup.open("temp.sbol");
        oup << network->globalHandle()->textData("sbol").toStdString();
    oup.close();

    if(sbol_doc)
        {
            deleteDocument(sbol_doc);
        }
    sbol_doc = createDocument();
    readDocument(sbol_doc,"D:\Tinkercell\BUILD\bin\temp.sbol");
    console()->message(QString::fromAscii(writeDocumentToString(sbol_doc)));
    //console()->message(isValidSBOL(sbol_doc));*/




}

	void SBOLTool::saveItems(NetworkHandle * network, const QString& filename)
	{
/*		if (!network || filename.isEmpty()) return;

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

		mainWindow->statusBar()->showMessage(tr("model saved in ") + filename);*/
	}

	void SBOLTool::saveNetwork(const QString& filename)
	{
	    console()->message("save started");
	    NetworkHandle * network = currentNetwork();
		if (!network || filename.isEmpty()) return;
        console()->message("save passed!");
		QList<GraphicsScene*> scenes = network->scenes();
		network->globalHandle()->textData("sbol") = QString::fromAscii(writeDocumentToString(sbol_doc));
		//NetworkHandle * network = currentNetwork();
		//if (network)
		//	saveItems(network,filename);

	}

void SBOLTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
    /*
        QLineEdit *DC_uri;
        QLineEdit *DC_displayId;
        QLineEdit *DC_name;
        QLineEdit *DC_description;
        QLineEdit *DC_type;
*/
    show();
	if (!scene) return;

	console()->message("Current Point");

    hideDS();
    hideSA();
	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;
	for(int i=0; i<items.size(); i++)
        {
            node = NodeGraphicsItem::cast(items[i]);
            if(node)
                {
                    console()->message(node->name);
                    std::string cur_uri = node->name.toStdString();
                    DNAComponent *cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());
                    DC_uri->setText(QString::fromAscii(getDNAComponentURI(cur_dc)));
                    DC_type->setText(QString::fromAscii(getDNAComponentType(cur_dc)));
                    DC_displayId->setText(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));
                    DC_name->setText(QString::fromAscii(getDNAComponentName(cur_dc)));
                    DC_description->setText(QString::fromAscii(getDNAComponentDescription(cur_dc)));


                    DNASequence *cur_ds = getDNAComponentSequence(cur_dc);
                    if(!cur_ds)
                        {
                            hideDS();
                        }
                    else
                        {
                            showDS();
                        }


                    SequenceAnnotation * cur_sa = 0;

                    for(int i=0; i<getNumSequenceAnnotationsFor(head_dc); i++)
                        {
                            SequenceAnnotation * temp_sa;
                            temp_sa = getNthSequenceAnnotationFor(head_dc,i);
                            if(getSequenceAnnotationSubComponent(temp_sa) == cur_dc)
                                {
                                    cur_sa = temp_sa;
                                    break;
                                }
                        }

                    if(!cur_sa)
                        {
                            hideSA();
                        }
                    else
                        {
                            showSA();
                        }
                    /*console()->message(QString::fromAscii(getDNAComponentURI(cur_dc)));
                    console()->message(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));*/
                }
        }

}

	void SBOLTool::nodeCollided(const QList<QGraphicsItem*>& items, NodeGraphicsItem * item, const QList<QPointF>& )
	{
		GraphicsScene * scene = currentScene();
		if (!scene || !item || items.isEmpty()) return;

		ItemHandle * handle = item->handle();
        NodeGraphicsItem *node_colided = item;
		bool partCollided = false;

        NodeGraphicsItem *colided_to;
		for (int i=0; i < items.size(); i++)
            {
                if(items[i])
                    {
                        if(NodeGraphicsItem::cast(items[i]) != 0)
                        {

                            colided_to = NodeGraphicsItem::cast(items[i]);
                            if(colided_to){
                                console()->message(node_colided->name);
                                console()->message(colided_to->name);
                            }

                        }
                    }
            }

        if(node_colided && colided_to)
            {
                int direction = 0;
                QRectF cur_rect = node_colided->sceneBoundingRect();

                if(node_colided->pos().x() < colided_to->pos().x())
                    {
                        direction = 1;
                    }
                else
                    {
                        direction = -1;
                    }
                cur_rect.adjust(direction*cur_rect.width(),0,direction*cur_rect.width(),0);
                colided_to->setPos(cur_rect.center());


                NodeGraphicsItem *push = colided_to;
                NodeGraphicsItem *pop = colided_to;
                while(push != 0)
                {
                    QRectF push_rect = push->sceneBoundingRect();
                    QList<QGraphicsItem*> cur_list = currentScene()->items(push_rect);
                    pop = 0;

                    for (int i=0; i<cur_list.size(); i++)
                        {
                            NodeGraphicsItem *cur_node;
                            if((cur_node = NodeGraphicsItem::cast(cur_list[i])) && (cur_node != push))
                                {
                                    console()->message("overlapping item");
                                    console()->message(cur_node->name);
                                    pop = cur_node;
                                    break;
                                }
                        }
                    if(pop)
                        {
                            QRectF pop_rect = pop->sceneBoundingRect();
                            pop_rect.adjust(direction*push_rect.width(), 0,direction*push_rect.width(),0);
                            pop->setPos(pop_rect.center());
                        }
                    push = pop;
                }
            }
	}

    void SBOLTool::itemsDropped(GraphicsScene * scene, const QString& name, QPointF point)
	{
	    std::string cur_type = "";
	    console()->message(name);
		scene->useDefaultBehavior(false);
		show();
		mode = 0;
		cur_cnt++;
		std::string temp;
		temp = authority+SSTR(cur_cnt);
		DNAComponent *cur_dc = createDNAComponent(sbol_doc, temp.c_str());
		setDNAComponentURI(cur_dc, temp.c_str());
		current_type = name.toStdString();

		if (name.toLower() == tr("promoter"))
            {
                cur_type = "promoter";
                mode = SBOL_PROMOTER;
            }
        if (name.toLower() == tr("assembly scar"))
            {
                cur_type = "assembly scar";
                mode = SBOL_ASSEMBLY_SCAR;
            }
        if (name.toLower() == tr("blunt restriction site"))
            {
                cur_type = "blunt restriction site";
                mode = SBOL_BLUNT_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("cds"))
            {
                cur_type = "cds";
                mode = SBOL_CDS;
            }
        if (name.toLower() == tr("five prime overhang"))
            {
                cur_type = "five prime overhang";
                mode = SBOL_FIVE_PRIME_OVERHANG;
            }
        if (name.toLower() == tr("five prime sticky restriction site"))
            {
                cur_type = "five prime sticky restriction site";
                mode = SBOL_FIVE_PRIME_STICKY_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("insulator"))
            {
                cur_type = "insulator";
                mode = SBOL_INSULATOR;
            }
        if (name.toLower() == tr("operator"))
            {
                cur_type = "operator";
                mode = SBOL_OPERATOR;
            }
        if (name.toLower() == tr("primer binding site"))
            {
                cur_type = "primer binding site";
                mode = SBOL_PRIMER_BINDING_SITE;
            }
        if (name.toLower() == tr("origin of replication"))
            {
                cur_type = "origin of replication";
                mode = SBOL_ORIGIN_OF_REPLICATION;
            }
        if (name.toLower() == tr("protease site"))
            {
                cur_type = "protease site";
                mode = SBOL_PROTEASE_SITE;
            }
        if (name.toLower() == tr("protein stability element"))
            {
                cur_type = "protein stability element";
                mode = SBOL_PROTEIN_STABILITY_ELEMENT;
            }
        if (name.toLower() == tr("restriction enzyme recognition site"))
            {
                cur_type = "restriction enzyme recognition site";
                mode = SBOL_RESTRICTION_ENZYME_RECOGNITION_SITE;
            }
        if (name.toLower() == tr("ribonuclease site"))
            {
                cur_type = "ribonuclease site";
                mode = SBOL_RIBONUCLEASE_SITE;
            }
        if (name.toLower() == tr("ribosome entry site"))
            {
                cur_type = "ribosome entry site";
                mode = SBOL_RIBOSOME_ENTRY_SITE;
            }
        if (name.toLower() == tr("rna stability element"))
            {
                cur_type = "rna stability element";
                mode = SBOL_RNA_STABILITY_ELEMENT;
            }
        if (name.toLower() == tr("signature"))
            {
                cur_type = "signature";
                mode = SBOL_SIGNATURE;
            }
        if (name.toLower() == tr("terminator"))
            {
                cur_type = "terminator";
                mode = SBOL_TERMINATOR;
            }
        if (name.toLower() == tr("three prime overhang"))
            {
                cur_type = "three prime overhang";
                mode = SBOL_THREE_PRIME_OVERHANG;
            }
        if (name.toLower() == tr("three prime sticky restriction site"))
            {
                cur_type = "three prime sticky restriction site";
                mode = SBOL_THREE_PRIME_STICKY_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("user defined"))
            {
                cur_type = "user defined";
                mode = SBOL_USER_DEFINED;
            }
        if(mode != 0)
            {
                setDNAComponentType(cur_dc, current_type.c_str());
                //setDNAComponentDisplayID(cur_dc, current_type.c_str());
                sceneClicked(scene,point,Qt::LeftButton,Qt::NoModifier);
                scene->useDefaultBehavior(true);
            }
		//smode = none;
		//setDNAComponentType(cur_dc, )
	}
//GraphicsScene * scene, QPointF point, QGraphicsItem * item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers
    void SBOLTool::sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
	{
        show();
	    QList<QGraphicsItem*> items;
        items = scene->items();
        NodeGraphicsItem * image = 0;
        ItemHandle * globalHandle = 0;
        globalHandle = scene->localHandle();
        if (!globalHandle)
            globalHandle = scene->globalHandle();

        QString appDir = QApplication::applicationDirPath();
        image = new NodeGraphicsItem;

        NodeGraphicsReader reader;

		if (mode == SBOL_PROMOTER)
		{
			reader.readXml(image, tr(":/images/sbol_promoter.xml"));
		}
		if (mode == SBOL_ORIGIN_OF_REPLICATION)
        {
            reader.readXml(image, tr(":/images/sbol_origin-of-replication.xml"));
        }
        if (mode == SBOL_CDS)
		{
			reader.readXml(image, tr(":/images/sbol_cds.xml"));
		}
		if (mode == SBOL_RIBOSOME_ENTRY_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_ribosome-entry-site.xml"));
		}
		if (mode == SBOL_TERMINATOR)
		{
			reader.readXml(image, tr(":/images/sbol_terminator.xml"));
		}
		if (mode == SBOL_OPERATOR)
		{
			reader.readXml(image, tr(":/images/sbol_operator.xml"));
		}
		if (mode == SBOL_INSULATOR)
		{
			reader.readXml(image, tr(":/images/sbol_insulator.xml"));
		}
		if (mode == SBOL_RIBONUCLEASE_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_ribonuclease-site.xml"));
		}
		if (mode == SBOL_RNA_STABILITY_ELEMENT)
		{
			reader.readXml(image, tr(":/images/sbol_rna-stability-element.xml"));
		}
		if (mode == SBOL_PROTEASE_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_protease-site.xml"));
		}
		if (mode == SBOL_PROTEIN_STABILITY_ELEMENT)
		{
			reader.readXml(image, tr(":/images/sbol_protein-stability-element.xml"));
		}
		if (mode == SBOL_PRIMER_BINDING_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_primer-binding-site.xml"));
		}
		if (mode == SBOL_RESTRICTION_ENZYME_RECOGNITION_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_restriction-enzyme-recognition-site.xml"));
		}
		if (mode == SBOL_BLUNT_RESTRICTION_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_blunt-restriction-site.xml"));
		}
		if (mode == SBOL_FIVE_PRIME_STICKY_RESTRICTION_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_five-prime-sticky-restriction-site.xml"));
		}
		if (mode == SBOL_THREE_PRIME_STICKY_RESTRICTION_SITE)
		{
			reader.readXml(image, tr(":/images/sbol_three-prime-sticky-restriction-site.xml"));
		}
		if (mode == SBOL_FIVE_PRIME_OVERHANG)
		{
			reader.readXml(image, tr(":/images/sbol_five-prime-overhang.xml"));
		}
		if (mode == SBOL_THREE_PRIME_OVERHANG)
		{
			reader.readXml(image, tr(":/images/sbol_three-prime-overhang.xml"));
		}
		if (mode == SBOL_ASSEMBLY_SCAR)
		{
			reader.readXml(image, tr(":/images/sbol_assembly-scar.xml"));
		}
		if (mode == SBOL_SIGNATURE)
		{
			reader.readXml(image, tr(":/images/sbol_signature.xml"));

		}
		if (mode == SBOL_USER_DEFINED)
		{
			reader.readXml(image, tr(":/images/sbol_user-defined.xml"));
		}
		if (mode != 0)
            {
                std::string temp;
                temp = authority+SSTR(cur_cnt);
                image->name = QString::fromStdString(temp);
                image->normalize();
                image->className = tr("SBOL Object");
                image->scale((image->defaultSize.width()*2)/image->sceneBoundingRect().width(),
                (image->defaultSize.height()*2)/image->sceneBoundingRect().height());
                image->setPos(point);
                image->setToolTip(tr("List of events in this model"));
                scene->insert(tr("SBOL Objects"),image);
                mode = 0;
                select(0);
            }
        else
            {
                hideSA();
                hideDS();
                //DC_uri->setText(QString::fromAscii(getDNAComponentURI(head_dc)));
                //DC_type->setText(QString::fromAscii(getDNAComponentDisplayID(head_dc)));
                DC_uri->setText(QString::fromAscii(getDNAComponentURI(head_dc)));
                DC_type->setText(QString::fromAscii(getDNAComponentType(head_dc)));
                DC_displayId->setText(QString::fromAscii(getDNAComponentDisplayID(head_dc)));
                DC_name->setText(QString::fromAscii(getDNAComponentName(head_dc)));
                DC_description->setText(QString::fromAscii(getDNAComponentDescription(head_dc)));
                console()->message("scene clicked");
                console()->message(QString::fromAscii(getDNAComponentURI(head_dc)));
                console()->message(QString::fromAscii(getDNAComponentDisplayID(head_dc)));

                addSA->hide();
                addDS->hide();
            }
        //show();
        return;
    }

	void SBOLTool::toolLoaded(Tool* tool)
	{
		static bool connected1 = false;

		if (mainWindow->tool(tr("Parts and Connections Catalog")) && !connected1)
		{
			Tool * tool = static_cast<Tool*>(mainWindow->tool(tr("Parts and Connections Catalog")));
			catalogWidget = static_cast<CatalogWidget*>(tool);

            //parse Module Tools/sbol-visual.xml and use that.

            QList<QToolButton*> newButtons = catalogWidget->addNewButtons(tr("SBOL"),
                                                                    QStringList() << tr("Promoter")
                                                                                  << tr("Operator")
                                                                                  << tr("Terminator")
                                                                                  << tr("CDS")
                                                                                  << tr("Ribosome Entry Site")
                                                                                  << tr("Insulator")
                                                                                  << tr("Ribonuclease Site")
                                                                                  << tr("Protease Site")
                                                                                  << tr("Protein Stability Element")
                                                                                  << tr("Origin Of Replication")
                                                                                  << tr("Primer Binding Site")
                                                                                  << tr("Restriction Enzyme Recognition Site")
                                                                                  << tr("Blunt Restriction Site")
                                                                                  << tr("Three Prime Sticky Restriction Site")
                                                                                  << tr("Five Prime Sticky Restriction Site")
                                                                                  << tr("Three Prime Overhang")
                                                                                  << tr("Five Prime Overhang")
                                                                                  << tr("Assembly Scar")
                                                                                  << tr("Signature")
                                                                                  << tr("User Defined"),
                                                                    QList<QIcon>() << QIcon(QPixmap(tr(":/images/sbol_promoter.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_operator.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_terminator.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_cds.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_ribosome-entry-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_insulator.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_ribonuclease-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_protease-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_protein-stability-element.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_origin-of-replication.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_primer-binding-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_restriction-enzyme-recognition-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_blunt-restriction-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_three-prime-sticky-restriction-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_five-prime-sticky-restriction-site.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_three-prime-overhang.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_five-prime-overhang.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_assembly-scar.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_signature.png")))
                                                                                  << QIcon(QPixmap(tr(":/images/sbol_user-defined.png"))),
                                                                    QStringList() << tr("Promoter")
                                                                                  << tr("Operator")
                                                                                  << tr("Terminator")
                                                                                  << tr("CDS")
                                                                                  << tr("Ribosome Entry Site")
                                                                                  << tr("Insulator")
                                                                                  << tr("Ribonuclease Site")
                                                                                  << tr("Protease Site")
                                                                                  << tr("Protein Stability Element")
                                                                                  << tr("Origin Of Replication")
                                                                                  << tr("Primer Binding Site")
                                                                                  << tr("Restriction Enzyme Recognition Site")
                                                                                  << tr("Blunt Restriction Site")
                                                                                  << tr("Three Prime Sticky Restriction Site")
                                                                                  << tr("Five Prime Sticky Restriction Site")
                                                                                  << tr("Three Prime Overhang")
                                                                                  << tr("Five Prime Overhang")
                                                                                  << tr("Assembly Scar")
                                                                                  << tr("Signature")
                                                                                  << tr("User Defined")
                                                                    );
			connected1 = true;
		}
	}
/*
	void WetLabTool::keyPressed(GraphicsScene* scene,QKeyEvent * keyEvent)
	{
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Space)
		{
			QList<ItemHandle*> handles = getHandle(scene->selected());
			bool containsExp = false;
			for (int i=0; i < handles.size(); ++i)
				if (handles[i] && handles[i]->isA("Experiment"))
				{
					containsExp = true;
					break;
				}
			if (mainWindow && containsExp)
			{
				Tool * tool = mainWindow->tool("Text Attributes");
				if (tool)
				{
					tool->select();
				}
			}
		}
	}

	void WetLabTool::mouseDoubleClicked (GraphicsScene * scene, QPointF , QGraphicsItem * item, Qt::MouseButton, Qt::KeyboardModifiers modifiers)
    {
		QList<ItemHandle*> handles = getHandle(scene->selected());
		bool containsExp = false;
		for (int i=0; i < handles.size(); ++i)
			if (handles[i] && handles[i]->isA("Experiment"))
			{
				containsExp = true;
				break;
			}
		if (mainWindow && containsExp)
		{
			Tool * tool = mainWindow->tool("Text Attributes");
			if (tool)
			{
				tool->select();
			}
		}
    }

		void WetLabTool::itemsAboutToBeInserted(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands)
	{
		if (!scene || !scene->network) return;

		QString home = homeDir();
		ConnectionGraphicsItem * connection = 0;
		ItemHandle * handle = 0;
		for (int i=0; i < items.size(); ++i)
			if ((connection = ConnectionGraphicsItem::cast(items[i])) &&
				(handle = connection->handle()) &&
				handle->isA("Experiment") &&
				!connection->centerRegionItem)
		{
			ArrowHeadItem * newDecorator = new ArrowHeadItem(home + tr("/Lab/microscope.xml"),connection);
			connection->centerRegionItem = newDecorator;
			items += newDecorator;
		}
	}
*/

}

