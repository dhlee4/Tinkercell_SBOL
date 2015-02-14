<<<<<<< HEAD
/**************************************
This is a very simple plugin
It listens to main window's signals
and displays dialog boxes showing user
inputs
**************************************/


#ifndef TINKERCELL_SAMPLE_PLUGIN1_H
#define TINKERCELL_SAMPLE_PLUGIN1_H

#include "Tool.h"

namespace Tinkercell
{

	class MyFirstExtension : public Tool
	{

		Q_OBJECT

	public:
		MyFirstExtension();
		bool setMainWindow(MainWindow * main);
	
	public slots:
		void mouseReleased(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
	};

}

#endif

=======
/**************************************
This is a very simple plugin
It listens to main window's signals
and displays dialog boxes showing user
inputs
**************************************/


#ifndef TINKERCELL_SAMPLE_PLUGIN1_H
#define TINKERCELL_SAMPLE_PLUGIN1_H

#include "Tool.h"

namespace Tinkercell
{

	class MyFirstExtension : public Tool
	{

		Q_OBJECT

	public:
		MyFirstExtension();
		bool setMainWindow(MainWindow * main);
	
	public slots:
		void mouseReleased(GraphicsScene * scene, QPointF point, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
		void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);
	};

}

#endif

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
