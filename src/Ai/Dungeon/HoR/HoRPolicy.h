// Pure rules for the Halls of Reflection bosses: Marwyn's Shared Suffering stays on its target.

#ifndef _PLAYERBOTS_HOR_POLICY_H
#define _PLAYERBOTS_HOR_POLICY_H

namespace HallsOfReflection
{
// Dispelling Shared Suffering splits its remaining damage over the whole party; groups heal through it instead.
constexpr bool SharedSufferingAllows(bool cure, bool targetHasSharedSuffering)
{
    return !cure || !targetHasSharedSuffering;
}
}  // namespace HallsOfReflection

#endif
