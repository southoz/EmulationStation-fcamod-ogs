#pragma once
#ifndef ES_APP_GUIS_GUI_DISPLAY_AUTO_DIM_OPTIONS_H
#define ES_APP_GUIS_GUI_DISPLAY_AUTO_DIM_OPTIONS_H

#include "GuiComponent.h"
#include "guis/GuiSettings.h"

class GuiDisplayAutoDimOptions : public GuiSettings
{
public:
	GuiDisplayAutoDimOptions(Window* window);
	~GuiDisplayAutoDimOptions();

private:
	void initializeMenu();

void manageDimScreenSaver(Window *window, bool auto_dim_enabled);
};

#endif // ES_APP_GUIS_GUI_DISPLAY_AUTO_DIM_OPTIONS_H
