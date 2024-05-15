#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

// Chunk
static const int CHUNK_SIZE = 32;
static const int CHUNK_SIZE2 = CHUNK_SIZE * CHUNK_SIZE;
static const int CHUNK_SIZE_P = CHUNK_SIZE + 2;
static const int CHUNK_SIZE_P2 = CHUNK_SIZE_P * CHUNK_SIZE_P;

// Chunk Manager
static const int CHUNK_COUNT = 21;
static const int MAX_HEIGHT = 256;
static const int MAX_HEIGHT_CHUNK_COUNT = MAX_HEIGHT / CHUNK_SIZE;

static const int MAX_ASYNC_LOAD_COUNT = 4;

typedef uint32_t VoxelVertex;

struct SkyboxVertex {
	Vector3 position;
	uint32_t face;
};

struct CameraConstantData {
	Matrix view;
	Matrix proj;
	Vector3 eyePos;
	float dummy;
};

struct ChunkConstantData {
	Matrix world;
};

struct SkyboxConstantData {
	Vector3 sunDir;
	float dummy;
};