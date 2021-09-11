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
	initializeMenu();
}

GuiQuitOptions::~GuiQuitOptions()
{
	mPopupDisplayed = false;
}

void GuiQuitOptions::initializeMenu()
{
	// full exit
	auto fullExitMenu = std::make_shared<SwitchComponent>(mWindow);
	fullExitMenu->setState(!Settings::getInstance()->getBool("ShowOnlyExit"));
	addWithLabel(_("VIEW MENU"), fullExitMenu);
	addSaveFunc([this, fullExitMenu]
		{
			bool old_value = !Settings::getInstance()->getBool("ShowOnlyExit");
			if (old_value != fullExitMenu->getState())
			{
				Settings::getInstance()->setBool("ShowOnlyExit", !fullExitMenu->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	// confirm to exit
	auto confirmToExit = std::make_shared<SwitchComponent>(mWindow);
	confirmToExit->setState(Settings::getInstance()->getBool("ConfirmToExit"));
	addWithLabel(_("CONFIRM TO \"QUIT\""), confirmToExit);
	addSaveFunc([this, confirmToExit]
		{
			bool old_value = Settings::getInstance()->getBool("ConfirmToExit");
			if (old_value != confirmToExit->getState())
			{
				Settings::getInstance()->setBool("ConfirmToExit", confirmToExit->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	addGroup(_("NO MENU"));

	// only exit action
	std::string only_exit_action = Settings::getInstance()->getString("OnlyExitAction");
	auto only_exit_action_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

	only_exit_action_list->add(_("SHUTDOWN"), "shutdown", only_exit_action == "shutdown");
	only_exit_action_list->add(_("SUSPEND"), "suspend", only_exit_action == "suspend");
	only_exit_action_list->add(_("QUIT EMULATIONSTATION"), "exit_es", only_exit_action == "exit_es");

	addWithLabel(_("ACTION TO EXECUTE"), only_exit_action_list);
	addSaveFunc([this, only_exit_action_list]
		{
			std::string old_value = Settings::getInstance()->getString("OnlyExitAction");
			if (old_value != only_exit_action_list->getSelected())
			{
				Settings::getInstance()->setString("OnlyExitAction", only_exit_action_list->getSelected());
				setVariable("reloadGuiMenu", true);
			}
		});

	if (!only_exit_action_list->hasSelection())
	{
		only_exit_action_list->selectFirstItem();
		only_exit_action = only_exit_action_list->getSelected();
	}

	// confirm to exit
	auto show_action_menu = std::make_shared<SwitchComponent>(mWindow);
	show_action_menu->setState(Settings::getInstance()->getBool("ShowOnlyExitActionAsMenu"));
	addWithLabel(_("SHOW ACTION NAME AS MENU OPTION"), show_action_menu);
	addSaveFunc([this, show_action_menu]
		{
			bool old_value = Settings::getInstance()->getBool("ShowOnlyExitActionAsMenu");
			if (old_value != show_action_menu->getState())
			{
				Settings::getInstance()->setBool("ShowOnlyExitActionAsMenu", show_action_menu->getState());
				setVariable("reloadGuiMenu", true);
			}
		});

	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::POWER_KEY))
	{
		addGroup(_("POWERKEY BUTTON"));

		// push powerkey double to shutdown
		auto powerkey = std::make_shared<SwitchComponent>(mWindow);
		bool powerkey_value = ApiSystem::getInstance()->getPowerkeyState();
		LOG(LogDebug) << "GuiQuitOptions::initializeMenu() - powerkey_value: " << Utils::String::boolToString(powerkey_value);
		powerkey->setState( powerkey_value );
		addWithLabel(_("PUSH TWO TIMES TO SHUTDOWN"), powerkey);
		addSaveFunc([this, powerkey, powerkey_value]
			{
				bool new_powerkey_value = powerkey->getState();
				if (powerkey_value != new_powerkey_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
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
		addSaveFunc([this, interval_time, interval_time_value]
			{
				float new_interval_time_value = Math::round( interval_time->getValue() );
				if (interval_time_value != new_interval_time_value)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
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

		// powerkey action
		std::string powerkey_action = ApiSystem::getInstance()->getPowerkeyAction();
		auto powerkey_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("ACTION TO EXECUTE"), false);

		powerkey_list->add(_("SHUTDOWN"), "shutdown", powerkey_action == "shutdown");
		powerkey_list->add(_("SUSPEND"), "suspend", powerkey_action == "suspend");

		addWithLabel(_("ACTION TO EXECUTE"), powerkey_list);
		addSaveFunc([this, powerkey_list, powerkey_action]
			{
				std::string new_powerkey_action = powerkey_list->getSelected();
				if (powerkey_action != new_powerkey_action)
				{
					if (!mPopupDisplayed)
					{
						mPopupDisplayed = true;
						mWindow->pushGui(new GuiMsgBox(mWindow,
							_("THE PROCESS MAY DURE SOME SECONDS.\nPLEASE WAIT."),
							_("OK"), [new_powerkey_action] { ApiSystem::getInstance()->setPowerkeyAction( new_powerkey_action ); },
							_("CANCEL"), [] { return; } ));
					}
					else
					{
						mPopupDisplayed = false;
						ApiSystem::getInstance()->setPowerkeyAction( new_powerkey_action );
					}
				}
			});

		if (!powerkey_list->hasSelection())
		{
			powerkey_list->selectFirstItem();
			powerkey_action = powerkey_list->getSelected();
		}
	}
}
