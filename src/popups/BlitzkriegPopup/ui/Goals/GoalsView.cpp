#include "GoalsView.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

#include "../../../../utils/formatCompletedAt.hpp"

namespace
{
  const ccColor4B CARD_COLOR{36, 36, 36, 255};
  const ccColor4B LIST_COLOR{30, 30, 30, 255};
  const ccColor4B ROW_COLOR{40, 40, 40, 255};
  const ccColor4B BUTTON_COLOR{32, 32, 32, 255};
  const ccColor4B ACCENT_COLOR{255, 0, 82, 255};
  const ccColor4B TRACK_COLOR{58, 58, 58, 255};
  const ccColor4B DONE_COLOR{99, 224, 110, 255};

  constexpr float PI = 3.14159265f;

  // A ring piece from angle `from` to `to`, radians, counter-clockwise
  void drawArc(CCDrawNode *node, CCPoint center, float inner, float outer, float from, float to, ccColor4F color)
  {
    const int segments = std::max(1, static_cast<int>(std::ceil(std::abs(to - from) / (2.f * PI) * 72.f)));
    const float step = (to - from) / segments;

    for (int i = 0; i < segments; ++i)
    {
      const float a = from + step * i;
      const float b = a + step;

      CCPoint quad[] = {
          center + CCPoint{std::cos(a) * inner, std::sin(a) * inner},
          center + CCPoint{std::cos(a) * outer, std::sin(a) * outer},
          center + CCPoint{std::cos(b) * outer, std::sin(b) * outer},
          center + CCPoint{std::cos(b) * inner, std::sin(b) * inner},
      };

      node->drawPolygon(quad, 4, color, 0.f, {0.f, 0.f, 0.f, 0.f});
    }
  }

  const char *getGoalCaption(DailyGoalType type)
  {
    switch (type)
    {
    case DailyGoalType::Runs:
      return "runs passed today";
    case DailyGoalType::Minutes:
      return "minutes played today";
    default:
      return "attempts today";
    }
  }

  // 640, 6.4k
  std::string formatShort(float value)
  {
    if (value >= 1000.f)
    {
      auto text = fmt::format("{:.1f}", value / 1000.f);

      if (text.ends_with(".0"))
        text.erase(text.size() - 2);

      return text + "k";
    }

    return fmt::format("{}", static_cast<int>(std::floor(value)));
  }

  // Progress of a locked achievement: 640/1000, 40/50%, 3.2/10h
  std::string formatProgress(AchievementDef const &achievement, float value)
  {
    const std::string_view id = achievement.id;
    value = std::min(value, achievement.target);

    if (id.starts_with("stages-") || id.starts_with("best-"))
      return fmt::format("{:.0f}/{:.0f}%", std::floor(value), achievement.target);

    if (id.starts_with("hours-"))
      return fmt::format("{:.1f}/{:.0f}h", value, achievement.target);

    return fmt::format("{}/{}", formatShort(value), formatShort(achievement.target));
  }

  // The same rule as the profile list
  std::string getDisplayName(std::string name)
  {
    if (Mod::get()->getSettingValue<bool>("enable-streamer-mode") && !name.empty())
      name = name.substr(0, 1) + "...";

    return name;
  }

  CCMenuItemSpriteExtra *createPill(
      const char *text,
      CCSize size,
      CCObject *target,
      SEL_MenuHandler selector,
      RectNode **bgOut = nullptr)
  {
    auto content = CCNode::create();
    content->setContentSize(size);

    auto bg = RectNode::create(size, ccc4FFromccc4B(BUTTON_COLOR), size.height / 2);
    content->addChild(bg);

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->limitLabelWidth(size.width - 8.f, .26f, .1f);
    label->setPosition(size / 2);
    content->addChild(label);

    if (bgOut)
      *bgOut = bg;

    auto item = CCMenuItemSpriteExtra::create(content, target, selector);
    item->m_scaleMultiplier = 1.05f;
    return item;
  }
}

GoalsView *GoalsView::create(CCSize const &size, Profile const *profile, std::time_t seenAt)
{
  auto ret = new GoalsView();
  if (ret && ret->init(size, profile, seenAt))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool GoalsView::init(CCSize const &size, Profile const *profile, std::time_t seenAt)
{
  if (!CCNode::init())
    return false;

  m_size = size;
  m_seenAt = seenAt;
  this->setContentSize(size);

  drawGoal();
  drawAchievements(profile);

  return true;
}

// ! --- Daily goal card --- !

void GoalsView::drawGoal()
{
  const float height = m_size.height;

  this->addChild(RectNode::create({GOAL_WIDTH, height}, ccc4FFromccc4B(CARD_COLOR), 8.f));

  auto title = CCLabelBMFont::create("Daily goal", "bigFont.fnt");
  title->setScale(.3f);
  title->setOpacity(200);
  title->setPosition({GOAL_WIDTH / 2, height - 11.f});
  this->addChild(title);

  // ! --- Ring with the numbers inside --- !
  m_ringCenter = CCPoint(GOAL_WIDTH / 2, height - 24.f - RING_RADIUS);

  m_ring = CCDrawNode::create();
  this->addChild(m_ring);

  m_progressLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_progressLabel->setPosition(m_ringCenter + CCPoint{0.f, 5.f});
  this->addChild(m_progressLabel);

  m_targetLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_targetLabel->setOpacity(150);
  m_targetLabel->setPosition(m_ringCenter - CCPoint{0.f, 9.f});
  this->addChild(m_targetLabel);

  m_captionLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_captionLabel->setOpacity(150);
  m_captionLabel->setPosition({GOAL_WIDTH / 2, m_ringCenter.y - RING_RADIUS - 9.f});
  this->addChild(m_captionLabel);

  // ! --- Type: runs, attempts, minutes --- !
  auto menu = CCMenu::create();
  menu->setPosition({0.f, 0.f});
  this->addChild(menu);

  const float padding = 6.f;
  const float gap = 3.f;
  const float typeWidth = (GOAL_WIDTH - padding * 2 - gap * 2) / 3;
  const float typeY = 8.f + PILL_HEIGHT * 1.5f + gap * 2;

  const std::pair<DailyGoalType, const char *> types[] = {
      {DailyGoalType::Runs, "Runs"},
      {DailyGoalType::Attempts, "Att."},
      {DailyGoalType::Minutes, "Min"},
  };

  for (std::size_t i = 0; i < std::size(types); ++i)
  {
    RectNode *bg = nullptr;

    auto item = createPill(types[i].second, {typeWidth, PILL_HEIGHT}, this, menu_selector(GoalsView::onGoalType), &bg);
    item->setTag(static_cast<int>(types[i].first));
    item->setPosition({padding + typeWidth / 2 + i * (typeWidth + gap), typeY});
    menu->addChild(item);

    m_typeButtons.push_back({types[i].first, bg});
  }

  // ! --- Amount: - 200 + --- !
  const float amountY = 8.f + PILL_HEIGHT / 2;
  const CCSize stepSize{24.f, PILL_HEIGHT};

  auto less = createPill("-", stepSize, this, menu_selector(GoalsView::onGoalLess));
  less->setPosition({padding + stepSize.width / 2, amountY});
  menu->addChild(less);

  auto more = createPill("+", stepSize, this, menu_selector(GoalsView::onGoalMore));
  more->setPosition({GOAL_WIDTH - padding - stepSize.width / 2, amountY});
  menu->addChild(more);

  m_amountLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_amountLabel->setPosition({GOAL_WIDTH / 2, amountY});
  this->addChild(m_amountLabel);

  updateGoal();
}

void GoalsView::updateGoal()
{
  const auto type = getDailyGoalType();
  const int amount = getDailyGoalAmount();
  const float progress = getDailyGoalProgress();
  const float share = std::clamp(progress / std::max(1, amount), 0.f, 1.f);
  const bool done = progress >= amount;

  // ! --- Ring: the track, then the done part clockwise from the top --- !
  m_ring->clear();

  const float outer = RING_RADIUS;
  const float inner = RING_RADIUS - RING_THICKNESS;

  drawArc(m_ring, m_ringCenter, inner, outer, 0.f, 2.f * PI, ccc4FFromccc4B(TRACK_COLOR));

  if (share > 0.f)
  {
    drawArc(
        m_ring,
        m_ringCenter,
        inner,
        outer,
        PI / 2 - 2.f * PI * share,
        PI / 2,
        ccc4FFromccc4B(done ? DONE_COLOR : ACCENT_COLOR));
  }

  // ! --- Numbers --- !
  m_progressLabel->setString(fmt::format("{}", static_cast<int>(std::floor(progress))).c_str());
  m_progressLabel->limitLabelWidth(inner * 2 - 8.f, .5f, .2f);
  m_progressLabel->setColor(done ? ccColor3B{99, 224, 110} : ccColor3B{255, 255, 255});

  m_targetLabel->setString(fmt::format("/ {}", amount).c_str());
  m_targetLabel->limitLabelWidth(inner * 2 - 10.f, .26f, .1f);

  m_captionLabel->setString(done ? "Done for today" : getGoalCaption(type));
  m_captionLabel->limitLabelWidth(GOAL_WIDTH - 12.f, .24f, .1f);

  m_amountLabel->setString(fmt::format("{}", amount).c_str());
  m_amountLabel->limitLabelWidth(GOAL_WIDTH - 70.f, .32f, .1f);

  for (auto const &button : m_typeButtons)
    button.bg->setColor(ccc4FFromccc4B(button.type == type ? ACCENT_COLOR : BUTTON_COLOR));
}

void GoalsView::onGoalType(CCObject *sender)
{
  auto *node = typeinfo_cast<CCNode *>(sender);

  if (!node)
    return;

  const auto type = static_cast<DailyGoalType>(node->getTag());

  if (type == getDailyGoalType())
    return;

  setDailyGoalType(type);
  updateGoal();
}

void GoalsView::onGoalLess(CCObject *)
{
  setDailyGoalAmount(getDailyGoalAmount() - getDailyGoalStep(getDailyGoalType()));
  updateGoal();
}

void GoalsView::onGoalMore(CCObject *)
{
  setDailyGoalAmount(getDailyGoalAmount() + getDailyGoalStep(getDailyGoalType()));
  updateGoal();
}

// ! --- Achievements of the profile --- !

void GoalsView::drawAchievements(Profile const *profile)
{
  const float left = GOAL_WIDTH + GAP;
  const float width = m_size.width - left;
  const float listHeight = m_size.height - HEADER_HEIGHT - 4.f;

  // ! --- Header --- !
  const auto &achievements = getAchievements();
  int unlockedCount = 0;

  if (profile)
  {
    for (auto const &achievement : achievements)
    {
      if (profile->data.achievements.contains(achievement.id))
        unlockedCount++;
    }
  }

  auto title = CCLabelBMFont::create(
      profile ? fmt::format("Achievements of {}", getDisplayName(profile->profileName)).c_str() : "Achievements",
      "bigFont.fnt");
  title->setAnchorPoint({0.f, .5f});
  title->limitLabelWidth(width - 50.f, .3f, .1f);
  title->setOpacity(200);
  title->setPosition({left + 2.f, m_size.height - HEADER_HEIGHT / 2});
  this->addChild(title);

  auto counter = CCLabelBMFont::create(fmt::format("{}/{}", unlockedCount, achievements.size()).c_str(), "bigFont.fnt");
  counter->setAnchorPoint({1.f, .5f});
  counter->setScale(.26f);
  counter->setOpacity(150);
  counter->setPosition({m_size.width - 2.f, m_size.height - HEADER_HEIGHT / 2});
  this->addChild(counter);

  // ! --- List frame --- !
  auto background = RectNode::create({width, listHeight}, ccc4FFromccc4B(LIST_COLOR), 8.f);
  background->setPosition({left, 0.f});
  this->addChild(background);

  if (!profile)
  {
    auto empty = CCLabelBMFont::create("Attach a profile to this level", "bigFont.fnt");
    empty->limitLabelWidth(width - 20.f, .3f, .1f);
    empty->setOpacity(150);
    empty->setPosition({left + width / 2, listHeight / 2});
    this->addChild(empty);
    return;
  }

  auto scroll = ScrollLayer::create({width - 8.f, listHeight - 8.f});
  scroll->setPosition({left + 4.f, 4.f});
  scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(3.f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center)
          ->setAutoGrowAxis(scroll->getContentHeight()));
  this->addChild(scroll);

  // ! --- Order: unlocked newest first, then locked by progress --- !
  struct Row
  {
    AchievementDef const *achievement;
    float value;
    std::time_t unlockedAt;
  };

  std::vector<Row> rows;

  for (auto const &achievement : achievements)
  {
    auto it = profile->data.achievements.find(achievement.id);
    rows.push_back({
        &achievement,
        achievement.value(*profile),
        it != profile->data.achievements.end() ? it->second : 0,
    });
  }

  std::stable_sort(rows.begin(), rows.end(), [](Row const &a, Row const &b)
                   {
                     const bool aUnlocked = a.unlockedAt > 0;
                     const bool bUnlocked = b.unlockedAt > 0;

                     if (aUnlocked != bUnlocked)
                       return aUnlocked;

                     if (aUnlocked)
                       return a.unlockedAt > b.unlockedAt;

                     return a.value / a.achievement->target > b.value / b.achievement->target; });

  // ! --- Rows --- !
  const float rowWidth = scroll->getContentWidth();

  for (auto const &row : rows)
  {
    const bool unlocked = row.unlockedAt > 0;
    const GLubyte textOpacity = unlocked ? 255 : 120;

    auto cell = CCNode::create();
    cell->setContentSize({rowWidth, ROW_HEIGHT});
    cell->addChild(RectNode::create({rowWidth, ROW_HEIGHT}, ccc4FFromccc4B(ROW_COLOR), 5.f));

    // ! Icon, a dot when the frame is missing
    const float iconSize = 13.f;

    if (auto icon = CCSprite::createWithSpriteFrameName(row.achievement->icon))
    {
      icon->setScale(iconSize / std::max(icon->getContentWidth(), icon->getContentHeight()));
      icon->setPosition({12.f, ROW_HEIGHT / 2});
      icon->setOpacity(unlocked ? 255 : 90);

      if (!unlocked)
        icon->setColor({120, 120, 120});

      cell->addChild(icon);
    }

    // ! Right side: date and "New", or a progress bar
    float textRight = rowWidth - 8.f;

    if (unlocked)
    {
      auto date = CCLabelBMFont::create(formatShortDate(row.unlockedAt).c_str(), "bigFont.fnt");
      date->setScale(.22f);
      date->setOpacity(150);
      date->setAnchorPoint({1.f, .5f});
      date->setPosition({rowWidth - 8.f, ROW_HEIGHT / 2});
      cell->addChild(date);

      textRight = rowWidth - 8.f - date->getScaledContentWidth() - 6.f;

      if (row.unlockedAt > m_seenAt)
      {
        auto newLabel = CCLabelBMFont::create("New", "bigFont.fnt");
        newLabel->setScale(.22f);

        const CCSize chipSize{newLabel->getScaledContentWidth() + 8.f, 11.f};
        auto chip = RectNode::create(chipSize, ccc4FFromccc4B(ACCENT_COLOR), chipSize.height / 2);
        chip->setPosition({textRight - chipSize.width, ROW_HEIGHT / 2 - chipSize.height / 2});
        cell->addChild(chip);

        newLabel->setPosition({textRight - chipSize.width / 2, ROW_HEIGHT / 2});
        cell->addChild(newLabel);

        textRight -= chipSize.width + 6.f;
      }
    }
    else
    {
      const float barWidth = 44.f;
      const float barHeight = 3.f;
      const float share = std::clamp(row.value / row.achievement->target, 0.f, 1.f);

      const CCPoint barOrigin{rowWidth - 8.f - barWidth, ROW_HEIGHT / 2 - 6.f};

      auto track = RectNode::create({barWidth, barHeight}, ccc4FFromccc4B(TRACK_COLOR), barHeight / 2);
      track->setPosition(barOrigin);
      cell->addChild(track);

      if (share > 0.f)
      {
        auto fill = RectNode::create({std::max(barHeight, barWidth * share), barHeight}, ccc4FFromccc4B(ACCENT_COLOR), barHeight / 2);
        fill->setPosition(barOrigin);
        cell->addChild(fill);
      }

      auto progress = CCLabelBMFont::create(formatProgress(*row.achievement, row.value).c_str(), "bigFont.fnt");
      progress->setAnchorPoint({1.f, .5f});
      progress->limitLabelWidth(barWidth + 10.f, .2f, .1f);
      progress->setOpacity(150);
      progress->setPosition({rowWidth - 8.f, ROW_HEIGHT / 2 + 4.f});
      cell->addChild(progress);

      textRight = barOrigin.x - 8.f;
    }

    // ! Title and description
    auto titleLabel = CCLabelBMFont::create(row.achievement->title, "bigFont.fnt");
    titleLabel->setAnchorPoint({0.f, .5f});
    titleLabel->limitLabelWidth(textRight - 24.f, .28f, .1f);
    titleLabel->setOpacity(textOpacity);
    titleLabel->setPosition({24.f, ROW_HEIGHT / 2 + 5.f});
    cell->addChild(titleLabel);

    auto description = CCLabelBMFont::create(row.achievement->description, "chatFont.fnt");
    description->setAnchorPoint({0.f, .5f});
    description->limitLabelWidth(textRight - 24.f, .42f, .2f);
    description->setOpacity(unlocked ? 170 : 100);
    description->setPosition({24.f, ROW_HEIGHT / 2 - 5.f});
    cell->addChild(description);

    scroll->m_contentLayer->addChild(cell);
  }

  scroll->m_contentLayer->updateLayout();
  scroll->scrollToTop();
}
