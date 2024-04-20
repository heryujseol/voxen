#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Vertex {
	Vector3 pos;
	Vector3 normal;
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