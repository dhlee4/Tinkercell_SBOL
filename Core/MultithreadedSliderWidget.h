/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

A slider widget that calls a C function whenver values in the slider are changed.
Uses CThread.

****************************************************************************/

#ifndef TINKERCELL_MULTITHREADEDSLIDER_H
#define TINKERCELL_MULTITHREADEDSLIDER_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QHash>
#include "CThread.h"

namespace Tinkercell
{
	/*! \brief This class is used to run specific functions inside a C dynamic library
	as a separate thread. Uses CThread to call the C functions
	\ingroup plugins
	*/
	class TINKERCELLCOREEXPORT MultithreadedSliderWidget : public QWidget
	{
		Q_OBJECT

	signals:
		
		/*!
		* \brief the values in the slider have changed
		*/
		void valuesChanged(DataTable<qreal>);

		/*!
		* \brief this signal is used to call a script function that responds to the slider widget
		*/
		void evalScript(const QString& string);

	public slots:
	
		/*!
		* \brief setup the sliders options and initial values
		* \param QStringList names for the sliders
		* \param QList<double> minimum value for each of the sliders
		* \param QList<double> maximum value for each of the sliders
		*/
		virtual void setSliders(const QStringList& options, const QList<double>& minValues, const QList<double>& maxValues);
		
		/*!
		* \brief set the sliders visible
		* \param QStringList names for the sliders
		*/
		virtual void setVisibleSliders(const QStringList& options);
		
		/*!
		* \brief set the sliders visible if the slider name has the given string as a substring
		* \param QString substring for the slider names
		*/
		virtual void setVisibleSliders(const QString& substring);
		
	public:

		/*!
		* \brief constructor
		* \param QWidget * parent
		* \param CThread * the thread that is already setup with the correct library and function
		* \param Qt::Orientation orientation
		*/
		MultithreadedSliderWidget(MainWindow * parent, CThread * thread=0, Qt::Orientation orientation = Qt::Horizontal);

		/*!
		* \brief constructor
		* \param QWidget * parent
		* \param QString the name of the dynamic library to load
		* \param QString name of function in the library with signature void f(Matrix)
		* \param Qt::Orientation orientation
		*/
		MultithreadedSliderWidget(MainWindow * parent, const QString & lib, const QString & functionName, Qt::Orientation orientation = Qt::Horizontal);
		
		/*!
		* \brief the cthread that is run every time the sliders change
		*/
		virtual CThread * thread() const;
		
		/*!
		* \brief the cthread that is run every time the sliders change
		*/
		virtual void setThread(CThread *);
		
		/*!
		* \brief the console command that is executed every time the sliders change
		*/
		virtual QString command() const;
		
		/*!
		* \brief the console command that is executed every time the sliders change
		*/
		virtual void setCommand(const QString&);

		/*!
		* \brief This is the data table that will be altered when no appropriate data is available. 
						For example, if one of the sliders is labeled "A" and the default table is set to "bla", then
						changing the slider for "A" will result in change to "A.bla[0,0]"
		*/
		virtual void setDefaultDataTable(const QString&);
		
		/*!
		* \brief table containing the variables, current values, min and max
		*/
		virtual DataTable<qreal> data() const;

	protected slots:

		/*!
		* \brief whenver the value text change, the function in the C library is called
		*/
		virtual void valueChanged();
		
		/*!
		* \brief whenver the sliders change, the function in the C library is called
		*/
		virtual void sliderChanged(int,int);
		
		/*!
		* \brief whenver the text change, the function in the C library is called
		*/
		virtual void minmaxChanged();
		
		/*!
		* \brief copy the values from the slider to the model
		*/
		virtual void saveValues();

	protected:
	
		/*!
		* \brief whenver the slides change, cthread->start() is called
		*/
		CThread * cthread;	
		/*!
		* \brief whenver the slides change, this command is called using console's evalScript
		*/
		QString scriptCommand;
		/*!
		* \brief orientation of the sliders
		*/
		Qt::Orientation orientation;		
		/*!
		* \brief table storing slider values
		*/
		DataTable<qreal> values;
		/*!
		* \brief slider labels in same order as sliders list
		*/
		QList< QLabel* > labels;
		/*!
		* \brief all the sliders
		*/
		QList< QSlider* > sliders;
		/*!
		* \brief slider min, max, and values in same order as sliders list
		*/
		QList< QLineEdit* > minline, maxline, valueline;
		/*!
		* \brief slider min and max in same order as sliders list
		*/
		QList<double> min, max;
		/*!
		* \brief slider layout
		*/
		QVBoxLayout * slidersLayout;
		/*!
		* \brief sliders by name
		*/
		QHash< QString, QWidget* > sliderWidgets;
		/*!
		* \brief main window
		*/
		MainWindow * mainWindow;
		/*!
		* \brief This is the data table that will be altered when no appropriate data is available. 
						For example, if one of the sliders is labeled "A" and the default table is set to "bla", then
						changing the slider for "A" will result in change to "A.bla[0,0]"
		*/
		QString defaultDataTable;
	};

	class SliderWithIndex : public QSlider
	{
		Q_OBJECT

		public:
			SliderWithIndex(int);

		signals:
			void valueChanged(int,int);

		private slots:
			void valueChangedSlot(int);

		private:
			int index;
	};
}

#endif
