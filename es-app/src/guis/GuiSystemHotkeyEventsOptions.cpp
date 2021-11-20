#include "guis/GuiSystemHotkeyEventsOptions.h"

#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "ApiSystem.h"


GuiSystemHotkeyEventsOptions::GuiSystemHotkeyEventsOptions(Window* window) : GuiSettings(window, _("SYSTEM HOTKEY EVENTS SETTINGS").c_str()), mPopupDisplayed(false)
{
	initializeMenu(window);
}

GuiSystemHotkeyEventsOptions::~GuiSystemHotkeyEventsOptions()
{

}

void GuiSystemHotkeyEventsOptions::initializeMenu(Window* window)
{
	// brightness events
	auto brightness = std::make_shared<SwitchComponent>(window);
	brightness->setState(ApiSystem::getInstance()->isSystemHotkeyBrightnessEvent());
	addWithLabel(_("BRIGHTNESS"), brightness);

	// volume events
	auto volume = std::make_shared<SwitchComponent>(window);
	volume->setState(ApiSystem::getInstance()->isSystemHotkeyVolumeEvent());
	addWithLabel(_("VOLUME"), volume);

	// wifi events
	auto wifi = std::make_shared<SwitchComponent>(window);
	wifi->setState(ApiSystem::getInstance()->isSystemHotkeyWifiEvent());
	addWithLabel(_("WIFI"), wifi);

	// performance events
	auto performance = std::make_shared<SwitchComponent>(window);
	performance->setState(ApiSystem::getInstance()->isSystemHotkeyPerformanceEvent());
	addWithLabel(_("PERFORMANCE"), performance);

	// suspend events
	auto suspend = std::make_shared<SwitchComponent>(window);
	suspend->setState(ApiSystem::getInstance()->isSystemHotkeySuspendEvent());
	addWithLabel(_("SUSPEND"), suspend);

	addSaveFunc([brightness, volume, wifi, performance, suspend]
		{
			ApiSystem::getInstance()->setSystemHotkeysValues( brightness->getState(), volume->getState(), wifi->getState(), performance->getState(), suspend->getState() );
		});
}
