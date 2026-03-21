#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class StartposLayer : CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
public:
    static StartposLayer* create();
	static CCScene* scene();

    void loadStartposLevelList();
protected:
    TaskHolder<web::WebResponse> m_listener;
	GJListLayer* m_levelList;
	cocos2d::CCMenu* m_searchBarMenu;
	geode::TextInput* m_searchBar;
	cocos2d::CCLabelBMFont* m_levelsCountLabel;
	cocos2d::CCLabelBMFont* m_pageLabel;
	CCMenuItemSpriteExtra* m_pageButton;
	CCMenuItemSpriteExtra* m_backButton;
	CCMenuItemSpriteExtra* m_leftButton;
	CCMenuItemSpriteExtra* m_rightButton;
	InfoAlertButton* m_infoButton;
	CCMenuItemSpriteExtra* m_randomButton;
	CCMenuItemSpriteExtra* m_firstButton;
	CCMenuItemSpriteExtra* m_lastButton;
	LoadingCircle* m_loadingCircle;
	size_t m_page = 0;
	size_t m_lvlsPerPage = 10;
	std::string m_query;
	std::vector<int> m_searchResults;
	std::string m_info = "HI!";

    bool init();
};