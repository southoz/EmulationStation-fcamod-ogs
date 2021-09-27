#pragma once
#ifndef ES_APP_GUIS_GUI_MENU_H
#define ES_APP_GUIS_GUI_MENU_H

#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "GuiComponent.h"

class GuiSettings;
class SystemData;

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window, bool animate = true);

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

	static void openThemeConfiguration(Window* mWindow, GuiComponent* s, std::shared_ptr<OptionListComponent<std::string>> theme_set, const std::string systemTheme = "");

	static void updateGameLists(Window* window, bool confirm = true);

private:
	void addEntry(std::string name, bool add_arrow, const std::function<void()>& func, const std::string iconName = "");	

	void addVersionInfo();
	void openCollectionSystemSettings();
	void openConfigInput();
	void openAdvancedSettings();
	void openQuitMenu();
	void openControllersSettings();
	void openScraperSettings();
	void openScreensaverOptions();
	void openSoundSettings();
	void openUISettings();
	void openSystemInformation();
	void openQuitSettings();
	void openMenusSettings();
	void openSystemHotkeyEventsSettings();
	void openRetroAchievementsSettings();
	void openNetworkSettings(bool selectWifiEnable = false);

	static void openWifiSettings(Window* win, std::string title, std::string data, const std::function<void(std::string)>& onsave);

	void openUpdateSettings();
	void openEmulatorSettings();
	void openSystemEmulatorSettings(SystemData* system);

	void createInputTextRow(GuiSettings * gui, std::string title, const char* settingsID, bool password, bool storeInSettings=false, const std::function<void(Window*, std::string/*title*/, std::string /*value*/, const std::function<void(std::string)>& onsave)>& customEditor = nullptr);
	void openDisplaySettings();

	MenuComponent mMenu;
	TextComponent mVersion;

};

#endif // ES_APP_GUIS_GUI_MENU_H
