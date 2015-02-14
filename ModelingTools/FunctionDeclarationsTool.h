<<<<<<< HEAD
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- Parameters and "Text Attributes".
Attributes are essentially a <name,value> pair that are used to characterize an item.

The AssignmentFunctionsTool also comes with two GraphicalTools, one for text attributes and one
for numerical attributes. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/

#ifndef TINKERCELL_ASSIGNMENTSANDFUNCTIONSTOOL_H
#define TINKERCELL_ASSIGNMENTSANDFUNCTIONSTOOL_H

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
#include <QSplashScreen>

#include "EquationGraph.h"
#include "NodeGraphicsItem.h"
#include "DataTable.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "NetworkHandle.h"
#include "MainWindow.h"

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
	/*!
	\brief This class provides the C API for the ConnectionInsertion class
	\ingroup capi
	*/
	class AssignmentFunctionsTool_FToS : public QObject
	{
		Q_OBJECT
	signals:
		void getForcingFunctionNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void getForcingFunctionAssignments(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void addForcingFunction(QSemaphore*,ItemHandle*,const QString&, const QString&);
		
	public slots:
		tc_strings getForcingFunctionNames(tc_items);
		tc_strings getForcingFunctionAssignments(tc_items);
		void addForcingFunction(long,const char*, const char*);
	};

	/*!
	\brief Inserts the "Functions" and "Assignments" data into all items and provides
	the widgets for viewing and editing each. The same table widget is used for both;
	the text is parsed to determine whether it is an assignment or function.
	\ingroup plugins
	*/
	class TINKERCELLEXPORT AssignmentFunctionsTool : public Tool
	{
		Q_OBJECT

	signals:
		void highlightItem(ItemHandle*,QColor);
		void clearLabels(ItemHandle * h);
	
	public:
		QList<ItemHandle*> itemHandles;
		AssignmentFunctionsTool();
		bool setMainWindow(MainWindow * main);
		QSize sizeHint() const;

	public slots:
		void eval();
		void select(int i=0);
		void deselect(int i=0);

		void setValue(int i, int j);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles, GraphicsScene::InsertType type);
		void itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers);
		void toolLoaded(Tool*);
		void addAttribute();
		void removeSelectedAttributes();
		void historyUpdate(int);
		void setupFunctionPointers( QLibrary * );

		void aboutToDisplayModel(const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void mouseMoved(GraphicsScene* scene, QGraphicsItem*, QPointF point, Qt::MouseButton, Qt::KeyboardModifiers, QList<QGraphicsItem*>& items);

	private slots:
		void getForcingFunctionNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void getForcingFunctionAssignments(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void addForcingFunction(QSemaphore*,ItemHandle*,const QString&, const QString&);
		void clickedFunctionImage();

	protected:
		void insertDataMatrix(ItemHandle * handle);
		QTableWidget tableWidget;
		QList<ItemHandle*> tableItems;

		void updateTable();

		static AssignmentFunctionsTool_FToS * fToS;
		void connectTCFunctions();

		virtual void keyPressEvent ( QKeyEvent * event );

	private:

		QStringList updatedFunctions, updatedFunctionNames;

		bool openedByUser;
		NodeGraphicsItem item;
		EquationGraph * graphWidget;
		QDialog * snapshotToolTip;
		QToolButton * snapshotIcon;
		QHash<QString, QPixmap> functionSnapshots;

		static tc_strings _getForcingFunctionNames(tc_items);
		static tc_strings _getForcingFunctionAssignments(tc_items);
		static void _addForcingFunction(long,const char*, const char*);
		static QString Self;
	};


}

#endif

=======
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class adds the "attributes" data to each item in Tinkercell.
Two types of attributes are added -- Parameters and "Text Attributes".
Attributes are essentially a <name,value> pair that are used to characterize an item.

The AssignmentFunctionsTool also comes with two GraphicalTools, one for text attributes and one
for numerical attributes. The buttons are drawn as NodeGraphicsItems using the datasheet.xml and
textsheet.xml files that define the NodeGraphicsItems.

****************************************************************************/

#ifndef TINKERCELL_ASSIGNMENTSANDFUNCTIONSTOOL_H
#define TINKERCELL_ASSIGNMENTSANDFUNCTIONSTOOL_H

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
#include <QSplashScreen>

#include "EquationGraph.h"
#include "NodeGraphicsItem.h"
#include "DataTable.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "NetworkHandle.h"
#include "MainWindow.h"

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
	/*!
	\brief This class provides the C API for the ConnectionInsertion class
	\ingroup capi
	*/
	class AssignmentFunctionsTool_FToS : public QObject
	{
		Q_OBJECT
	signals:
		void getForcingFunctionNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void getForcingFunctionAssignments(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void addForcingFunction(QSemaphore*,ItemHandle*,const QString&, const QString&);
		
	public slots:
		tc_strings getForcingFunctionNames(tc_items);
		tc_strings getForcingFunctionAssignments(tc_items);
		void addForcingFunction(long,const char*, const char*);
	};

	/*!
	\brief Inserts the "Functions" and "Assignments" data into all items and provides
	the widgets for viewing and editing each. The same table widget is used for both;
	the text is parsed to determine whether it is an assignment or function.
	\ingroup plugins
	*/
	class TINKERCELLEXPORT AssignmentFunctionsTool : public Tool
	{
		Q_OBJECT

	signals:
		void highlightItem(ItemHandle*,QColor);
		void clearLabels(ItemHandle * h);
	
	public:
		QList<ItemHandle*> itemHandles;
		AssignmentFunctionsTool();
		bool setMainWindow(MainWindow * main);
		QSize sizeHint() const;

	public slots:
		void eval();
		void select(int i=0);
		void deselect(int i=0);

		void setValue(int i, int j);
		void itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& item, const QList<ItemHandle*>& handles, GraphicsScene::InsertType type);
		void itemsSelected(GraphicsScene*, const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers);
		void toolLoaded(Tool*);
		void addAttribute();
		void removeSelectedAttributes();
		void historyUpdate(int);
		void setupFunctionPointers( QLibrary * );

		void aboutToDisplayModel(const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void displayModel(QTabWidget& widgets, const QList<ItemHandle*>& items, QHash<QString,qreal>& constants, QHash<QString,QString>& equations);
		void mouseMoved(GraphicsScene* scene, QGraphicsItem*, QPointF point, Qt::MouseButton, Qt::KeyboardModifiers, QList<QGraphicsItem*>& items);

	private slots:
		void getForcingFunctionNames(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void getForcingFunctionAssignments(QSemaphore*,QStringList*,const QList<ItemHandle*>&);
		void addForcingFunction(QSemaphore*,ItemHandle*,const QString&, const QString&);
		void clickedFunctionImage();

	protected:
		void insertDataMatrix(ItemHandle * handle);
		QTableWidget tableWidget;
		QList<ItemHandle*> tableItems;

		void updateTable();

		static AssignmentFunctionsTool_FToS * fToS;
		void connectTCFunctions();

		virtual void keyPressEvent ( QKeyEvent * event );

	private:

		QStringList updatedFunctions, updatedFunctionNames;

		bool openedByUser;
		NodeGraphicsItem item;
		EquationGraph * graphWidget;
		QDialog * snapshotToolTip;
		QToolButton * snapshotIcon;
		QHash<QString, QPixmap> functionSnapshots;

		static tc_strings _getForcingFunctionNames(tc_items);
		static tc_strings _getForcingFunctionAssignments(tc_items);
		static void _addForcingFunction(long,const char*, const char*);
		static QString Self;
	};


}

#endif

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
