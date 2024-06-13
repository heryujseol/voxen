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

struct InstanceVertex {
	Vector3 position;
	Vector3 normal;
	Vector2 texcoord;
};

struct InstanceInfo {
	Matrix instanceWorld;
	uint32_t type;
};

struct CameraConstantData {
	Matrix view;
	Matrix proj;
	Vector3 eyePos;
	float dummy1;
	Vector3 eyeDir;
	float dummy2;
	Matrix invProj;
};

struct ChunkConstantData {
	Matrix world;
};

struct SkyboxConstantData {
	Vector3 sunDir;
	float skyScale;
	Vector3 normalHorizonColor;
	uint32_t dateTime;
	Vector3 normalZenithColor;
	float sunStrength;
	Vector3 sunHorizonColor;
	float moonStrength;
	Vector3 sunZenithColor;
	float dummy3;
};

struct CloudConstantData {
	Matrix world;
	Vector3 volumeColor;
	float cloudScale;
};

struct EnvMapConstantData {
	Matrix view[6];
	Matrix proj;
};