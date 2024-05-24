#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

typedef uint32_t VoxelVertex;

struct SkyboxVertex {
	Vector3 position;
};

struct CloudVertex {
	Vector3 position;
	uint8_t face;
};

struct SamplingVertex {
	Vector3 position;
	Vector2 texcoord;
};

struct CameraConstantData {
	Matrix view;
	Matrix proj;
	Vector3 eyePos;
	float dummy1;
	Vector3 eyeDir;
	float dummy2;
};

struct ChunkConstantData {
	Matrix world;
};

struct SkyboxConstantData {
	Vector3 sunDir;
	float skyScale;
	Vector3 sunStrength;
	float sunAltitude;
	Vector3 moonStrength;
	float sectionAltitudeBounary;
	Vector3 horizonColor;
	float showAltitudeBoundary;
	Vector3 zenithColor;
	float dummy3;
};

struct CloudConstantData {
	Matrix world;
	Vector3 volumeColor;
	float density;
};