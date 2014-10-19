//
//  SceneLights.h
//  assignment3
//
//  Created by Charles Yang on 10/16/14.
//
//

#ifndef __assignment3__SceneLights__
#define __assignment3__SceneLights__

#include <stdio.h>
#include "stglew.h"

class SceneLights
{
public:



    // Call the enabling and etc.
    // Call once during setup
    static void setupLights();
    static void setupLightData();
    static void setupSpireLights( STPoint3 spirePositions[], float spireY, int spireCount);

    // Call on each draw call
    static void positionLights();

    // uniform
    static void setGlUniforms();

    static STPoint3 getLightPosition(int lightId);
    static float* getLightPositionf(int lightId);

    // Dynamically add a light. probably for something else.
    // Used mostly internally
    static int addLight();

    static int lightCount;
    static bool usingGlLights;
    static bool usingSpireLights;

private:

    // Setups all the light objects

    static bool checkLightData();

    static void manualSetupLights();

    static bool finishedSetup;



};





#endif /* defined(__assignment3__SceneLights__) */
