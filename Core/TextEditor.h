<<<<<<< HEAD
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This file defines the TextEditor class. The TextEditor and Canvas are two ways to define a network.
The Canvas is used for graphical reprentation of a network, whereas the TextEditor is used for
text-based representation of a network.

****************************************************************************/

#ifndef TINKERCELL_TEXTEDITOR_H
#define TINKERCELL_TEXTEDITOR_H

#include <stdlib.h>
#include <QtGui>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QString>
#include <QFileDialog>
#include <QtDebug>
#include <QTextEdit>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QLabel>
#include <QSplitter>
#include <QGridLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QSyntaxHighlighter>
#include <QUndoCommand>

#include "DataTable.h"
#include "CodeEditor.h"
#include "HistoryWindow.h"
#include "SymbolsTable.h"
#include "Tool.h"

namespace Tinkercell
{
	class ItemHandle;
	class ItemData;
	class TextEditor;
	class TextUndoCommand;
	class MainWindow;
	class ConsoleWindow;

	/*! \brief This is the window that allows used to construct networks using text, as
	opposed to graphics, which is done by GraphicsScene. The TextEditor requires a supporting
	tool that parses the text and calls the itemsInserted or itemsRemoved methods. Without a
	supporting parser tool, the TextEditor will not do anything.
	\ingroup core
	*/
	class TINKERCELLCOREEXPORT TextEditor : public CodeEditor
	{
		Q_OBJECT
		
		friend class TextUndoCommand;
		friend class NetworkWindow;
		friend class NetworkHandle;
		friend class SymbolsTable;
		friend class MainWindow;

	public:

		static bool SideBarEnabled;

		/*! \brief default constructor*/
		TextEditor( NetworkHandle * , QWidget * parent = 0);
		/*! \brief destructor -- removes all the text items*/
		~TextEditor();
		/*! \brief insert a text item
		\param ItemHandle* the item*/
		void insert( ItemHandle* );
		/*! \brief insert text items
		\param QList<ItemHandle*> the items*/
		void insert( const QList<ItemHandle*>& );
		/*! \brief remove an item
		\param ItemHandle* the item*/
		void remove( ItemHandle* );
		/*! \brief remove text items
		\param QList<ItemHandle*> the items*/
		void remove( const QList<ItemHandle*>& );
		/*! \brief clear existing items and insert new items
		\param QList<ItemHandle*> the new items*/
		void setItems( const QList<ItemHandle*>& );
		/*! \brief all the items represented by the text in this TextEditor*/
		QList<ItemHandle*>& items();
		/*! \brief push a command to the undo/redo stack
		\param QUndoCommand* */
		void push(QUndoCommand*);
		/*! \brief gets the selected text*/
		QString selectedText() const;
		/*!
		* \brief the context menu that is shown during right-click event on a text editor with text selected.
		Plugins can add new actions to this menu.
		*/
		QMenu * contextSelectionMenu;
		/*!
		* \brief the context menu that is shown during right-click event on a text editor with no text selected.
		Plugins can add new actions to this menu.
		*/
		QMenu * contextEditorMenu;
		/*! \brief the network handle represented in this text editor*/
		NetworkHandle * network;
		/*! \brief the network window containing this text editor*/
		NetworkWindow * networkWindow;
		/*! \brief the main window containing this network*/
		MainWindow * mainWindow() const;
		/*! \brief same as network->mainWindow->console()*/
		ConsoleWindow * console() const;
		/*! \brief same as networkWindow->handle*/
		ItemHandle * localHandle() const;
		/*! \brief same as network->globalHandle()*/
		ItemHandle * globalHandle() const;

	signals:
		/*! \brief some text inside this editor has been changed
		\param QString old text
		\param QString new text
		*/
		void textChanged(TextEditor *, const QString&, const QString&, const QString&);
		/*! \brief the cursor has moved to a different line
		\param int index of the current line
		\param QString current line text
		*/
		void lineChanged(TextEditor *, int, const QString&);
		/*!
		* \brief signal that is emitted when items are inserted in this TextEditor.
		* \param NetworkHandle* 
		* \param QList<ItemHandle*> new item handles
		*/
		void itemsInserted(NetworkHandle *, const QList<ItemHandle*>&);
		/*!
		* \brief signal that is emitted when items are removed from this TextEditor.
		* \param NetworkHandle* 
		* \param QList<ItemHandle*> removed item handles
		*/
		void itemsRemoved(NetworkHandle *, const QList<ItemHandle*>& );
		/*! \brief request to parse the text in the current text editor
		\param TextEditor* editor
		*/
		void parse(TextEditor *);

	public slots:

		/*! \brief calls main window's popOut
		* \return void*/
		virtual void popOut();
		/*! \brief calls main window's popIn
		* \return void*/
		virtual void popIn();
		/*! \brief undo last edit*/
		virtual void undo();
		/*! \brief redo last undo*/
		virtual void redo();
		/*! \brief select all text*/
		virtual void selectAll();
		/*! \brief copy selected text*/
		virtual void copy();
		/*! \brief cut selected text*/
		virtual void cut();
		/*! \brief paste text from clipboard*/
		virtual void paste();
		/*! \brief print text
		\param QPrinter */
		virtual void print(QPrinter * printer);

	protected:
		/*! \brief previously accessed line number. This is to keep track of when a line is modified*/
		int prevBlockNumber;
		/*! \brief current line number. This is to keep track of when a line is modified*/
		int changedBlockNumber;
		/*! \brief previously accessed line. This is to keep track of when a line is modified*/
		QString prevBlockText;
		/*! \brief current line. This is to keep track of when a line is modified*/
		QString changedBlockText;
		/*! \brief current text. This is to keep track of when the text is modified*/
		QString prevText;
		/*! \brief listens to keyboard events in order to determine when the current line has changed*/
		virtual void keyPressEvent ( QKeyEvent * event );
		/*! \brief listens to mouse events just to activate this window*/
		virtual void mousePressEvent ( QMouseEvent * event );

		/*! \brief all the items represented by the text in this TextEditor*/
		QList<ItemHandle*> allItems;
		/*! \brief creates context menu with actions in the contextMenu member*/
		virtual void contextMenuEvent ( QContextMenuEvent * event );
		/*! \brief emits line changed and text changed if needed*/
		virtual void mouseReleaseEvent ( QMouseEvent * event );

	private slots:
		/*! \brief finds out whether a whole line has changed */
		void textChangedSlot();
	};

	/*! \brief this command performs a text change
	* \ingroup undo*/
	class TINKERCELLCOREEXPORT TextUndoCommand : public QUndoCommand
	{

	public:
		/*! \brief constructor
		* \param TextEditor* editor where change happened
		* \param QString new text
		*/
		TextUndoCommand(TextEditor *, const QString&, const QString&);
		/*! \brief redo the change*/
		void redo();
		/*! \brief undo the change*/
		void undo();

	private:
		/*! \brief text before*/
		QString oldText;
		/*! \brief text after*/
		QString newText;
		/*! \brief TextEditor where the change happened*/
		TextEditor * textEdit;
	};
}
#endif
=======
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This file defines the TextEditor class. The TextEditor and Canvas are two ways to define a network.
The Canvas is used for graphical reprentation of a network, whereas the TextEditor is used for
text-based representation of a network.

****************************************************************************/

#ifndef TINKERCELL_TEXTEDITOR_H
#define TINKERCELL_TEXTEDITOR_H

#include <stdlib.h>
#include <QtGui>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QString>
#include <QFileDialog>
#include <QtDebug>
#include <QTextEdit>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QLabel>
#include <QSplitter>
#include <QGridLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QSyntaxHighlighter>
#include <QUndoCommand>

#include "DataTable.h"
#include "CodeEditor.h"
#include "HistoryWindow.h"
#include "SymbolsTable.h"
#include "Tool.h"

namespace Tinkercell
{
	class ItemHandle;
	class ItemData;
	class TextEditor;
	class TextUndoCommand;
	class MainWindow;
	class ConsoleWindow;

	/*! \brief This is the window that allows used to construct networks using text, as
	opposed to graphics, which is done by GraphicsScene. The TextEditor requires a supporting
	tool that parses the text and calls the itemsInserted or itemsRemoved methods. Without a
	supporting parser tool, the TextEditor will not do anything.
	\ingroup core
	*/
	class TINKERCELLCOREEXPORT TextEditor : public CodeEditor
	{
		Q_OBJECT
		
		friend class TextUndoCommand;
		friend class NetworkWindow;
		friend class NetworkHandle;
		friend class SymbolsTable;
		friend class MainWindow;

	public:

		static bool SideBarEnabled;

		/*! \brief default constructor*/
		TextEditor( NetworkHandle * , QWidget * parent = 0);
		/*! \brief destructor -- removes all the text items*/
		~TextEditor();
		/*! \brief insert a text item
		\param ItemHandle* the item*/
		void insert( ItemHandle* );
		/*! \brief insert text items
		\param QList<ItemHandle*> the items*/
		void insert( const QList<ItemHandle*>& );
		/*! \brief remove an item
		\param ItemHandle* the item*/
		void remove( ItemHandle* );
		/*! \brief remove text items
		\param QList<ItemHandle*> the items*/
		void remove( const QList<ItemHandle*>& );
		/*! \brief clear existing items and insert new items
		\param QList<ItemHandle*> the new items*/
		void setItems( const QList<ItemHandle*>& );
		/*! \brief all the items represented by the text in this TextEditor*/
		QList<ItemHandle*>& items();
		/*! \brief push a command to the undo/redo stack
		\param QUndoCommand* */
		void push(QUndoCommand*);
		/*! \brief gets the selected text*/
		QString selectedText() const;
		/*!
		* \brief the context menu that is shown during right-click event on a text editor with text selected.
		Plugins can add new actions to this menu.
		*/
		QMenu * contextSelectionMenu;
		/*!
		* \brief the context menu that is shown during right-click event on a text editor with no text selected.
		Plugins can add new actions to this menu.
		*/
		QMenu * contextEditorMenu;
		/*! \brief the network handle represented in this text editor*/
		NetworkHandle * network;
		/*! \brief the network window containing this text editor*/
		NetworkWindow * networkWindow;
		/*! \brief the main window containing this network*/
		MainWindow * mainWindow() const;
		/*! \brief same as network->mainWindow->console()*/
		ConsoleWindow * console() const;
		/*! \brief same as networkWindow->handle*/
		ItemHandle * localHandle() const;
		/*! \brief same as network->globalHandle()*/
		ItemHandle * globalHandle() const;

	signals:
		/*! \brief some text inside this editor has been changed
		\param QString old text
		\param QString new text
		*/
		void textChanged(TextEditor *, const QString&, const QString&, const QString&);
		/*! \brief the cursor has moved to a different line
		\param int index of the current line
		\param QString current line text
		*/
		void lineChanged(TextEditor *, int, const QString&);
		/*!
		* \brief signal that is emitted when items are inserted in this TextEditor.
		* \param NetworkHandle* 
		* \param QList<ItemHandle*> new item handles
		*/
		void itemsInserted(NetworkHandle *, const QList<ItemHandle*>&);
		/*!
		* \brief signal that is emitted when items are removed from this TextEditor.
		* \param NetworkHandle* 
		* \param QList<ItemHandle*> removed item handles
		*/
		void itemsRemoved(NetworkHandle *, const QList<ItemHandle*>& );
		/*! \brief request to parse the text in the current text editor
		\param TextEditor* editor
		*/
		void parse(TextEditor *);

	public slots:

		/*! \brief calls main window's popOut
		* \return void*/
		virtual void popOut();
		/*! \brief calls main window's popIn
		* \return void*/
		virtual void popIn();
		/*! \brief undo last edit*/
		virtual void undo();
		/*! \brief redo last undo*/
		virtual void redo();
		/*! \brief select all text*/
		virtual void selectAll();
		/*! \brief copy selected text*/
		virtual void copy();
		/*! \brief cut selected text*/
		virtual void cut();
		/*! \brief paste text from clipboard*/
		virtual void paste();
		/*! \brief print text
		\param QPrinter */
		virtual void print(QPrinter * printer);

	protected:
		/*! \brief previously accessed line number. This is to keep track of when a line is modified*/
		int prevBlockNumber;
		/*! \brief current line number. This is to keep track of when a line is modified*/
		int changedBlockNumber;
		/*! \brief previously accessed line. This is to keep track of when a line is modified*/
		QString prevBlockText;
		/*! \brief current line. This is to keep track of when a line is modified*/
		QString changedBlockText;
		/*! \brief current text. This is to keep track of when the text is modified*/
		QString prevText;
		/*! \brief listens to keyboard events in order to determine when the current line has changed*/
		virtual void keyPressEvent ( QKeyEvent * event );
		/*! \brief listens to mouse events just to activate this window*/
		virtual void mousePressEvent ( QMouseEvent * event );

		/*! \brief all the items represented by the text in this TextEditor*/
		QList<ItemHandle*> allItems;
		/*! \brief creates context menu with actions in the contextMenu member*/
		virtual void contextMenuEvent ( QContextMenuEvent * event );
		/*! \brief emits line changed and text changed if needed*/
		virtual void mouseReleaseEvent ( QMouseEvent * event );

	private slots:
		/*! \brief finds out whether a whole line has changed */
		void textChangedSlot();
	};

	/*! \brief this command performs a text change
	* \ingroup undo*/
	class TINKERCELLCOREEXPORT TextUndoCommand : public QUndoCommand
	{

	public:
		/*! \brief constructor
		* \param TextEditor* editor where change happened
		* \param QString new text
		*/
		TextUndoCommand(TextEditor *, const QString&, const QString&);
		/*! \brief redo the change*/
		void redo();
		/*! \brief undo the change*/
		void undo();

	private:
		/*! \brief text before*/
		QString oldText;
		/*! \brief text after*/
		QString newText;
		/*! \brief TextEditor where the change happened*/
		TextEditor * textEdit;
	};
}
#endif
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
