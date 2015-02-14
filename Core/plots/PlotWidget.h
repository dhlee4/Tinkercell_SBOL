/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 This tool displays a plot based on the DataTable contained.
 
****************************************************************************/

#ifndef TINKERCELL_PLOTWIDGET_H
#define TINKERCELL_PLOTWIDGET_H

#include <QtGui>
#include <QList>
#include <QStringList>
#include <QString>
#include <QTableWidget>
#include <QPainterPath>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSplitter>
#include <QSemaphore>
#include <QMainWindow>
#include <QButtonGroup>
#include <QTextCharFormat>
#include "DataTable.h"
#include "PlotTool.h"

namespace Tinkercell
{
	/*!
	\brief A widget containing a data plot, legend and options. This class does not perform any plotting. 
			This class serves as a template for other widgets that perform the plotting. 
	\ingroup plotting
	*/
	class TINKERCELLCOREEXPORT PlotWidget : public QWidget
	{
		Q_OBJECT
	
	protected:
		/*! \brief tool bar containing all the options for this widget*/
		QToolBar toolBar;
	
	public:
		/*! \brief used for identifying the plot type*/
		PlotTool::PlotType type;
		/*! \brief constructor with plot tool as parent*/
		PlotWidget(PlotTool * parent = 0);
		/*! \brief constructor with plot tool as parent*/
		PlotWidget(const DataTable<qreal>&, PlotTool * parent = 0);			
		/*! \brief destructor*/
		virtual ~PlotWidget();
		/*! \brief get the data inside this plot*/
		virtual DataTable<qreal>* data();
		/*! \brief indicates whether or not this plot widget is capable of plotting one graph on top of another*/
		virtual bool canAppendData() const;
		/*! \brief append more data to the currently existing plot*/
		virtual void appendData(const DataTable<qreal>&, const QString& title, int x = 0);
		/*! \brief update data for the current plot*/
		virtual void updateData(const DataTable<qreal>&, const QString& title, int x = 0);
		/*! \brief get the data inside this plot as teb-delimited text*/
		virtual QString dataToString(const QString& delim=QString("\t"));

	public slots:
		/*! \brief export data is some format
			\param QString format*/
		virtual void exportData(const QString&, const QString & file);
		/*! \brief set log scale (if applicable)*/
		virtual void setLogScale(int axis, bool set=true);
		/*! \brief set plot title*/
		virtual void setTitle(const QString & title);
		
	protected:
		/*! \brief the plot tool that contains this widget*/
		PlotTool * plotTool;
		/*! \brief title string*/
		QString title;
		/*! \brief category string*/
		QString category;
		/*! \brief key events*/
		virtual void keyPressEvent ( QKeyEvent * event );
		
		friend class PlotTool;
	};

}
#endif
