/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This tool allows users to edit a text graphics item on the sceen.
The tool also updates the name of a handle when the text item is changed (and vise versa)

****************************************************************************/
#ifndef TINKERCELL_TEXTGRAPHICSTOOL_H
#define TINKERCELL_TEXTGRAPHICSTOOL_H

#include <QtGui>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QtDebug>
#include <QAction>
#include <QFile>
#include <QUndoCommand>
#include <QGraphicsRectItem>
#include <QGraphicsWidget>
#include <QSlider>
#include <QToolButton>
#include <QToolBar>
#include <QButtonGroup>
#include <QTableView>

#include "NetworkHandle.h"
#include "ItemHandle.h"
#include "Tool.h"
#include "TextGraphicsItem.h"

namespace Tinkercell
{

	class ChangeParentCommand;

	/*!
	\brief A tool that provides GUI feature for placing and editing text objects
	\ingroup plugins
	*/
	class TextGraphicsTool : public Tool
	{
		Q_OBJECT;

	public:
		TextGraphicsTool(QToolBar*);
		bool setMainWindow(MainWindow * main);
		void setText(TextGraphicsItem* item, const QString& text);
	signals:
		void itemsRenamed(NetworkHandle*, const QList<ItemHandle*>&, const QList<QString>&, const QList<QString>&);
	public slots:
		void itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>& handles);
		void itemsAboutToBeMoved(GraphicsScene *, QList<QGraphicsItem*>& , QList<QPointF>&, QList<QUndoCommand*>&);
		void insertText();
		void insertTextWith();
		void mousePressed(GraphicsScene*,QPointF, Qt::MouseButton, Qt::KeyboardModifiers);
		void itemsSelected(GraphicsScene*,const QList<QGraphicsItem*>&, QPointF, Qt::KeyboardModifiers);
		void itemsRemoved(GraphicsScene*,QList<QGraphicsItem*>&, QList<ItemHandle*>&, QList<QUndoCommand*>&);
		void mouseDoubleClicked (GraphicsScene*, QPointF, QGraphicsItem *, Qt::MouseButton, Qt::KeyboardModifiers);
		void keyPressed(GraphicsScene*,QKeyEvent*);
		void escapeSignal(const QWidget*);
		void getFont();
	private:
		void clear();
		TextGraphicsItem * targetItem;
		QString oldText;
		QWidget widget;
		bool inserting;
		ChangeParentCommand * command;
		QFont font;
		QToolBar * toolBar;
		QAction * fontAction;
	};

	/*! \brief this command changes the name of the handle of an item*/
	class ChangeTextCommand : public QUndoCommand
	{
	public:
		ChangeTextCommand(const QString& name, QGraphicsItem * item, const QString& newname);
		ChangeTextCommand(const QString& name, const QList<QGraphicsItem*>& items, const QList<QString>& newnames);
		ChangeTextCommand(const QString& name, QGraphicsItem * item, const QString& newname, const QFont& newfont);
		ChangeTextCommand(const QString& name, const QList<QGraphicsItem*>& items, const QList<QString>& newnames, const QList<QFont>& newfonts);
		void redo();
		void undo();
	private:
		QList<TextGraphicsItem*> textItems;
		QList<QFont> oldFont;
		QList<QFont> newFont;
		QList<QString> oldText;
		QList<QString> newText;
	};

}

#endif
