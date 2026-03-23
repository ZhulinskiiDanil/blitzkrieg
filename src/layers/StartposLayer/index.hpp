#pragma once
#include <Geode/Geode.hpp>

#include "../../config.hpp"
#include "../../serialization/startpos/index.hpp"
#include "../../utils/encodeURIComponent.hpp"
#include "../../ui/RectNode.hpp"

class StartPosLayer : public CCLayer,
											public SetIDPopupDelegate,
											public LevelManagerDelegate,
											public TableViewDelegate,
											public TableViewDataSource,
											public CCScrollLayerExtDelegate
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

	TaskHolder<web::WebResponse> m_listener;
	std::string m_query;
	size_t m_page = 1;
	size_t m_lvlsPerPage = 10;
	size_t m_totalLevels = 0;
	size_t m_totalPages = 0;
	std::vector<GJGameLevel *> m_levels;
	std::vector<StartPosLevel> m_searchResults;
	std::unordered_map<int, std::string> m_downloadUrls;

	std::string m_info = "HI!";
	float m_levelCellHeigth = 90.0f;
	float m_customCellHeigth = 90.0f;
	bool m_isLoading = false;

	bool init() override;

	void onOpenDownloadLink(CCObject *sender);

	void page(size_t page);
	void showLoading();

	void loadLevels();
	void loadLevelsFinished(CCArray *levels, char const *key) override;
	void loadLevelsFailed(char const *key, int p1) override;

	virtual int numberOfRowsInSection(unsigned int section, TableView *tableView) override;
	virtual unsigned int numberOfSectionsInTableView(TableView *tableView) override;
	virtual TableViewCell *cellForRowAtIndexPath(CCIndexPath &indexPath, TableView *tableView) override;
	virtual float cellHeightForRowAtIndexPath(CCIndexPath &indexPath, TableView *tableView) override;
};