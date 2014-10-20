//
//  SceneLights.cpp
//  assignment3
//
//  Created by Charles Yang on 10/16/14.
//
//

#include "SceneLights.h"

#include "stglew.h"
#include <vector>

int SceneLights::lightCount = 2;
bool SceneLights::usingGlLights = true;
bool SceneLights::finishedSetup = false;
bool SceneLights::usingSpireLights = true;

// Declare the 8 lights here...
/*
 light0: center
 light1: some ambient or smth

 lights2-8: the spires
 

 */

// Center light
static float lightPosition0[] = {25.0f, 3, 0, 1.0f};
static float ambientLight0[]  = {1, 1, 1, 1.0};
//static float ambientLight0[]  = {0.20, 0.20, 0.20, 1.0};
//static float diffuseLight0[]  = {.6, .6, .7, 1.0};
static float diffuseLight0[]  = {0, 1, 1, 1.0};
static float specularLight0[] = {1, 1, 1, 1.0};

// Some other light, probably player light
static float lightPosition1[] = {28, 8, 3, 1.0f};
static float ambientLight1[]  = {0.10, 0.10, 0.10, 1.0};
static float diffuseLight1[]  = {.10, .10, .100, 1.0};
static float specularLight1[] = {.10, .10, .100, 1.0};

// Spire Lights
static float ambientLight_spire[]  = {0, 0, 0, 1.0};
static float diffuseLight_spire[]  = {0, .8, 1, 1.0};
//static float specularLight_spire[] = {0,0,0,0};
static float specularLight_spire[] = {.300, .300, .300, 1.0};

//static float lightPosition2[] = {-100.7924, -4, 0.772084, 1.0f};
//static float ambientLight2[]  = {0, 0, 0, 1.0};
//static float diffuseLight2[]  = {.3, .3, .4, 1.0};
//static float specularLight2[] = {0.20, 0.20, 0.4, 1.0};





//So i can access gl_light0 with index
static const int lightEnum[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};

static std::vector<float *> lightPositions;
static std::vector<float *> ambientLights;
static std::vector<float *> diffuseLights;
static std::vector<float *> specularLights;

// Manually plug each array into the vectors...Sigh
void SceneLights::manualSetupLights(){
    lightPositions.push_back(lightPosition0);
    lightPositions.push_back(lightPosition1);

    ambientLights.push_back(ambientLight0);
    ambientLights.push_back(ambientLight1);

    diffuseLights.push_back(diffuseLight0);
    diffuseLights.push_back(diffuseLight1);

    specularLights.push_back(specularLight0);
    specularLights.push_back(specularLight1);

}

// Pass in the array of spire lights
void SceneLights::setupSpireLights( STPoint3 spirePositions[], float spireY, int spireCount){
//    return;

    if (!usingSpireLights) return;

    for (int i = 0; i < spireCount; i ++){
        auto pos = spirePositions[i];
//        std::cout << "Pos: " << "( " << pos.x << ", " << pos.y << " )" << std::endl;

        float *foo;
        foo = new float[4];
        foo[0] = pos.x - 5;
        foo[1] = pos.z ;
        foo[2] = pos.y / 1.08;
        foo[3] = 1.0f;

        lightPositions.push_back(foo);
        ambientLights.push_back(ambientLight_spire);
        diffuseLights.push_back(diffuseLight_spire);
        specularLights.push_back(specularLight_spire);

        lightCount++;
    }
}

void SceneLights::setupLightData(){
    manualSetupLights();
}

// write this to make sure the data is intact... sigh
bool SceneLights::checkLightData()
{
    return true;
}

void SceneLights::setupLights()
{
    if (finishedSetup) return;

    setupLightData();

    // attributes in the shader (see examples of this in normalmap.frag)
    glEnable(GL_LIGHTING);

    // Add all light color attributes
    for (int i = 0; i < lightCount; i++){
        int light = lightEnum[i];
        glEnable(light);
        glLightfv(light, GL_SPECULAR,  specularLights[i]);
        glLightfv(light, GL_AMBIENT,   ambientLights[i]);
        glLightfv(light, GL_DIFFUSE,   diffuseLights[i]);

//        if (light == GL_LIGHT0) || light == GL_LIGHT1){
        if (light == GL_LIGHT0 || light == GL_LIGHT1){
            glLightf(light, GL_CONSTANT_ATTENUATION, 0.0);
            glLightf(light, GL_LINEAR_ATTENUATION, 1.0/20.0);
        } else {
            glLightf(light, GL_CONSTANT_ATTENUATION, 0);
            glLightf(light, GL_LINEAR_ATTENUATION, 1/30.0);
        }
    }


//    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.5);

    finishedSetup = true;
}

void SceneLights::positionLights()
{
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPositions[0]);
//    glLightfv(GL_LIGHT1, GL_POSITION, lightPositions[1]);
//    glLightfv(GL_LIGHT2, GL_POSITION, lightPositions[2]);

    for (int i = 0; i< lightCount; i++){
        glLightfv(lightEnum[i], GL_POSITION, lightPositions[i]);
    }
}

void SceneLights::setGlUniforms()
{

}

STPoint3 SceneLights::getLightPosition(int lightId)
{
    auto lp = lightPositions[lightId];
    return STPoint3( lp[0], lp[1], lp[2] );
}

float* SceneLights::getLightPositionf(int lightId){
    return lightPositions[lightId];
}

int SceneLights::addLight(){
    return 0;
}




