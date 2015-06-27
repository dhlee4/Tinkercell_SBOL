/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT

 This tool handles connections that represent SBOL

****************************************************************************/

#ifndef TINKERCELL_SBOLTOOL_H
#define TINKERCELL_SBOLTOOL_H

#define LIBXML_STATIC
//for Errors of SBOL.h -> .cpp

#include <QtGui>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QAction>
#include <QFile>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QUndoCommand>
#include <QGraphicsRectItem>
#include <QGraphicsWidget>
#include <QSlider>
#include <QToolButton>
#include <QButtonGroup>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPair>
#include <QAbstractButton>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QToolBar>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDockWidget>
#include <QScrollArea>
#include <QSplashScreen>
#include <QDialog>
#include <string>
#include <map>

#include "GraphicsScene.h"
#include "NodeGraphicsItem.h"
#include "NodeGraphicsReader.h"
#include "NodeGraphicsWriter.h"
#include "NodesTree.h"
#include "ConnectionsTree.h"
#include "CatalogWidget.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "collisiondetection.h"


#ifndef TINKERCELLEXPORT
#ifdef Q_WS_WIN
#   if defined(TC_EXPORTS) || defined(TinkerCell_EXPORTS)
#       define TINKERCELLEXPORT __declspec(dllexport)
#   else
#       define TINKERCELLEXPORT __declspec(dllimport)
#   endif
#else
#    define TINKERCELLEXPORT
#endif
#endif


extern "C"{
#include "sbol.h"
}

namespace Tinkercell
{
	/*! This class allows users to insert wet-lab experiment in the scene
	*/
	class TINKERCELLEXPORT SBOLTool : public Tool
	{
		Q_OBJECT

	public:
		SBOLTool();
		bool setMainWindow(MainWindow * main);
		template<typename T> void dc_str_item_changed(T call_function, QLineEdit*);
		template<typename T> void ds_str_item_changed(T call_function, QLineEdit*);
		template<typename T> void sa_str_item_changed(T call_function, QLineEdit*);
		template<typename T> void sa_int_item_changed(T call_function, QLineEdit*);
		template<typename T> void co_str_item_changed(T call_function, QLineEdit*);
    signals:
		void dataChanged(const QList<ItemHandle*>&);
		void itemsAboutToBeInserted(GraphicsScene * scene, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles,GraphicsScene::InsertType);
        void networkLoaded(NetworkHandle*);
        void alignCompactHorizontal();


	private slots:
	    void select(int);
	    void toolLoaded (Tool * tool);
//		void mouseDoubleClicked (GraphicsScene * scene, QPointF point, QGraphicsItem *, Qt::MouseButton, Qt::KeyboardModifiers modifiers);
//		void itemsAboutToBeInserted(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands);
		void itemsDropped(GraphicsScene *, const QString&, QPointF);
		void sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
		void itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles);
        //void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles,GraphicsScene::InsertType);
        void nodeCollided(const QList<QGraphicsItem*>& items, NodeGraphicsItem * item, const QList<QPointF>& );

		void saveSBOLFile();
		void exportSBOL(QSemaphore*, QString);
        void loadNetwork(const QString& filename, bool * b);
        void saveNetwork(const QString& filename);
        void saveItems(NetworkHandle*, const QString& filename);

		void dc_uriChanged();
		void dc_displayidChanged();
        void dc_nameChanged();
        void dc_descriptionChanged();

        //void co_uriChanged();
		void co_displayidChanged();
        void co_nameChanged();
        void co_descriptionChanged();

        void ds_uriChanged();
        void ds_nucleotidesChanged();

        void sa_uriChanged();
        void sa_bioStartChanged();
        void sa_bioEndChanged();
        void sa_strandChanged();

		void showSA();
        void showDS();
        void showDS(DNASequence *);
        void hideSA();
        void hideDS();
        void level_up();
        void level_down();

        void importSBOLDocument();
        NodeHandle* renderSBOLDocument(SBOLObject *target);

	private:
		ConnectionsTree * connectionsTree;
		NodesTree * nodesTree;
		CatalogWidget * catalogWidget;
		int mode;
		QGroupBox * groupBox0;//for collection
        QGroupBox * groupBox1;
        QGroupBox * groupBox2;
        QGroupBox * groupBox3;

        QLineEdit *SA_uri;
        QLineEdit *SA_bioStart;
        QLineEdit *SA_bioEnd;
        QLineEdit *SA_strand;

        QLineEdit *DC_uri;
        QLineEdit *DC_displayId;
        QLineEdit *DC_name;
        QLineEdit *DC_description;
        QLineEdit *DC_type;

        QLineEdit *CO_uri;
        QLineEdit *CO_displayId;
        QLineEdit *CO_name;
        QLineEdit *CO_description;

        std::string authority = "http://example.com/dc";
        std::string authority_ds = "http://example.com/ds";
        std::string authority_sa = "http://example.com/sa";
        int cur_cnt = 0;
        int cur_ds_cnt = 0;
        int cur_sa_cnt = 0;
        QLineEdit *DS_uri;
        QLineEdit *DS_nucleotides;

        QPushButton * addSA;
        QPushButton * addDS;
        QPushButton * delSA;
        QPushButton * delDS;
        QPushButton * leveldown;
        QPushButton * levelup;
        NodeGraphicsItem item;

        std::string current_type;
//        Document * sbol_doc;
        DNAComponent * head_dc;
        void init_glymps();
        std::map<std::string, std::string> glymps_map, r_glymps_map;
        std::string type_temp;
        bool importing;
        std::map<SBOLObject*, NodeHandle*> node_map;
        std::map<GraphicsScene*, Document*> doc_map;
        std::vector<std::string> top_level_uri;

	};


}


#endif
