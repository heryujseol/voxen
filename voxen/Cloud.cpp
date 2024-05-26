#include "Cloud.h"
#include "Utils.h"
#include "DXUtils.h"

#include <algorithm>
Cloud::Cloud()
	: m_mapCenterPosition(0.0f, 0.0f, 0.0f), m_mapDataOffset(0.0f, 0.0f, 0.0f), m_speed(10.0f),
	  m_height(192.0f), m_stride(sizeof(CloudVertex)), m_offset(0),
	  m_samplingStride(sizeof(SamplingVertex)), m_samplingOffset(0)
{
	for (int i = 0; i < CLOUD_MAP_SIZE; ++i) {
		std::fill(m_map[i], m_map[i] + CLOUD_MAP_SIZE, false);
	}
	for (int i = 0; i < CLOUD_DATA_MAP_SIZE; ++i) {
		std::fill(m_dataMap[i], m_dataMap[i] + CLOUD_DATA_MAP_SIZE, false);
	}
};

Cloud::~Cloud(){};

bool Cloud::Initialize(Vector3 cameraPosition)
{
	for (int i = 0; i < CLOUD_DATA_MAP_SIZE; ++i) {
		for (int j = 0; j < CLOUD_DATA_MAP_SIZE; ++j) {
			float noise1 = Utils::PerlinFbm((float)i / CLOUD_DATA_MAP_SIZE,
				(float)j / CLOUD_DATA_MAP_SIZE, CLOUD_DATA_MAP_SIZE * 0.125f, 3);
			float noise2 = Utils::PerlinFbm((float)i / CLOUD_DATA_MAP_SIZE,
				(float)j / CLOUD_DATA_MAP_SIZE, CLOUD_DATA_MAP_SIZE * 0.5f, 1);
			m_dataMap[i][j] = noise1 > 0.2f || noise2 > 0.5f;
		}
	}

	m_mapCenterPosition = Utils::CalcOffsetPos(cameraPosition, CLOUD_SCALE_SIZE);
	m_mapCenterPosition.y = 0.0f;
	m_mapDataOffset = m_mapCenterPosition;

	if (!BuildCloud())
		return false;

	if (!BuildSquare())
		return false;

	return true;
}

void Cloud::Update(float dt, Vector3 cameraPosition)
{
	m_mapCenterPosition.x -= m_speed * dt;

	Vector3 newMapCenterPosition = m_mapCenterPosition;
	Vector3 diffPos = cameraPosition - m_mapCenterPosition;
	if (diffPos.x > CLOUD_SCALE_SIZE) {
		newMapCenterPosition.x += CLOUD_SCALE_SIZE;
		m_mapDataOffset.x += CLOUD_SCALE_SIZE;
	}
	else if (diffPos.z > CLOUD_SCALE_SIZE) {
		newMapCenterPosition.z += CLOUD_SCALE_SIZE;
		m_mapDataOffset.z += CLOUD_SCALE_SIZE;
	}
	else if (diffPos.x < 0.0f) {
		newMapCenterPosition.x -= CLOUD_SCALE_SIZE;
		m_mapDataOffset.x -= CLOUD_SCALE_SIZE;
	}
	else if (diffPos.z < 0.0f) {
		newMapCenterPosition.z -= CLOUD_SCALE_SIZE;
		m_mapDataOffset.z -= CLOUD_SCALE_SIZE;
	}

	if (newMapCenterPosition != m_mapCenterPosition) {
		m_mapCenterPosition = newMapCenterPosition;
		BuildCloud();
	}

	if (m_vertices.empty())
		return;

	Vector3 worldPosition = m_mapCenterPosition + Vector3(0.0f, m_height, 0.0f);
	m_constantData.world = Matrix::CreateScale(CLOUD_SCALE_SIZE, 4.0f, CLOUD_SCALE_SIZE) *
						   Matrix::CreateTranslation(worldPosition);
	m_constantData.world = m_constantData.world.Transpose();
	DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);
}

void Cloud::Render()
{
	Graphics::context->CopyResource(
		Graphics::cloudRenderBuffer.Get(), Graphics::basicRenderBuffer.Get());
	Graphics::context->OMSetRenderTargets(
		1, Graphics::cloudRTV.GetAddressOf(), Graphics::basicDSV.Get());

	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());
	Graphics::context->PSSetConstantBuffers(2, 1, m_constantBuffer.GetAddressOf());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);

	Blend();
}

void Cloud::Blend()
{
	Graphics::context->OMSetRenderTargets(1, Graphics::basicRTV.GetAddressOf(), nullptr);
	Graphics::SetPipelineStates(Graphics::cloudBlendPSO);

	Graphics::context->IASetIndexBuffer(m_samplingIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_samplingVertexBuffer.GetAddressOf(), &m_samplingStride, &m_samplingOffset);

	Graphics::context->ResolveSubresource(Graphics::cloudResolvedBuffer.Get(), 0,
		Graphics::cloudRenderBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	Graphics::context->PSSetShaderResources(0, 1, Graphics::cloudSRV.GetAddressOf());

	Graphics::context->DrawIndexed((UINT)m_samplingIndices.size(), 0, 0);
}

bool Cloud::BuildCloud()
{
	for (int i = 0; i < CLOUD_MAP_SIZE; ++i) {
		for (int j = 0; j < CLOUD_MAP_SIZE; ++j) {
			int x =
				((int)(m_mapDataOffset.x / CLOUD_SCALE_SIZE) + i - (int)(CLOUD_MAP_SIZE * 0.5f)) %
				CLOUD_DATA_MAP_SIZE;
			int z =
				((int)(m_mapDataOffset.z / CLOUD_SCALE_SIZE) + j - (int)(CLOUD_MAP_SIZE * 0.5f)) %
				CLOUD_DATA_MAP_SIZE;
		
			if (x < 0)
				x += CLOUD_DATA_MAP_SIZE;
			if (z < 0)
				z += CLOUD_DATA_MAP_SIZE;

			m_map[i][j] = m_dataMap[x][z];
		}
	}

	m_vertices.clear();
	m_indices.clear();
	for (int i = 0; i < CLOUD_MAP_SIZE; ++i) {
		for (int j = 0; j < CLOUD_MAP_SIZE; ++j) {
			if (!m_map[i][j])
				continue;
			bool x_n = true, x_p = true, z_n = true, z_p = true;
			if (i > 0 && m_map[i - 1][j])
				x_n = false;
			if (i < CLOUD_MAP_SIZE - 1 && m_map[i + 1][j])
				x_p = false;
			if (j > 0 && m_map[i][j - 1])
				z_n = false;
			if (j < CLOUD_MAP_SIZE - 1 && m_map[i][j + 1])
				z_p = false;

			int x = i - (int)(CLOUD_MAP_SIZE * 0.5f);
			int z = j - (int)(CLOUD_MAP_SIZE * 0.5f);
			CreateCloudMesh(x, z, x_n, x_p, z_n, z_p);
		}
	}

	if (!m_vertices.empty()) {
		m_vertexBuffer.Reset();
		m_indexBuffer.Reset();
		m_constantBuffer.Reset();
		if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
			std::cout << "failed create vertex buffer in cloud" << std::endl;
			return false;
		}

		if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
			std::cout << "failed create index buffer in cloud" << std::endl;
			return false;
		}

		m_constantData.cloudScale = CLOUD_SCALE_SIZE * CLOUD_MAP_SIZE * 0.5;
		m_constantData.volumeColor = Vector3(1.0f, 1.0f, 1.0f);
		m_constantData.world = Matrix();
		if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
			std::cout << "failed create constant buffer in cloud" << std::endl;
			return false;
		}
	}

	return true;
}

void Cloud::CreateCloudMesh(int x, int z, bool x_n, bool x_p, bool z_n, bool z_p)
{
	uint32_t originVertexSize = (uint32_t)m_vertices.size();
	uint32_t faceCount = 0;

	CloudVertex vertex;
	// ¿ÞÂÊ
	if (x_n) {
		vertex.face = 0;
		vertex.position = Vector3(x + 0.0f, 1.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 1.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 0.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 0.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		faceCount++;
	}

	// ¿À¸¥ÂÊ
	if (x_p) {
		vertex.face = 1;
		vertex.position = Vector3(x + 1.0f, 1.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 1.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 0.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 0.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		faceCount++;
	}

	// ¾Æ·§¸é
	vertex.face = 2;
	vertex.position = Vector3(x + 0.0f, 0.0f, z + 0.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 1.0f, 0.0f, z + 0.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 1.0f, 0.0f, z + 1.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 0.0f, 0.0f, z + 1.0f);
	m_vertices.push_back(vertex);
	faceCount++;

	// À­¸é
	vertex.face = 3;
	vertex.position = Vector3(x + 0.0f, 1.0f, z + 1.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 1.0f, 1.0f, z + 1.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 1.0f, 1.0f, z + 0.0f);
	m_vertices.push_back(vertex);
	vertex.position = Vector3(x + 0.0f, 1.0f, z + 0.0f);
	m_vertices.push_back(vertex);
	faceCount++;

	// ¾Õ¸é
	if (z_n) {
		vertex.face = 4;
		vertex.position = Vector3(x + 0.0f, 1.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 1.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 0.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 0.0f, z + 0.0f);
		m_vertices.push_back(vertex);
		faceCount++;
	}

	// µÞ¸é
	if (z_p) {
		vertex.face = 5;
		vertex.position = Vector3(x + 1.0f, 1.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 1.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 0.0f, 0.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		vertex.position = Vector3(x + 1.0f, 0.0f, z + 1.0f);
		m_vertices.push_back(vertex);
		faceCount++;
	}

	for (uint32_t i = 0; i < faceCount; ++i) {
		m_indices.push_back(originVertexSize + i * 4);
		m_indices.push_back(originVertexSize + 1 + i * 4);
		m_indices.push_back(originVertexSize + 2 + i * 4);

		m_indices.push_back(originVertexSize + i * 4);
		m_indices.push_back(originVertexSize + 2 + i * 4);
		m_indices.push_back(originVertexSize + 3 + i * 4);
	}
}

bool Cloud::BuildSquare()
{
	CreateSquareMesh();

	if (!DXUtils::CreateVertexBuffer(m_samplingVertexBuffer, m_samplingVertices)) {
		std::cout << "failed create sampling vertex buffer in cloud" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_samplingIndexBuffer, m_samplingIndices)) {
		std::cout << "failed create sampling index buffer in cloud" << std::endl;
		return false;
	}

	return true;
}

void Cloud::CreateSquareMesh()
{
	SamplingVertex vertex;

	vertex.position = Vector3(-1.0f, 1.0f, 0.0f);
	vertex.texcoord = Vector2(0.0f, 0.0f);
	m_samplingVertices.push_back(vertex);

	vertex.position = Vector3(1.0f, 1.0f, 0.0f);
	vertex.texcoord = Vector2(1.0f, 0.0f);
	m_samplingVertices.push_back(vertex);

	vertex.position = Vector3(1.0f, -1.0f, 0.0f);
	vertex.texcoord = Vector2(1.0f, 1.0f);
	m_samplingVertices.push_back(vertex);

	vertex.position = Vector3(-1.0f, -1.0f, 0.0f);
	vertex.texcoord = Vector2(0.0f, 1.0f);
	m_samplingVertices.push_back(vertex);


	m_samplingIndices.push_back(0);
	m_samplingIndices.push_back(1);
	m_samplingIndices.push_back(2);

	m_samplingIndices.push_back(0);
	m_samplingIndices.push_back(2);
	m_samplingIndices.push_back(3);
}