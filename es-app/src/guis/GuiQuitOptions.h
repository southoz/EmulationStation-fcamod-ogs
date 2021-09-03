#pragma once
#ifndef ES_APP_GUIS_GUI_QUIT_OPTIONS_H
#define ES_APP_GUIS_GUI_QUIT_OPTIONS_H

#include "components/BusyComponent.h"
#include "GuiComponent.h"
#include "guis/GuiSettings.h"

class GuiQuitOptions : public GuiSettings
{
public:
	GuiQuitOptions(Window* window);
	~GuiQuitOptions();

private:
	void initializeMenu(Window* window);

};

#endif // ES_APP_GUIS_GUI_QUIT_OPTIONS_H
