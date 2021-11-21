#pragma once
#ifndef ES_APP_GUIS_GUI_RETROACHIEVEMENTS_OPTIONS_H
#define ES_APP_GUIS_GUI_RETROACHIEVEMENTS_OPTIONS_H

#include "GuiComponent.h"
#include "guis/GuiSettings.h"

class GuiRetroachievementsOptions : public GuiSettings
{
public:
	GuiRetroachievementsOptions(Window* window);
	~GuiRetroachievementsOptions();

private:
	void initializeMenu();
};

#endif // ES_APP_GUIS_GUI_RETROACHIEVEMENTS_OPTIONS_H
