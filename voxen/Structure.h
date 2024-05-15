#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

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