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
        groupBox1->show();
        groupBox2->hide();
        groupBox3->hide();
        delSA->hide();
        delDS->hide();
        setLayout(layout4);
        show();
        mode = 0;

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
/*        DNAComponent * cur_dc;
        cur_dc = getDNAComponent(sbol_doc, DC_uri->text().toStdString().c_str());
        if(!cur_dc)
            {
                return;
            }
        DNASequence * cur_ds = getDNAComponentSequence(cur_dc);
        if(!cur_ds)
            {
                return;
            }

        setDNAComponentSequence(cur_dc,0);*/


        //no release DNAComponent sequence

        /*
        deleteDNASequence(cur_ds);
        */

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

            //GraphicsScene * scene, QPointF point, QGraphicsItem * item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers


			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(toolLoaded(Tool*))); // for loading tool. Initialization

            sbol_doc = createDocument();

            std::string temp = authority+SSTR(cur_cnt);
            head_dc = createDNAComponent(sbol_doc, temp.c_str());
            head_dc = getDNAComponent(sbol_doc, temp.c_str());
            setDNAComponentDisplayID(head_dc,"Main");


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
    console()->message(file);
    writeDocument(sbol_doc, file.toStdString().c_str());
    return;
}

void SBOLTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
	if (!scene) return;
    hideDS();
    hideSA();
	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;
	for(int i=0; i<items.size(); i++)
        {
            node = NodeGraphicsItem::cast(items[i]);
            if(node)
                {
                    std::string cur_uri = node->name.toStdString();
                    DNAComponent *cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());
                    DC_uri->setText(QString::fromAscii(getDNAComponentURI(cur_dc)));
                    DC_displayId->setText(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));

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

                    //DC_type->setText((QString::fromAscii(getDNAComponentType(cur_dc))));
                    /*console()->message(QString::fromAscii(getDNAComponentURI(cur_dc)));
                    console()->message(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));*/
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
//                setDNAComponentType(cur_dc, current_type.c_str());
                setDNAComponentDisplayID(cur_dc, current_type.c_str());
                sceneClicked(scene,point,Qt::LeftButton,Qt::NoModifier);
                scene->useDefaultBehavior(true);
            }
		//smode = none;
		//setDNAComponentType(cur_dc, )
	}
//GraphicsScene * scene, QPointF point, QGraphicsItem * item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers
    void SBOLTool::sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
	{

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
                image->scale(image->defaultSize.width()/image->sceneBoundingRect().width()*2,
                image->defaultSize.height()/image->sceneBoundingRect().height());
                image->setPos(point);
                image->setToolTip(tr("List of events in this model"));
                scene->insert(tr("Events box inserted"),image);
                mode = 0;
                select(0);
            }
        else
            {
                hideSA();
                hideDS();
                DC_uri->setText(QString::fromAscii(getDNAComponentURI(head_dc)));
                DC_displayId->setText(QString::fromAscii(getDNAComponentDisplayID(head_dc)));
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

