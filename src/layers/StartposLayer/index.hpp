#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class StartposLayer : CCLayer, SetIDPopupDelegate, LevelManagerDelegate
{
public:
	static StartposLayer *create();
	static CCScene *scene();

	void loadStartPosLevelList();

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

	TaskHolder<web::WebResponse> m_listener;
	std::string m_query;
	size_t m_page = 0;
	size_t m_lvlsPerPage = 10;
	std::vector<int> m_searchResults;

	std::string m_info = "HI!";

	bool init();
};