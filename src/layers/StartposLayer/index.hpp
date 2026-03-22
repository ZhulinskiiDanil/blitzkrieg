#pragma once
#include <Geode/Geode.hpp>

#include "../../config.hpp"
#include "../../serialization/startpos/index.hpp"

using namespace geode::prelude;

class StartPosLayer : CCLayer, SetIDPopupDelegate, LevelManagerDelegate
{
public:
	static StartPosLayer *create();
	static CCScene *scene();
	~StartPosLayer();

	void loadStartPosLevelList();
	void reload();

protected:
	GJListLayer *m_levelList;
	CCMenu *m_searchBarMenu;
	TextInput *m_searchBar;
	CCLabelBMFont *m_levelsCountLabel;
	CCLabelBMFont *m_pageLabel;
	CCMenuItemSpriteExtra *m_pageButton;
	CCMenuItemSpriteExtra *m_backButton;
	CCMenuItemSpriteExtra *m_leftButton;
	CCMenuItemSpriteExtra *m_rightButton;
	InfoAlertButton *m_infoButton;
	CCMenuItemSpriteExtra *m_randomButton;
	CCMenuItemSpriteExtra *m_firstButton;
	CCMenuItemSpriteExtra *m_lastButton;
	LoadingCircle *m_loadingCircle;

	CCLayer *m_levelsList;

	TaskHolder<web::WebResponse> m_listener;
	std::string m_query;
	size_t m_page = 1;
	size_t m_lvlsPerPage = 10;
	size_t m_totalLevels = 0;
	std::vector<GJGameLevel *> m_levels;
	std::vector<StartPosLevel> m_searchResults;

	std::string m_info = "HI!";

	bool init() override;
	void loadLevels();
	void loadLevelsFinished(CCArray *levels, char const *key) override;
	void loadLevelsFailed(char const *key, int p1) override;
};