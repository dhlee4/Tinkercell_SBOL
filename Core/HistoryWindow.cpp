<<<<<<< HEAD
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This is the history stack class that is used to store the undo/redo commands.

****************************************************************************/

#include "HistoryWindow.h"

namespace Tinkercell
{

	void HistoryWindow::undo() 
	{ 
		if (stack()) stack()->undo();
	}

	void HistoryWindow::redo() 
	{ 
		if (stack()) stack()->redo();
	}

	void HistoryWindow::push(QUndoCommand * command) 
	{ 
		if (stack()) stack()->push(command);
	}

}

=======
/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This is the history stack class that is used to store the undo/redo commands.

****************************************************************************/

#include "HistoryWindow.h"

namespace Tinkercell
{

	void HistoryWindow::undo() 
	{ 
		if (stack()) stack()->undo();
	}

	void HistoryWindow::redo() 
	{ 
		if (stack()) stack()->redo();
	}

	void HistoryWindow::push(QUndoCommand * command) 
	{ 
		if (stack()) stack()->push(command);
	}

}

>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
