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

extern "C"{
#include "sbol.h"
}

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

        Document * doc = createDocument();


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
        return;
    }

    void SBOLTool::showDS()
    {
        groupBox2->show();
        addDS->hide();
        delDS->show();
        return;
    }

	bool SBOLTool::setMainWindow(MainWindow * main)
    {
		Tool::setMainWindow(main);
        if (mainWindow != 0)
        {
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



            //setLayout(layout4);
//            dockWidget->setLayout(layout4);


			//toolLoaded(0);
        }

        return true;
    }

void SBOLTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
	if (!scene) return;

	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;
	for(int i=0; i<items.size(); i++)
        {
            node = NodeGraphicsItem::cast(items[i]);
            if(node)
                {
                    console()->message(node->name);
                }
        }

}

    void SBOLTool::itemsDropped(GraphicsScene * scene, const QString& name, QPointF point)
	{
	    console()->message(name);
		scene->useDefaultBehavior(false);
		show();
		mode = 0;
		current_type = name.toStdString();
		if (name.toLower() == tr("promoter"))
            {
                mode = SBOL_PROMOTER;
            }
        if (name.toLower() == tr("assembly scar"))
            {
                mode = SBOL_ASSEMBLY_SCAR;
            }
        if (name.toLower() == tr("blunt restriction site"))
            {
                mode = SBOL_BLUNT_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("cds"))
            {
                mode = SBOL_CDS;
            }
        if (name.toLower() == tr("five prime overhang"))
            {
                mode = SBOL_FIVE_PRIME_OVERHANG;
            }
        if (name.toLower() == tr("five prime sticky restriction site"))
            {
                mode = SBOL_FIVE_PRIME_STICKY_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("insulator"))
            {
                mode = SBOL_INSULATOR;
            }
        if (name.toLower() == tr("operator"))
            {
                mode = SBOL_OPERATOR;
            }
        if (name.toLower() == tr("primer binding site"))
            {
                mode = SBOL_PRIMER_BINDING_SITE;
            }
        if (name.toLower() == tr("origin of replication"))
            {
                mode = SBOL_ORIGIN_OF_REPLICATION;
            }
        if (name.toLower() == tr("protease site"))
            {
                mode = SBOL_PROTEASE_SITE;
            }
        if (name.toLower() == tr("protein stability element"))
            {
                mode = SBOL_PROTEIN_STABILITY_ELEMENT;
            }
        if (name.toLower() == tr("restriction enzyme recognition site"))
            {
                mode = SBOL_RESTRICTION_ENZYME_RECOGNITION_SITE;
            }
        if (name.toLower() == tr("ribonuclease site"))
            {
                mode = SBOL_RIBONUCLEASE_SITE;
            }
        if (name.toLower() == tr("ribosome entry site"))
            {
                mode = SBOL_RIBOSOME_ENTRY_SITE;
            }
        if (name.toLower() == tr("rna stability element"))
            {
                mode = SBOL_RNA_STABILITY_ELEMENT;
            }
        if (name.toLower() == tr("signature"))
            {
                mode = SBOL_SIGNATURE;
            }
        if (name.toLower() == tr("terminator"))
            {
                mode = SBOL_TERMINATOR;
            }
        if (name.toLower() == tr("three prime overhang"))
            {
                mode = SBOL_THREE_PRIME_OVERHANG;
            }
        if (name.toLower() == tr("three prime sticky restriction site"))
            {
                mode = SBOL_THREE_PRIME_STICKY_RESTRICTION_SITE;
            }
        if (name.toLower() == tr("user defined"))
            {
                mode = SBOL_USER_DEFINED;
            }
        if(mode != 0)
            {
                sceneClicked(scene,point,Qt::LeftButton,Qt::NoModifier);
                scene->useDefaultBehavior(true);
            }
		//smode = none;
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
                image->name = QString::fromStdString(current_type);
                image->normalize();
                image->className = tr("SBOL Object");
                image->scale(image->defaultSize.width()/image->sceneBoundingRect().width()*2,
                image->defaultSize.height()/image->sceneBoundingRect().height());
                image->setPos(point);
                image->setToolTip(tr("List of events in this model"));
                scene->insert(tr("Events box inserted"),image);
                mode = 0;
                select(0);
                return;
            }
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

