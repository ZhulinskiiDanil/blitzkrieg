#include "./mergeProfileStages.hpp"
#include "../serialization/profile/index.hpp"

Profile mergeProfiles(
    const Profile &oldProfile,
    const Profile &newProfile,
    bool automaticallyCloseRuns);