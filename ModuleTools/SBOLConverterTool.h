#ifndef SBOLCONVERTERTOOL_H_INCLUDED
#define SBOLCONVERTERTOOL_H_INCLUDED

/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT

 This tool handles connections that represent SBOL

****************************************************************************/

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



namespace Tinkercell
{

	class TINKERCELLEXPORT SBOLConverterTool : public Tool
	{
		Q_OBJECT

	public:
		SBOLConverterTool();
		bool setMainWindow(MainWindow * main);

    signals:
		void itemsAboutToBeInserted(GraphicsScene * scene, QList<QGraphicsItem*>& , QList<ItemHandle*>&, QList<QUndoCommand*>&,GraphicsScene::InsertType);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles,GraphicsScene::InsertType);
        void networkLoaded(NetworkHandle*);

	private slots:
	    void select(int);
	    void toolLoaded (Tool * tool);
		void itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles);
		void p_cancel();
		void paste_part_scene();

	private:
        NodeGraphicsItem item;
        QVBoxLayout * layout;
        QListWidget * scenelist;

        QPushButton * b_paste;
        QPushButton * b_cancel;

	};


}

#endif // SBOLCONVERTERTOOL_H_INCLUDED
