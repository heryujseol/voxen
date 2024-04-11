#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Vertex {
	Vector3 pos;
	Vector3 normal;
};

struct GlobalConstantData {
	Matrix view;
	Matrix proj;
};

struct ChunkConstantData {
	Matrix world;
};