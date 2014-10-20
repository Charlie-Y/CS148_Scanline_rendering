// main.cpp

// For this project, we use OpenGL, GLUT
// and GLEW (to load OpenGL extensions)
//
#include "stglew.h"
#include "SceneLights.h"

#include <stdio.h>
#include <string.h>

//
// Globals used by this application.
// As a rule, globals are Evil, but this is a small application
// and the design of GLUT makes it hard to avoid them.
//

// Window size, kept for screenshots
static int gWindowSizeX = 0;
static int gWindowSizeY = 0;

static int initialWindowX = 1280;
static int initialWindowY = initialWindowX * 9 / 16;

// File locations
std::string vertexShader;
std::string fragmentShader;
std::string normalMap;
std::string displacementMap;
std::string meshOBJ;

STShaderProgram *shader;

STVector3 mPosition;
STVector3 mLookAt;
STVector3 mRight;
STVector3 mUp;

// Stored mouse position for camera rotation, panning, and zoom.
int gPreviousMouseX = -1;
int gPreviousMouseY = -1;
int gMouseButton = -1;
bool mesh = true; // draw mesh
bool smooth = false; // smooth/flat shading for mesh
bool normalMapping = true; // true=normalMapping, false=displacementMapping
bool proxyType=true; // false: use cylinder; true: use sphere

std::vector<STTriangleMesh*> gTriangleMeshes;
std::vector<STTriangleMesh*> gTriangleMeshes2;
STPoint3 gMassCenter;
std::pair<STPoint3,STPoint3> gBoundingBox;

STTriangleMesh* gManualTriangleMesh = 0;
int TesselationDepth = 100;

int SOME_SEED = 8;

//Nice reference ground
bool DRAW_GROUND = true;
bool DRAW_SKYBOX = false;

// Tile generation stuff
int TILES_PER_SIDE = 120;
int NUM_TILES = TILES_PER_SIDE * TILES_PER_SIDE;
float ALL_TILE_DIM = 120;
float TILE_DIM = ALL_TILE_DIM / (float) TILES_PER_SIDE;
float TILE_Y_SCALE = 4;
float TILE_SPACING = .0;

// Mesh reference strings
const std::string TILE_MESH_STR = "meshes/cube.obj";
//const std::string TILE_MESH_STR = "box_1.obj";
const std::string MOON_CYL_MESH_STR = "meshes/cylinder.obj";
const std::string MOON_MESH_STR = "meshes/sphere_fine.obj";
const std::string SKYBOX_MESH_STR = "meshes/sphere_fine.obj";

const std::string TILE_MESH_STRS[] = {"box_1.obj", "box_2.obj", "box_3.obj", "box_4.obj", "spire_thick.obj", "spire_thin.obj", "spire_diag_zag.obj", "spire_flat_zag.obj", "plane.obj"};
const int TILE_MESH_CT = 9;

enum TileType { BASIC_TILE, FLYER_TILE, ROAD_TILE, SPIRE_TILE, MOON_FLYER_TILE};
enum TileMeshIndex { ROAD_TILE_MESH = 0, BLANK_TILE_MESH = 1, SPIRE_TILE_MESH = 2, FLYER_TILE_MESH = 3, SPIRE_1_MESH = 4, SPIRE_2_MESH = 5, SPIRE_3_MESH = 6, SPIRE_4_MESH = 7, DUMMY_MESH = 8};

const int SPIRE_TYPES = 4;

// Tile mesh and positioning stuff
std::vector<STVector3*> tilePositions;
//std::vector<STTriangleMesh*> tileMeshes;
std::vector<int> randSeeds;
bool rerepulseTiles = false;
float tileStartY = -.9f;
float xDropScale = .4;

// Flyer stuff
int chanceToFly = 20;
bool noFlyers = true;
static std::vector<bool> moonTileStatuses;

// Tile Road stuff
int roadWidth = 1;
float roadY = -7.5;

// Tile Spire stuff
static float SPIRE_WIDTH = 1.4;
static int SPIRE_X = 37;
static int SPIRE_X_OFF = 13;
static int SPIRE_Z = 15;
static int SPIRE_Y = -4;

static int SPIRE_COUNT = 6;
static STPoint3 SPIRE_POS[6] = {
    STPoint3( 0 + SPIRE_X_OFF, SPIRE_Z, 0),
    STPoint3(SPIRE_X + SPIRE_X_OFF, SPIRE_Z, 10),
    STPoint3(-SPIRE_X + SPIRE_X_OFF, SPIRE_Z, -4),
    STPoint3(0 + SPIRE_X_OFF, -SPIRE_Z, 0),
    STPoint3(SPIRE_X + SPIRE_X_OFF, -SPIRE_Z, 10),
    STPoint3(-SPIRE_X + SPIRE_X_OFF, -SPIRE_Z, -4),

};

static std::vector<bool> spireTileStatus;
//static bool setupSpireStatuses = false;

// Moon stuff
float lightPosition[] = {25.0f, 3, 0, 1.0f};
float lightPosition2[] = {28, 8, 3, 1.0f};
float lightPosition_cam[] = {-100.7924, -4, 0.772084, 1.0f};

STVector3 *moonPos = 0;
STTriangleMesh* centerMoonMesh = 0;
float MOON_SCALE = 2.0f;
float moon_repulsion = 35;
//float moon_pos[] = {10, 3, 0};
float moon_pos[4];
float moon_pos2[4];
float MOON_SCALE_2 = .8;


float moon_repulsion_factor = 2;
float moonNudge = .3f;


// Background and skybox stuff
float groundY = -10;
float groundZ = 100;
float groundMeshScale = 120;

STTriangleMesh* groundMesh = 0;
float skyBoxScale = 100;
STTriangleMesh* skyBox = 0;


GLuint sceneBuffer, glowBuffer, blurBuffer;
GLuint FBOGlow, FBOHorizBlur, FBOVertBlur;

enum fboTexInd {fboGlowInd = 0,fboHorizInd = 1, fboVertInd = 2};

bool glow = true;
bool glowMapDrawn = false;

// texture handlers
GLuint texFBO[5];
int fboTextureCount = 0;
// renderbuffer handlers
GLuint rb[2];

GLuint prepareFBO(int w, int h, int colorCount);
void printFramebufferInfo(GLenum target, GLuint fbo);


/* ---------------------- Material Arrays ------------------------ */

static float ma_none[]  = { 0.0, 0.0, 0.0, 1.0 };
static float ma_slim[]  = { 0.3, 0.3, 0.3, 1.0 };
static float ma_white[]  = { 1.0, 1.0, 1.0, 1.0 };
static float materialSpecular[] = { 0.8, 0.8, 0.8, 1.0 };
static float ma_shininess          = 8.0;  // # between 1 and 128.

/* ---------------------- Camera Utility Code ------------------------ */

void SetUpAndRight()
{
    mRight = STVector3::Cross(mLookAt - mPosition, mUp);
    mRight.Normalize();
    mUp = STVector3::Cross(mRight, mLookAt - mPosition);
    mUp.Normalize();
}

void resetCamera()
{
//    mLookAt=STVector3(0.f,0,0.f);
//    mPosition=STVector3(lightPosition_cam[0], lightPosition_cam[1], lightPosition_cam[2]);
//    mPosition=STVector3(-63.9866, -8.3577, 0.400466);
//    mUp=STVector3(0.f,1.f,0.f);

//    mLookAt=STVector3 (-0.470653, -4.09034, -6.42632);
//    mPosition=STVector3(-64.493, -7.91901, 0.699518);
//    mUp=STVector3(-0.0606277, 0.998125, -0.00842466);

//    mLookAt=STVector3(-0.789271, -3.63282, -8.41105);
//    mPosition=STVector3        (-66.5, -7.59023, 1.22555);
//    mUp=STVector3(-0.061855, 0.998014, -0.0119321);

    mLookAt=STVector3(-1.18019, -4.345, -0.918852);
    mPosition=STVector3 (-63.8881, -10.6729, -0.120506);
    mUp=STVector3(-0.0999899, 0.99456, 0.0292133);
    SetUpAndRight();
}

void resetUp()
{
    mUp = STVector3(0.f,1.f,0.f);
    mRight = STVector3::Cross(mLookAt - mPosition, mUp);
    mRight.Normalize();
    mUp = STVector3::Cross(mRight, mLookAt - mPosition);
    mUp.Normalize();
}

void printVec(STVector3* vec){
        std::cout << "(" << vec->x << ", " << vec->y << ", " << vec->z << ")" << std::endl;
}


// Returns a float between a max and a min
float randRange(float min, float max)
{
    return ((double) rand() / (RAND_MAX+1)) * (max-min+1) + min;
}

// Returns a float between a max and a min, but using a seed as the random
float randRangeFromSeed( float min, float max, int randSeed){
    return ((double) randSeed / (RAND_MAX+1)) * (max-min+1) + min;
}

/* ---------------------- Geometry Utility Code ------------------------ */


void assignMaterials(STTriangleMesh *mesh, float ambient[], float diffuse[], float specular[], float shininess){
    mesh->mMaterialAmbient[0]= ambient[0];
    mesh->mMaterialAmbient[1]= ambient[1];
    mesh->mMaterialAmbient[2]= ambient[2];
    mesh->mMaterialDiffuse[0]= diffuse[0];
    mesh->mMaterialDiffuse[1]= diffuse[1];
    mesh->mMaterialDiffuse[2]= diffuse[2];
    mesh->mMaterialSpecular[0]= specular[0];
    mesh->mMaterialSpecular[1]= specular[1];
    mesh->mMaterialSpecular[2]= specular[2];
    mesh->mShininess= shininess;
}

void assignVec4f( float dest[4], float source[4]){
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest[3] = source[3];
}

void assignVec3f( float dest[3], float source[3]){
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

//maybe write an obj at x,y coordinates, if I need it..


/* ---------------------- Tile Logic ------------------------ */

// Create a better tile assignment system
// Each tile is given a tyle type based on its index and a bunch of helper functions
// And then



bool isGroundTile(int i){
    int pos = i % TILES_PER_SIDE;
    if (abs(pos - TILES_PER_SIDE / 2) < roadWidth){
        return true;
    }
    return false;
}

bool isFlyerTile(int i){
    if (noFlyers) return false;
    int seed = randSeeds[i];
    return seed % 1000 < 12;
}

bool isMoonFlyerTile(int i){
    return moonTileStatuses[i];
}

// Now are these tile positions or 3D locations...
// 3D locations

// Sets true or false
void setupSpireStatuses(){
    for (int j = 0; j < NUM_TILES; j++){
        STVector3 *pos = tilePositions[j];
        STPoint2 *pos_pt = new STPoint2(pos->x, pos->z);

        bool result = false;



        for (int k = 0; k < SPIRE_COUNT; k++){
            // If it's within SPIRE_WIDTH
            STPoint2 spire_pos = STPoint2(SPIRE_POS[k].x, SPIRE_POS[k].y);


            if (STPoint2::Dist( spire_pos, *pos_pt) < SPIRE_WIDTH){
//                std::cout << "Pos "<< j << " true: " << "( " << pos_pt->x << ", " << pos_pt->y << " )" << std::endl;
                result = true;
                break;
            }
        }
        // Otherwise it's not
        //            std::cout << "Pos "<< j << " false: " << pos_pt << std::endl;
        spireTileStatus.push_back(result);
    }

    if (spireTileStatus.size() != tilePositions.size()) {
        std::cout << "mismatch" << std::endl;
        std::cout << "status: " << spireTileStatus.size() << std::endl;
        std::cout << "pos: " << tilePositions.size() << std::endl;
    }

}

bool isSpireTile(int i){
    return spireTileStatus[i];
}

// This is where all the tile assignment logic goes
TileType getTileType(int i){

    if (isGroundTile(i))
        return ROAD_TILE;

    if (isSpireTile(i))
        return SPIRE_TILE;

    if (isMoonFlyerTile(i))
        return MOON_FLYER_TILE;
    if (isFlyerTile(i))
        return FLYER_TILE;

    return BASIC_TILE;
}

// Returns the correct mesh in the gTriangleMeshes array
STTriangleMesh *getTileMesh(TileType type, int i){
    switch (type){
        case BASIC_TILE:
            return gTriangleMeshes[ BLANK_TILE_MESH ];
        case ROAD_TILE:
            return gTriangleMeshes[ ROAD_TILE_MESH ];
        case FLYER_TILE:
            return gTriangleMeshes[ FLYER_TILE_MESH ];
        case MOON_FLYER_TILE:
            if (randSeeds[i]% 2 == 0){
                return gTriangleMeshes[ FLYER_TILE_MESH ];
            } else {
                return gTriangleMeshes[ BLANK_TILE_MESH ];
            }
        case SPIRE_TILE:
            long seed = randSeeds[i];
            TileMeshIndex mesh = SPIRE_1_MESH;
            switch (seed % 4) {
                case 0:
                    mesh = SPIRE_1_MESH;
                    break;
                case 1:
                    mesh = SPIRE_3_MESH;
                    break;
                case 2:
                    mesh = SPIRE_3_MESH;
                    break;
                case 3:
                    mesh = SPIRE_4_MESH;
                    break;
            }

            return gTriangleMeshes[ mesh ];
    }
    return gTriangleMeshes[BLANK_TILE_MESH];
}

/* ---------------------- Setup Code ------------------------ */

// Build a giant cube around everything
void setupSkyBox(){
    skyBox = new STTriangleMesh();
    skyBox->Read(SKYBOX_MESH_STR);
    skyBox->Build();
    skyBox->mDrawAxis = false;

    for (int i = 0; i < skyBox->mNormals.size(); i++){
        auto norm = skyBox->mNormals[i];
        norm->x *=-1;
        norm->y *=-1;
        norm->z *=-1;
    }

//    assignMaterials(skyBox, ma_white, ma_white, ma_white, 2);
        assignMaterials(skyBox, ma_slim, ma_slim, ma_white, 2);

//    auto img = new STImage("images/displacementmap.jpeg");
//    skyBox->mSurfaceColorTex = new STTexture(img);
//    skyBox->mSurfaceColorImg = img;
}

//Build all the tiles required for this
void setupTiles()
{
//    build a grid of points
    int tilesPerSide = TILES_PER_SIDE;
    float dim = ALL_TILE_DIM;
    float tileDim = TILE_DIM;
    float centerOffset = dim / 2.f;
    for (int x = 0; x < tilesPerSide; x++){
        for (int z = 0; z < tilesPerSide; z++){
            float tx = x * tileDim - centerOffset;
            float ty = tileStartY;
            float tz = z * tileDim - centerOffset;
            STVector3 *pos = new STVector3(tx, ty, tz);
            tilePositions.push_back(pos);
        }
    }
    // Load all the meshes
    for (int i = 0; i < TILE_MESH_CT; i++){
        STTriangleMesh::LoadObj(gTriangleMeshes, "meshes/" + TILE_MESH_STRS[i] );
    }

    for (int i = 0; i < gTriangleMeshes.size(); i++){
        gTriangleMeshes[i]->mDrawAxis = false;
    }
    // Setup random seeds
    for (int i = 0; i <  TILES_PER_SIDE * TILES_PER_SIDE; i++){
        randSeeds.push_back( rand() );
    }

}

// Build the center two moons
void setupCenter()
{
    centerMoonMesh = new STTriangleMesh();
    centerMoonMesh->Read(MOON_MESH_STR);
    centerMoonMesh->mDrawAxis = false;
    centerMoonMesh->Build();
    centerMoonMesh->LoopSubdivide();
    auto cmm = centerMoonMesh;
    cmm->mMaterialDiffuse[0] = 1;
    cmm->mMaterialDiffuse[1] = 1;
    cmm->mMaterialDiffuse[2] = 1;
    cmm->mMaterialAmbient[0] = .8;
    cmm->mMaterialAmbient[1] = .8;
    cmm->mMaterialAmbient[2] = .8;
    float *lightPosition0 = SceneLights::getLightPositionf(0);
    float *lightPosition1 = SceneLights::getLightPositionf(1);
    assignVec3f(moon_pos, lightPosition0);
    assignVec3f(moon_pos2, lightPosition1);
    moonPos = new STVector3(moon_pos[0], moon_pos[1], moon_pos[2]);
}

// Setup a nice reference ground
void setupGround()
{

    float leftX   = groundMeshScale;
    float rightX  = -leftX;
    float nearZ   = groundMeshScale;
    float farZ    = -nearZ;

    groundMesh= new STTriangleMesh();
    for (int i = 0; i < TesselationDepth+1; i++){
        for (int j = 0; j < TesselationDepth+1; j++) {
            float s0 = (float) i / (float) TesselationDepth;
            float x0 =  s0 * (rightX - leftX) + leftX;
            float t0 = (float) j / (float) TesselationDepth;
            float z0 = t0 * (farZ - nearZ) + nearZ;

            groundMesh->AddVertex(x0,(x0*x0+z0*z0)*0.0f,z0,s0,t0);
        }
    }
    for (int i = 0; i < TesselationDepth; i++){
        for (int j = 0; j < TesselationDepth; j++) {
            unsigned int id0=i*(TesselationDepth+1)+j;
            unsigned int id1=(i+1)*(TesselationDepth+1)+j;
            unsigned int id2=(i+1)*(TesselationDepth+1)+j+1;
            unsigned int id3=i*(TesselationDepth+1)+j+1;
            groundMesh->AddFace(id0,id2,id1);
            groundMesh->AddFace(id0,id3,id2);
        }
    }
    groundMesh->mSurfaceColorTex = new STTexture( new STImage("images/tron_city.jpg"));
    groundMesh->Build();

    groundMesh->mDrawAxis = false;
    groundMesh->mMaterialAmbient[0]=.3;
    groundMesh->mMaterialAmbient[1]=.3;
    groundMesh->mMaterialAmbient[2]=.3;
    groundMesh->mMaterialDiffuse[0]=0.1f;
    groundMesh->mMaterialDiffuse[1]=0.1f;
    groundMesh->mMaterialDiffuse[2]=0.1f;
    groundMesh->mMaterialSpecular[0]=0.0f;
    groundMesh->mMaterialSpecular[1]=0.0f;
    groundMesh->mMaterialSpecular[2]=0.0f;
    groundMesh->mShininess=8.0f;
}

void positionTiles(){
    setupSpireStatuses();

    for (int i = 0; i <  NUM_TILES; i++){
        auto pos = tilePositions[i];
        pos->y += randRange(-2, 8);
        // Slant
        pos->y -= (ALL_TILE_DIM/8.0 - pos->x) * xDropScale + roadY * .5;
        // If it is near the center, drop it to ground level
        if (isGroundTile(i)){
            pos->y = roadY;
            // staircase
            pos->y += ((pos->x - ALL_TILE_DIM ) * xDropScale/1.0 ) / 4.0;
            //racndom staircase fluc
            //            pos->y -= (float)(randSeeds[i] % 20) / 80.0;
            pos->y += 13;
        } else if( getTileType(i) == MOON_FLYER_TILE){
;
            pos->y = moonPos->y ;
            pos->x /= 4.5;
            pos->x += moonPos->x / 1.2;
            pos->z /= 6;
        }

    }
    // Cache if it is a spireTile or not

}

void repulseTiles(){
    rerepulseTiles = false;
    // FOr tileposition, check if it is within some distnace of the sphere
    // if it is within the repulsion distance, then drop it down the y axis
    STPoint3 *moonPoint = new STPoint3(*moonPos);
    for (int i = 0; i < tilePositions.size(); i++){
        STVector3 *pos = tilePositions[i];
        STPoint3 *pos_pt = new STPoint3(*pos);

        float lengthToMoon = STPoint3::Dist(*moonPoint, *pos_pt);
        float repulsion = moon_repulsion - lengthToMoon;

        if (repulsion > 0){
            if (randSeeds[i] % 1000 < 40){
                moonTileStatuses.push_back(true);
            } else {
                pos->y -= repulsion * moon_repulsion_factor;
                moonTileStatuses.push_back(false);
            }

        } else {
            moonTileStatuses.push_back(false);
//            pos->y = tileStartY;
        }
    }
}

void setupScene()
{
    setupSkyBox();
    setupTiles();
    setupGround();
    setupCenter();


    repulseTiles();
    positionTiles();
}


//
// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
//
void Setup()
{

    srand(SOME_SEED);

    // Set up lighting variables in OpenGL
    // Once we do this, we will be able to access them as built-in
    // attributes in the shader (see examples of this in normalmap.frag)

    SceneLights::setupLightData();
    SceneLights::setupSpireLights(SPIRE_POS, SPIRE_Y, SPIRE_COUNT);
    SceneLights::setupLights();

    shader = new STShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);

    resetCamera();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    setupScene();

    if (glow){

        FBOGlow = prepareFBO(initialWindowX, initialWindowY, 1);
        FBOHorizBlur = prepareFBO(initialWindowX, initialWindowY, 1);
        FBOVertBlur = prepareFBO(initialWindowX, initialWindowY, 1);

//        printFramebufferInfo(GL_DRAW_FRAMEBUFFER, FBOGlow);
//        printFramebufferInfo(GL_DRAW_FRAMEBUFFER, FBOHorizBlur);

        if (FBOGlow == 0){
            printf("ERROR");
        }
//        glDrawBuffer(GL_NONE);
//        glReadBuffer(GL_NONE);

    }
}


// Draw center sphere
void drawCenter(){
//    return ;
    glPushMatrix();
    glTranslatef(moonPos->x, moonPos->y, moonPos->z);
    // Should be unit cube meshes
    glScalef(MOON_SCALE, MOON_SCALE, MOON_SCALE);
    centerMoonMesh->Draw(smooth);
    glPopMatrix();

//    glPushMatrix();
//    glTranslatef(moon_pos2[0], moon_pos2[1], moon_pos2[2]);
//    // Should be unit cube meshes
//    glScalef(MOON_SCALE_2, MOON_SCALE_2, MOON_SCALE_2);
//    centerMoonMesh->Draw(smooth);
//    glPopMatrix();
}

// Draws the tile into the scene.
void drawTileByType(TileType type, int i){
    auto pos = tilePositions[i];
    float randScale, randAngle, randX, randZ;
    bool tileSmooth = smooth;
    STTriangleMesh *mesh;
    mesh = getTileMesh(type, i);
    long seed = randSeeds[i];


    glPushMatrix();
    switch (type){
        case BASIC_TILE:
            randScale = randRangeFromSeed(.8, 3, randSeeds[i]) * (TILE_DIM - TILE_SPACING);
            glTranslatef(pos->x, pos->y, pos->z);
            glScalef( randScale, TILE_Y_SCALE, randScale);
            break;
        case ROAD_TILE:
//            glScalef( TILE_Y_SCALE / 2.0, 1, TILE_Y_SCALE / 10.0 );
            glScalef( 1, 1, 1 );
            glTranslatef(pos->x, pos->y, pos->z);
            break;
        case SPIRE_TILE:
//            glScalef( TILE_Y_SCALE, 1, TILE_Y_SCALE);
            glTranslatef(pos->x, 0, pos->z);


//            glScalef(-1, 1, 1);
//            randZ = randRangeFromSeed(2, 5.2, seed);
            randX = randRangeFromSeed(.8, 2.2, seed);
            glScalef( randX, 3, 2);

            if (pos->x <= 0) {
                randAngle = -30;
            } else {
                randAngle = 30;
            }
//            randAngle = randRangeFromSeed(0,90, seed);
            glRotatef(randAngle, 0, 1, 0);
            break;

        case FLYER_TILE:
            glTranslatef(pos->x, pos->y + randRangeFromSeed(20, 40, randSeeds[i]), pos->z);
            randScale = randRangeFromSeed(.8, 3, randSeeds[i]) * (TILE_DIM - TILE_SPACING);
            glScalef( randScale /1.4, randScale /1.4, randScale /1.4);
            break;
        case MOON_FLYER_TILE:
//            std::cout << pos->y << std::endl;
            glTranslatef(pos->x , 0 , pos->z);
            randScale = randRangeFromSeed(.8, 2, randSeeds[i]) * (TILE_DIM - TILE_SPACING) / 7.0 ;
//            randScale = 1.0/7.0;
            glScalef( randScale , 1 , randScale );
            glTranslatef(0, pos->y , 0);
            glScalef( 1 , randScale , 1 );
            break;

    }


    mesh->Draw(tileSmooth);
    glPopMatrix();
}

// Draw all the tiles at their given positions
void drawTiles(){
    for (int i = 0; i < tilePositions.size(); i++){
        TileType tileType = getTileType(i);
        drawTileByType(tileType, i);
    }
}

void drawSkyBox(){
    if (DRAW_SKYBOX){
        glPushMatrix();
        glScalef( skyBoxScale, skyBoxScale, skyBoxScale);
        skyBox->Draw(true);
        glPopMatrix();
    }
}

void drawGround(){
    if (DRAW_GROUND){
        glPushMatrix();
        glTranslatef(groundZ, 0, 0);
        float scaleFactor = 1.4;
        glScalef(scaleFactor, scaleFactor, scaleFactor);
        glRotatef(90, 0, 0, 1);
        glRotatef(90, 0, -1, 0);
        glRotatef(-20, 0, 0, -1);
        groundMesh->Draw(smooth);
        glPopMatrix();
    }
}

void drawGlowMapDummy(int texId){
    if (glow && glowMapDrawn){
        auto mesh = gTriangleMeshes[DUMMY_MESH];
        mesh->mSurfaceColorTex->mTexId = texFBO[texId];

        shader->SetUniform("fullScreenQuad", 1.0);

//        float scaleFactor = 20;
//        glScalef(scaleFactor , scaleFactor, scaleFactor* 16.0/9.0);
//        glRotatef(90, 0, 0, 1);
//        glRotatef(90, 0, -1, 0);
        mesh->Draw(false);
        shader->SetUniform("fullScreenQuad", -1.0);
        glPopMatrix();
    }
}

void drawScene()
{


    shader->SetUniform("colorMapping", 1.0);

    drawGround();
    drawTiles();

    shader->SetUniform("normalFlipping", 1.0);
    drawCenter();
    shader->SetUniform("normalFlipping", -1.0);

}

void drawGlowmap(){
    // Draw only the road tiles for now...
    //    for (int i = 0; i < tilePositions.size(); i++){
    //        TileType tileType = getTileType(i);
    //        switch (tileType) {
    //            case ROAD_TILE:
    //            case SPIRE_TILE:
    //                drawTileByType(tileType, i);
    //                break;
    //            default:
    //                break;
    //        }
    //    }

//    shader->SetUniform("glowMapping", 1.0);
    shader->SetUniform("colorMapping", 1.0);

//    drawGround();
    drawTiles();

    shader->SetUniform("normalFlipping", 1.0);
    drawCenter();
    shader->SetUniform("normalFlipping", -1.0);

    shader->SetUniform("glowMapping", -1.0);
    glowMapDrawn = true;
}

// Display the output image from our vertex and fragment shaders
//
void DisplayCallback()
{


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetUpAndRight();

    gluLookAt(mPosition.x,mPosition.y,mPosition.z,
              mLookAt.x,mLookAt.y,mLookAt.z,
              mUp.x,mUp.y,mUp.z);

    SceneLights::positionLights();


    // Bind the textures we've loaded into openGl to
    // the variable names we specify in the fragment
    // shader.
    shader->SetTexture("colorTex", 10);

    // Invoke the shader.  Now OpenGL will call our
    // shader programs on anything we draw.
    shader->Bind();

    // For the vertex shader...
    shader->SetUniform("fullScreenQuad", -1.0);
    shader->SetUniform("glowMapping", -1.0);
    shader->SetUniform("colorMapping", 1.0);
    shader->SetUniform("blur", -1.0);
    shader->SetUniform("blurV", -1.0);
    shader->SetUniform("blend", -1.0);


    if(!glow){
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Draw scene solid

        drawScene();

    } else {

        // draw scene first


//        STImage *img = new STImage(initialWindowX, initialWindowY);

        // draw glowmap to FBP
        if (!glowMapDrawn){
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOGlow);

//            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set Drawing buffers
            GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1,  attachments);

            //needs to be only the lights
            drawGlowmap();
            glowMapDrawn = true;

            // Draw vertical blur
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOHorizBlur);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->SetUniform("blur", 1.0);
            shader->SetUniform("blurV", 1.0);

            drawGlowMapDummy(fboGlowInd);

            // Draw horizontal blur
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOVertBlur);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->SetUniform("blurV", -1.0);
            drawGlowMapDummy(fboHorizInd);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


        }

        // drawglowmap onto quad
//        glClearColor(0,0,0,0);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        drawGlowmap();

//        drawGlowMapDummy(fboHorizInd);
//        drawGlowMapDummy(fboGlowInd);

        // Draw the actual scene
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        drawScene();

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        // needs to be drawn with alpha
        shader->SetUniform("blend", 1.0);
        drawGlowMapDummy(fboVertInd);
        glDisable(GL_BLEND);

    }
    
    
    shader->UnBind();
    
    
    glutSwapBuffers();
}


void CleanUp()
{
    for(unsigned int id=0;id<gTriangleMeshes.size();id++)delete gTriangleMeshes[id];
    if(gManualTriangleMesh!=0)
        delete gManualTriangleMesh;
}

/**
 * Camera adjustment methods
 */
void RotateCamera(float delta_x, float delta_y)
{
    float yaw_rate=1.f;
    float pitch_rate=1.f;

    mPosition -= mLookAt;
    STMatrix4 m;
    m.EncodeR(-1*delta_x*yaw_rate, mUp);
    mPosition = m * mPosition;
    m.EncodeR(-1*delta_y*pitch_rate, mRight);
    mPosition = m * mPosition;

    mPosition += mLookAt;
}

void ZoomCamera(float delta_y)
{
    STVector3 direction = mLookAt - mPosition;
    float magnitude = direction.Length();
    direction.Normalize();
    float zoom_rate = 0.1f*magnitude < 0.5f ? .1f*magnitude : .5f;
    if(delta_y * zoom_rate + magnitude > 0)
    {
        mPosition += (delta_y * zoom_rate) * direction;
    }
}

void StrafeCamera(float delta_x, float delta_y)
{
    float strafe_rate = 0.05f;

    mPosition -= strafe_rate * delta_x * mRight;
    mLookAt   -= strafe_rate * delta_x * mRight;
    mPosition += strafe_rate * delta_y * mUp;
    mLookAt   += strafe_rate * delta_y * mUp;
}

//


//
// Reshape the window and record the size so
// that we can use it for screenshots.
//
void ReshapeCallback(int w, int h)
{
    gWindowSizeX = w;
    gWindowSizeY = h;

    glViewport(0, 0, gWindowSizeX, gWindowSizeY);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    // Set up a perspective projection
    float aspectRatio = (float) gWindowSizeX / (float) gWindowSizeY;
    gluPerspective(30.0f, aspectRatio, .1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SpecialKeyCallback(int key, int x, int y)
{
    switch(key) {
        case GLUT_KEY_LEFT:
            StrafeCamera(10,0);
            break;
        case GLUT_KEY_RIGHT:
            StrafeCamera(-10,0);
            break;
        case GLUT_KEY_DOWN:
            StrafeCamera(0,-10);
//            moonPos->y -= moonNudge;
//            rerepulseTiles = true;
            break;
        case GLUT_KEY_UP:

            StrafeCamera(0,10);
//            moonPos->y += moonNudge;
//            rerepulseTiles = true;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void KeyCallback(unsigned char key, int x, int y)
{
    switch(key) {
        case 's': {
            //
            // Take a screenshot, and save as screenshot.jpg
            //
            STImage* screenshot = new STImage(gWindowSizeX, gWindowSizeY);
            screenshot->Read(0,0);
            screenshot->Save("screenshot.jpg");
            delete screenshot;
        }
            break;
        case 'r':
            resetCamera();
            break;
        case 'u':
            resetUp();
            break;
        case 'm': // switch between the mesh you create and the mesh from file
            mesh = !mesh;
            break;
            //case 'p': //switch proxy type between sphere and cylinder
            //	proxyType=!proxyType;
            //	if(proxyType) gTriangleMesh->CalculateTextureCoordinatesViaSphericalProxy();
            //	else gTriangleMesh->CalculateTextureCoordinatesViaCylindricalProxy(-1,1,0,0,1);
            //	break;
        case 'n': // switch between normalMapping and displacementMapping
            normalMapping = !normalMapping;
            break;
        case 'f': // switch between smooth shading and flat shading
            smooth = !smooth;
            break;
            //case 'l': // do loop subdivision
            //    if(mesh){
            //        gTriangleMesh->LoopSubdivide();
            //		if(proxyType) gTriangleMesh->CalculateTextureCoordinatesViaSphericalProxy();
            //		else gTriangleMesh->CalculateTextureCoordinatesViaCylindricalProxy(-1,1,0,0,1);
            //    }
            //    else
            //        gManualTriangleMesh->LoopSubdivide();
            //    break;
            //case 'w':
            //    gTriangleMesh->Write("output.obj");
            //    break;
        case 'a':
//            for(unsigned int id=0;id<gTriangleMeshes.size();id++)
//                gTriangleMeshes[id]->mDrawAxis=!gTriangleMeshes[id]->mDrawAxis;
//            if(gManualTriangleMesh!=0)
//                gManualTriangleMesh->mDrawAxis=!gManualTriangleMesh->mDrawAxis;
            break;

        case 'l':
            printVec(&mLookAt);
            printVec(&mPosition);
            printVec(&mUp);
            break;
        case 'q':
            exit(0);
        default:
            break;
    }

    glutPostRedisplay();
}

/**
 * Mouse event handler
 */
void MouseCallback(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON)
    {
        gMouseButton = button;
    } else
    {
        gMouseButton = -1;
    }

    if (state == GLUT_UP)
    {
        gPreviousMouseX = -1;
        gPreviousMouseY = -1;
    }
}

/**
 * Mouse active motion callback (when button is pressed)
 */
void MouseMotionCallback(int x, int y)
{
    if (gPreviousMouseX >= 0 && gPreviousMouseY >= 0)
    {
        //compute delta
        float deltaX = x-gPreviousMouseX;
        float deltaY = y-gPreviousMouseY;
        gPreviousMouseX = x;
        gPreviousMouseY = y;

        //orbit, strafe, or zoom
        if (gMouseButton == GLUT_LEFT_BUTTON)
        {
            RotateCamera(deltaX, deltaY);
        }
        else if (gMouseButton == GLUT_MIDDLE_BUTTON)
        {
            StrafeCamera(deltaX, deltaY);
        }
        else if (gMouseButton == GLUT_RIGHT_BUTTON)
        {
            ZoomCamera(deltaY);
        }

    } else
    {
        gPreviousMouseX = x;
        gPreviousMouseY = y;
    }

}


GLuint createRGBATexture(int w, int h) {

    GLuint tex2;

    glGenTextures(1, &tex2);

    glBindTexture(GL_TEXTURE_2D, tex2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 w,
                 h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 0);

    glBindTexture(GL_TEXTURE_2D, 0);

//    int max[1] = {0};
//    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, max);
//    std::cout << max[0]<< std::endl;

    return(tex2);
}

GLuint createDepthTexture(int w, int h) {

    GLuint tex;

    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
//    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 w,
                 h, 
                 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    return(tex);
}

void printFramebufferInfo(GLenum target, GLuint fbo) {

    glBindFramebuffer(target,fbo);
    int res;

    GLint buffer;
    int i = 0;
    do {
        glGetIntegerv(GL_DRAW_BUFFER0+i, &buffer);

        if (buffer != GL_NONE) {

            printf("Shader Output Location %d - color attachment %d\n", i, buffer - GL_COLOR_ATTACHMENT0);

            glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
            printf("\tAttachment Type: %s\n", res==GL_TEXTURE?"Texture":"Render Buffer");
            glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
            printf("\tAttachment object name: %d\n",res);
        }
        ++i;
        
    } while (buffer != GL_NONE);
}

GLuint prepareFBO(int w, int h, int colorCount) {

    GLuint fbo;
    // Generate one frame buffer
    glGenFramebuffers(1, &fbo);
    // bind it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    // attach textures for colors
    texFBO[fboTextureCount] = createRGBATexture(w,h);
    glFramebufferTextureEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texFBO[fboTextureCount], 0);
    fboTextureCount++;

    GLuint depthFBO;
    depthFBO = createDepthTexture(w,h);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthFBO, 0);

    // check if everything is OK
    GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (e) {

        case GL_FRAMEBUFFER_UNDEFINED:
            printf("FBO Undefined\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
            printf("FBO Incomplete Attachment\n");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
            printf("FBO Incomplete Draw Buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
            printf("FBO Missing Attachment\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED :
            printf("FBO Unsupported\n");
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            printf("FBO OK\n");
            break;
        default:
            printf("FBO Problem?\n");
    }


    if (e != GL_FRAMEBUFFER_COMPLETE)
        return (0);
    // unbind fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    //	printFramebuffersInfo(fbo);
    
    return fbo;
}

void usage()
{
    printf("usage: assignment3 vertShader fragShader objMeshFile normalMappingTexture displacementMappingTexture\n");
}

int main(int argc, char** argv)
{
    if (argc != 6)
        usage();

    vertexShader   = argc>1?std::string(argv[1]):std::string("kernels/default.vert");
    fragmentShader = argc>2?std::string(argv[2]):std::string("kernels/phong.frag");
    meshOBJ        = argc>3?std::string(argv[3]):std::string("scene.obj");
    normalMap      = argc>4?std::string(argv[4]):std::string("images/tron_city.jpg");
    displacementMap= argc>5?std::string(argv[5]):std::string("images/displacementmap.jpeg");

    //
    // Initialize GLUT.
    //
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(initialWindowX, initialWindowY);
    glutCreateWindow("CS148 Assignment 3");

    //
    // Initialize GLEW.
    //
#ifndef __APPLE__
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
               "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif
    
    // Be sure to initialize GLUT (and GLEW for this assignment) before
    // initializing your application.
    
    Setup();
    
    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutSpecialFunc(SpecialKeyCallback);
    glutKeyboardFunc(KeyCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MouseMotionCallback);
    glutIdleFunc(DisplayCallback);
    
    glutMainLoop();
    
    // Cleanup code should be called here.
    CleanUp();
    
    return 0;
}
