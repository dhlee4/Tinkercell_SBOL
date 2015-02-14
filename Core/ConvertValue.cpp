/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This file defines a set of functions that can be used to convert C data types, 
such as char**, char*, and int to Qt data types, such as QStringList, QString, 
and QGraphicsItem.

****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <QtDebug>
#include "ItemHandle.h"
#include "MainWindow.h"
#include "SymbolsTable.h"
#include "ConvertValue.h"

namespace Tinkercell
{

	tc_matrix emptyMatrix()
	{
		tc_matrix m;
		m.values = 0;
		m.rownames.length = 0;
		m.colnames.length = 0;
		m.rownames.strings = 0;
		m.colnames.strings = 0;
		m.cols = 0;
		m.rows = 0;
		return m;
	}

	ItemHandle* ConvertValue(long o)
	{
		MainWindow * main = MainWindow::instance();
		if (main && main->isValidHandlePointer((void*)o))
			return static_cast<ItemHandle*>((void*)o);
		return 0;
	}

	long ConvertValue(ItemHandle* item)
	{
		void * p = (void*)item;
		return (long)(p);
	}

	QList<ItemHandle*>* ConvertValue(tc_items A)
	{
		MainWindow * main = MainWindow::instance();
		QList<ItemHandle*> * list = new QList<ItemHandle*>();
		if (main)
		{
			for (int i=0; i < A.length; ++i)
				if (main->isValidHandlePointer((void*)(A.items[i])))
					(*list) += static_cast<ItemHandle*>((void*)(A.items[i]));
		}
		return list;
	}

	tc_items ConvertValue(const QList<ItemHandle*>& list)
	{
		tc_items A;
		void * p = 0;
		A.length = list.size();
		A.items = 0;
		if (A.length > 0)
		{
			A.items = new long[A.length];
			for (int i=0; i < list.size(); ++i)
			{
				p = (void*)list[i];
				A.items[i] = (long)(p);
			}
		}
		return A;
	}

	QString ConvertValue(const char* c)
	{
		return QString(c);
	}

	const char* ConvertValue(const QString& s)
	{
		 QByteArray text = s.toLocal8Bit();
		 char * data = new char[text.size() + 1];
		 strcpy(data, text.data());
		 return data;
	}	

	DataTable<qreal>* ConvertValue(tc_matrix m)
	{
		DataTable<qreal>* D = new DataTable<qreal>;
		if (m.rows < 0 || m.cols < 0) return D;
		D->resize(m.rows,m.cols);

		for (int i=0; i < m.rows && m.rownames.strings && m.rownames.strings[i]; ++i)
			D->setRowName(i,ConvertValue(m.rownames.strings[i]));

		for (int i=0; i < m.cols && m.colnames.strings && m.colnames.strings[i]; ++i)
			D->setColumnName(i,ConvertValue(m.colnames.strings[i]));

		for (int i=0; i < m.rows; ++i)
			for (int j=0; j < m.cols; ++j)
			{
				D->value(i,j) = tc_getMatrixValue(m,i,j);
			}

		return D;
	}
	
	tc_matrix ConvertValue(const DataTable<qreal>& D)
	{
		tc_matrix m;

		m.rownames.length = m.rows = D.rows();
		m.colnames.length = m.cols = D.columns();
		
		if (m.rows > 0)
			m.rownames.strings = new char*[m.rows];	
		else
			m.rownames.strings = 0;
			
		if (m.cols > 0)			
			m.colnames.strings = new char*[m.cols];
		else
			m.colnames.strings = 0;
		
		if (m.rows > 0 && m.cols > 0)
			m.values = new double[m.rows * m.cols];
		else
			m.values = 0;

		for (int i=0; i < m.rows; ++i)
		{
			m.rownames.strings[i] = (char*)ConvertValue(D.rowName(i));
		}

		for (int i=0; i < m.cols; ++i)
		{
			m.colnames.strings[i] = (char*)ConvertValue(D.columnName(i));
		}

		for (int i=0; i < m.rows; ++i)
			for (int j=0; j < m.cols; ++j)
			{
				tc_setMatrixValue(m,i,j,D.at(i,j));
			}

		return m;
	}
	
	DataTable<QString>* ConvertValue(tc_table m)
	{
		DataTable<QString>* D = new DataTable<QString>;
		if (m.rows < 0 || m.cols < 0) return D;
		D->resize(m.rows,m.cols);

		for (int i=0; i < m.rows && m.rownames.strings && m.rownames.strings[i]; ++i)
			D->setRowName(i,ConvertValue(m.rownames.strings[i]));

		for (int i=0; i < m.cols && m.colnames.strings && m.colnames.strings[i]; ++i)
			D->setColumnName(i,ConvertValue(m.colnames.strings[i]));

		for (int i=0; i < m.rows; ++i)
			for (int j=0; j < m.cols; ++j)
			{
				D->value(i,j) = ConvertValue(tc_getTableValue(m,i,j));
			}

		return D;
	}
	
	tc_table ConvertValue(const DataTable<QString>& D)
	{
		tc_table m;

		m.rownames.length = m.rows = D.rows();
		m.colnames.length = m.cols = D.columns();		
		
		if (m.rows > 0)
			m.rownames.strings = new char*[m.rows];	
		else
			m.rownames.strings = 0;
			
		if (m.cols > 0)
			m.colnames.strings = new char*[m.cols];
		else
			m.colnames.strings = 0;
		
		if (m.rows > 0 && m.cols > 0)
			m.strings = new char*[m.rows * m.cols];
		else
			m.strings = 0;

		for (int i=0; i < m.rows; ++i)
		{			
			m.rownames.strings[i] = (char*)ConvertValue(D.rowName(i));
		}

		for (int i=0; i < m.cols; ++i)
		{
			m.colnames.strings[i] = (char*)ConvertValue(D.columnName(i));
		}

		for (int i=0; i < m.rows; ++i)
			for (int j=0; j < m.cols; ++j)
			{
				tc_setTableValue(m,i,j,(char*)ConvertValue(D.at(i,j)));
			}

		return m;
	}
	
	QStringList ConvertValue(tc_strings c)
	{
		QStringList slist;
		for (int i=0; i < c.length && c.strings && c.strings[i]; ++i)
			slist += ConvertValue(c.strings[i]);
		return slist;
	}

	tc_strings ConvertValue(const QStringList& list)
	{
		tc_strings A;
		if (list.size() < 1)
		{
			A.length = 0;
			A.strings = 0;
			return A;
		}
		char ** cs = new char*[list.size()];
		for (int i=0; i < list.size(); ++i)
		{
			cs[i] = (char*)ConvertValue(list[i]);
		}
		A.length = list.size();
		A.strings = cs;
		return A;
	}
}

