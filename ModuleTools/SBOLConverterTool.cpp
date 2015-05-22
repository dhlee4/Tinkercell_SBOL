
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
#include "GlobalSettings.h"
#include "BasicGraphicsToolbar.h"
#include "SBOLConverterTool.h"
#include <sstream>
#include <functional>
#include <fstream>
#include <map>
namespace Tinkercell
{

    void SBOLConverterTool::toolLoaded (Tool * tool)
    {

    }

    void SBOLConverterTool::itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles)
    {
		for (int i=0; i < handles.size(); ++i)
		{
			if (handles[i] && handles[i]->isA(tr("sbol_dnacomponent")))
					handles[i]->tools += this;
		}
        return;
    }

    void SBOLConverterTool::select(int)
    {
        scenelist->clear();
        QList<NetworkHandle*> list = mainWindow->networks();
		for (int i=0; i < list.size(); ++i)
			if (list[i])
			{
                scenelist->addItem(new QListWidgetItem(list[i]->windowTitle()));
			}
        show();
    }

    void SBOLConverterTool::paste_part_scene()
    {
        QListWidgetItem temp;

        GraphicsScene *scene;
        QList<NetworkHandle*> list = mainWindow->networks();
        if(scenelist->selectedItems().size() == 1)
            {
                for (int i=0; i < list.size(); ++i)
                    if (list[i])
                    {
                        if(scenelist->selectedItems()[0]->text() == list[i]->windowTitle())
                            {
                                for(int j=0; j<list[i]->scenes().size(); j++)
                                    {
                                        scene = list[i]->scenes()[j];
                                        if(scene)
                                            {
                                                break;
                                            }
                                    }

                            }
                    }


//copy start
    NodeFamily * nodeFamily;
        if(!scene)
            {
                QMessageBox::information(mainWindow,tr("Error")
                    , tr("Scene is not valid!"));
                hide();
                return;
            }
    QWidget* treeWidget = mainWindow->tool(tr("Nodes Tree"));
    NodesTree * nodesTree = static_cast<NodesTree*>(treeWidget);
    nodeFamily = nodesTree->getFamily("promoter");

    NodeHandle * hnode = new NodeHandle(nodeFamily);
    hnode->name = tr("aa");
    hnode->type = NodeHandle::TYPE;

    NodeGraphicsItem * node = 0, *image = 0;

    for(int i=0; i<nodeFamily->graphicsItems.size(); ++i)
        {
            image = (NodeGraphicsItem::topLevelNodeItem(nodeFamily->graphicsItems[i]));
            if (image)
                {
                    node = image->clone();
                }
        }

/*    if (ds_target)
        {
            DataTable<QString> new_dat;
            new_dat.value(tr("sequence"),0) = QString::fromStdString(cur_seq);
            hnode->textDataTable(tr("Text Attributes")) = new_dat;
        }
*/

    node->className = tr("NodeGraphicsItem");

    setHandle(node,hnode);

    QString s;
    QTransform t;
    QPointF p(100,100);
    qreal z;

    z = 2.0;

    if (node)
    {
        t.setMatrix(1.0, 0.0, 0.0, 0.0 ,1.0, 0.0, 0.0, 0.0, 1.0);
        node->refresh();
    }


    node->setTransform(t);
    node->setPos(p);
    node->setZValue(z);
    node->scale(node->defaultSize.width()/node->sceneBoundingRect().width(),
                                 node->defaultSize.height()/node->sceneBoundingRect().height());

    QList<QGraphicsItem*> items;

    items << node;

    TextGraphicsItem *text = new TextGraphicsItem;
    setHandle(text, hnode);

    t.setMatrix(1.0, 0.0, 0.0, 0.0 ,1.0, 0.0, 0.0, 0.0, 1.0);

    QFont cur_font;
    cur_font.setFamily(tr("MS Shell Dlg 2"));
    cur_font.setPointSize(8);

    text->setPlainText(tr("ss"));
    text->moveBy(node->boundingRect().center().x(),node->boundingRect().bottom());

    text->setZValue(z+1);
    text->setFont(cur_font);

    items << text;
    QList<QUndoCommand*> commands;
    QList<ItemHandle*> handles = getHandle(items);

    emit itemsAboutToBeInserted(scene, items , handles, commands, GraphicsScene::LOADED);

    commands << new InsertGraphicsCommand(tr("insert"),scene,items);

    QUndoCommand * command = new CompositeCommand(tr("load"), commands);
    command->redo();
    scene->network->updateSymbolsTable();

    emit itemsInserted((NetworkHandle*) 0 , handles);
    emit itemsInserted(scene, items, handles, GraphicsScene::LOADED);
    emit networkLoaded(scene->network);

//copy end




                hide();
                return;
            }
        else
            {
                QMessageBox::information(mainWindow,tr("Select One Scene")
                         , tr("Please select one scene to paste"));
            }

    }
    void SBOLConverterTool::p_cancel()
    {
        hide();
        return;
    }

    SBOLConverterTool::SBOLConverterTool() : Tool(tr("Convert SBOL to Part"))
    {
        /*GlobalSettings::OPEN_FILE_EXTENSIONS << "TIC" << "tic";
        GlobalSettings::SAVE_FILE_EXTENSIONS << "TIC" << "tic";
        countHistory = 0;
        restoreDialog = 0;
        restoreButton = 0;*/
        scenelist = new QListWidget;
        layout = new QVBoxLayout;
        layout->addWidget(scenelist);
        layout->addWidget(b_paste = new QPushButton("Paste Part", this));
        layout->addWidget(b_cancel = new QPushButton("Cancel", this));
        setLayout(layout);

        connect(b_paste, SIGNAL(pressed()), this, SLOT(paste_part_scene()));
        connect(b_cancel, SIGNAL(pressed()), this, SLOT(p_cancel()));



        NodeGraphicsReader reader;
        reader.readXml(&item,tr(":/images/DNATool.xml"));
		item.setToolTip(tr("SBOL Converter"));
		setToolTip(tr("SBOL Converter"));

		item.normalize();
		item.scale(35.0/item.sceneBoundingRect().width(),35.0/item.sceneBoundingRect().height());

		ToolGraphicsItem * gitem = new ToolGraphicsItem(this);
		addGraphicsItem(gitem);
		gitem->addToGroup(&item);

		addAction(QIcon(), tr("SBOL Converter"), tr("Convert SBOL objects into Part"));

    }

    bool SBOLConverterTool::setMainWindow(MainWindow * main)
    {
        Tool::setMainWindow(main);
        if (mainWindow)
        {
            connect(this, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&, GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsInserted(GraphicsScene * , const QList<QGraphicsItem*>& , const QList<ItemHandle*>&,GraphicsScene::InsertType)));


            connect(this, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)), mainWindow, SIGNAL(itemsAboutToBeInserted(GraphicsScene * , QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType)));
            connect(this,SIGNAL(networkLoaded(NetworkHandle*)),mainWindow,SIGNAL(networkLoaded(NetworkHandle*)));

            /*
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
*/
            connect(mainWindow,SIGNAL(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)),
						  this, SLOT(itemsInserted(NetworkHandle*,const QList<ItemHandle*>&)));
            return true;
        }
        return false;
    }

}
