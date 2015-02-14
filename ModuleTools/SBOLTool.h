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

	private slots:
		void toolLoaded (Tool * tool);
//		void mouseDoubleClicked (GraphicsScene * scene, QPointF point, QGraphicsItem *, Qt::MouseButton, Qt::KeyboardModifiers modifiers);
//		void keyPressed(GraphicsScene*,QKeyEvent *);
//		void itemsAboutToBeInserted(GraphicsScene* scene, QList<QGraphicsItem *>& items, QList<ItemHandle*>& handles, QList<QUndoCommand*>& commands);
		void itemsDropped(GraphicsScene *, const QString&, QPointF);
		void sceneClicked(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

	private:
		ConnectionsTree * connectionsTree;
		NodesTree * nodesTree;
		CatalogWidget * catalogWidget;
		int mode;
        QGroupBox * groupBox1;
        QGroupBox * groupBox2;
        QLineEdit * name1;
        QLineEdit * name2;
        QLineEdit * conc;
        QLineEdit * rate;

	};


}


#endif