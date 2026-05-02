#pragma once 


#ifdef GV_Editor
BEGIN_LOGIC_UNIT(LocalState, GV_CHUNK_LOCAL_STATE)

    UI_SEPARATOR("Player Perception")

    UI_PARAM_FLOAT(playerDistance, 0.0f, "Distance to Player")
    UI_PARAM_BOOL(playerVisible, 0, "Player Visible")

    UI_SEPARATOR("Local Crime")

    UI_PARAM_INT(localBounty, 0, "Local Bounty")

    UI_SEPARATOR("Threat")

    UI_PARAM_FLOAT(threatLevel, 0.0f, "Threat Level")

    UI_SEPARATOR("Environment")

    UI_PARAM_BOOL(hasNearbyFood, 0, "Nearby Food")
    UI_PARAM_BOOL(hasNearbyBed, 0, "Nearby Bed")
    UI_PARAM_BOOL(hasNearbyGuards, 0, "Nearby Guards")

    UI_SEPARATOR("Situation")

    UI_PARAM_BOOL(isInCombat, 0, "Is In Combat")

END_LOGIC_UNIT
#endif