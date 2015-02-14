/****************************************************************************
 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 see COPYRIGHT.TXT

 This tool displays a plot based on the DataTable contained.
 
****************************************************************************/

#include <math.h>
#include <QGroupBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QDesktopServices>
#include "ExternalCode/qwt/qwt_scale_engine.h"
#include "GraphicsScene.h"
#include "MainWindow.h"
#include "ConsoleWindow.h"
#include "PlotTool.h"
#include "PlotTextWidget.h"
#include "GlobalSettings.h"

namespace Tinkercell
{	
	PlotWidget::PlotWidget(PlotTool * parent) : QWidget(parent), plotTool(parent)
	{
	}
	
	DataTable<qreal>* PlotWidget::data()
	{
		return 0;
	}
	
	void PlotWidget::updateData(const DataTable<qreal>&, const QString& , int)
	{
	}
	
	void PlotWidget::setLogScale(int , bool )
	{
	}
	
	void PlotWidget::setTitle(const QString& s)
	{
		title = s;
		setWindowTitle(s);
	}
	
	void PlotWidget::exportData(const QString & type, const QString&)
	{
		QString output = dataToString("\t");
		
		if (type.toLower() == tr("clipboard"))
		{
			QClipboard * clipboard = QApplication::clipboard();
		
			if (!clipboard)
			{
				if (plotTool && plotTool->console())
					plotTool->console()->error(tr("No clipboard available."));
				return;
			}

			clipboard->setText(output);
			
			if (plotTool && plotTool->console())
				plotTool->console()->message(tr("Tab-delimited data copied to clipboard."));

		}
		else
		if (plotTool && type.toLower() == tr("text"))
		{
			DataTable<qreal> table;
			if (data()) 
			{
				table = *data();
				QString filename = GlobalSettings::tempDir() + tr("/plot.csv");
				QFile file(filename);
				if (file.open(QFile::WriteOnly | QFile::Text))
				{
					QTextStream out(&file);
					out << dataToString(",");
					file.close();
					QDesktopServices::openUrl(QUrl(filename));
				}
			}
			
		}
		else
		{
			QMessageBox::information(this,tr("Feature not available"),tr("This export option is not available for the current plot"));
		}
	}
	
	QString PlotWidget::dataToString(const QString& delim)
	{
		QString output;
		
		DataTable<qreal> * dat = data();
		
		if (dat)
		{
			DataTable<qreal>& table = *dat;
			
			QStringList colnames = table.columnNames(), 
						rownames = table.rowNames();
		
			bool printRows = true;
			for (int i=0; i < rownames.size(); ++i)
				if (rownames.at(i).isEmpty() || rownames.at(i).isNull())
				{
					printRows = false;
					break;
				}
		
			output += tr("#");
			for (int i=0; i < colnames.size(); ++i)
			{
				if (i == 0 && !printRows)
					output += colnames.at(i);
				else
					output += delim + colnames.at(i);
			}
			
			output += tr("\n");
		
			for (int i=0; i < table.rows(); ++i)
			{
				if (printRows)
				{
					output += rownames.at(i) + delim;
				}
			
				for (int j=0; j < table.columns(); ++j)
				{
					if (j == 0)
						output += QString::number(table.at(i,j));
					else
						output += delim + QString::number(table.at(i,j));
				}
			
				output += tr("\n");
			}
		}
		
		return output;
	}
	
	bool PlotWidget::canAppendData() const
	{
		return false;
	}
	
	void PlotWidget::appendData(const DataTable<qreal>&, const QString& , int)
	{
	}
	
	void PlotWidget::keyPressEvent ( QKeyEvent * )
	{
	}

	PlotWidget::~PlotWidget()
	{
	}
}

