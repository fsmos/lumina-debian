//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "AppMenu.h"
#include "LSession.h"
#include <LuminaOS.h>

AppMenu::AppMenu(QWidget* parent) : QMenu(parent){
  appstorelink = LOS::AppStoreShortcut(); //Default application "store" to display (AppCafe in PC-BSD)
  controlpanellink = LOS::ControlPanelShortcut(); //Default control panel
  APPS.clear();
  watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherUpdate()) );
  //QTimer::singleShot(200, this, SLOT(start()) ); //Now start filling the menu
  start(); //do the initial run during session init so things are responsive immediately.
  connect(QApplication::instance(), SIGNAL(LocaleChanged()), this, SLOT(watcherUpdate()) );
  connect(QApplication::instance(), SIGNAL(IconThemeChanged()), this, SLOT(watcherUpdate()) );
}

AppMenu::~AppMenu(){

}

QHash<QString, QList<XDGDesktop> >* AppMenu::currentAppHash(){
  return &APPS;
}

//===========
//  PRIVATE
//===========
void AppMenu::updateAppList(){
  //Make sure the title/icon are updated as well (in case of locale/icon change)
  this->setTitle(tr("Applications"));
  this->setIcon( LXDG::findIcon("system-run","") );
  //Now update the lists
  this->clear();
  APPS.clear();
  QList<XDGDesktop> allfiles = LXDG::systemDesktopFiles();
  APPS = LXDG::sortDesktopCats(allfiles);
  APPS.insert("All", LXDG::sortDesktopNames(allfiles));
  lastHashUpdate = QDateTime::currentDateTime();
  //Now fill the menu
  bool ok; //for checking inputs
    //Add link to the file manager
    this->addAction( LXDG::findIcon("user-home", ""), tr("Open Home"), this, SLOT(launchFileManager()) );
    //--Look for the app store
    XDGDesktop store = LXDG::loadDesktopFile(appstorelink, ok);
    if(ok){
      this->addAction( LXDG::findIcon(store.icon, ""), tr("Install Applications"), this, SLOT(launchStore()) );
    }
    //--Look for the control panel
    store = LXDG::loadDesktopFile(controlpanellink, ok);
    if(ok){
      this->addAction( LXDG::findIcon(store.icon, ""), tr("Control Panel"), this, SLOT(launchControlPanel()) );
    }
    this->addSeparator();
    //--Now create the sub-menus
    QStringList cats = APPS.keys();
    cats.sort(); //make sure they are alphabetical
    for(int i=0; i<cats.length(); i++){
      //Make sure they are translated and have the right icons
      QString name, icon;
      if(cats[i]=="All"){continue; } //skip this listing for the menu
      else if(cats[i] == "Multimedia"){ name = tr("Multimedia"); icon = "applications-multimedia"; }
      else if(cats[i] == "Development"){ name = tr("Development"); icon = "applications-development"; }
      else if(cats[i] == "Education"){ name = tr("Education"); icon = "applications-education"; }
      else if(cats[i] == "Game"){ name = tr("Games"); icon = "applications-games"; }
      else if(cats[i] == "Graphics"){ name = tr("Graphics"); icon = "applications-graphics"; }
      else if(cats[i] == "Network"){ name = tr("Network"); icon = "applications-internet"; }
      else if(cats[i] == "Office"){ name = tr("Office"); icon = "applications-office"; }
      else if(cats[i] == "Science"){ name = tr("Science"); icon = "applications-science"; }
      else if(cats[i] == "Settings"){ name = tr("Settings"); icon = "preferences-system"; }
      else if(cats[i] == "System"){ name = tr("System"); icon = "applications-system"; }
      else if(cats[i] == "Utility"){ name = tr("Utility"); icon = "applications-utilities"; }
      else if(cats[i] == "Wine"){ name = tr("Wine"); icon = "wine"; }
      else{ name = tr("Unsorted"); icon = "applications-other"; }

      QMenu *menu = new QMenu(name, this);
      menu->setIcon(LXDG::findIcon(icon,""));
      connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(launchApp(QAction*)) );
      QList<XDGDesktop> appL = APPS.value(cats[i]);
      for( int a=0; a<appL.length(); a++){
	if(appL[a].actions.isEmpty()){
	  //Just a single entry point - no extra actions
          QAction *act = new QAction(LXDG::findIcon(appL[a].icon, ""), appL[a].name, this);
          act->setToolTip(appL[a].comment);
          act->setWhatsThis(appL[a].filePath);
          menu->addAction(act);
	}else{
	  //This app has additional actions - make this a sub menu
	  // - first the main menu/action
	  QMenu *submenu = new QMenu(appL[a].name, this);
	    submenu->setIcon( LXDG::findIcon(appL[a].icon,"") );
	      //This is the normal behavior - not a special sub-action (although it needs to be at the top of the new menu)
	      QAction *act = new QAction(LXDG::findIcon(appL[a].icon, ""), appL[a].name, this);
              act->setToolTip(appL[a].comment);
              act->setWhatsThis(appL[a].filePath);
	    submenu->addAction(act);
	    //Now add entries for every sub-action listed
	    for(int sa=0; sa<appL[a].actions.length(); sa++){
              QAction *sact = new QAction(LXDG::findIcon(appL[a].actions[sa].icon, appL[a].icon), appL[a].actions[sa].name, this);
              sact->setToolTip(appL[a].comment);
              sact->setWhatsThis("-action \""+appL[a].actions[sa].ID+"\" \""+appL[a].filePath+"\"");
              submenu->addAction(sact);		    
	    }
	  menu->addMenu(submenu);
	}
      }
      this->addMenu(menu);
    }
    emit AppMenuUpdated();
}

//=================
//  PRIVATE SLOTS
//=================
void AppMenu::start(){
  //Setup the watcher
  watcher->addPaths(LXDG::systemApplicationDirs());
  //Now fill the menu the first time
  updateAppList();
}

void AppMenu::watcherUpdate(){
  updateAppList(); //Update the menu listings
}

void AppMenu::launchStore(){
  LSession::LaunchApplication("lumina-open \""+appstorelink+"\"");
}

void AppMenu::launchControlPanel(){
  LSession::LaunchApplication("lumina-open \""+controlpanellink+"\"");
}

void AppMenu::launchFileManager(){
  QString fm = "lumina-open \""+QDir::homePath()+"\"";
  LSession::LaunchApplication(fm);
}

void AppMenu::launchApp(QAction *act){
  QString appFile = act->whatsThis();
  if(appFile.startsWith("-action")){
    LSession::LaunchApplication("lumina-open "+appFile); //already has quotes put in place properly
  }else{
    LSession::LaunchApplication("lumina-open \""+appFile+"\"");
  }
}
