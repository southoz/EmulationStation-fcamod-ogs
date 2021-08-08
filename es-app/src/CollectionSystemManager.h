#pragma once
#ifndef ES_APP_COLLECTION_SYSTEM_MANAGER_H
#define ES_APP_COLLECTION_SYSTEM_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <unordered_map>

class FileData;
class FolderData;
class SystemData;
class Window;
struct SystemEnvironmentData;

enum CollectionSystemType
{
	AUTO_ALL_GAMES,
	AUTO_LAST_PLAYED,
	AUTO_AT2PLAYERS,
	AUTO_AT4PLAYERS,
	AUTO_NEVER_PLAYED,
	AUTO_FAVORITES,

	AUTO_ARCADE,

	CUSTOM_COLLECTION,

	CPS1_COLLECTION,
	CPS2_COLLECTION,
	CPS3_COLLECTION,
	CAVE_COLLECTION,
	NEOGEO_COLLECTION,
	SEGA_COLLECTION,
	IREM_COLLECTION,
	MIDWAY_COLLECTION,
	CAPCOM_COLLECTION,
	TECMO_COLLECTION,
	SNK_COLLECTION,
	NAMCO_COLLECTION,
	TAITO_COLLECTION,
	KONAMI_COLLECTION,
	JALECO_COLLECTION,
	ATARI_COLLECTION,
	NINTENDO_COLLECTION,
	SAMMY_COLLECTION,
	ACCLAIM_COLLECTION,
	PSIKYO_COLLECTION,
	KANEKO_COLLECTION,
	COLECO_COLLECTION,
	ATLUS_COLLECTION,
	BANPRESTO_COLLECTION,

	AUTO_VERTICALARCADE // batocera
};

struct CollectionSystemDecl
{
	CollectionSystemType type; // type of system
	std::string name;
	std::string longName;
	std::string defaultSort;
	std::string themeFolder;
	bool isCustom;
	bool displayIfEmpty;
};

struct CollectionSystemData
{
	SystemData* system;
	CollectionSystemDecl decl;
	bool isEnabled;
	bool isPopulated;
	bool needsSave;
};

class CollectionSystemManager
{
public:
	static std::vector<CollectionSystemDecl> getSystemDecls();

	CollectionSystemManager(Window* window);
	~CollectionSystemManager();

	static CollectionSystemManager* get();
	static void init(Window* window);
	static void deinit();
	void saveCustomCollection(SystemData* sys);

	void loadCollectionSystems(bool async=false);
	void loadEnabledListFromSettings();
	void updateSystemsList();

	void refreshCollectionSystems(FileData* file);
	void updateCollectionSystem(FileData* file, CollectionSystemData sysData);
	void deleteCollectionFiles(FileData* file);

	inline std::map<std::string, CollectionSystemData>& getAutoCollectionSystems() { return mAutoCollectionSystemsData; };
	inline std::map<std::string, CollectionSystemData> getCustomCollectionSystems() { return mCustomCollectionSystemsData; };
	inline SystemData* getCustomCollectionsBundle() { return mCustomCollectionsBundle; };
	std::vector<std::string> getUnusedSystemsFromTheme();
	SystemData* addNewCustomCollection(std::string name);

	bool isThemeGenericCollectionCompatible(bool genericCustomCollections);
	bool isThemeCustomCollectionCompatible(std::vector<std::string> stringVector);
	std::string getValidNewCollectionName(std::string name, int index = 0);

	void setEditMode(std::string collectionName);
	void exitEditMode();
	inline bool isEditing() { return mIsEditingCustom; };
	inline std::string getEditingCollection() { return mEditingCollection; };
	bool toggleGameInCollection(FileData* file);

	SystemData* getSystemToView(SystemData* sys);
	void updateCollectionFolderMetadata(SystemData* sys);
	void populateAutoCollection(CollectionSystemData* sysData);

private:
	static CollectionSystemManager* sInstance;
	SystemEnvironmentData* mCollectionEnvData;
	std::map<std::string, CollectionSystemDecl> mCollectionSystemDeclsIndex;
	std::map<std::string, CollectionSystemData> mAutoCollectionSystemsData;
	std::map<std::string, CollectionSystemData> mCustomCollectionSystemsData;
	Window* mWindow;
	bool mIsEditingCustom;
	std::string mEditingCollection;
	CollectionSystemData* mEditingCollectionSystemData;

	void initAutoCollectionSystems();
	void initCustomCollectionSystems();
	SystemData* getAllGamesCollection();
	SystemData* createNewCollectionEntry(std::string name, CollectionSystemDecl sysDecl, bool index = true);
	
	void populateCustomCollection(CollectionSystemData* sysData, std::unordered_map<std::string, FileData*>* pMap = nullptr);

	void removeCollectionsFromDisplayedSystems();
	void addEnabledCollectionsToDisplayedSystems(std::map<std::string, CollectionSystemData>* colSystemData, std::unordered_map<std::string, FileData*>* pMap);

	std::vector<std::string> getSystemsFromConfig();
	std::vector<std::string> getSystemsFromTheme();
	std::vector<std::string> getCollectionsFromConfigFolder();
	std::vector<std::string> getCollectionThemeFolders(bool custom);
	std::vector<std::string> getUserCollectionThemeFolders();

	void trimCollectionCount(FolderData* rootFolder, int limit);
	void sortLastPlayed(SystemData* system);

	bool themeFolderExists(std::string folder);

	bool includeFileInAutoCollections(FileData* file);

	SystemData* mCustomCollectionsBundle;
};

std::string getCustomCollectionConfigPath(std::string collectionName);
std::string getCollectionsFolder();
bool systemSort(SystemData* sys1, SystemData* sys2);

#endif // ES_APP_COLLECTION_SYSTEM_MANAGER_H
