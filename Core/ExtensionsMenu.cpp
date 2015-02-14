#include <QMessageBox>
#include "ExtensionsMenu.h"
#include "GlobalSettings.h"

namespace Tinkercell
{
	ExtensionsMenu::ExtensionsMenu(MainWindow * main)
        : QMenu(tr("Extensions"), main)
    {
      QCoreApplication::setOrganizationName(Tinkercell::GlobalSettings::ORGANIZATIONNAME);
      QCoreApplication::setOrganizationDomain(Tinkercell::GlobalSettings::PROJECTWEBSITE);
      QCoreApplication::setApplicationName(Tinkercell::GlobalSettings::ORGANIZATIONNAME);

      QSettings * settings = MainWindow::getSettings();

      settings->beginGroup("MainWindow");

      QStringList doNotLoad = settings->value("doNotLoadPlugins", QStringList()).toStringList();

      settings->endGroup();
      
      delete settings;

      for (int i = 0; i < doNotLoad.size(); ++i)
        doNotLoadPluginNames += doNotLoad.at(i).toLower();

      connect(main,
              SIGNAL(toolAboutToBeLoaded( Tool * , bool * )),
              this,
              SLOT(toolAboutToBeLoaded( Tool * , bool * )));
			  
	  
	  toggleToolsLayoutAction = toggleHistoryLayoutAction = toggleConsoleLayoutAction = 0;
	  
	  if (main && main->settingsMenu)
	  {
			QMenu * menu = new QMenu(tr("Change layout"),main->settingsMenu);
			main->settingsMenu->addMenu(menu);
			
			if (MainWindow::defaultToolWindowOption == MainWindow::TabWidget)			
				toggleToolsLayoutAction = menu->addAction(tr("Use dock widgets for tools"));			
			else			
				toggleToolsLayoutAction = menu->addAction(tr("Use tool-box widgets for tools"));				
			
			if (MainWindow::defaultHistoryWindowOption == MainWindow::TabWidget)			
				toggleHistoryLayoutAction = menu->addAction(tr("Use dock widgets for history"));			
			else			
				toggleHistoryLayoutAction = menu->addAction(tr("Use tool-box widgets for history"));
				
			if (MainWindow::defaultConsoleWindowOption == MainWindow::TabWidget)			
				toggleConsoleLayoutAction = menu->addAction(tr("Use dock widgets for console"));			
			else			
				toggleConsoleLayoutAction = menu->addAction(tr("Use tool-box widgets for console"));

			connect(toggleToolsLayoutAction,SIGNAL(triggered()),this,SLOT(toggleToolsLayout()));
			connect(toggleHistoryLayoutAction,SIGNAL(triggered()),this,SLOT(toggleHistoryLayout()));
			connect(toggleConsoleLayoutAction,SIGNAL(triggered()),this,SLOT(toggleConsoleLayout()));
	  }
    }
	
	void ExtensionsMenu::toggleHistoryLayout()
	{
		if (MainWindow::defaultHistoryWindowOption == MainWindow::DockWidget)
		{
			toggleToolsLayoutAction->setText(tr("Use dock widgets for history"));
			MainWindow::defaultHistoryWindowOption =  MainWindow::TabWidget;
		}
		else
		{
			toggleToolsLayoutAction->setText(tr("Use tool-box widgets for history"));
			MainWindow::defaultHistoryWindowOption =  MainWindow::DockWidget;
		}
		QMessageBox::information(this,tr("History Window Layout"),tr("The change in layout will take effect the next time TinkerCell starts"));
	}
	
	void ExtensionsMenu::toggleConsoleLayout()
	{
		if (MainWindow::defaultConsoleWindowOption == MainWindow::DockWidget)
		{
			toggleConsoleLayoutAction->setText(tr("Use dock widgets for console"));
			MainWindow::defaultConsoleWindowOption =  MainWindow::TabWidget;
		}
		else
		{
			toggleConsoleLayoutAction->setText(tr("Use tool-box widgets for console"));
			MainWindow::defaultConsoleWindowOption =  MainWindow::DockWidget;
		}
		QMessageBox::information(this,tr("Console Window Layout"),tr("The change in layout will take effect the next time TinkerCell starts"));
	}
	
	void ExtensionsMenu::toggleToolsLayout()
	{
		if (MainWindow::defaultToolWindowOption == MainWindow::DockWidget)
		{
			toggleToolsLayoutAction->setText(tr("Use dock widgets for tools"));
			MainWindow::defaultToolWindowOption =  MainWindow::TabWidget;
		}
		else
		{
			toggleToolsLayoutAction->setText(tr("Use tool-box widgets for tools"));
			MainWindow::defaultToolWindowOption =  MainWindow::DockWidget;
		}
		QMessageBox::information(this,tr("Tool Window Layout"),tr("The change in layout will take effect the next time TinkerCell starts"));
	}

    ExtensionsMenu::~ExtensionsMenu()
    {
      saveSettings();
    }

    void ExtensionsMenu::saveSettings()
    {
      QCoreApplication::setOrganizationName(Tinkercell::GlobalSettings::ORGANIZATIONNAME);
      QCoreApplication::setOrganizationDomain(Tinkercell::GlobalSettings::PROJECTWEBSITE);
      QCoreApplication::setApplicationName(Tinkercell::GlobalSettings::ORGANIZATIONNAME);

      QSettings * settings = MainWindow::getSettings();

      QStringList doNotLoad;

      for (int i = 0; i < actions.size(); ++i)
        if (actions[i] && !actions[i]->isChecked())
          doNotLoad << actions[i]->text();

      settings->beginGroup("MainWindow");

      settings->setValue("doNotLoadPlugins", QVariant(doNotLoad));

      settings->endGroup();
      
      delete settings;
    }

    void ExtensionsMenu::toolAboutToBeLoaded( Tool * tool, bool * shouldLoad )
    {
      if (!tool)
        return;

      QString name = tool->name;

	  QMenu * menu = 0;
	  QString s = tool->category;
	  if (s.isEmpty()) s = tr("misc.");
	  
	  for (int i=0; i < menus.size(); ++i)
		if (menus[i] && menus[i]->title() == s)
		{
			menu = menus[i];
			break;
		}
	  
	  if (!menu)
	  {
		menu = this->addMenu(s);
		menus << menu;
	  }
	  
      QAction * action = menu->addAction(name);
	  action->setCheckable(true);

      actions << action;

      if (doNotLoadPluginNames.contains(name.toLower()) && shouldLoad)
      {
        (*shouldLoad) = false;
        action->setChecked(false);
      }
      else
      {
        action->setChecked(true);
      }
	  connect(action,SIGNAL(toggled(bool)),this,SLOT(toggleTool(bool)));
    }
	
	void ExtensionsMenu::toggleTool(bool)
	{
		QMessageBox::information(this->parentWidget(),tr("Plug-in menu"),tr("Changes in the plug-ins will take effect the next time TinkerCell starts"));
	}

}

