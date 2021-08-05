#pragma once
#ifndef ES_APP_GUIS_UPDATABLE_GUI_SETTINGS_H
#define ES_APP_GUIS_UPDATABLE_GUI_SETTINGS_H

#include "guis/GuiSettings.h"

class GuiComponent;
class GuiSettings;


class UpdatableGuiSettings : public GuiSettings
{
public:
	UpdatableGuiSettings(Window* window, const std::string title);
	virtual ~UpdatableGuiSettings(); // just calls save();

	virtual void update(int deltaTime);

	virtual void addUpdatableComponent(GuiComponent* component);

	std::vector<GuiComponent *> mUpdatables;
};

#endif // ES_APP_GUIS_UPDATABLE_GUI_SETTINGS_H