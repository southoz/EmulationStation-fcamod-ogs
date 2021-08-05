#include "guis/UpdatableGuiSettings.h"

#include "views/ViewController.h"
#include "SystemConf.h"
#include "Settings.h"
#include "SystemData.h"
#include "Window.h"
#include "Log.h"
#include "EsLocale.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "components/UpdatableTextComponent.h"


UpdatableGuiSettings::UpdatableGuiSettings(Window* window, const std::string title) : GuiSettings(window, title)
{
}

UpdatableGuiSettings::~UpdatableGuiSettings()
{
	mUpdatables.clear();
}

void UpdatableGuiSettings::addUpdatableComponent(GuiComponent *component)
{
	mUpdatables.push_back(component);
}

void UpdatableGuiSettings::update(int deltaTime)
{
	for (GuiComponent *updatable : mUpdatables)
		updatable->update(deltaTime);
}
