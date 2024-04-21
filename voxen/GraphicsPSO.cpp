#include "GraphicsPSO.h"

GraphicsPSO::GraphicsPSO()
	: inputLayout(nullptr), topology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED), vertexShader(nullptr),
	  rasterizerState(nullptr), pixelShader(nullptr), samplerStates(), depthStencilState(nullptr)
{
}

GraphicsPSO::GraphicsPSO(const GraphicsPSO& rhs)
	: inputLayout(rhs.inputLayout), topology(rhs.topology), vertexShader(rhs.vertexShader),
	  rasterizerState(rhs.rasterizerState), pixelShader(rhs.pixelShader),
	  samplerStates(rhs.samplerStates), depthStencilState(rhs.depthStencilState)
{
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

	return *this;
}

GraphicsPSO::~GraphicsPSO() {}
