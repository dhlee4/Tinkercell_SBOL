/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT

 This tool handles connections that represent SBOL

****************************************************************************/

#ifndef TINKERCELL_SBOLTOOL_H
#define TINKERCELL_SBOLTOOL_H

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

#include "GraphicsScene.h"
#include "NodeGraphicsItem.h"
#include "NodeGraphicsReader.h"
#include "NodeGraphicsWriter.h"
#include "NodesTree.h"
#include "ConnectionsTree.h"
#include "CatalogWidget.h"
#include "ItemHandle.h"
#include "Tool.h"



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

	signals:
		void itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&);
		void dataChanged(const QList<ItemHandle*>&);

	private slots:
		void toolLoaded (Tool * tool);
//		void mouseDoubleClicked (GraphicsScene * scene, QPointF point, QGraphicsItem *, Qt::MouseButton, Qt::KeyboardModifiers modifiers);
//		void keyPressed(GraphicsScene*,QKeyEvent *);
//		void itemsAboutToBeInserted(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands);
		void itemsDropped(GraphicsScene *, const QString&, QPointF);
		void sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
		void saveSBOLFile();
		void exportSBOL(QSemaphore*, const QString&);

		void showSA();
        void showDS();
        void hideSA();
        void hideDS();

	private:
		ConnectionsTree * connectionsTree;
		NodesTree * nodesTree;
		CatalogWidget * catalogWidget;
		int mode;
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

        std::string current_type;


	};


}


#endif
