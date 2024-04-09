#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <directxtk/SimpleMath.h>

using namespace Microsoft::WRL; 
using namespace DirectX::SimpleMath;


struct Vertex {
	Vector3 pos;
	Vector3 normal;
};

class Block 
{
public:
	Block();
	~Block();
	bool IsActive();
	void SetActive(bool active);

private:
	bool m_isActive;
};