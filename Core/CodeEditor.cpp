/****************************************************************************
This file is a combination of two example programs included in the Qt Toolkit.
A few modifications have been added, but the majority of the code is from Qt's
demo programs
****************************************************************************/

#include <QtGui>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "CodeEditor.h"

namespace Tinkercell
{

	void CodeEditor::setCompleter(QCompleter *completer)
	{
		if (c)
			QObject::disconnect(c, 0, this, 0);

		c = completer;

		if (!c)
			return;

		c->setWidget(this);
		c->setCompletionMode(QCompleter::PopupCompletion);
		c->setCaseSensitivity(Qt::CaseInsensitive);
		QObject::connect(c, SIGNAL(activated(const QString&)),
			this, SLOT(insertCompletion(const QString&)));
	}

	QCompleter *CodeEditor::completer() const
	{
		return c;
	}

	void CodeEditor::insertCompletion(const QString& completion)
	{
		if (c->widget() != this)
			return;
		QTextCursor tc = textCursor();
		int extra = completion.length() - c->completionPrefix().length();
		tc.movePosition(QTextCursor::Left);
		tc.movePosition(QTextCursor::EndOfWord);
		tc.insertText(completion.right(extra));
		setTextCursor(tc);
	}

	QString CodeEditor::textUnderCursor() const
	{
		QTextCursor tc = textCursor();
		tc.select(QTextCursor::WordUnderCursor);
		return tc.selectedText();
	}

	void CodeEditor::focusInEvent(QFocusEvent *e)
	{
		if (c)
			c->setWidget(this);
		QPlainTextEdit::focusInEvent(e);
	}

	void CodeEditor::keyPressEvent(QKeyEvent *e)
	{
		if (c && c->popup()->isVisible()) {
			// The following keys are forwarded by the completer to the widget
			switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return; // let the completer do default behavior
		default:
			break;
			}
		}
		
		if (e->modifiers() & Qt::ControlModifier)
		{ 
			if (e->key() == Qt::Key_Equal || e->key() == Qt::Key_Plus) //zoom in
			{
				zoomIn();
				return;
			}
			if (e->key() == Qt::Key_Minus || e->key() == Qt::Key_Underscore) //zoom out
			{
				zoomOut();
				return;
			}
		}
		
		QString space;
		if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
		{
			QString s = textCursor().block().text();
			int i = 0;
			for (i=0; i < s.length() && s[i].isSpace(); ++i) { }
			if (i > 0)
				space = s.left(i);
		}

		bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
		if (!c || !isShortcut) // dont process the shortcut when we have a completer
			QPlainTextEdit::keyPressEvent(e);
		
		if (!space.isEmpty())
		{
			QTextCursor cursor = textCursor();
			cursor.insertText(space);
		}

		const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
		if (!c || (ctrlOrShift && e->text().isEmpty()))
			return;

		static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
		bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
		QString completionPrefix = textUnderCursor();

		if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
			|| eow.contains(e->text().right(1)))) {
				c->popup()->hide();
				return;
		}

		if (completionPrefix != c->completionPrefix()) {
			c->setCompletionPrefix(completionPrefix);
			c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
		}
		QRect cr = cursorRect();
		cr.setWidth(c->popup()->sizeHintForColumn(0)
			+ c->popup()->verticalScrollBar()->sizeHint().width());
		c->complete(cr); // popup it up!
	}


	CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), c(0)
	{
		lineNumberArea = new LineNumberArea(this);
		setTabStopWidth ( 8 );
		lineHighlightColor = QColor(tr("#FCFFAE"));
		lineNumberBackground = QColor(tr("#E1E1E1"));
		lineNumberText = QColor(tr("#002446"));

		connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
		connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
		connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

		updateLineNumberAreaWidth(0);
		highlightCurrentLine();
		
		/* create find replace dialog */
		findReplaceDialog = new QDialog(this);
		findLineEdit = new QLineEdit;
		replaceLineEdit = new QLineEdit;
		QPushButton * findButton = new QPushButton("&Find");
		QPushButton * replaceButton = new QPushButton("&Replace");
		QPushButton * closeButton = new QPushButton("&Cancel");
		
		QHBoxLayout * hlayout; 
		QVBoxLayout * vlayout = new QVBoxLayout;
		
		hlayout = new QHBoxLayout;
		hlayout->addWidget(new QLabel("   find: "));
		hlayout->addWidget(findLineEdit);
		vlayout->addLayout(hlayout,0);
		
		hlayout = new QHBoxLayout;
		hlayout->addWidget(new QLabel("replace: "));
		hlayout->addWidget(replaceLineEdit);
		vlayout->addLayout(hlayout,0);
		
		hlayout = new QHBoxLayout;
		hlayout->addSpacing(1);
		hlayout->addWidget(findButton);
		hlayout->addWidget(replaceButton);
		hlayout->addWidget(closeButton);
		hlayout->addSpacing(1);
		vlayout->addLayout(hlayout,0);
		
		findReplaceDialog->setLayout(vlayout);
		
		connect(findButton, SIGNAL(pressed()), this, SLOT(find()));
		connect(replaceButton, SIGNAL(pressed()), this, SLOT(replace()));
		connect(closeButton, SIGNAL(pressed()), findReplaceDialog, SLOT(reject()));
	}

	int CodeEditor::lineNumberAreaWidth()
	{
		int digits = 1;
		int max = qMax(1, blockCount());
		while (max >= 10) {
			max /= 10;
			++digits;
		}

		int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

		return space;
	}



	void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
	{
		setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
	}



	void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
	{
		if (dy)
			lineNumberArea->scroll(0, dy);
		else
			lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

		if (rect.contains(viewport()->rect()))
			updateLineNumberAreaWidth(0);
	}



	void CodeEditor::resizeEvent(QResizeEvent *e)
	{
		QPlainTextEdit::resizeEvent(e);

		QRect cr = contentsRect();
		lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
	}



	void CodeEditor::highlightCurrentLine()
	{
		QList<QTextEdit::ExtraSelection> extraSelections;

		if (!isReadOnly()) {
			QTextEdit::ExtraSelection selection;

			selection.format.setBackground(lineHighlightColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			extraSelections.append(selection);
		}

		setExtraSelections(extraSelections);
	}



	void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
	{
		QPainter painter(lineNumberArea);
		painter.fillRect(event->rect(), lineNumberBackground);


		QTextBlock block = firstVisibleBlock();
		int blockNumber = block.blockNumber();
		int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
		int bottom = top + (int) blockBoundingRect(block).height();

		while (block.isValid() && top <= event->rect().bottom()) {
			if (block.isVisible() && bottom >= event->rect().top()) {
				QString number = QString::number(blockNumber + 1);
				painter.setPen(lineNumberText);
				painter.setFont(this->font());
				painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
					Qt::AlignRight, number);
			}

			block = block.next();
			top = bottom;
			bottom = top + (int) blockBoundingRect(block).height();
			++blockNumber;
		}
	}

	/*!
	\fn QTextEdit::zoomIn(int range)

	Zooms in on the text by making the base font size \a range
	points larger and recalculating all font sizes to be the new size.
	This does not change the size of any images.

	\sa zoomOut()
	*/
	void CodeEditor::zoomIn(int range)
	{
		QFont f = font();
		const int newSize = f.pointSize() + range;
		if (newSize <= 0)
			return;
		f.setPointSize(newSize);
		setFont(f);
	}

	/*!
	\fn QTextEdit::zoomOut(int range)

	\overload

	Zooms out on the text by making the base font size \a range points
	smaller and recalculating all font sizes to be the new size. This
	does not change the size of any images.

	\sa zoomIn()
	*/
	void CodeEditor::zoomOut(int range)
	{
		zoomIn(-range);
	}

	void CodeEditor::wheelEvent ( QWheelEvent * wheelEvent )
	{
		if (wheelEvent == 0) return;

		if (wheelEvent->modifiers() == Qt::ControlModifier)
		{
			if (wheelEvent->delta() > 0)
				zoomIn();
			else
				zoomOut();
		}
		else
		{
			QPlainTextEdit::wheelEvent(wheelEvent);
		}
	}
	
	QString CodeEditor::text() const
	{
		return toPlainText();
	}

	void CodeEditor::setText(const QString & s)
	{
		setPlainText(s);
	}
	
	bool CodeEditor::find(const QString & s)
	{
		return QPlainTextEdit::find(s);
	}
	
	bool CodeEditor::replace(const QString& old_string, const QString& new_string)
	{
		QString text = toPlainText();
		QString oldText = text;
		if (!text.contains(old_string)) return false;
		text.replace(old_string,new_string);
		setPlainText(text);
		return true;
	}
	
	void CodeEditor::find()
	{
		if (findLineEdit)
			find(findLineEdit->text());
	}

	void CodeEditor::replace()
	{
		if (findLineEdit && replaceLineEdit)
			replace(findLineEdit->text(),replaceLineEdit->text());
	}
	
	void CodeEditor::showFindReplaceDialog()
	{
		if (findReplaceDialog)
			findReplaceDialog->show();
	}
}

