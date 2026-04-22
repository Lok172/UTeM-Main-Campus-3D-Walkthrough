#define STB_IMAGE_IMPLEMENTATION 
#define MINIAUDIO_IMPLEMENTATION
#include <windows.h> 
#include <GL/glut.h> 
#include <vector> 
#include <string> 
#include <fstream> 
#include <sstream> 
#include <iostream> 
#include <cmath> 
#include "stb_image.h" 
#include "miniaudio.h"

const float PI = 3.14;
float moveSpeed = 0.01f;
float Yaw = 0.0f;
float rotSpeed = 0.3f;

bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;
bool rotateLeft = false;
bool rotateRight = false;

float PosX = 0.0f;
float PosY = 1.0f;
float PosZ = 0.0f;

bool showDebug = false;

ma_engine gAudioEngine;

// carpark2 moving cars 
struct MovingCar {
	int parts[4]; // car_grey, car_yellow, car_white, car_black 
	int state = 0; // movement state 
}; 
MovingCar carA;
MovingCar carB;
const float CAR_LOOP_SPEED_A = 0.05f;
const float CAR_LOOP_SPEED_B = 0.003f;

static const char* gObj[] = {
	"Models/lobby_grey.obj",
	"Models/lobby_yellow.obj",
	"Models/lobby_palegrey.obj",
	"Models/lobby_black.obj",
	"Models/leftwing_brown.obj",
	"Models/carpark1WHITE.obj",
	"Models/carpark1GREY.obj",
	"Models/carpark1GREEN.obj",
	"Models/carpark1BLACK.obj",
	"Models/roundabout_grey.obj",
	"Models/roundabout_dirt.obj",
	"Models/roundabout_yellow.obj",
	"Models/roundabout_bush.obj",
	"Models/roundabout_oak.obj",
	"Models/roundabout_oakLeaf.obj",
	"Models/carpark2_white.obj",
	"Models/carpark2_black.obj",
	"Models/leftwing_palegrey.obj",
	"Models/leftwing_grey.obj",
	"Models/leftwing_yellow.obj",
	"Models/rightwing_grey.obj",
	"Models/rightwing_yellow.obj",
};

static const char* gRepeatObj[] = {
	"Models/RepeatObj/road.obj",
	"Models/RepeatObj/arrow_white.obj",
	"Models/RepeatObj/streetlamp_grey.obj",
	"Models/RepeatObj/streetlamp_yellow.obj",
	"Models/RepeatObj/tree_oak.obj",
	"Models/RepeatObj/tree_oakLeaf.obj",
	"Models/RepeatObj/car_grey.obj",
	"Models/RepeatObj/car_yellow.obj",
	"Models/RepeatObj/car_white.obj",
	"Models/RepeatObj/car_black.obj",
};

static const char* gTextures[] = {
"Texture/grey.png",
"Texture/yellow.jpg",
"Texture/pale grey.jpg",
"Texture/black.png",
"Texture/brown.jpg",
"Texture/bush.png",
"Texture/white.jpg",
"Texture/grass.jpg",
"Texture/DIRT.jpg",
"Texture/Oak_leaf.jpg",
"Texture/Oak.jpg",
"Texture/road.png"
};

struct Vec3 {
	float x, y, z;
};

struct TriangleIndex {
	int a, b, c;
};

struct QuadIndex {
	int a, b, c, d;  // 4 vertex indices
};

struct Model {
	std::vector<Vec3> vertices;
	std::vector<TriangleIndex> triangles;
	std::vector<QuadIndex> gQuads;
	float scaleX = 0.05f;
	float scaleY = 0.05f;
	float scaleZ = 0.05f;

	float posX = 0.0f;
	float posY = 0.0f;
	float posZ = 0.0f;

	float rotX = 0.0f;
	float rotY = 0.0f;
	float rotZ = 0.0f;

	int tex = 0;
	bool repeat = false;
	bool forceTall = false; // Flag to make small objects act like tall walls
};

struct repeatModel {
	int modelID; // Model ID
	int ite;      // Iterations
	int Tex;      // Texture ID

	// Start Position
	float startX, startY, startZ;

	// Move/Translation
	float moveX, moveY, moveZ;

	// Rotation
	float rotX, rotY, rotZ;

	// Scale
	float scaleX, scaleY, scaleZ;
};

//modelId, Number of repeat, texture index, startX, startY,startZ, moveX, moveY, moveZ?rotateX, rotateY, rotateZ, scaleX, scaleY, scaleZ

struct AABB {
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;
};

// List of all colliders in the world (buildings, trees.....)
std::vector<AABB> gWorldColliders;
std::vector<AABB> gFloors;

const float PLAYER_WIDTH = 0.5f; // Width of the player box
const float PLAYER_HEIGHT = 0.5f; // Height of the player box

bool CheckCollision(const AABB& b1, const AABB& b2) {
	return (b1.minX <= b2.maxX && b1.maxX >= b2.minX) &&
		(b1.minY <= b2.maxY && b1.maxY >= b2.minY) &&
		(b1.minZ <= b2.maxZ && b1.maxZ >= b2.minZ);
}

std::vector <repeatModel> para = {
	//road
	{0, 4, 11,   -13.5f, 0.0f, 15.0f,   3.0f, 0.0f, 0.0f,    0.0f, 90.0f, 0.0f,    0.5f, 0.1f, 0.5f},
	{0, 1, 11,   -3.0f, 0.01f, 14.0f,   0.0f, 0.0f, 0.0f,    0.0f, 140.0f, 0.0f,   0.5f, 0.1f, 0.5f},
	{0, 1, 11,   -1.2f, 0.1f, 11.85f,   0.0f, 0.0f, 0.0f,    5.0f, 140.0f, -2.5f,  0.5f, 0.1f, 0.5f},
	{0, 5, 11,    0.6f, 0.2f, 9.7f,     1.8f, 0.0f, -2.15f,  0.0f, 140.0f, 0.0f,   0.5f, 0.1f, 0.5f},
	{0, 1, 11,    9.6f, 0.1f, -1.05f,   0.0f, 0.0f, 0.0f,    -5.0f, 140.0f, 2.5f,  0.5f, 0.1f, 0.5f},
	{0, 5, 11,   10.5, 0.0f, -13.5f,    0.0f, 0.0f, 3.0f,    0.0f, 0.0f, 0.0f,     0.5f, 0.1f, 0.5f},
	//arrow
	{1, 1, 6,    -8.0f, 0.0f, 4.0f,     0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.03f, 0.1f, 0.03f},
	{1, 1, 6,    -8.5f, 0.0f, -5.5f,    0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.03f, 0.1f, 0.03f},
	{1, 1, 6,    -5.5f, 0.0f, -8.5f,    0.0f, 0.0f, 0.0f,    0.0f, -90.0f, 0.0f,   0.03f, 0.1f, 0.03f},
	{1, 1, 6,    -2.5f, 0.0f, -5.5f,    0.0f, 0.0f, 0.0f,    0.0f, 180.0f, 0.0f,   0.03f, 0.1f, 0.03f},
	{1, 1, 6,    3.5f, 0.0f, -8.0f,     0.0f, 0.0f, 0.0f,    0.0f, -90.0f, 0.0f,   0.03f, 0.1f, 0.03f},
	//streetlamp
	{2, 5, 0,    -4.55f, 0.0f, 10.0f,   0.3f, 0.0f, -3.0f,   0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{3, 5, 1,    -4.55f, 0.0f, 10.0f,   0.3f, 0.0f, -3.0f,   0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{2, 6, 0,    -8.35f, 0.0f, 12.5f,   0.1f, 0.0f, -3.0f,   0.0f, -90.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	{3, 6, 1,    -8.35f, 0.0f, 12.5f,   0.1f, 0.0f, -3.0f,   0.0f, -90.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	{2, 5, 0,    -4.55f, 0.0f, -7.5f,   3.0f, 0.0f, -0.1f,   0.0f, 180.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	{3, 5, 1,    -4.55f, 0.0f, -7.5f,   3.0f, 0.0f, -0.1f,   0.0f, 180.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	{2, 4, 0,    -1.55f, 0.0f, -3.3f,   3.0f, 0.0f, -0.5f,   0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{3, 4, 1,    -1.55f, 0.0f, -3.3f,   3.0f, 0.0f, -0.5f,   0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{2, 9, 0,    -14.55f, 0.0f, 12.0f,  0.0f, 0.0f, -3.0f,   0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{3, 9, 1,    -14.55f, 0.0f, 12.0f,  0.0f, 0.0f, -3.0f,   0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{2, 8, 0,    -12.0f, 0.0f, -14.5f,  3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{3, 8, 1,    -12.0f, 0.0f, -14.5f,  3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{2, 6, 0,    11.75f, 0.0f, -14.5f,  0.0f, 0.0f, 2.8f,    0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{3, 6, 1,    11.75f, 0.0f, -14.5f,  0.0f, 0.0f, 2.8f,    0.0f, 90.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{2, 5, 0,    -14.55f, 0.0f, 16.2f,  2.7f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{3, 5, 1,    -14.55f, 0.0f, 16.2f,  2.7f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{2, 6, 0,    -1.55f, 0.0f, 14.5f,   2.7f, 0.0f, -3.3f,   0.0f, 45.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{3, 6, 1,    -1.55f, 0.0f, 14.5f,   2.7f, 0.0f, -3.3f,   0.0f, 45.0f, 0.0f,    0.05f, 0.09f, 0.05f},
	{2, 2, 0,    13.55f, 0.0f, -0.1f,   3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{3, 2, 1,    13.55f, 0.0f, -0.1f,   3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.05f, 0.09f, 0.05f},
	{2, 2, 0,    13.55f, 0.0f, -9.9f,   3.0f, 0.0f, 0.0f,    0.0f, 180.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	{3, 2, 1,    13.55f, 0.0f, -9.9f,   3.0f, 0.0f, 0.0f,    0.0f, 100.0f, 0.0f,   0.05f, 0.09f, 0.05f},
	//tree
	{4, 1, 10,   -7.85f, 0.0f, 11.5f,   0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 1, 9,    -7.85f, 0.0f, 11.5f,   0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 1, 10,   -3.55f, 0.0f, 10.5f,   0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 1, 9,    -3.55f, 0.0f, 10.5f,   0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 5, 10,   -14.55f, 0.0f, 17.2f,  2.7f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 5, 9,    -14.55f, 0.0f, 17.2f,  2.7f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 5, 10,   -1.85f, 0.0f, 15.5f,   2.7f, 0.0f, -3.3f,   0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 5, 9,    -1.85f, 0.0f, 15.5f,   2.7f, 0.0f, -3.3f,   0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 10, 10,  -14.55f, 0.0f, 13.0f,  0.0f, 0.0f, -3.0f,   0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 10, 9,   -14.55f, 0.0f, 13.0f,  0.0f, 0.0f, -3.0f,   0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 8, 10,   -14.0f, 0.0f, -14.5f,  3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 8, 9,    -14.0f, 0.0f, -14.5f,  3.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 3, 10,   -13.0f, 0.0f, -13.5f,  1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 3, 9,    -13.0f, 0.0f, -13.5f,  1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{4, 3, 10,   -13.55f, 0.0f, -13.0f, 1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	{5, 3, 9,    -13.55f, 0.0f, -13.0f, 1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 0.0f,     0.1f, 0.1f, 0.1f},
	//car
	{6, 1, 0,    -2.3f, 0.3f, 8.5f,     0.0f, 0.0f, 0.0f,    0.0f, 45.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{7, 1, 1,    -2.3f, 0.3f, 8.5f,     0.0f, 0.0f, 0.0f,    0.0f, 45.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{8, 1, 6,    -2.3f, 0.3f, 8.5f,     0.0f, 0.0f, 0.0f,    0.0f, 45.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{9, 1, 3,    -2.3f, 0.3f, 8.5f,     0.0f, 0.0f, 0.0f,    0.0f, 45.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{6, 6, 0,    14.0f, 0.0f, -8.6f,    0.0f, 0.0f, 1.25f,   0.0f, 90.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{7, 6, 1,    14.0f, 0.0f, -8.6f,    0.0f, 0.0f, 1.25f,   0.0f, 90.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{8, 6, 6,    14.0f, 0.0f, -8.6f,    0.0f, 0.0f, 1.25f,   0.0f, 90.0f, 0.0f,    0.2f, 0.2f, 0.2f},
	{9, 6, 3,    14.0f, 0.0f, -8.6f,    0.0f, 0.0f, 1.25f,   0.0f, 90.0f, 0.0f,    0.2f, 0.2f, 0.2f},
};

static const int NUM_TEXTURES = std::size(gTextures);
static GLuint gTex[NUM_TEXTURES] = {};

std::vector<Model> gModels;

float gAngleX = 0.0f;
float gAngleY = 0.0f;
float gZoom = 15.0f;
static float gDist = 5.0f;
static bool gDrag = false;
static int gLastX = 0, gLastY = 0;
int winW = 800, winH = 600;

// Load single texture
GLuint LoadTextureJPEG(const char* path)
{
	int w = 0, h = 0, n = 0;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(path, &w, &h, &n, 3);
	if (!data) {
		return 0;
	}
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
		GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	stbi_image_free(data);
	return tex;
}

// Load all texture by loop the LoadTextureJPEG
bool LoadAllTextures()
{
	bool ok = true;
	for (int i = 0; i < NUM_TEXTURES; ++i) {
		if (gTex[i]) {
			glDeleteTextures(1, &gTex[i]);
			gTex[i] = 0;
		}
		gTex[i] = LoadTextureJPEG(gTextures[i]);
		if (!gTex[i]) {
			printf("Failed to load texture %d: %s %s\n\n\n", i, gTextures[i], stbi_failure_reason());
			ok = false;
		}
	}
	return ok;
}


//Normal Calculation 
Vec3 ComputeFaceNormal(const Vec3& a, const Vec3& b, const Vec3& c)
{
	Vec3 u{ b.x - a.x, b.y - a.y, b.z - a.z };
	Vec3 v{ c.x - a.x, c.y - a.y, c.z - a.z };

	Vec3 n{
	u.y * v.z - u.z * v.y,
	u.z * v.x - u.x * v.z,
	u.x * v.y - u.y * v.x
	};

	float len = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	if (len > 0.00001f) {
		n.x /= len;
		n.y /= len;
		n.z /= len;
	}
	return n;
}

// OBJ Loader
bool LoadOBJ(const char* path, Model& model)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "\n Cannot open OBJ: " << path << "\n";
		MessageBoxA(NULL, "One or more obj file(s) failed to upload ", "Warning", MB_OK | MB_ICONWARNING);
		return false;
	}

	model.vertices.clear();
	model.triangles.clear();

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;
		std::stringstream ss(line);
		std::string type;
		ss >> type;

		//Load vertex
		if (type == "v") {
			Vec3 v;
			ss >> v.x >> v.y >> v.z;
			model.vertices.push_back(v);
		}

		//Load face
		else if (type == "f") {
			std::vector<int> faceIdx;
			std::string token;
			while (ss >> token) {
				std::string indexStr;
				size_t slashPos = token.find('/');
				if (slashPos == std::string::npos) {
					indexStr = token;
				}
				else {
					indexStr = token.substr(0, slashPos);
				}

				if (!indexStr.empty()) {
					int idx = std::stoi(indexStr);
					faceIdx.push_back(idx - 1);
				}
			}

			if (faceIdx.size() >= 3) {
				for (size_t i = 1; i + 1 < faceIdx.size(); ++i) {
					TriangleIndex tri;
					tri.a = faceIdx[0];
					tri.b = faceIdx[i];
					tri.c = faceIdx[i + 1];
					model.triangles.push_back(tri);
				}
			}

			if (faceIdx.size() == 4) {
				QuadIndex q;
				q.a = faceIdx[0];
				q.b = faceIdx[1];
				q.c = faceIdx[2];
				q.d = faceIdx[3];
				model.gQuads.push_back(q);
			}
		}
	}

	return !model.vertices.empty() && !model.triangles.empty();
}

void duplicate(const char* path, int iterations, int repeatTex,
	float sX, float sY, float sZ,    // Start Pos
	float mX, float mY, float mZ,    // Move Step
	float rX, float rY, float rZ,    // Rotation
	float scX, float scY, float scZ) // Scale
{
	float x = sX;
	float y = sY;
	float z = sZ;

	for (int i = 0; i < iterations; i++)
	{
		Model m;
		if (LoadOBJ(path, m))
		{
			if (std::string(path).find("car_") != std::string::npos) {
				m.forceTall = true;
			}

			m.posX = x;
			m.posY = y;
			m.posZ = z;

			m.rotY = rY;
			m.rotX = rX;
			m.rotZ = rZ;

			m.scaleX = scX;
			m.scaleY = scY;
			m.scaleZ = scZ;

			m.tex = repeatTex;
			m.repeat = true;
			gModels.push_back(std::move(m));

			x += mX;
			y += mY;
			z += mZ;
		}

	}
	std::cout << path << " loaded for " << iterations << " times \n";
}

void DrawWireframeBox(const AABB& box) {
	glBegin(GL_LINES);
	// Bottom Face
	glVertex3f(box.minX, box.minY, box.minZ); glVertex3f(box.maxX, box.minY, box.minZ);
	glVertex3f(box.maxX, box.minY, box.minZ); glVertex3f(box.maxX, box.minY, box.maxZ);
	glVertex3f(box.maxX, box.minY, box.maxZ); glVertex3f(box.minX, box.minY, box.maxZ);
	glVertex3f(box.minX, box.minY, box.maxZ); glVertex3f(box.minX, box.minY, box.minZ);

	// Top Face
	glVertex3f(box.minX, box.maxY, box.minZ); glVertex3f(box.maxX, box.maxY, box.minZ);
	glVertex3f(box.maxX, box.maxY, box.minZ); glVertex3f(box.maxX, box.maxY, box.maxZ);
	glVertex3f(box.maxX, box.maxY, box.maxZ); glVertex3f(box.minX, box.maxY, box.maxZ);
	glVertex3f(box.minX, box.maxY, box.maxZ); glVertex3f(box.minX, box.maxY, box.minZ);

	// Vertical Pillars
	glVertex3f(box.minX, box.minY, box.minZ); glVertex3f(box.minX, box.maxY, box.minZ);
	glVertex3f(box.maxX, box.minY, box.minZ); glVertex3f(box.maxX, box.maxY, box.minZ);
	glVertex3f(box.maxX, box.minY, box.maxZ); glVertex3f(box.maxX, box.maxY, box.maxZ);
	glVertex3f(box.minX, box.minY, box.maxZ); glVertex3f(box.minX, box.maxY, box.maxZ);
	glEnd();
}

// car headlight
void DrawHeadlightBeam(float carX, float carY, float carZ, float carRotY) 
{
	// Save current state
	glPushMatrix(); 
	glTranslatef(carX, carY + 0.15f, carZ); // Slightly above ground, centered on car 
	glRotatef(carRotY, 0.0f, 1.0f, 0.0f); // Align beam with car direction 
	
	// Beam
	const float beamLength = 3.0f;
	const float beamRadius = 0.6f;
	const int segments = 16; // Additive blending for glow effect
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive 
	glDepthMask(GL_FALSE); // Don't write to depth buffer 
	glDisable(GL_LIGHTING); // Pure color 
	
	// Bright white with transparency
	glColor4f(1.0f, 1.0f, 0.9f, 0.4f); // Slight warm tint 
	
	glBegin(GL_TRIANGLE_FAN);
	// Tip of the beam 
	glVertex3f(0.0f, 0.0f, 0.0f); 
	
	// Base 
	for (int i = 0; i <= segments; ++i) {
		float angle = i * 2.0f * PI / segments;
		float x = beamRadius * cosf(angle);
		float z = beamRadius * sinf(angle);
		glVertex3f(x, 0.0f, beamLength);
	}
	glEnd();
	
	// faint inner cone 
	glColor4f(1.0f, 1.0f, 0.95f, 0.6f); 
	glBegin(GL_TRIANGLE_FAN); 
	glVertex3f(0.0f, 0.0f, 0.0f); 
	for (int i = 0; i <= segments; ++i) {
		float angle = i * 2.0f * PI / segments;
		float x = beamRadius * 0.4f * cosf(angle);
		float z = beamRadius * 0.4f * sinf(angle);
		glVertex3f(x, 0.0f, beamLength * 0.8f);
	}
	glEnd();
	
	// restore state 
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glDisable(GL_BLEND); 
	glPopMatrix();
}

// Models and textures draw here
void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float totalYaw = Yaw + gAngleY;

	float radYaw = totalYaw * PI / 180.0f;
	float radPitch = gAngleX * PI / 180.0f; // Mouse Up/Down

	float dirX = sin(radYaw) * cos(radPitch);
	float dirY = -sin(radPitch);
	float dirZ = -cos(radYaw) * cos(radPitch);

	// Camera Position (Eye)
	float eyeX = PosX;
	float eyeY = PosY + 0.5f;
	float eyeZ = PosZ;

	// Target Position (Eye + Direction)
	float centerX = eyeX + dirX;
	float centerY = eyeY + dirY;
	float centerZ = eyeZ + dirZ;

	// Set the view
	gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0.0f, 1.0f, 0.0f);

	// ===== RESET STATE FOR GROUND =====
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glNormal3f(1.0f, 1.0f, 1.0f);

	GLfloat matBlack[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat matZero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, matBlack);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matBlack);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matZero);

	// Draw ground
	glBegin(GL_POLYGON);
	glVertex3f(-15.0f, 0.0f, -15.0f);//top left
	glVertex3f(12.0f, 0.0f, -15.0f);//top right
	glVertex3f(12.0f, 0.0f, -3.0f);//bottom right
	glVertex3f(-3.0f, 0.0f, 15.0f);//bottom left
	glVertex3f(-15.0f, 0.0f, 15.0f);//bottom left
	glEnd();

	// Draw carpark2 area
	glBegin(GL_POLYGON);
	glVertex3f(12.0f, 0.0f, -10.0f);//top left
	glVertex3f(18.0f, 0.0f, -10.0f);//top right
	glVertex3f(18.0f, 0.0f, 0.0f);//bottom right
	glVertex3f(12.0f, 0.0f, 0.0f);//bottom left
	glEnd();

	// Draw extra plane
	glBegin(GL_POLYGON);
	glVertex3f(-15.0f, 0.01f, 15.0f);//top left
	glVertex3f(-2.8f, 0.01f, 15.0f);//top left
	glVertex3f(9.0f, 0.01f, 0.0f);//top right
	glVertex3f(12.0f, 0.01f, 0.0f);//bottom right
	glVertex3f(-2.8f, 0.01f, 18.0f);//bottom left
	glVertex3f(-15.0f, 0.1f, 18.0f);//bottom left
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(12.0f, 0.01f, -3.0f);//top
	glVertex3f(12.0f, 0.01f, 0.0f);//bottom right
	glVertex3f(9.0f, 0.01f, 0.0f);//bottom left
	glEnd();

	GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat matSpec[] = { 0.1f, 0.1f, 0.1f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, matWhite);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matWhite);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	int i = 0;
	for (const Model& m : gModels) {
		glPushMatrix();
		glTranslatef(m.posX, m.posY, m.posZ);
		glRotatef(m.rotX, 1.0f, 0.0f, 0.0f);
		glRotatef(m.rotY, 0.0f, 1.0f, 0.0f);
		glRotatef(m.rotZ, 0.0f, 0.0f, 1.0f);
		glScalef(m.scaleX, m.scaleY, m.scaleZ);

		if (m.repeat == false)
		{
			glBindTexture(GL_TEXTURE_2D, gTex[m.tex]);
			glBegin(GL_TRIANGLES);
			for (const auto& q : m.triangles) {
				const Vec3& v0 = m.vertices[q.a];
				const Vec3& v1 = m.vertices[q.b];
				const Vec3& v2 = m.vertices[q.c];

				Vec3 n = ComputeFaceNormal(v0, v1, v2);
				glNormal3f(n.x, n.y, n.z);

				glTexCoord2f(0.f, 1.f); glVertex3f(v0.x, v0.y, v0.z);
				glTexCoord2f(1.f, 1.f); glVertex3f(v1.x, v1.y, v1.z);
				glTexCoord2f(1.f, 0.f); glVertex3f(v2.x, v2.y, v2.z);
			}
			glEnd();
		}
		if (m.repeat == true) {

			glBindTexture(GL_TEXTURE_2D, gTex[m.tex]);
			if (!m.gQuads.empty()) {
				glBegin(GL_QUADS);
				for (const auto& q : m.gQuads) {
					const Vec3& v0 = m.vertices[q.a];
					const Vec3& v1 = m.vertices[q.b];
					const Vec3& v2 = m.vertices[q.c];
					const Vec3& v3 = m.vertices[q.d];

					Vec3 n = ComputeFaceNormal(v0, v1, v2);

					glNormal3f(n.x, n.y, n.z);

					glTexCoord2f(0.f, 1.f);  glVertex3fv(&v0.x);
					glTexCoord2f(1.f, 1.f);  glVertex3fv(&v1.x);
					glTexCoord2f(1.f, 0.f);  glVertex3fv(&v2.x);
					glTexCoord2f(0.f, 0.f);  glVertex3fv(&v3.x);
				}
				glEnd();
			}

			if (!m.triangles.empty()) {
				glBegin(GL_TRIANGLES);
				for (const auto& q : m.triangles) {
					const Vec3& v0 = m.vertices[q.a];
					const Vec3& v1 = m.vertices[q.b];
					const Vec3& v2 = m.vertices[q.c];

					Vec3 n = ComputeFaceNormal(v0, v1, v2);
					glNormal3f(n.x, n.y, n.z);

					glTexCoord2f(0.f, 1.f); glVertex3f(v0.x, v0.y, v0.z);
					glTexCoord2f(1.f, 1.f); glVertex3f(v1.x, v1.y, v1.z);
					glTexCoord2f(1.f, 0.f); glVertex3f(v2.x, v2.y, v2.z);
				}
				glEnd();
			}

		}

		glPopMatrix();
		i++;
	}


	// draw headlight beams for moving cars at car_yellow 
	if (carA.parts[1] < gModels.size()) {
		const Model& yellowA = gModels[carA.parts[1]];
		DrawHeadlightBeam(yellowA.posX, yellowA.posY, yellowA.posZ, yellowA.rotY); 
	}
	if (carB.parts[1] < gModels.size()) {
		const Model& yellowB = gModels[carB.parts[1]];
		DrawHeadlightBeam(yellowB.posX, yellowB.posY, yellowB.posZ, yellowB.rotY);
	}

	if (showDebug) {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glLineWidth(2.0f);

		// 1. Draw World Colliders (RED)
		glColor3f(1.0f, 0.0f, 0.0f);
		for (const auto& box : gWorldColliders) {
			DrawWireframeBox(box);
		}

		// 2. Draw Player Box (GREEN)
		glColor3f(0.0f, 1.0f, 0.0f);
		AABB playerBox;
		playerBox.minX = PosX - PLAYER_WIDTH / 2;
		playerBox.maxX = PosX + PLAYER_WIDTH / 2;
		playerBox.minY = PosY;
		playerBox.maxY = PosY + PLAYER_HEIGHT;
		playerBox.minZ = PosZ - PLAYER_WIDTH / 2;
		playerBox.maxZ = PosZ + PLAYER_WIDTH / 2;
		DrawWireframeBox(playerBox);

		// Restore settings
		glLineWidth(1.0f);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}
	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	if (h == 0) h = 1;
	float aspect = (float)w / (float)h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, aspect, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

float ToRad(float deg) {
	return deg * PI / 180.0f;
}

void AddModelCollision(const Model& m) {
	float minX = 99999.0f, maxX = -99999.0f;
	float minY = 99999.0f, maxY = -99999.0f;
	float minZ = 99999.0f, maxZ = -99999.0f;

	// Pre-calculate rotation trig values
	float rX = ToRad(m.rotX);
	float rY = ToRad(m.rotY);
	float rZ = ToRad(m.rotZ);

	float sx = sin(rX), cx = cos(rX);
	float sy = sin(rY), cy = cos(rY);
	float sz = sin(rZ), cz = cos(rZ);

	for (const auto& v : m.vertices) {
		// 1. Scale
		float x = v.x * m.scaleX;
		float y = v.y * m.scaleY;
		float z = v.z * m.scaleZ;

		// 2. Rotate
		// Rotate X
		float ty = y * cx - z * sx;
		float tz = y * sx + z * cx;
		y = ty; z = tz;

		// Rotate Y
		float tx = x * cy + z * sy;
		tz = -x * sy + z * cy;
		x = tx; z = tz;

		// Rotate Z
		tx = x * cz - y * sz;
		ty = x * sz + y * cz;
		x = tx; y = ty;

		// 3. Translate
		x += m.posX;
		y += m.posY;
		z += m.posZ;

		// 4. Update Min/Max
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
		if (z < minZ) minZ = z;
		if (z > maxZ) maxZ = z;
	}

	// Calculate the size of the box
	float width = maxX - minX;
	float depth = maxZ - minZ;

	if (width > 20.0f || depth > 20.0f) {
		AABB floorBox;
		floorBox.minX = minX; floorBox.maxX = maxX;
		floorBox.minY = minY; floorBox.maxY = maxY;
		floorBox.minZ = minZ; floorBox.maxZ = maxZ;
		gFloors.push_back(floorBox); // Save to floor list
		return;
	}

	if (m.forceTall) {
		maxY += 20.0f; // Make the invisible wall 20 units taller
	}

	// Store the computed box
	AABB box;
	box.minX = minX; box.maxX = maxX;
	box.minY = minY; box.maxY = maxY;
	box.minZ = minZ; box.maxZ = maxZ;
	gWorldColliders.push_back(box);
}

void InitGL()
{
	// === Texture === 
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// === Lighting basic === 
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_COLOR_MATERIAL);

	GLfloat matAmbient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat matSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat matShininess[] = { 50.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

	// Brighter light
	GLfloat lightPos[] = { 5.0f, 5.0f, 10.0f, 1.0f };  // Positional light (w=1)
	GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

	GLfloat globalAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	glClearColor(0.65f, 0.78f, 0.91f, 1.0f);
}

// move carA in rectangle path
void MoveGroupedCar(MovingCar& car, float minX, float maxX, float minZ, float maxZ) {
	Model& ref = gModels[car.parts[0]];
	float dx = 0.0f, dz = 0.0f;
	float rotY = ref.rotY; switch (car.state) {
		case 0: dz = CAR_LOOP_SPEED_A;
			rotY = 0.0f;
			if (ref.posZ >= maxZ)
				car.state = 1;
			break;
		case 1: dx = -CAR_LOOP_SPEED_A; 
			rotY = -90.0f; 
			if (ref.posX <= minX)
				car.state = 2; 
			break; 
		case 2: dz = -CAR_LOOP_SPEED_A;
			rotY = 180.0f;
			if (ref.posZ <= minZ)
				car.state = 3;
			break;
		case 3: dx = CAR_LOOP_SPEED_A;
			rotY = 90.0f;
			if (ref.posX >= maxX)
				car.state = 0;
			break;
	}
	for (int i = 0; i < 4; i++) {
		gModels[car.parts[i]].posX += dx;
		gModels[car.parts[i]].posZ += dz;
		gModels[car.parts[i]].rotY = rotY;
	}
} 

// move carB back and forth along x axis
void MoveGroupedCarXOnly(MovingCar& car, float minX, float maxX) {
	Model& ref = gModels[car.parts[0]];
	float dx = (car.state == 0) ? CAR_LOOP_SPEED_B : -CAR_LOOP_SPEED_B;
	float rotY = (car.state == 0) ? 90.0f : -90.0f;
	if (ref.posX >= maxX) car.state = 1;
	if (ref.posX <= minX) car.state = 0;
	for (int i = 0; i < 4; i++) {
		gModels[car.parts[i]].posX += dx;
		gModels[car.parts[i]].rotY = rotY;
	}
}

// rebuild colliders especially for moving obj from current transformations
void RebuildAllColliders()
{
	gWorldColliders.clear();
	for (const auto& m : gModels)
		AddModelCollision(m);
}

void Idle() {
	float totalYaw = Yaw + gAngleY;
	float rad = totalYaw * PI / 180.0f;
	float s = sin(rad);
	float c = cos(rad);

	const float STEP_HEIGHT = 0.5f;

	float dx = 0.0f;
	float dz = 0.0f;

	if (moveForward) 
	{ 
		dx += s * moveSpeed; 
		dz -= c * moveSpeed; 
	}
	if (moveBackward) 
	{ 
		dx -= s * moveSpeed; 
		dz += c * moveSpeed; 
	}
	if (moveLeft) 
	{ 
		dx -= c * moveSpeed; 
		dz -= s * moveSpeed; 
	}
	if (moveRight) 
	{ 
		dx += c * moveSpeed; 
		dz += s * moveSpeed; 
	}

	float nextX = PosX + dx;
	float nextZ = PosZ;

	AABB playerBoxX;
	playerBoxX.minX = nextX - PLAYER_WIDTH / 2;
	playerBoxX.maxX = nextX + PLAYER_WIDTH / 2;
	playerBoxX.minY = PosY;
	playerBoxX.maxY = PosY + PLAYER_HEIGHT;
	playerBoxX.minZ = nextZ - PLAYER_WIDTH / 2;
	playerBoxX.maxZ = nextZ + PLAYER_WIDTH / 2;

	bool collisionX = false;
	for (const auto& wall : gWorldColliders) {
		if (CheckCollision(playerBoxX, wall)) {
			if (wall.maxY > PosY + STEP_HEIGHT) {
				collisionX = true;
				break;
			}
		}
	}
	if (!collisionX) PosX = nextX;

	nextX = PosX;
	nextZ = PosZ + dz;

	AABB playerBoxZ;
	playerBoxZ.minX = nextX - PLAYER_WIDTH / 2;
	playerBoxZ.maxX = nextX + PLAYER_WIDTH / 2;
	playerBoxZ.minY = PosY;
	playerBoxZ.maxY = PosY + PLAYER_HEIGHT;
	playerBoxZ.minZ = nextZ - PLAYER_WIDTH / 2;
	playerBoxZ.maxZ = nextZ + PLAYER_WIDTH / 2;

	bool collisionZ = false;
	for (const auto& wall : gWorldColliders) {
		if (CheckCollision(playerBoxZ, wall)) {
			if (wall.maxY > PosY + STEP_HEIGHT) {
				collisionZ = true;
				break;
			}
		}
	}
	if (!collisionZ) PosZ = nextZ;


	float targetY = 0.0f; // Default Ground level

	// Helper Lambda to check a list of boxes for ground
	auto CheckGround = [&](const std::vector<AABB>& boxes) {
		for (const auto& box : boxes) {
			// Check if player is strictly inside the X/Z bounds of this box
			if (PosX >= box.minX && PosX <= box.maxX &&
				PosZ >= box.minZ && PosZ <= box.maxZ) {

				if (box.maxY <= PosY + STEP_HEIGHT) {
					if (box.maxY > targetY) {
						targetY = box.maxY;
					}
				}
			}
		}
	};

	CheckGround(gFloors);
	CheckGround(gWorldColliders);

	// Apply Gravity
	float gravitySpeed = 0.1f;

	if (PosY > targetY) {
		PosY -= gravitySpeed;
		if (PosY < targetY) PosY = targetY; // Landed
	}
	else if (PosY < targetY) {
		PosY = targetY;
	}

	if (rotateLeft) Yaw -= rotSpeed;
	if (rotateRight) Yaw += rotSpeed;

	// CarA
	MoveGroupedCar(
		carA,
		13.2f, 16.8f, // X range
		-9.5f, -0.5f // Z range
	); 
	// CarB
	MoveGroupedCarXOnly(carB, 14.8f, 15.6f );
	RebuildAllColliders();
	glutPostRedisplay();
}


void InitAudio() {
	ma_result result;
	result = ma_engine_init(NULL, &gAudioEngine);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize audio engine.\n");
		return;
	}
	ma_engine_play_sound(&gAudioEngine, "Music/background.mp3", NULL);
}

void CleanupAudio() {
	ma_engine_uninit(&gAudioEngine);
}

void KeyDown(unsigned char key, int x, int y)
{
	static bool full = false;
	switch (key) {
	case 'b': case 'B':
		showDebug = !showDebug;
		break;
	case 27: // ESC 
		exit(0);
		break;
	case 'w': case 'W':
		moveForward = true;
		break;
	case 's': case 'S':
		moveBackward = true;
		break;
	case 'a': case 'A':
		moveLeft = true;
		break;
	case 'd': case 'D':
		moveRight = true;
		break;
	case 'q': case 'Q':
		rotateLeft = true;
		break;
	case 'e': case 'E':
		rotateRight = true;
		break;

	case 'f': case 'F':
		if (!full) glutFullScreen();
		else glutReshapeWindow(winW, winH);
		full = !full;
		break;
	}
}

void KeyUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': case 'W':
		moveForward = false;
		break;
	case 's': case 'S':
		moveBackward = false;
		break;
	case 'a': case 'A':
		moveLeft = false;
		break;
	case 'd': case 'D':
		moveRight = false;
		break;
	case 'q': case 'Q':
		rotateLeft = false;
		break;
	case 'e': case 'E':
		rotateRight = false;
		break;
	}
}

void MouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		gDrag = (state == GLUT_DOWN);
		gLastX = x; gLastY = y;
	}
	if (button == 3 && state == GLUT_DOWN) { gDist -= 0.3f; if (gDist < 1.5f) gDist = 1.5f; }
	if (button == 4 && state == GLUT_DOWN) { gDist += 0.3f; if (gDist > 20.f) gDist = 20.f; }

	glutPostRedisplay();
}

void MouseMotion(int x, int y) {
	if (!gDrag) return; int dx = x - gLastX;
	int dy = y - gLastY;
	gLastX = x;
	gLastY = y;
	const float sens = 0.4f;
	gAngleY += dx * sens; // yaw 
	gAngleX += dy * sens; // pitch 
	if (gAngleX > 89.f) gAngleX = 89.f;
	if (gAngleX < -89.f) gAngleX = -89.f;
	glutPostRedisplay();
}


// main
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(400, 150);
	glutCreateWindow("FTMK Building");
	InitGL();
	

	if (!LoadAllTextures()) {
		MessageBoxA(NULL, "One or more texture(s) failed to upload ", "Warning", MB_OK | MB_ICONWARNING);
	}
	gModels.clear();

	std::cout << "OBJ files: \n";

	for (int i = 0; i < std::size(gObj); i++)
	{
		Model m;
		if (LoadOBJ(gObj[i], m))
		{
			// adjust model's position, rotation, scale, texture index here
			if (i == 0) { //lobby_grey.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 0; //gTextures[0]
			}
			else if (i == 1) { //lobby_yellow.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 1; //gTextures[1]
			}
			else if (i == 2) { //lobby_palegrey.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 2; //gTextures[2]
			}
			else if (i == 3) { //lobby_black.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 3; // gTextures[3]
			}
			else if (i == 4) { //leftwing_brown.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 4; //gTextures[4]
			}
			else if (i == 5) { //carpark1WHITE.obj
				m.posX = -1.0f; m.posY = 0.0f;  m.posZ = 6.5f;
				m.rotY = 89.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.027f; m.scaleY = 0.015f; m.scaleZ = 0.027f;
				m.tex = 6; //gTextures[6]
			}
			else if (i == 6) { //carpark1GREY.obj
				m.posX = -1.0f; m.posY = 0.0f;  m.posZ = 6.5f;
				m.rotY = 89.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.027f; m.scaleY = 0.015f; m.scaleZ = 0.027f;
				m.tex = 0; //gTextures[0]
			}
			else if (i == 7) { //carpark1GREEN.obj
				m.posX = -1.0f; m.posY = 0.0f;  m.posZ = 6.5f;
				m.rotY = 89.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.027f; m.scaleY = 0.015f; m.scaleZ = 0.027f;
				m.tex = 7; //gTextures[7]
			}
			else if (i == 8) { //carpark1BLACK.obj
				m.posX = -1.0f; m.posY = 0.0f;  m.posZ = 6.5f;
				m.rotY = 89.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.027f; m.scaleY = 0.015f; m.scaleZ = 0.027f;
				m.tex = 3; //gTextures[3]
			}
			else if (i == 9) { //roundabout_grey.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 0; //gTextures[0]
			}
			else if (i == 10) { //roundabout_dirt.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 8; //gTextures[8]
			}
			else if (i == 11) { //roundabout_yellow.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 1; //gTextures[1]
			}
			else if (i == 12) { //roundabout_bush.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 5; //gTextures[5]
			}
			else if (i == 13) { //roundabout_oak.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 10; //gTextures[10]
			}
			else if (i == 14) { //roundabout_oakLeaf.obj
				m.posX = -5.5f; m.posY = 0.0f;  m.posZ = -5.5f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.05f; m.scaleY = 0.05f; m.scaleZ = 0.05f;
				m.tex = 9; //gTextures[9]
			}
			else if (i == 15) { //carpark2_white.obj
				m.posX = 12.5f; m.posY = 0.01f;  m.posZ = -8.0f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.012f; m.scaleY = 0.01f; m.scaleZ = 0.015f;
				m.tex = 6; //gTextures[6]
			}
			else if (i == 16) { //carpark2_black.obj
				m.posX = 12.5f; m.posY = 0.01f;  m.posZ = -8.0f;
				m.rotY = 0.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.012f; m.scaleY = 0.01f; m.scaleZ = 0.015f;
				m.tex = 3; //gTextures[3]
			}
			else if (i == 17) { //leftwing_palegrey.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 2; //gTextures[2]
			}
			else if (i == 18) { //leftwing_grey.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 0; //gTextures[0]
			}
			else if (i == 19) { //leftwing_yellow.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 1; //gTextures[1]
			}
			else if (i == 20) { //righwing_grey.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 0; //gTextures[0]
			}
			else if (i == 21) { //rightwing_yellow.obj
				m.posX = -10.0f;  m.posY = 0.17f;  m.posZ = -9.0f;
				m.rotY = -130.0f; m.rotX = 0.0f; m.rotZ = 0.0f;
				m.scaleX = 0.12f; m.scaleY = 0.2f; m.scaleZ = 0.12f;
				m.tex = 1; //gTextures[1]
			}

			gModels.push_back(std::move(m));
			std::cout << gObj[i] << " loaded \n";
		}
	}

	std::cout << "\nRepeat Models\n";

	for (int i = 0; i < para.size(); i++)
	{
		int id = para[i].modelID;

		if (id >= 0 && id < std::size(gRepeatObj))
		{
			duplicate(gRepeatObj[id],
				para[i].ite,
				para[i].Tex,
				para[i].startX, para[i].startY, para[i].startZ,
				para[i].moveX, para[i].moveY, para[i].moveZ,
				para[i].rotX, para[i].rotY, para[i].rotZ,
				para[i].scaleX, para[i].scaleY, para[i].scaleZ
			);
		}
		else {
			std::cout << "Error: Invalid model ID " << id << "\n";
		}
	}

	std::cout << "\n\nModels loaded \n";
	std::cout << "Textures loaded \n";

	// grp car parts
	int foundA = 0, foundB = 0;
	for (int i = 0; i < gModels.size(); i++) {
		// Only repeated car Models
		if (!gModels[i].repeat)
			continue;
		// CarA
		if (fabs(gModels[i].posZ + 7.35f) < 0.05f && foundA < 4)
			carA.parts[foundA++] = i;
		// CarB
		if (fabs(gModels[i].posZ + 3.60f) < 0.05f && foundB < 4)
			carB.parts[foundB++] = i;
	}
	glutIdleFunc(Idle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);

	InitAudio();
	std::cout << "Music loaded \n";

	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutMainLoop();
	CleanupAudio();
	return 0;
}
