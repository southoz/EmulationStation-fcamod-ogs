#pragma once
#ifndef ES_APP_GUIS_GUI_AUTO_SUSPEND_OPTIONS_H
#define ES_APP_GUIS_GUI_AUTO_SUSPEND_OPTIONS_H

#include "components/BusyComponent.h"
#include "GuiComponent.h"
#include "guis/GuiSettings.h"

class GuiAutoSuspendOptions : public GuiSettings
{
public:
	GuiAutoSuspendOptions(Window* window);
	~GuiAutoSuspendOptions();

private:
	void initializeMenu();

	void manageSuspendScreenSaver(Window *window, bool auto_suspend_enabled);
};

#endif // ES_APP_GUIS_GUI_AUTO_SUSPEND_OPTIONS_H
