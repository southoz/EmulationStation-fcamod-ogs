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
	bool brightness_value = ApiSystem::getInstance()->isSystemHotkeyBrightnessEvent();
	brightness->setState(brightness_value);
	addWithLabel(_("BRIGHTNESS"), brightness);
	addSaveFunc([this, brightness_value, brightness]
		{
			bool new_brightness_value = brightness->getState();
			if (brightness_value != new_brightness_value)
			{
				if (!mPopupDisplayed)
				{
					mPopupDisplayed = true;
					mWindow->pushGui(new GuiMsgBox(mWindow,
						_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
						_("OK"), [new_brightness_value] { ApiSystem::getInstance()->setSystemHotkeyBrightnessEvent( new_brightness_value ); },
						_("CANCEL"), [] { return; } ));
				}
				else
				{
					mPopupDisplayed = false;
					ApiSystem::getInstance()->setSystemHotkeyBrightnessEvent( new_brightness_value );
				}
			}
		});

	// volume events
	auto volume = std::make_shared<SwitchComponent>(window);
	bool volume_value = ApiSystem::getInstance()->isSystemHotkeyVolumeEvent();
	volume->setState(volume_value);
	addWithLabel(_("VOLUME"), volume);
	addSaveFunc([this, volume_value, volume]
		{
			bool new_volume_value = volume->getState();
			if (volume_value != new_volume_value)
			{
				if (!mPopupDisplayed)
				{
					mPopupDisplayed = true;
					mWindow->pushGui(new GuiMsgBox(mWindow,
						_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
						_("OK"), [new_volume_value] { ApiSystem::getInstance()->setSystemHotkeyVolumeEvent( new_volume_value ); },
						_("CANCEL"), [] { return; } ));
				}
				else
				{
					mPopupDisplayed = false;
					ApiSystem::getInstance()->setSystemHotkeyVolumeEvent( new_volume_value );
				}
			}
		});

	// wifi events
	auto wifi = std::make_shared<SwitchComponent>(window);
	bool wifi_value = ApiSystem::getInstance()->isSystemHotkeyWifiEvent();
	wifi->setState(wifi_value);
	addWithLabel(_("WIFI"), wifi);
	addSaveFunc([this, wifi_value, wifi]
		{
			bool new_wifi_value = wifi->getState();
			if (wifi_value != new_wifi_value)
			{
				if (!mPopupDisplayed)
				{
					mPopupDisplayed = true;
					mWindow->pushGui(new GuiMsgBox(mWindow,
						_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
						_("OK"), [new_wifi_value] { ApiSystem::getInstance()->setSystemHotkeyWifiEvent( new_wifi_value ); },
						_("CANCEL"), [] { return; } ));
				}
				else
				{
					mPopupDisplayed = false;
					ApiSystem::getInstance()->setSystemHotkeyWifiEvent( new_wifi_value );
				}
			}
		});

	// performance events
	auto performance = std::make_shared<SwitchComponent>(window);
	bool performance_value = ApiSystem::getInstance()->isSystemHotkeyPerformanceEvent();
	performance->setState(performance_value);
	addWithLabel(_("PERFORMANCE"), performance);
	addSaveFunc([this, performance_value, performance]
		{
			bool new_performance_value = performance->getState();
			if (performance_value != new_performance_value)
			{
				if (!mPopupDisplayed)
				{
					mPopupDisplayed = true;
					mWindow->pushGui(new GuiMsgBox(mWindow,
						_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
						_("OK"), [new_performance_value] { ApiSystem::getInstance()->setSystemHotkeyPerformanceEvent( new_performance_value ); },
						_("CANCEL"), [] { return; } ));
				}
				else
				{
					mPopupDisplayed = false;
					ApiSystem::getInstance()->setSystemHotkeyPerformanceEvent( new_performance_value );
				}
			}
		});

	// suspend events
	auto suspend = std::make_shared<SwitchComponent>(window);
	bool suspend_value = ApiSystem::getInstance()->isSystemHotkeySuspendEvent();
	suspend->setState(suspend_value);
	addWithLabel(_("SUSPEND"), suspend);
	addSaveFunc([this, suspend_value, suspend]
		{
			bool new_suspend_value = suspend->getState();
			if (suspend_value != new_suspend_value)
			{
				if (!mPopupDisplayed)
				{
					mPopupDisplayed = true;
					mWindow->pushGui(new GuiMsgBox(mWindow,
						_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
						_("OK"), [new_suspend_value] { ApiSystem::getInstance()->setSystemHotkeySuspendEvent( new_suspend_value ); },
						_("CANCEL"), [] { return; } ));
				}
				else
				{
					mPopupDisplayed = false;
					ApiSystem::getInstance()->setSystemHotkeySuspendEvent( new_suspend_value );
				}
			}
		});

}
