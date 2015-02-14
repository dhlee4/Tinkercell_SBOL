<<<<<<< HEAD
/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT
 
 Automatically manage gene regulatory network matters

****************************************************************************/

#ifndef TINKERCELL_DNASEQUENCEVIEWINGTOOL_H
#define TINKERCELL_DNASEQUENCEVIEWINGTOOL_H

#include <QtGui>
#include <QtDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTimeLine>
#include <QTextEdit>
#include <QTextCursor>
#include <QGroupBox>

#include "NodeGraphicsItem.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "LabelingTool.h"

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
	class DNASequenceViewer;
	
	class DNASequenceViewerTextEdit : public QTextEdit
	{
		Q_OBJECT
		
	public:
		
		DNASequenceViewerTextEdit(QWidget * parent = 0);
		void updateText(const QList<ItemHandle*>& nodes);
		void updateNodes();
		
	signals:
		void clearLabels();
		void highlight(ItemHandle*,QColor);
		void sequenceChanged(ItemHandle* , const QString&);
		void updateAnnotationsTable(ItemHandle*);
		
	protected:
		//virtual void contextMenuEvent ( QContextMenuEvent * event );
		//virtual void mouseDoubleClickEvent ( QMouseEvent * event );
		virtual void mouseMoveEvent ( QMouseEvent * e );
		virtual void keyPressEvent ( QKeyEvent * event );
		
		int currentNodeIndex();
		QList<ItemHandle*> nodes;
		QList<QColor> colors;
		
		friend class DNASequenceViewer;
	};

	class TINKERCELLEXPORT DNASequenceViewer : public Tool
	{
		Q_OBJECT
	public:
		DNASequenceViewer();
		bool setMainWindow(MainWindow*);
		
	public slots:
		void select(int i=0);
		void deselect(int i=0);
		void itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles);
		void itemsRemoved(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&);
		void itemsSelected(GraphicsScene *,const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers);
		void toolLoaded(Tool*);
		void displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&);
		void sequenceChanged(ItemHandle*, const QString&);
		void updateAnnotationsTable(ItemHandle*);
	
	private slots:
	
		void tableValueChanged(int,int);
		
	private:
	
		DNASequenceViewerTextEdit * textEdit;
		QTableWidget * annotationsTable;
		QGroupBox * annotationsGroup;
		ItemHandle * currentHandle;
		
		bool updateText(GraphicsScene *,const QList<QGraphicsItem*>&);
		bool openedByUser;
		NodeGraphicsItem item;
		QDockWidget * dockWidget;
		
		friend class DNASequenceViewerTextEdit;
	};
}

#endif

=======
/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT
 
 Automatically manage gene regulatory network matters

****************************************************************************/

#ifndef TINKERCELL_DNASEQUENCEVIEWINGTOOL_H
#define TINKERCELL_DNASEQUENCEVIEWINGTOOL_H

#include <QtGui>
#include <QtDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTimeLine>
#include <QTextEdit>
#include <QTextCursor>
#include <QGroupBox>

#include "NodeGraphicsItem.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "LabelingTool.h"

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
	class DNASequenceViewer;
	
	class DNASequenceViewerTextEdit : public QTextEdit
	{
		Q_OBJECT
		
	public:
		
		DNASequenceViewerTextEdit(QWidget * parent = 0);
		void updateText(const QList<ItemHandle*>& nodes);
		void updateNodes();
		
	signals:
		void clearLabels();
		void highlight(ItemHandle*,QColor);
		void sequenceChanged(ItemHandle* , const QString&);
		void updateAnnotationsTable(ItemHandle*);
		
	protected:
		//virtual void contextMenuEvent ( QContextMenuEvent * event );
		//virtual void mouseDoubleClickEvent ( QMouseEvent * event );
		virtual void mouseMoveEvent ( QMouseEvent * e );
		virtual void keyPressEvent ( QKeyEvent * event );
		
		int currentNodeIndex();
		QList<ItemHandle*> nodes;
		QList<QColor> colors;
		
		friend class DNASequenceViewer;
	};

	class TINKERCELLEXPORT DNASequenceViewer : public Tool
	{
		Q_OBJECT
	public:
		DNASequenceViewer();
		bool setMainWindow(MainWindow*);
		
	public slots:
		void select(int i=0);
		void deselect(int i=0);
		void itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles);
		void itemsRemoved(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&);
		void itemsSelected(GraphicsScene *,const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers);
		void toolLoaded(Tool*);
		void displayModel(QTabWidget&, const QList<ItemHandle*>&, QHash<QString,qreal>&, QHash<QString,QString>&);
		void sequenceChanged(ItemHandle*, const QString&);
		void updateAnnotationsTable(ItemHandle*);
	
	private slots:
	
		void tableValueChanged(int,int);
		
	private:
	
		DNASequenceViewerTextEdit * textEdit;
		QTableWidget * annotationsTable;
		QGroupBox * annotationsGroup;
		ItemHandle * currentHandle;
		
		bool updateText(GraphicsScene *,const QList<QGraphicsItem*>&);
		bool openedByUser;
		NodeGraphicsItem item;
		QDockWidget * dockWidget;
		
		friend class DNASequenceViewerTextEdit;
	};
}

#endif

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
