#include "GraphicsPSO.h"

GraphicsPSO::GraphicsPSO()
	: inputLayout(nullptr), topology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED), vertexShader(nullptr),
	  rasterizerState(nullptr), pixelShader(nullptr), samplerStates(), depthStencilState(nullptr),
	  blendState(nullptr), blendFactor{1.0f, 1.0f, 1.0f, 1.0f}
{
}

GraphicsPSO::GraphicsPSO(const GraphicsPSO& rhs)
	: inputLayout(rhs.inputLayout), topology(rhs.topology), vertexShader(rhs.vertexShader),
	  rasterizerState(rhs.rasterizerState), pixelShader(rhs.pixelShader),
	  samplerStates(rhs.samplerStates), depthStencilState(rhs.depthStencilState)
{
	blendFactor[0] = rhs.blendFactor[0];
	blendFactor[1] = rhs.blendFactor[1];
	blendFactor[2] = rhs.blendFactor[2];
	blendFactor[3] = rhs.blendFactor[3];
}

GraphicsPSO GraphicsPSO::operator=(const GraphicsPSO& other)
{
	inputLayout = other.inputLayout;
	topology = other.topology;

	vertexShader = other.vertexShader;

	rasterizerState = other.rasterizerState;

	pixelShader = other.pixelShader;

	samplerStates = other.samplerStates;

	depthStencilState = other.depthStencilState;
	
	blendState = other.blendState;
	blendFactor[0] = other.blendFactor[0];
	blendFactor[1] = other.blendFactor[1];
	blendFactor[2] = other.blendFactor[2];
	blendFactor[3] = other.blendFactor[3];

	return *this;
}

GraphicsPSO::~GraphicsPSO() {}
