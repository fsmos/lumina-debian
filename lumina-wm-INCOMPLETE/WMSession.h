//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_MANAGER_SESSION_H
#define _LUMINA_DESKTOP_WINDOW_MANAGER_SESSION_H

#include <QObject>
#include <QStringList>

#include "LScreenSaver.h"
#include "LXcbEventFilter.h"

class WMSession : public QObject{
	Q_OBJECT
public:
	WMSession();
	~WMSession();

	void start();

private:
	//XCB Event Watcher
	EventFilter *EFILTER;
	//ScreenSaver
	LScreenSaver *SS;

	//Window Manager


public slots:
	void reloadIcons();
	void newInputsAvailable(QStringList);

private slots:
	
};

#endif