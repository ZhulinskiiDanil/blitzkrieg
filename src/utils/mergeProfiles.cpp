#include "./mergeProfiles.hpp"

Profile mergeProfiles(
    const Profile &oldProfile,
    const Profile &newProfile,
    bool automaticallyCloseRuns)
{
    Profile result;

    result.id = oldProfile.id;
    result.profileName = oldProfile.profileName;
    result.data.tags = newProfile.data.tags;
    // The days played stay with the profile, whatever its runs become
    result.data.history = oldProfile.data.history;
    result.data.achievements = oldProfile.data.achievements;
    result.data.stages = mergeProfileStages(
        oldProfile.data.stages,
        newProfile.data.stages,
        automaticallyCloseRuns);

    return result;
}