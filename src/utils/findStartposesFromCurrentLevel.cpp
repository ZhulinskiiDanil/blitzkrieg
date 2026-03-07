#include "findStartposesFromCurrentLevel.hpp"

StartPosesResult findStartposesFromCurrentLevel()
{
  std::vector<float> m_2_1_percentages;
  std::vector<float> m_2_2_percentages;

  for (auto child : CCArrayExt<StartPosObject *>(PlayLayer::get()->m_objects))
  {
    if (auto startPos = typeinfo_cast<StartPosObject *>(child))
    {
      const float levelLength = PlayLayer::get()->m_levelLength;
      const float levelTime = PlayLayer::get()->timeForPos({levelLength, 0.f}, 0.f, 0.f, true, 0.f);

      const float startPosX = startPos->getPositionX();
      const float startPosPercentByPosX = (startPosX / levelLength) * 100.f;

      const float startPosTime = PlayLayer::get()->timeForPos({startPosX, 0.f}, 0.f, 0.f, true, 0.f);
      const float startPosPercentByTime = (startPosTime / levelTime) * 100.f;

      m_2_1_percentages.push_back(startPosPercentByPosX);
      m_2_2_percentages.push_back(startPosPercentByTime);
    }
  }

  std::sort(m_2_1_percentages.begin(), m_2_1_percentages.end());
  std::sort(m_2_2_percentages.begin(), m_2_2_percentages.end());

  return {m_2_1_percentages, m_2_2_percentages};
}