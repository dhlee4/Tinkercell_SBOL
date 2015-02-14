<<<<<<< HEAD
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- "Parameters" and "Text Attributes".
Attributes are essentially a <name,value> pair that are used to characterize an item.

The BasicInformationTool also comes with two GraphicalTools, one for text attributes and one
for numerical attributes. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/

#ifndef TINKERCELL_MODELSUMMARYTOOL_H
#define TINKERCELL_MODELSUMMARYTOOL_H

#include <stdlib.h>
#include <QtGui>
#include <QIcon>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHash>
#include <QGroupBox>
#include <QUndoCommand>
#include <QGraphicsRectItem>
#include <QGraphicsWidget>
#include <QSlider>
#include <QToolButton>
#include <QButtonGroup>
#include <QTableView>
#include <QTabWidget>
#include <QPushButton>

#include "NodeGraphicsItem.h"
#include "DataTable.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "MainWindow.h"
#include "NetworkHandle.h"
#include "SpinBoxDelegate.h"

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
	class TINKERCELLEXPORT ModelSummaryTool : public Tool
	{
		Q_OBJECT;

	public:
		ModelSummaryTool();
		bool setMainWindow(MainWindow * main);
		QSize sizeHint() const;

	signals:
		void aboutToDisplayModel(const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void dataChanged(const QList<ItemHandle*>&);

	public slots:
		void select(int i=0);
		void deselect(int i=0);
		void updateToolTips(const QList<ItemHandle*>&);
		void itemsInserted(NetworkHandle *, const QList<ItemHandle*>& handles);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& , const QList<ItemHandle*>& );
		void keyPressed(GraphicsScene* scene,QKeyEvent * keyEvent);
		void mouseDoubleClicked(GraphicsScene* scene, QPointF, QGraphicsItem* item, Qt::MouseButton, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
		void setValue(int,int);

	protected:
		QGroupBox groupBox;
		QTableWidget tableWidget;
		TextComboDoubleDelegate delegate;
		QWidget * currentWidget;
		QTabWidget * tabWidget;

	private slots:
		void currentChanged ( int index );
		void updateTables();
		void fixedAction();

	private:
		bool openedByUser;
		NodeGraphicsItem item;
		QList<ItemHandle*> itemHandles;
		QAction * toggleFixedAction, *separator;
		QPushButton * closeButton;

		friend class VisualTool;
	};
}

#endif
=======
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- "Parameters" and "Text Attributes".
Attributes are essentially a <name,value> pair that are used to characterize an item.

The BasicInformationTool also comes with two GraphicalTools, one for text attributes and one
for numerical attributes. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/

#ifndef TINKERCELL_MODELSUMMARYTOOL_H
#define TINKERCELL_MODELSUMMARYTOOL_H

#include <stdlib.h>
#include <QtGui>
#include <QIcon>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHash>
#include <QGroupBox>
#include <QUndoCommand>
#include <QGraphicsRectItem>
#include <QGraphicsWidget>
#include <QSlider>
#include <QToolButton>
#include <QButtonGroup>
#include <QTableView>
#include <QTabWidget>
#include <QPushButton>

#include "NodeGraphicsItem.h"
#include "DataTable.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "MainWindow.h"
#include "NetworkHandle.h"
#include "SpinBoxDelegate.h"

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
	class TINKERCELLEXPORT ModelSummaryTool : public Tool
	{
		Q_OBJECT;

	public:
		ModelSummaryTool();
		bool setMainWindow(MainWindow * main);
		QSize sizeHint() const;

	signals:
		void aboutToDisplayModel(const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void dataChanged(const QList<ItemHandle*>&);

	public slots:
		void select(int i=0);
		void deselect(int i=0);
		void updateToolTips(const QList<ItemHandle*>&);
		void itemsInserted(NetworkHandle *, const QList<ItemHandle*>& handles);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& , const QList<ItemHandle*>& );
		void keyPressed(GraphicsScene* scene,QKeyEvent * keyEvent);
		void mouseDoubleClicked(GraphicsScene* scene, QPointF, QGraphicsItem* item, Qt::MouseButton, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
		void setValue(int,int);

	protected:
		QGroupBox groupBox;
		QTableWidget tableWidget;
		TextComboDoubleDelegate delegate;
		QWidget * currentWidget;
		QTabWidget * tabWidget;

	private slots:
		void currentChanged ( int index );
		void updateTables();
		void fixedAction();

	private:
		bool openedByUser;
		NodeGraphicsItem item;
		QList<ItemHandle*> itemHandles;
		QAction * toggleFixedAction, *separator;
		QPushButton * closeButton;

		friend class VisualTool;
	};
}

#endif
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
