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
#include "BasicGraphicsToolbar.h"
#include <sstream>
#include <functional>
#include <fstream>
#include <map>
#include <qglobal.h>

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


static Document * sbol_doc;

namespace Tinkercell
{

    SBOLTool::SBOLTool() : Tool(tr("SBOL Tool"),tr("Module tools"))
    {
        type_temp = "http://purl.obolibrary.org/obo/";
        init_glymps();
        setPalette(QPalette(QColor(255,255,255,255)));
        setAutoFillBackground(true);

        QGridLayout * layout0 = new QGridLayout;
        QGridLayout * layout1 = new QGridLayout;
        QGridLayout * layout2 = new QGridLayout;
        QGridLayout * layout3 = new QGridLayout;

        layout0->addWidget(new QLabel(tr("uri")),0,0);
        layout0->addWidget(new QLabel(tr("displayId")),1,0);
        layout0->addWidget(new QLabel (tr("name")),2,0);
        layout0->addWidget(new QLabel(tr("description")),3,0);

        layout0->addWidget(CO_uri = new QLineEdit,0,1);
        layout0->addWidget(CO_displayId = new QLineEdit,1,1);
        layout0->addWidget(CO_name = new QLineEdit,2,1);
        layout0->addWidget(CO_description = new QLineEdit,3,1);

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

        groupBox0 = new QGroupBox(tr("Collection"));
        groupBox0->setLayout(layout0);

        groupBox1 = new QGroupBox(tr("DNA Component"));
        groupBox1->setLayout(layout1);

        groupBox2 = new QGroupBox(tr("DNA Sequence"));
        groupBox2->setLayout(layout2);

        groupBox3 = new QGroupBox(tr("Sequence Annotation"));
        groupBox3->setLayout(layout3);

        QVBoxLayout * layout4 = new QVBoxLayout;
        layout4->addWidget(groupBox0);
        layout4->addWidget(groupBox1);
        layout4->addWidget(groupBox2);
        layout4->addWidget(groupBox3);
        layout4->addWidget(addDS = new QPushButton("Add DNASequence", this));
        layout4->addWidget(delDS = new QPushButton("Delete DNASequence", this));
        layout4->addWidget(addSA = new QPushButton("Add SequenceAnnotation", this));
        layout4->addWidget(delSA = new QPushButton("Delete SequenceAnnotation", this));
        layout4->addWidget(levelup = new QPushButton("Level above", this));
        layout4->addWidget(leveldown = new QPushButton("Level below", this));
        connect(addSA, SIGNAL(pressed()), this, SLOT(showSA()));
        connect(addDS, SIGNAL(pressed()), this, SLOT(showDS()));
        connect(delSA, SIGNAL(pressed()), this, SLOT(hideSA()));
        connect(delDS, SIGNAL(pressed()), this, SLOT(hideDS()));
        connect(levelup, SIGNAL(pressed()), this, SLOT(level_up()));
        connect(leveldown, SIGNAL(pressed()), this, SLOT(level_down()));

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

        //connect(CO_uri,SIGNAL(editingFinished()), this, SLOT(co_uriChanged()));
        connect(CO_displayId,SIGNAL(editingFinished()), this, SLOT(co_displayidChanged()));
        connect(CO_name,SIGNAL(editingFinished()), this, SLOT(co_nameChanged()));
        connect(CO_description,SIGNAL(editingFinished()), this, SLOT(co_descriptionChanged()));

        groupBox0->hide();
        groupBox1->show();
        groupBox2->hide();
        groupBox3->hide();
        delSA->hide();
        delDS->hide();
        setLayout(layout4);
        mode = 0;

        QString appDir = QCoreApplication::applicationDirPath();
		NodeGraphicsReader reader;
		reader.readXml(&item,tr(":/images/DNATool.xml"));
		item.setToolTip(tr("SBOL Export"));
		setToolTip(tr("SBOL export"));

		item.normalize();
		item.scale(35.0/item.sceneBoundingRect().width(),35.0/item.sceneBoundingRect().height());

		ToolGraphicsItem * gitem = new ToolGraphicsItem(this);
		addGraphicsItem(gitem);
		gitem->addToGroup(&item);

		addAction(QIcon(), tr("SBOL Export"), tr("Export SBOL compliant Document"));
    }

    void SBOLTool::level_down()
    {
        GraphicsScene * scene = currentScene();



        QList<ItemHandle*> handles = getHandle(scene->selected());
        if(handles.size() != 1)
            {
                QMessageBox::information(mainWindow,tr("error"), tr("only one item"));
                return;
            }
        ItemHandle* cur_handle = handles[0];
        std::string cur_uri = authority+"/"+ cur_handle->name.toStdString();


        DNAComponent *cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());
        DNASequence *cur_ds = getDNASequence(sbol_doc,(char*)cur_handle->name.toStdString().c_str());
        if(!cur_ds)
            {
                std::string temp_uri;
                temp_uri = cur_uri+"_ds";
                cur_ds = getDNASequence(sbol_doc,(char*)temp_uri.c_str());
            }
        Collection *cur_co = getCollection(sbol_doc, cur_uri.c_str());
        if(!cur_co)
            {
                std::string temp_uri;
                temp_uri = cur_uri+"_co";
                cur_co = getCollection(sbol_doc, (char*)temp_uri.c_str());
            }

        scene->selectAll();
        QList<QGraphicsItem*> sel_list = scene->selected();
        QList<QGraphicsItem*> sbol_list;

        ItemHandle *cur_gl = scene->globalHandle();

        for(int i=0; i<sel_list.size(); i++)
            {
                ItemHandle *handle = getHandle(sel_list[i]);
                if(handle->isA(tr("sbol")))
                    {
                        sbol_list.append(sel_list[i]);
                    }
            }

        if(cur_dc)
            {
                for(int i=0; i<sbol_list.size(); i++)
                    {
                        sbol_list[i]->hide();
                    }
                int cnt = getNumSequenceAnnotationsFor(cur_dc);
                QMessageBox::information(mainWindow,tr("subcomponent number"),QString::fromStdString(SSTR(cnt)));
                DNASequence *cur_ds = getDNAComponentSequence(cur_dc);
                renderSBOLDocument((SBOLObject*)cur_ds);
                for(int i=0; i<cnt; i++)
                    {
                        SequenceAnnotation *cur_sa = getNthSequenceAnnotationFor(cur_dc,i);
                        DNAComponent* comp_dc = getSequenceAnnotationSubComponent(cur_sa);
                        renderSBOLDocument((SBOLObject*) comp_dc);
                    }
                cur_gl->textData(tr("call_stack")) = cur_gl->textData(tr("call_stack"))+tr("|")
                                        + QString::fromAscii(getDNAComponentURI(cur_dc));
            }
        else if(cur_ds)
            {
                QMessageBox::information(mainWindow,tr("Error"),tr("DNA Sequence don't have subcomponent"));
            }
        else if(cur_co)
            {
                for(int i=0; i<sbol_list.size(); i++)
                    {
                        sbol_list[i]->hide();
                    }
                int cnt = getNumDNAComponentsIn(cur_co);
                QMessageBox::information(mainWindow,tr("component number"),QString::fromStdString(SSTR(cnt)));
                cur_gl->textData(tr("call_stack")) = cur_gl->textData(tr("call_stack"))+tr("|")
                                        + QString::fromAscii(getCollectionURI(cur_co));
            }

    }
    void SBOLTool::level_up()
    {
        GraphicsScene *scene = currentScene();
        QStringList hier = currentScene()->globalHandle()->textData(tr("call_stack")).split("|");

        if(hier.size() < 2)
            {
                QMessageBox::information(mainWindow,tr("rw"),tr("This is top-level"));
                return;
            }
        hier.pop_back();
        QString level_above = hier[hier.size()-1];
        currentScene()->globalHandle()->textData(tr("call_stack")) = hier.join(tr("|"));

        console()->message(currentScene()->globalHandle()->textData(tr("call_stack")));

        QMessageBox::information(mainWindow,tr("rw"),level_above);

        DNAComponent* cur_dc = getDNAComponent(sbol_doc,level_above.toAscii());
        Collection* cur_co = getCollection(sbol_doc,level_above.toAscii());

        scene->selectAll();
        QList<QGraphicsItem*> sel_list = scene->selected();
        QList<QGraphicsItem*> sbol_list;

        ItemHandle *cur_gl = scene->globalHandle();

        for(int i=0; i<sel_list.size(); i++)
            {
                ItemHandle *handle = getHandle(sel_list[i]);
                if(handle->isA(tr("sbol")))
                    {
                        sbol_list.append(sel_list[i]);
                    }
            }
        if(level_above == tr(""))
            {
                for(int i=0; i<sbol_list.size(); i++)
                    {
                        sbol_list[i]->hide();
                    }
                for(int i=0; i<top_level_uri.size(); i++)
                    {
                        Collection* cur_co = getCollection(sbol_doc, top_level_uri[i].c_str());
                        DNAComponent* cur_dc = getDNAComponent(sbol_doc, top_level_uri[i].c_str());
                        ItemHandle* cur_handle;
                        if(cur_dc)
                            cur_handle = node_map[(SBOLObject*)cur_dc];
                        else if(cur_co)
                            cur_handle = node_map[(SBOLObject*)cur_co];

                        QList<QGraphicsItem*> cur_items = cur_handle->graphicsItems;
                        for(int j=0; j<cur_items.size(); j++)
                            {
                                cur_items[j]->show();
                            }
                    }
                cur_gl->textData(tr("call_stack")) = cur_gl->textData(tr("call_stack"))+tr("|")
                                        + QString::fromAscii(getCollectionURI(cur_co));
            }
        else if(cur_dc)
            {
                for(int i=0; i<sbol_list.size(); i++)
                    {
                        sbol_list[i]->hide();
                    }
                int cnt = getNumSequenceAnnotationsFor(cur_dc);
                QMessageBox::information(mainWindow,tr("subcomponent number"),QString::fromStdString(SSTR(cnt)));
                DNASequence *cur_ds = getDNAComponentSequence(cur_dc);
                for(int i=0; i<cnt; i++)
                    {
                        SequenceAnnotation *cur_sa = getNthSequenceAnnotationFor(cur_dc,i);
                        DNAComponent* comp_dc = getSequenceAnnotationSubComponent(cur_sa);
                    }
            }
        else if(cur_co)
            {
                for(int i=0; i<sbol_list.size(); i++)
                    {
                        sbol_list[i]->hide();
                    }
                int cnt = getNumDNAComponentsIn(cur_co);
                QMessageBox::information(mainWindow,tr("component number"),QString::fromStdString(SSTR(cnt)));
                cur_gl->textData(tr("call_stack")) = cur_gl->textData(tr("call_stack"))+tr("|")
                                        + QString::fromAscii(getCollectionURI(cur_co));
            }

    }

    void SBOLTool::itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles)
    {
        mode = 0;
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->isA(tr("Part")) && !handles[i]->isA(tr("Empty")) && !handles[i]->tools.contains(this))
					handles[i]->tools += this;
            if (handles[i] && handles[i]->isA(tr("SBOL")))
                {
                    handles[i]->tools += this;
                    if(importing) return;
                    DNAComponent *cur_dc;
                    std::string cur_uri = authority+"/"+NodeHandle::cast(handles[i])->name.toStdString();
                    if(sbol_doc == 0)
                        {
                            sbol_doc = createDocument();
                            doc_map[currentScene()] = sbol_doc;
                        }
                    if (handles[i]->isA(tr("sbol_dnacomponent")))
                        {
                            cur_dc = createDNAComponent(sbol_doc, (char*)cur_uri.c_str());
                            setDNAComponentDisplayID(cur_dc,(char*)NodeHandle::cast(handles[i])->name.toStdString().c_str());
                            console()->message(NodeHandle::cast(handles[i])->name);
                            console()->message(QString::fromStdString(cur_uri));
                            std::string cur_type = "";
                            if (handles[i]->isA(tr("sbol_promoter")))
                                {
                                    cur_type = "promoter";
                                    mode = SBOL_PROMOTER;
                                }
                            if (handles[i]->isA(tr("sbol_assembly-scar")))
                                {
                                    cur_type = "assembly scar";
                                    mode = SBOL_ASSEMBLY_SCAR;
                                }
                            if (handles[i]->isA(tr("sbol_blunt-restriction-site")))
                                {
                                    cur_type = "blunt restriction site";
                                    mode = SBOL_BLUNT_RESTRICTION_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_cds")))
                                {
                                    cur_type = "cds";
                                    mode = SBOL_CDS;
                                }
                            if (handles[i]->isA(tr("sbol_five-prime-overhang")))
                                {
                                    cur_type = "five prime overhang";
                                    mode = SBOL_FIVE_PRIME_OVERHANG;
                                }
                            if (handles[i]->isA(tr("sbol_five-prime-sticky-restriction-site")))
                                {
                                    cur_type = "five prime sticky restriction site";
                                    mode = SBOL_FIVE_PRIME_STICKY_RESTRICTION_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_insulator")))
                                {
                                    cur_type = "insulator";
                                    mode = SBOL_INSULATOR;
                                }
                            if (handles[i]->isA(tr("sbol_operator")))
                                {
                                    cur_type = "operator";
                                    mode = SBOL_OPERATOR;
                                }
                            if (handles[i]->isA(tr("sbol_primer-binding-site")))
                                {
                                    cur_type = "primer binding site";
                                    mode = SBOL_PRIMER_BINDING_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_origin-of-replication")))
                                {
                                    cur_type = "origin of replication";
                                    mode = SBOL_ORIGIN_OF_REPLICATION;
                                }
                            if (handles[i]->isA(tr("sbol_protease-site")))
                                {
                                    cur_type = "protease site";
                                    mode = SBOL_PROTEASE_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_protein-stability-element")))
                                {
                                    cur_type = "protein stability element";
                                    mode = SBOL_PROTEIN_STABILITY_ELEMENT;
                                }
                            if (handles[i]->isA(tr("sbol_restriction-enzyme-recognition-site")))
                                {
                                    cur_type = "restriction enzyme recognition site";
                                    mode = SBOL_RESTRICTION_ENZYME_RECOGNITION_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_ribonuclease-site")))
                                {
                                    cur_type = "ribonuclease site";
                                    mode = SBOL_RIBONUCLEASE_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_ribosome-entry-site")))
                                {
                                    cur_type = "ribosome entry site";
                                    mode = SBOL_RIBOSOME_ENTRY_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_rna-stability-element")))
                                {
                                    cur_type = "rna stability element";
                                    mode = SBOL_RNA_STABILITY_ELEMENT;
                                }
                            if (handles[i]->isA(tr("sbol_signature")))
                                {
                                    cur_type = "signature";
                                    mode = SBOL_SIGNATURE;
                                }
                            if (handles[i]->isA(tr("sbol_terminator")))
                                {
                                    cur_type = "terminator";
                                    mode = SBOL_TERMINATOR;
                                }
                            if (handles[i]->isA(tr("sbol_three-prime-overhang")))
                                {
                                    cur_type = "three prime overhang";
                                    mode = SBOL_THREE_PRIME_OVERHANG;
                                }
                            if (handles[i]->isA(tr("sbol_three-prime-sticky-restriction-site")))
                                {
                                    cur_type = "three prime sticky restriction site";
                                    mode = SBOL_THREE_PRIME_STICKY_RESTRICTION_SITE;
                                }
                            if (handles[i]->isA(tr("sbol_user-defined")))
                                {
                                    cur_type = "user defined";
                                    mode = SBOL_USER_DEFINED;
                                }
                            if (handles[i]->isA(tr("sbol_collection")))
                                {

                                }
                            if (mode != 0)
                                {
                                    setDNAComponentType(cur_dc, r_glymps_map[cur_type].c_str());
                                    mode = 0;
                                }
                            NodeHandle *node = NodeHandle::cast(handles[i]);
                            if (handles[i]->isA(tr("SBOL")) && handles[i]->hasTextData(tr("Text Attributes")))
                            {
                                DataTable<QString> data(handles[i]->textDataTable(tr("Text Attributes")));
                                std::string tempp = cur_uri+"_ds";
                                if(data.hasRow(tr("sequence")))
                                    {
                                        DNASequence *cur_ds = createDNASequence(sbol_doc,(char*)tempp.c_str());
                                        setDNASequenceNucleotides(cur_ds, (char*)data.value(tr("sequence"),0).toLower().toStdString().c_str());
                                        setDNAComponentSequence(cur_dc,cur_ds);
                                    }
                            }
                        }
                    else if(handles[i]->isA("sbol_dna"))
                        {
                            DNASequence * cur_ds;
                            cur_uri = cur_uri+"_ds";
                            console()->message(QString::fromStdString(cur_uri));
                            cur_ds = createDNASequence(sbol_doc, (char*)cur_uri.c_str());
                            if (handles[i]->isA(tr("SBOL")) && handles[i]->hasTextData(tr("Text Attributes")))
                            {
                                DataTable<QString> data(handles[i]->textDataTable(tr("Text Attributes")));
                                setDNASequenceNucleotides(cur_ds, (char*)data.value(tr("sequence"),0).toLower().toStdString().c_str());
                            }
                        }
                    else if(handles[i]->isA("sbol_collection"))
                        {
                            Collection * cur_co;
                            cur_uri = cur_uri+"_co";
                            cur_co = createCollection(sbol_doc,(char*)cur_uri.c_str());
                            setCollectionDisplayID(cur_co,(char*)NodeHandle::cast(handles[i])->name.toStdString().c_str());
                        }



                }
		}
        return;
    }

    void SBOLTool::init_glymps()
    {
        std::map<std::string,std::string> temp_glymps_map;
        temp_glymps_map.clear();
        temp_glymps_map["SO:0000167"] = "promoter";
        temp_glymps_map["SO:0001953"] = "assembly scar";
        temp_glymps_map["SO:0000057"] = "operator";
        temp_glymps_map["SO:0000316"] = "cds";
        temp_glymps_map["SO:0000139"] = "ribosome entry site";
        temp_glymps_map["SO:0000141"] = "terminator";
        temp_glymps_map["SO:0000627"] = "insulator";
        temp_glymps_map["SBOL:0000008"] = "ribonuclease site";
        temp_glymps_map["SO:0001957"] = "rna stability element";
        temp_glymps_map["SO:0001956"] = "protease site";
        temp_glymps_map["SO:0001955"] = "protein stability element";
        temp_glymps_map["SO:0000296"] = "origin of replication";
        temp_glymps_map["SO:0005850"] = "primer binding site";
        temp_glymps_map["SO:0001687"] = "restriction enzyme recognition site";
        temp_glymps_map["SO:0001691"] = "blunt restriction site";
        temp_glymps_map["SBOL:0000016"] = "five prime sticky restriction site";
        temp_glymps_map["SBOL:0000017"] = "three prime sticky restriction site";
        temp_glymps_map["SO:0001933"] = "five prime overhang";
        temp_glymps_map["SO:0001932"] = "three prime overhang";
        temp_glymps_map["SBOL:0000020"] = "signature";
        temp_glymps_map["SBOL:0000021"] = "user defined";

        for(std::map<std::string,std::string>::iterator it = temp_glymps_map.begin(); it != temp_glymps_map.end(); ++it)
            {
                r_glymps_map[it->second] = type_temp+it->first;
                glymps_map[type_temp+it->first] = it->second;
            }
    }

    template<typename T>
    void SBOLTool::co_str_item_changed(T call_function, QLineEdit* cur_item)
    {
        Collection *cur_co;

        GraphicsScene * scene = currentScene();
        if (!scene || scene->selected().size() != 1)
            {

                if (cur_co = getCollection(sbol_doc, CO_uri->text().toAscii()))
                    {
                        (call_function)(cur_co, cur_item->text().toAscii());
                    }
                return;
            }

        QGraphicsItem * selectedItem = scene->selected()[0];

        if(!selectedItem) return;


        NodeGraphicsItem *cur_node = NodeGraphicsItem::cast(selectedItem);
        ItemHandle* hcur_node = getHandle(cur_node);

        std::string cur_uri = authority+"/"+ hcur_node->name.toStdString();

        cur_co = getCollection(sbol_doc, cur_uri.c_str());
        if(!cur_co)
            {
                std::string temp_uri;
                temp_uri = cur_uri+"_co";
                cur_co = getCollection(sbol_doc, (char*)temp_uri.c_str());
            }
        if(cur_co)
            {
                (call_function)(cur_co, cur_item->text().toAscii());
            }
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
        ItemHandle* hcur_node = getHandle(cur_node);

        std::string cur_uri = authority+"/"+ hcur_node->name.toStdString();
        cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());


        if(cur_dc)
            {
                (call_function)(cur_dc, cur_item->text().toAscii());
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
        ItemHandle* hcur_node = getHandle(cur_node);

        cur_dc= getDNAComponent(sbol_doc, hcur_node->name.toAscii());
        DNASequence *cur_ds;

        if(cur_ds = getDNAComponentSequence(cur_dc))
        {
            (call_function)(cur_ds, cur_item->text().toAscii());
        }
        else if(cur_ds = getDNASequence(sbol_doc,(char*)hcur_node->name.toStdString().c_str()))
        {
            (call_function)(cur_ds, cur_item->text().toAscii());
        }
        else
        {
            std::string temp_uri;
            temp_uri = authority+"/"+ hcur_node->name.toStdString()+"_ds";
            cur_ds = getDNASequence(sbol_doc,(char*)temp_uri.c_str());
            if(cur_ds)
                {
                    (call_function)(cur_ds, cur_item->text().toAscii());
                }
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
        ItemHandle* hcur_node = getHandle(cur_node);

        std::string cur_uri = authority+"/"+ hcur_node->name.toStdString();
        cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());

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
        ItemHandle* hcur_node = getHandle(cur_node);

        std::string cur_uri = authority+"/"+ hcur_node->name.toStdString();
        cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());

        cur_dc= getDNAComponent(sbol_doc, hcur_node->name.toAscii());
        SequenceAnnotation *cur_sa;

        if(cur_sa = getSequenceAnnotation(sbol_doc,SA_uri->text().toAscii()))
        {
            (call_function)(cur_sa, cur_item->text().toInt());
        }
    }

    //void SBOLTool::co_uriChanged(){co_str_item_changed(&setCollectionURI, CO_uri);}
    void SBOLTool::co_displayidChanged(){co_str_item_changed(&setCollectionDisplayID, CO_displayId);}
    void SBOLTool::co_nameChanged(){co_str_item_changed(&setCollectionName, CO_name);}
    void SBOLTool::co_descriptionChanged(){co_str_item_changed(&setCollectionDescription,CO_description);}

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

    void SBOLTool::showDS(DNASequence *cur_ds)
    {

        groupBox2->show();
        addDS->hide();
        delDS->show();

        if(!cur_ds)
            {
                return;
            }
        DS_uri->setText(QString::fromAscii(getDNASequenceURI(cur_ds)));
        DS_nucleotides->setText(QString::fromAscii(getDNASequenceNucleotides(cur_ds)));
        return;
    }

    void SBOLTool::showDS()
    {
        DNAComponent * cur_dc;
        cur_dc = getDNAComponent(sbol_doc, DC_uri->text().toStdString().c_str());
        if(!cur_dc)
            {
                return;
            }

        DNASequence * cur_ds = getDNAComponentSequence(cur_dc);
        if(!cur_ds)
            {
                cur_ds_cnt++;
                std::string temp_name = authority_ds+SSTR(cur_ds_cnt);
                cur_ds = createDNASequence(sbol_doc, temp_name.c_str());
                setDNAComponentSequence(cur_dc,cur_ds);
            }
        DS_uri->setText(QString::fromAscii(getDNASequenceURI(cur_ds)));
        DS_nucleotides->setText(QString::fromAscii(getDNASequenceNucleotides(cur_ds)));
        return;
    }

    void SBOLTool::select(int)
    {
        exportSBOL((QSemaphore*)0,tr(""));
    }

	bool SBOLTool::setMainWindow(MainWindow * main)
    {
		Tool::setMainWindow(main);
		console()->message(QCoreApplication::applicationDirPath());
		console()->message(GlobalSettings::homeDir());
        if (mainWindow != 0)
        {
            cur_cnt++;

            setWindowTitle(tr("Information Box"));
            setWindowIcon(QIcon(tr(":/images/about.png")));
            setWindowFlags(Qt::Dialog);
            mainWindow->addToViewMenu(this);

            connect(mainWindow,SIGNAL(mousePressed(GraphicsScene *, QPointF, Qt::MouseButton, Qt::KeyboardModifiers)),
				this,SLOT(sceneClicked(GraphicsScene *, QPointF, Qt::MouseButton, Qt::KeyboardModifiers)));
            connect(mainWindow,
				SIGNAL(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers)),
				this,
				SLOT(itemsSelected(GraphicsScene *, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers))
				);
            connect(mainWindow,SIGNAL(loadNetwork(const QString&, bool*)),this,SLOT(loadNetwork(const QString&, bool*)));
            connect(mainWindow,SIGNAL(saveNetwork(const QString&)),this,SLOT(saveNetwork(const QString&)));
			connect(mainWindow,SIGNAL(toolLoaded(Tool*)),this,SLOT(toolLoaded(Tool*))); // for loading tool. Initialization
            connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)),
						  this, SLOT(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)));
            connect(this, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&, GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&,GraphicsScene::InsertType)));


            connect(this, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)));
            connect(this,SIGNAL(networkLoaded(NetworkHandle*)),mainWindow,SIGNAL(networkLoaded(NetworkHandle*)));

            sbol_doc = 0;
            head_dc = 0;
            importing = false;

		if (mainWindow->tool(tr("Collision Detection")))
		{
			QWidget * widget = mainWindow->tool(tr("Collision Detection"));
			CollisionDetection * collisionDetection = static_cast<CollisionDetection*>(widget);
			if (collisionDetection)
			{
				connect(collisionDetection,SIGNAL(nodeCollided(const QList<QGraphicsItem*>& , NodeGraphicsItem * , const QList<QPointF>&)),
						this, SLOT( nodeCollided(const QList<QGraphicsItem*>& , NodeGraphicsItem * , const QList<QPointF>& )));
			}
		}

        QWidget * widget = mainWindow->tool(tr("Basic Graphics Toolbox"));
        BasicGraphicsToolbar * basicToolBox = static_cast<BasicGraphicsToolbar*>(widget);
        if (basicToolBox)
        {
            connect(this,SIGNAL(alignCompactHorizontal()),basicToolBox, SLOT(alignCompactHorizontal()));
        }

            GlobalSettings::OPEN_FILE_EXTENSIONS << "SBOL" << "sbol";

            if (mainWindow->fileMenu)
            {
                QList<QAction*> actions = mainWindow->fileMenu->actions();

                QAction * targetAction = 0;
                QMenu * exportmenu = 0, * importmenu = 0;

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
                                importmenu = actions[i]->menu();
                                targetAction = actions[i];
                            }
                    }

                if (!exportmenu)
                {
                    for (int i=0; i < actions.size(); ++i)
                        if (actions[i] && actions[i]->text() == tr("&Close page"))
                        {
                            exportmenu = new QMenu(tr("&Export"));
                            importmenu = new QMenu(tr("&Import"));
                            mainWindow->fileMenu->insertMenu(actions[i],importmenu);
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
                    exportmenu->addAction(tr("SBOL"),this,SLOT(saveSBOLFile()));
                }

                if(!importmenu)
                    {
                        importmenu = new QMenu(tr("&Import"));
                        mainWindow->fileMenu->insertMenu(targetAction, importmenu);
                    }
                if(importmenu)
                    {
                        importmenu->addAction(tr("SBOL"),this,SLOT(importSBOLDocument()));
                    }
            }
        }

        return true;
    }


	void SBOLTool::nodeCollided(const QList<QGraphicsItem*>& items, NodeGraphicsItem * item, const QList<QPointF>& )
	{

		GraphicsScene * scene = currentScene();
		if (!scene || !item || items.isEmpty()) return;

		ItemHandle * handle = item->handle();

		QList<QGraphicsItem*> select;

		QGraphicsItem * SBOLItem = 0;

		if ((handle = getHandle(item)) && handle->isA("SBOL"))
		{
			SBOLItem = item;
			select << SBOLItem;
		}
		else
			for (int i=0; i < items.size(); ++i)
				if (NodeGraphicsItem::cast(items[i]) && (handle = getHandle(items[i])) && handle->isA("SBOL"))
				{
					SBOLItem = items[i];
					select << SBOLItem;
					break;
				}

		if (SBOLItem)
		{
			QGraphicsItem * itemLeft = SBOLItem, * itemRight = SBOLItem;
			ItemHandle * h = getHandle(SBOLItem);
			while (itemLeft)
			{
				QRectF p1(itemLeft->sceneBoundingRect());
				p1.adjust(-10.0,0,-10.0,0.0);
				QList<QGraphicsItem*> items = scene->items(p1);
				itemLeft = 0;
				for (int i=0; i < items.size(); ++i)
					if (!select.contains(items[i]) && NodeGraphicsItem::cast(items[i]) && (handle = getHandle(items[i])) && handle->isA("SBOL"))
					{
						itemLeft = items[i];
						select << itemLeft;
						break;
					}
			}
			while (itemRight)
			{
				QRectF p2(itemRight->sceneBoundingRect());
				p2.adjust(10.0,0,10.0,0.0);
				QList<QGraphicsItem*> items = scene->items(p2);
				itemRight = 0;
				for (int i=0; i < items.size(); ++i)
					if (!select.contains(items[i]) && NodeGraphicsItem::cast(items[i]) && (handle = getHandle(items[i])) && handle->isA("SBOL"))
					{
						itemRight = items[i];
						select << itemRight;
						break;
					}
			}

			if (select.isEmpty()) return;

			scene->selected() = select;
			scene->select(0);

			emit alignCompactHorizontal();
		}
	}


void SBOLTool::importSBOLDocument()
{
    QString file = QFileDialog::getOpenFileName(this, tr("import SBOL Document"), homeDir());
    if (file.isNull() || file.isEmpty()) return;
    GraphicsScene * scene = mainWindow->newScene();
    top_level_uri.clear();

    ItemHandle* cur_gl = scene->globalHandle();

    cur_gl->textData(tr("call_stack")) = tr("");

    console()->message(file);
    deleteDocument(sbol_doc);
    sbol_doc = createDocument();
    doc_map[scene] = sbol_doc;
    readDocument(sbol_doc, (char*)file.toStdString().c_str());

    console()->message("File Read");
    console()->message(QString::fromAscii(writeDocumentToString(sbol_doc)));
    console()->message(QString::fromStdString(SSTR(getNumDNAComponents(sbol_doc))));
    int numDNAComponent = getNumDNAComponents(sbol_doc);
    int numDNASequence = getNumDNASequences(sbol_doc);
    int numCollection = getNumCollections(sbol_doc);
    importing = true;

    bool import_all = true;
    std::map<SBOLObject*, bool> obj_map;

    obj_map.clear();


    for(int i=0; i<numCollection; i++)
        {
            Collection* cur_co = getNthCollection(sbol_doc,i);
            node_map[(SBOLObject*)cur_co] = renderSBOLDocument((SBOLObject*)cur_co);
//            if(!import_all)
                {
                    for(int j=0; j<getNumDNAComponentsIn(cur_co); j++)
                        {
                            obj_map[(SBOLObject*)getNthDNAComponentIn(cur_co,j)] = true;
                        }
                }
            top_level_uri.push_back(getCollectionURI(cur_co));
        }
//    if(!import_all)
        {
            for(int i=0; i<numDNAComponent; i++)
            {
                DNAComponent *cur_dc = getNthDNAComponent(sbol_doc,i);
                if(cur_dc)
                    {
                        int numsa = getNumSequenceAnnotationsFor(cur_dc);
                        for(int j=0; j<numsa; j++)
                            {
                                SequenceAnnotation *cur_sa = getNthSequenceAnnotationFor(cur_dc,j);
                                if(cur_sa)
                                    {
                                        obj_map[(SBOLObject*)getSequenceAnnotationSubComponent(cur_sa)] = true;
                                    }
                            }
                    }
            }
        }
    for(int i=0; i<numDNAComponent; i++)
        {
            DNAComponent *cur_dc = getNthDNAComponent(sbol_doc,i);
//            if(obj_map.find((SBOLObject*)cur_dc) == obj_map.end())
                {
                    NodeHandle* cur_node = renderSBOLDocument((SBOLObject*)cur_dc);
                    node_map[(SBOLObject*)cur_dc] = cur_node;
                    if(obj_map.find((SBOLObject*)cur_dc) != obj_map.end())
                        {
                            for(int j=0; j<cur_node->graphicsItems.size(); j++)
                                {
                                    cur_node->graphicsItems[j]->hide();
                                }
      //                  if(!import_all)
                        }
                    else
                        {
                            top_level_uri.push_back(getDNAComponentURI(cur_dc));
                        }
                    {
                        DNASequence *cur_ds = getDNAComponentSequence(cur_dc);
                        if(cur_ds)
                            {
                                obj_map[(SBOLObject*)cur_ds] = true;
                            }
                    }

                }
        }
    for(int i=0; i<numDNASequence; i++)
        {
            DNASequence *cur_ds = getNthDNASequence(sbol_doc,i);
//            if(obj_map.find((SBOLObject*) cur_ds) == obj_map.end())
                {
                    NodeHandle* cur_node = renderSBOLDocument((SBOLObject*)getNthDNASequence(sbol_doc,i));
                    node_map[(SBOLObject*)cur_ds] = cur_node;
                    if(obj_map.find((SBOLObject*) cur_ds) != obj_map.end())
                        {
                            QList<QGraphicsItem*> cur_graphics = cur_node->graphicsItems;
                            for(int j=0; j<cur_node->graphicsItems.size(); j++)
                                {
                                    cur_node->graphicsItems[j]->hide();
                                }
                        }
                    else
                        {
                            top_level_uri.push_back(getDNASequenceURI(cur_ds));
                        }
                }
        }



    importing = false;
    currentScene()->fitAll();
    scene->setSceneRect(0.0,0.0,10000.0,10000.0);
    scene->update();

}
NodeHandle* SBOLTool::renderSBOLDocument(SBOLObject* target)
{
    //screw that. positioning problem will be solved later.
    GraphicsScene * scene = currentScene();
    if(!scene || !mainWindow) return 0;
    if(!mainWindow->tool(tr("Nodes Tree"))) return 0;



    QWidget* treeWidget = mainWindow->tool(tr("Nodes Tree"));
    NodesTree * nodesTree = static_cast<NodesTree*>(treeWidget);
    DNAComponent * dc_target = (DNAComponent*) target;
    std::string cur_type;
    NodeFamily * nodeFamily;
    DNASequence *ds_target;
    Collection *co_target;
    std::string cur_seq;
    std::string display_name;



    //Start to Render the target
    if(isDNAComponent(sbol_doc,target))
        {

         //Define node
            dc_target = (DNAComponent*) target;
            display_name = getDNAComponentDisplayID(dc_target);
            cur_type = getDNAComponentType(dc_target);
            cur_type = "sbol_"+glymps_map[cur_type];
            if (!nodesTree->getFamily(QString::fromStdString(cur_type))) return 0;
            nodeFamily = nodesTree->getFamily(QString::fromStdString(cur_type));

            if(getDNAComponentSequence(dc_target))
            {
                ds_target = getDNAComponentSequence(dc_target);
                cur_seq = getDNASequenceNucleotides(ds_target);
            }
        }
    else if(isDNASequence(sbol_doc,target))
        {
            ds_target = (DNASequence*) target;
            display_name = getDNASequenceURI(ds_target);
            cur_type = "sbol_dna";
            if (!nodesTree->getFamily(QString::fromStdString(cur_type))) return 0;
            nodeFamily = nodesTree->getFamily(QString::fromStdString(cur_type));
            cur_seq = getDNASequenceNucleotides(ds_target);
        }
    else if(isCollection(sbol_doc,target))
        {
            co_target = (Collection*) target;
            display_name = getCollectionDisplayID(co_target);
            cur_type = "sbol_collection";
            if (!nodesTree->getFamily(QString::fromStdString(cur_type))) return 0;
            nodeFamily = nodesTree->getFamily(QString::fromStdString(cur_type));
        }

    NodeHandle * hnode = new NodeHandle(nodeFamily);
    //hnode->name = QString::fromAscii(getDNAComponentDisplayID(dc_target));
    hnode->name = QString::fromStdString(display_name);
    hnode->type = NodeHandle::TYPE;





    //setup graphicsItem

    NodeGraphicsItem * node = 0, *image = 0;


    for(int i=0; i<nodeFamily->graphicsItems.size(); ++i)
        {
            image = (NodeGraphicsItem::topLevelNodeItem(nodeFamily->graphicsItems[i]));
            if (image)
                {
                    node = image->clone();
                }
        }

    if (ds_target)
        {
            DataTable<QString> new_dat;
            new_dat.value(tr("sequence"),0) = QString::fromStdString(cur_seq);
            hnode->textDataTable(tr("Text Attributes")) = new_dat;
        }
    node->className = tr("NodeGraphicsItem");

    setHandle(node,hnode);

    QString s;
    QTransform t;
    console()->message(QString::number(qrand()));
    QPoint p(qrand()%1000,qrand()%1000);
    qreal z;

    //NodeGraphicsItem * node = readNode(nodeReader,s,t,p,z,sn);

    int sceneNumber= 0;
    z = 2.0;
//work on

    t.setMatrix(1.0, 0.0, 0.0, 0.0 ,1.0, 0.0, 0.0, 0.0, 1.0);

    //will it move funky behavior?
    node->setBoundingBoxVisible(false);
    node->setPos(QPointF());
    node->resetTransform();
    node->normalize();
    node->setTransform(t);
    node->setPos(mapToParent(p).rx(),mapToParent(p).ry());
    node->setZValue(z);
    QList<QGraphicsItem*> items;

    items << node;

    TextGraphicsItem *text = new TextGraphicsItem;
    setHandle(text, hnode);

    t.setMatrix(1.0, 0.0, 0.0, 0.0 ,1.0, 0.0, 0.0, 0.0, 1.0);

    QFont cur_font;
    cur_font.setFamily(tr("MS Shell Dlg 2"));
    cur_font.setPointSize(22);
    text->setParent((QObject*)node);
    text->setPlainText(QString::fromStdString(display_name));
    text->setPos(node->boundingRect().width()/2, node->boundingRect().height());
    text->setTransform(t);
    text->setZValue(z+1);
    text->setFont(cur_font);

    items << text;
    QList<QUndoCommand*> commands;
    QList<ItemHandle*> handles = getHandle(items);

    emit itemsAboutToBeInserted(scene, items , handles, commands, GraphicsScene::LOADED);

    commands << new InsertGraphicsCommand(tr("insert"),scene,items);

    QUndoCommand * command = new CompositeCommand(tr("load"), commands);
    scene->network->push(command);
    node->setPos(p.rx(),p.ry());
    text->setPos(node->boundingRect().center().x(),node->boundingRect().bottom());
    node->update();
    text->update();
    scene->update();
    scene->network->updateSymbolsTable();

    emit itemsInserted((NetworkHandle*) 0 , handles);
    emit itemsInserted(scene, items, handles, GraphicsScene::LOADED);
    emit networkLoaded(scene->network);

    return hnode;

}


void SBOLTool::saveSBOLFile()
{
    exportSBOL((QSemaphore*)0, tr(""));

    return;
}

void SBOLTool::exportSBOL(QSemaphore* sem, QString file)
{
    sbol_doc = doc_map[currentScene()];
    bool flag = false;
    QList<QGraphicsItem*> sel_list = currentScene()->selected();
    if(sel_list.size() == 0)
        {
            currentScene()->selectAll();
            sel_list = currentScene()->selected();
        }
    for(int i=0; i<sel_list.size(); i++)
        {
            ItemHandle *handle = getHandle(sel_list[i]);
            if(!handle|| (!handle->isA("Part") && !handle->isA("SBOL")))
                {
                    flag = true;
                    break;
                }
        }
    if(flag)
        {
            QMessageBox::information(mainWindow,tr("SBOL export"), tr("some of selected item(s) is/are not compatible for SBOL export"));
            return;
        }

    Document *exp_sbol_doc = 0;

    if(sbol_doc)
        {
            QString ffile = QFileDialog::getSaveFileName (this, tr("Save SBOL file"), homeDir());
            if (ffile.isNull() || ffile.isEmpty()) return;
            writeDocumentToFile(sbol_doc,ffile.toAscii());
            return;
        }
    else
        {
            exp_sbol_doc = createDocument();
        }


    for(int i=0; i<sel_list.size(); i++)
        {
            ItemHandle *handle = getHandle(sel_list[i]);
            if(!handle->isA("SBOL"))
                {
                    DNAComponent *cur_dc;
                    DNASequence *cur_ds;
                    std::string cur_type = "";
                    std::string cur_desc = "";
                    if(handle->isA("coding"))
                        {
                            cur_type = "cds";
                        }
                    else if(handle->isA("rbs"))
                        {
                            cur_type = "ribosome entry site";
                        }
                    else if(handle->isA("terminator"))
                        {
                            cur_type = "terminator";
                        }
                    else if(handle->isA("promoter"))
                        {
                            cur_type = "promoter";
                        }
                    else if(handle->isA("operator"))
                        {
                            cur_type = "operator";
                            if(handle->isA("repressor binding site"))
                                {
                                    cur_desc = "repressor binding site";
                                }
                            else if(handle->isA("activator binding site"))
                                {
                                    cur_desc = "activator binding site";

                                }
                        }
                    NodeHandle *cur_node = NodeHandle::cast(handle);
                    std::string cur_uri = authority+"/"+cur_node->name.toStdString();
                    cur_dc = createDNAComponent(exp_sbol_doc,(char*)cur_uri.c_str());
                    setDNAComponentDisplayID(cur_dc, (char*)cur_node->name.toStdString().c_str());
                    if(cur_type != "")
                        {
                            setDNAComponentType(cur_dc, (char*)r_glymps_map[cur_type].c_str());
                        }
                    if(cur_desc != "")
                        {
                            setDNAComponentDescription(cur_dc,cur_desc.c_str());
                        }
                    if (handle->hasTextData(tr("Text Attributes")))
                    {
                        DataTable<QString> data(handle->textDataTable(tr("Text Attributes")));
                        std::string tempp = cur_uri+"_ds";
                        if(data.hasRow(tr("sequence")))
                            {
                                cur_ds = createDNASequence(exp_sbol_doc,(char*)tempp.c_str());
                                setDNASequenceNucleotides(cur_ds, (char*)data.value(tr("sequence"),0).toLower().toStdString().c_str());
                                setDNAComponentSequence(cur_dc,cur_ds);
                            }
                    }
                }
        }

    QString ffile = QFileDialog::getSaveFileName (this, tr("Save SBOL file"), homeDir());
    if (ffile.isNull() || ffile.isEmpty()) return;
    writeDocumentToFile(exp_sbol_doc,ffile.toAscii());
    deleteDocument(exp_sbol_doc);
    return;
}

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
        doc_map[currentScene()] = sbol_doc;
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
}


	void SBOLTool::saveItems(NetworkHandle * network, const QString& filename)
	{
	}

	void SBOLTool::saveNetwork(const QString& filename)
	{
	    if(!sbol_doc) return;
	    console()->message("save started");
	    NetworkHandle * network = currentNetwork();
		if (!network || filename.isEmpty()) return;
        console()->message("save passed!");
		QList<GraphicsScene*> scenes = network->scenes();
		network->globalHandle()->textData("sbol") = QString::fromAscii(writeDocumentToString(sbol_doc));
	}

void SBOLTool::itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
{
	if (!scene) return;
	sbol_doc = doc_map[scene];

	ItemHandle *handle = 0;
	for(int i=0; i<items.size(); i++)
        {
            handle = getHandle(items[i]);
            if(handle && handle->isA("SBOL"))
                {
                    console()->message("SBOL OBJECT!!");
                    show();
                }
            if(!NodeGraphicsItem::cast(items[i])) return;
            NodeGraphicsItem *node = NodeGraphicsItem::cast(items[i]);
            std::string cur_uri = authority+"/"+handle->name.toStdString();
            console()->message(QString::fromStdString(cur_uri));
            /*if(!getDNAComponent(sbol_doc,(char*)cur_uri.c_str()))
                {
                    return;
                }*/
        }
    if(items.size() ==0) return;

    hideDS();
    hideSA();
	QList<QGraphicsItem*> nodeItems;
	NodeGraphicsItem * node = 0;
	for(int i=0; i<items.size(); i++)
        {
            ItemHandle *handle = getHandle(items[i]);
            node = NodeGraphicsItem::cast(items[i]);
            if(node)
                {
                    console()->message(node->name);
                    std::string cur_uri = authority+"/"+ handle->name.toStdString();
                    console()->message(QString::fromStdString(cur_uri));
                    console()->message(handle->name);
                    DNAComponent *cur_dc = getDNAComponent(sbol_doc, cur_uri.c_str());
                    DNASequence *cur_ds = getDNASequence(sbol_doc,(char*)handle->name.toStdString().c_str());
                    if(!cur_ds)
                        {
                            std::string temp_uri;
                            temp_uri = cur_uri+"_ds";
                            cur_ds = getDNASequence(sbol_doc,(char*)temp_uri.c_str());
                        }
                    Collection *cur_co = getCollection(sbol_doc, cur_uri.c_str());
                    if(!cur_co)
                        {
                            std::string temp_uri;
                            temp_uri = cur_uri+"_co";
                            cur_co = getCollection(sbol_doc, (char*)temp_uri.c_str());
                        }
                    if(cur_dc)
                        {
                            groupBox0->hide();
                            groupBox1->show();
                            DC_uri->setText(QString::fromAscii(getDNAComponentURI(cur_dc)));
                            std::string cur_temp = getDNAComponentType(cur_dc);
                            console()->message(QString::fromStdString(cur_temp));
                            DC_type->setText(QString::fromStdString(glymps_map[cur_temp]));
                            DC_displayId->setText(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));
                            DC_name->setText(QString::fromAscii(getDNAComponentName(cur_dc)));
                            DC_description->setText(QString::fromAscii(getDNAComponentDescription(cur_dc)));

                            cur_ds = getDNAComponentSequence(cur_dc);
                            if(!cur_ds)
                                {
                                    hideDS();
                                }
                            else
                                {
                                    showDS();
                                }

                            if(head_dc)
                                {
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
                                }
                                else
                                    {
                                        hideSA();
                                    }
                            }
                            else if(cur_ds)
                                {
                                    groupBox0->hide();
                                    groupBox1->hide();
                                    showDS(cur_ds);

                                }
                            else if(cur_co)
                                {
                                    groupBox0->show();
                                    groupBox1->hide();
                                    CO_uri->setText(QString::fromAscii(getCollectionURI(cur_co)));
                                    CO_displayId->setText(QString::fromAscii(getCollectionDisplayID(cur_co)));
                                    CO_name->setText(QString::fromAscii(getCollectionName(cur_co)));
                                    CO_description->setText(QString::fromAscii(getCollectionDescription(cur_co)));

                                }
                    /*console()->message(QString::fromAscii(getDNAComponentURI(cur_dc)));
                    console()->message(QString::fromAscii(getDNAComponentDisplayID(cur_dc)));*/
                }
        }

        //show();

}

    void SBOLTool::itemsDropped(GraphicsScene * scene, const QString& name, QPointF point)
	{
	}

    void SBOLTool::sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
	{
	    hide();
        return;
    }

	void SBOLTool::toolLoaded(Tool* tool)
	{
		static bool connected1 = false;
	}
}

