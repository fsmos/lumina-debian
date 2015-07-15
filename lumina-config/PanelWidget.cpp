//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PanelWidget.h"
#include "ui_PanelWidget.h"

#include "LPlugins.h"
#include "GetPluginDialog.h"
#include "mainUI.h"
#include <LuminaXDG.h>

#include <QSettings>
#include <QStringList>


PanelWidget::PanelWidget(QWidget *parent, QWidget *Main, LPlugins *Pinfo) : QWidget(parent), ui(new Ui::PanelWidget){
  ui->setupUi(this);
  mainui = Main;
  PINFO = Pinfo;
  //Fill up the menu's with the valid entries
  ui->combo_align->addItem(tr("Top/Left"), "left");
  ui->combo_align->addItem(tr("Center"), "center");
  ui->combo_align->addItem(tr("Bottom/Right"), "right");
  ui->combo_edge->addItem(tr("Top"), "top");
  ui->combo_edge->addItem(tr("Bottom"), "bottom");
  ui->combo_edge->addItem(tr("Left"), "left");
  ui->combo_edge->addItem(tr("Right"), "right");
	
  LoadIcons();
  //Now connect any other signals/slots
  connect(ui->combo_edge, SIGNAL(currentIndexChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->combo_align, SIGNAL(currentIndexChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->spin_plength, SIGNAL(valueChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->spin_pxthick, SIGNAL(valueChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->check_autohide, SIGNAL(stateChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->group_customcolor, SIGNAL(toggled(bool)), this, SLOT(ItemChanged()) );
	
}

PanelWidget::~PanelWidget(){
	
}

void PanelWidget::LoadSettings(QSettings *settings, int Dnum, int Pnum){
  pnum = Pnum; dnum = Dnum; //save these for later
  ui->label->setText( QString(tr("Panel %1")).arg(QString::number(Pnum+1) ) );
  QString prefix = "panel"+QString::number(Dnum)+"."+QString::number(Pnum)+"/";
  qDebug() << "Loading Panel Settings:" << prefix;
  //Now load the settings into the GUI
  int tmp = ui->combo_align->findData( settings->value(prefix+"pinLocation","center").toString().toLower() );
  if(tmp>=0){ ui->combo_align->setCurrentIndex( tmp ); }
  tmp = ui->combo_edge->findData( settings->value(prefix+"location","top").toString().toLower() );
  if(tmp>=0){ ui->combo_edge->setCurrentIndex( tmp ); }
  ui->spin_plength->setValue( settings->value( prefix+"lengthPercent",100).toInt() );
  ui->spin_pxthick->setValue( settings->value( prefix+"height",30).toInt() );
  ui->check_autohide->setChecked( settings->value(prefix+"hidepanel", false).toBool() );
  ui->group_customcolor->setChecked( settings->value(prefix+"customcolor",false).toBool() );
  ui->label_color_sample->setWhatsThis( settings->value(prefix+"color","rgba(255,255,255,160)").toString());
  ui->list_plugins->clear();
  QStringList plugs = settings->value(prefix+"pluginlist",QStringList()).toStringList();
  for(int i=0; i<plugs.length(); i++){
    QString pid = plugs[i].section("---",0,0);
      if(pid.startsWith("applauncher")){
	bool ok = false;
	XDGDesktop desk = LXDG::loadDesktopFile(pid.section("::",1,1),ok);
	if(ok){
	  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	  ui->list_plugins->addItem(it);
	}
      }else{
        LPI info = PINFO->panelPluginInfo(pid);
        if(!info.ID.isEmpty()){
          QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	  ui->list_plugins->addItem(it);
        }
      }
  }
  reloadColorSample();
}

void PanelWidget::SaveSettings(QSettings *settings){//save the current settings
  QString prefix = "panel"+QString::number(dnum)+"."+QString::number(pnum)+"/";
  qDebug() << "Saving panel settings:" << prefix;
  settings->setValue(prefix+"location", ui->combo_edge->currentData().toString() );
  settings->setValue(prefix+"pinLocation", ui->combo_align->currentData().toString() );
  settings->setValue(prefix+"lengthPercent", ui->spin_plength->value() );
  settings->setValue(prefix+"height", ui->spin_pxthick->value() );
  settings->setValue(prefix+"hidepanel", ui->check_autohide->isChecked() );
  settings->setValue(prefix+"customColor", ui->group_customcolor->isChecked() );
  settings->setValue(prefix+"color", ui->label_color_sample->whatsThis() );
  QStringList plugs;
  for(int i=0; i<ui->list_plugins->count(); i++){
     plugs << ui->list_plugins->item(i)->whatsThis();
  }
  settings->setValue(prefix+"pluginlist", plugs );
	
}

int PanelWidget::PanelNumber(){
  return pnum;
}

void PanelWidget::ChangePanelNumber(int newnum){
  ui->label->setText( QString(tr("Panel %1")).arg(QString::number(newnum+1) ) );
  pnum = newnum; //So we can retain the current settings, but will save them with a different number
}

void PanelWidget::LoadIcons(){
  ui->tool_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_remplugin->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_upplugin->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_downplugin->setIcon( LXDG::findIcon("go-down","") );
  ui->tool_selectcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->toolBox->setItemIcon(0,LXDG::findIcon("transform-move",""));
  ui->toolBox->setItemIcon(1,LXDG::findIcon("preferences-desktop-display",""));
  ui->toolBox->setItemIcon(2,LXDG::findIcon("preferences-plugin",""));
}

void PanelWidget::reloadColorSample(){
  ui->label_color_sample->setStyleSheet("background: "+ui->label_color_sample->whatsThis());
}

void PanelWidget::on_tool_rm_clicked(){
  emit PanelRemoved(pnum);
}

void PanelWidget::ItemChanged(){
  emit PanelChanged();
}

void PanelWidget::UseColorChanged(){
	
  emit PanelChanged();
}

void PanelWidget::on_tool_selectcolor_clicked(){
  QString color = static_cast<MainUI*>(mainui)->getColorStyle(ui->label_color_sample->whatsThis());
  if( color.isEmpty()){ return; }
  ui->label_color_sample->setWhatsThis(color);
  reloadColorSample();
  emit PanelChanged();
}

void PanelWidget::on_tool_addplugin_clicked(){
  GetPluginDialog dlg(mainui);
	dlg.LoadPlugins("panel", PINFO);
	dlg.exec();
  if(!dlg.selected){ return; } //cancelled
  QString pan = dlg.plugID; //getNewPanelPlugin();
  if(pan == "applauncher"){
    //Prompt for the application to add
    XDGDesktop app = static_cast<MainUI*>(mainui)->getSysApp();
    if(app.filePath.isEmpty()){ return; } //cancelled
    pan.append("::"+app.filePath);
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(app.icon,""), app.name);
      it->setWhatsThis(pan);
    ui->list_plugins->addItem(it);
    ui->list_plugins->setCurrentItem(it);
    ui->list_plugins->scrollToItem(it);
  }else{
    if(pan.isEmpty()){ return; } //nothing selected
    //Add the new plugin to the list
    LPI info = PINFO->panelPluginInfo(pan);
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name);
      it->setWhatsThis(info.ID);
    ui->list_plugins->addItem(it);
    ui->list_plugins->setCurrentItem(it);
    ui->list_plugins->scrollToItem(it);
  }
  emit PanelChanged();
}

void PanelWidget::on_tool_remplugin_clicked(){
  if(ui->list_plugins->currentRow() < 0){ return; }
  delete ui->list_plugins->takeItem( ui->list_plugins->currentRow() );
  emit PanelChanged();
}

void PanelWidget::on_tool_upplugin_clicked(){
  int row = ui->list_plugins->currentRow();
  if( row <= 0){ return; }
  ui->list_plugins->insertItem(row-1, ui->list_plugins->takeItem(row));
  ui->list_plugins->setCurrentRow(row-1);
  emit PanelChanged();
}

void PanelWidget::on_tool_downplugin_clicked(){
  int row = ui->list_plugins->currentRow();
  if( row < 0 || row >= (ui->list_plugins->count()-1) ){ return; }
  ui->list_plugins->insertItem(row+1, ui->list_plugins->takeItem(row));
  ui->list_plugins->setCurrentRow(row+1);	
  emit PanelChanged();
}
