#pragma once
#ifndef ES_APP_GUIS_GUI_POWERKEY_OPTIONS_H
#define ES_APP_GUIS_GUI_POWERKEY_OPTIONS_H

#include "GuiComponent.h"
#include "guis/GuiSettings.h"

class GuiPowerkeyOptions : public GuiSettings
{
public:
	GuiPowerkeyOptions(Window* window);
	~GuiPowerkeyOptions();

private:
	void initializeMenu();
};

#endif // ES_APP_GUIS_GUI_POWERKEY_OPTIONS_H
