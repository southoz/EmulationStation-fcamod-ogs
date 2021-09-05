#include "guis/GuiQuitOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "ApiSystem.h"
#include "Log.h"


GuiQuitOptions::GuiQuitOptions(Window* window) : GuiSettings(window, _("\"QUIT\" SETTINGS").c_str()), mPopupDisplayed(false)
{
	initializeMenu(window);
}

GuiQuitOptions::~GuiQuitOptions()
{
	mPopupDisplayed = false;
}

void GuiQuitOptions::initializeMenu(Window* window)
{
	// full exit
	auto fullExitMenu = std::make_shared<SwitchComponent>(mWindow);
	fullExitMenu->setState(!Settings::getInstance()->getBool("ShowOnlyExit"));
	addWithLabel(_("VIEW MENU"), fullExitMenu);
	addSaveFunc([fullExitMenu] { Settings::getInstance()->setBool("ShowOnlyExit", !fullExitMenu->getState()); });

	// confirm to exit
	auto confirmToExit = std::make_shared<SwitchComponent>(mWindow);
	confirmToExit->setState(Settings::getInstance()->getBool("ConfirmToExit"));
	addWithLabel(_("CONFIRM ACTION"), confirmToExit);
	addSaveFunc([confirmToExit] { Settings::getInstance()->setBool("ConfirmToExit", confirmToExit->getState()); });

	// only exit action
	std::string only_exit_action = Settings::getInstance()->getString("OnlyExitAction");
	auto only_exit_action_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("SCRAPE FROM"), false);

	only_exit_action_list->add(_("SHUTDOWN"), "shutdown", only_exit_action == "shutdown");
	only_exit_action_list->add(_("SUSPEND"), "suspend", only_exit_action == "suspend");
	only_exit_action_list->add(_("QUIT EMULATIONSTATION"), "exit_es", only_exit_action == "exit_es");

	addWithLabel(_("ACTION TO EXECUTE TO \"QUIT\" (NO MENU)"), only_exit_action_list);
	addSaveFunc([only_exit_action_list] { Settings::getInstance()->setString("OnlyExitAction", only_exit_action_list->getSelected()); });

	if (!only_exit_action_list->hasSelection())
	{
		only_exit_action_list->selectFirstItem();
		only_exit_action = only_exit_action_list->getSelected();
	}

	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::POWER_KEY))
	{
		addGroup("POWERKEY BUTTON");

		// push powerkey double to shutdown
		auto powerkey = std::make_shared<SwitchComponent>(mWindow);
		bool powerkey_value = ApiSystem::getInstance()->getPowerkeyState();
		LOG(LogDebug) << "GuiQuitOptions::initializeMenu() - powerkey_value: " << Utils::String::boolToString(powerkey_value);
		powerkey->setState( powerkey_value );
		addWithLabel(_("PUSH TWO TIMES TO SHUTDOWN"), powerkey);
		addSaveFunc([this, window, powerkey, powerkey_value]
			{
				bool new_powerkey_value = powerkey->getState();
				if (powerkey_value != new_powerkey_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						window->pushGui(new GuiMsgBox(window,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_powerkey_value] { ApiSystem::getInstance()->setPowerkeyState( new_powerkey_value ); },
							_("CANCEL"), [] { return; } ));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyState( new_powerkey_value );
					}
				}

			});

		// max interval time
		auto interval_time = std::make_shared<SliderComponent>(mWindow, 1.f, 10.f, 1.f, "s", true);
		float interval_time_value = (float) ApiSystem::getInstance()->getPowerkeyIntervalTime();
		LOG(LogDebug) << "GuiQuitOptions::initializeMenu() - interval_time_value: " << std::to_string(interval_time_value);
		interval_time->setValue( interval_time_value );
		addWithLabel(_("TIME INTERVAL"), interval_time);
		addSaveFunc([this, window, interval_time, interval_time_value]
			{
				float new_interval_time_value = Math::round( interval_time->getValue() );
				if (interval_time_value != new_interval_time_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						window->pushGui(new GuiMsgBox(window,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_interval_time_value] { ApiSystem::getInstance()->setPowerkeyIntervalTime( (int) new_interval_time_value ); },
							_("CANCEL"), [] { return; } ));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyIntervalTime( (int) new_interval_time_value );
					}
				}
			});
	}
}
