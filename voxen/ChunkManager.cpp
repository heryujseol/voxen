#include "ChunkManager.h"
#include "Graphics.h"
#include "Utils.h"
#include "DXUtils.h"
#include "MeshGenerator.h"

#include <iostream>

ChunkManager::ChunkManager() {}

ChunkManager::~ChunkManager() {}

bool ChunkManager::Initialize(Vector3 cameraChunkPos)
{
	UINT poolSize = CHUNK_COUNT_P * CHUNK_COUNT_P * MAX_HEIGHT_CHUNK_COUNT_P;
	for (UINT i = 0; i < poolSize; ++i) {
		m_chunkPool.push_back(new Chunk(i));
	}

	m_basicVertexBuffers.resize(poolSize);
	m_basicIndexBuffers.resize(poolSize);

	m_waterVertexBuffers.resize(poolSize);
	m_waterIndexBuffers.resize(poolSize);

	m_constantBuffers.resize(poolSize);

	m_instanceVertexBuffers.resize(Block::INSTANCE_TYPE_COUNT);
	m_instanceIndexBuffers.resize(Block::INSTANCE_TYPE_COUNT);
	m_instanceInfoBuffers.resize(Block::INSTANCE_TYPE_COUNT);
	m_instanceInfoList.resize(Block::INSTANCE_TYPE_COUNT);
	if (!MakeInstanceVertexBuffer())
		return false;
	if (!MakeInstanceInfoBuffer())
		return false;

	UpdateChunkList(cameraChunkPos);

	return true;
}

void ChunkManager::Update(Camera& camera)
{
	if (camera.m_isOnChunkDirtyFlag) {
		UpdateChunkList(camera.GetChunkPosition());
		camera.m_isOnChunkDirtyFlag = false;
	}

	UpdateLoadChunkList();
	UpdateUnloadChunkList();
	UpdateRenderChunkList(camera);
	UpdateInstanceInfoList(camera);
}

void ChunkManager::RenderBasic()
{
	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::atlasMapSRV.Get(),
		Graphics::grassColorMapSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	for (auto& c : m_renderChunkList) {
		if (c->IsEmptyBasic())
			continue;

		UINT id = c->GetID();
		UINT stride = sizeof(VoxelVertex);
		UINT offset = 0;

		Graphics::context->IASetIndexBuffer(m_basicIndexBuffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
		Graphics::context->IASetVertexBuffers(
			0, 1, m_basicVertexBuffers[id].GetAddressOf(), &stride, &offset);
		Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffers[id].GetAddressOf());

		Graphics::context->DrawIndexed((UINT)c->GetBasicIndices().size(), 0, 0);
	}
}

void ChunkManager::RenderInstance()
{
	UINT indexCountPerInstance[4] = { 36, 12, 24, 6 };
	for (int i = 0; i < Block::INSTANCE_TYPE_COUNT; ++i) {
		Graphics::context->IASetIndexBuffer(
			m_instanceIndexBuffers[i].Get(), DXGI_FORMAT_R32_UINT, 0);

		std::vector<UINT> strides = { sizeof(InstanceVertex), sizeof(InstanceInfo) };
		std::vector<UINT> offsets = { 0, 0 };
		std::vector<ID3D11Buffer*> buffers = { m_instanceVertexBuffers[i].Get(),
			m_instanceInfoBuffers[i].Get() };
		Graphics::context->IASetVertexBuffers(0, 2, buffers.data(), strides.data(), offsets.data());
		Graphics::context->DrawIndexedInstanced(
			indexCountPerInstance[i], (UINT)m_instanceInfoList[i].size(), 0, 0, 0);
	}
}

void ChunkManager::RenderWater() {}

void ChunkManager::UpdateChunkList(Vector3 cameraChunkPos)
{
	std::map<std::tuple<int, int, int>, bool> loadedChunkMap;
	for (int i = 0; i < MAX_HEIGHT_CHUNK_COUNT; ++i) {
		for (int j = 0; j < CHUNK_COUNT; ++j) {
			for (int k = 0; k < CHUNK_COUNT; ++k) {
				int y = Chunk::CHUNK_SIZE * (i - 2);
				int x = (int)cameraChunkPos.x + Chunk::CHUNK_SIZE * (j - CHUNK_COUNT / 2);
				int z = (int)cameraChunkPos.z + Chunk::CHUNK_SIZE * (k - CHUNK_COUNT / 2);

				if (m_chunkMap.find(std::make_tuple(x, y, z)) ==
					m_chunkMap.end()) { // found chunk to be loaded
					Chunk* chunk = GetChunkFromPool();
					if (chunk) {
						chunk->SetPosition(Vector3((float)x, (float)y, (float)z));

						m_chunkMap[std::make_tuple(x, y, z)] = chunk;
						m_loadChunkList.push_back(chunk);
					}
				}
				else
					loadedChunkMap[std::make_tuple(x, y, z)] = true;
			}
		}
	}

	for (auto& p : m_chunkMap) { // { 1, 2, 3 } -> { 1, 2 } : 3 unload
		if (loadedChunkMap.find(p.first) == loadedChunkMap.end() &&
			m_chunkMap[p.first]->IsLoaded()) {

			m_unloadChunkList.push_back(p.second);
		}
	}
}

void ChunkManager::UpdateLoadChunkList()
{
	int loadCount = 0;

	while (!m_loadChunkList.empty() && loadCount < MAX_ASYNC_LOAD_COUNT) {
		Chunk* chunk = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		////////////////////////////////////
		// check start time
		static long long sum = 0;
		static long long count = 0;
		auto start_time = std::chrono::steady_clock::now();
		////////////////////////////////////


		// load chunk
		chunk->Initialize();
		MakeBuffer(chunk);

		// set load value
		loadCount++;
		chunk->SetLoad(true);


		////////////////////////////////////
		// check end time
		auto end_time = std::chrono::steady_clock::now();
		auto duration =
			std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
		sum += duration.count();
		count++;
		std::cout << "Function Average duration: " << (double)sum / (double)count << " microseconds"
				  << std::endl;
		////////////////////////////////////
	}
}

void ChunkManager::UpdateUnloadChunkList()
{
	while (!m_unloadChunkList.empty()) {
		Chunk* chunk = m_unloadChunkList.back();
		m_unloadChunkList.pop_back();

		Vector3 pos = chunk->GetPosition();
		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;
		m_chunkMap.erase(std::make_tuple(x, y, z));

		ClearChunkBuffer(chunk);
		ReleaseChunkToPool(chunk);

		chunk->Clear();
		chunk->SetLoad(false);
	}
}

void ChunkManager::UpdateRenderChunkList(Camera& camera)
{
	m_renderChunkList.clear();

	for (auto& p : m_chunkMap) {
		if (!p.second->IsLoaded())
			continue;

		if (p.second->IsEmpty())
			continue;

		if (!FrustumCulling(p.second->GetPosition(), camera))
			continue;

		m_renderChunkList.push_back(p.second);
	}
}

void ChunkManager::UpdateInstanceInfoList(Camera& camera)
{
	// clear all info
	for (int i = 0; i < Block::INSTANCE_TYPE_COUNT; ++i)
		m_instanceInfoList[i].clear();

	// check instance in chunk managerList
	for (auto& c : m_renderChunkList) {
		// check distance
		Vector3 chunkOffset = c->GetPosition();
		Vector3 chunkCenterPosition = chunkOffset + Vector3(Chunk::CHUNK_SIZE * 0.5);
		Vector3 diffPosition = chunkCenterPosition - camera.GetPosition();
		if (diffPosition.Length() > (float)MAX_INSTANCE_RENDER_DISTANCE)
			continue;

		// set info
		const std::unordered_map<uint8_t, std::vector<Vector3>>& instanceMap = c->GetInstanceMap();
		for (auto& p : instanceMap) {
			uint8_t type = p.first;
			for (auto& pos : p.second) {
				InstanceInfo info;
				info.type = type;

				// type
				// noise position
				// noise scale
				// rotate?
				info.instanceWorld =
					Matrix::CreateTranslation(chunkOffset + pos + Vector3(0.5f)).Transpose();
				
				// Block 형태 instance
				// Cross 형태 instance
				// Fence 형태 instance
				// Square 형태 instance
				m_instanceInfoList[Block::GetInstanceType(type)].push_back(info);
			}
		}
	}

	for (int i = 0; i < Block::INSTANCE_TYPE_COUNT; ++i) {
		D3D11_BUFFER_DESC desc;
		m_instanceInfoBuffers[i]->GetDesc(&desc);

		UINT bufferInstanceCount = desc.ByteWidth / sizeof(InstanceInfo);
		if (m_instanceInfoList[i].size() > bufferInstanceCount) {
			m_instanceInfoBuffers[i].Reset();
			m_instanceInfoBuffers[i] = nullptr;

			DXUtils::CreateInstanceBuffer(
				m_instanceInfoBuffers[i], (UINT)m_instanceInfoList[i].size() + 1024);
		}

		DXUtils::UpdateInstanceBuffer(m_instanceInfoBuffers[i], m_instanceInfoList[i]);
	}
}

bool ChunkManager::FrustumCulling(Vector3 position, Camera& camera)
{
	Matrix invMat = camera.GetProjectionMatrix().Invert() * camera.GetViewMatrix().Invert();

	std::vector<Vector3> worldPos = { Vector3::Transform(Vector3(-1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 1.0f), invMat) };

	std::vector<Vector4> planes = { DirectX::XMPlaneFromPoints(
										worldPos[0], worldPos[1], worldPos[2]),
		DirectX::XMPlaneFromPoints(worldPos[7], worldPos[6], worldPos[5]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[5], worldPos[1]),
		DirectX::XMPlaneFromPoints(worldPos[3], worldPos[2], worldPos[6]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[0], worldPos[3]),
		DirectX::XMPlaneFromPoints(worldPos[1], worldPos[5], worldPos[6]) };

	for (int i = 0; i < 6; ++i) {
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position)) < 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 0.0f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 32.f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 32.0f, 0.0f))) <=
			0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 0.0f, 32.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 0.0f, 32.0f))) <=
			0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 32.f, 32.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 32.0f, 32.0f))) <=
			0.0f)
			continue;
		return false;
	}
	return true;
}

bool ChunkManager::MakeBuffer(Chunk* chunk)
{
	if (!chunk->IsEmpty()) {
		UINT id = chunk->GetID();
		
		ChunkConstantData tempConstantData = chunk->GetConstantData();
		tempConstantData.world = tempConstantData.world.Transpose();
		if (!DXUtils::CreateConstantBuffer(m_constantBuffers[id], tempConstantData)) {
			std::cout << "failed create constant buffer in chunk manager" << std::endl;
			return false;
		}

		// basic
		if (!chunk->IsEmptyBasic()) {
			if (!DXUtils::CreateVertexBuffer(m_basicVertexBuffers[id], chunk->GetBasicVertices())) {
				std::cout << "failed create vertex buffer in chunk manager" << std::endl;
				return false;
			}
			if (!DXUtils::CreateIndexBuffer(m_basicIndexBuffers[id], chunk->GetBasicIndices())) {
				std::cout << "failed create index buffer in chunk manager" << std::endl;
				return false;
			}
		}

		// water
		if (!chunk->IsEmptyWater()) {
			if (!DXUtils::CreateVertexBuffer(m_waterVertexBuffers[id], chunk->GetWaterVertices())) {
				std::cout << "failed create vertex buffer in chunk manager" << std::endl;
				return false;
			}
			if (!DXUtils::CreateIndexBuffer(m_waterIndexBuffers[id], chunk->GetWaterIndices())) {
				std::cout << "failed create index buffer in chunk manager" << std::endl;
				return false;
			}
		}
	}

	return true;
}

void ChunkManager::ClearChunkBuffer(Chunk* chunk)
{
	UINT id = chunk->GetID();

	m_basicVertexBuffers[id].Reset();
	m_basicIndexBuffers[id].Reset();
	m_waterVertexBuffers[id].Reset();
	m_waterIndexBuffers[id].Reset();
	m_constantBuffers[id].Reset();

	m_basicVertexBuffers[id] = nullptr;
	m_basicIndexBuffers[id] = nullptr;
	m_waterVertexBuffers[id] = nullptr;
	m_waterIndexBuffers[id] = nullptr;
	m_constantBuffers[id] = nullptr;
}

Chunk* ChunkManager::GetChunkFromPool()
{
	if (!m_chunkPool.empty()) {
		Chunk* chunk = m_chunkPool.back();
		m_chunkPool.pop_back();
		return chunk;
	}
	return nullptr;
}

void ChunkManager::ReleaseChunkToPool(Chunk* chunk) { m_chunkPool.push_back(chunk); }

bool ChunkManager::MakeInstanceVertexBuffer()
{
	std::vector<InstanceVertex> instanceVertices;
	std::vector<uint32_t> instanceIndices;

	// Instance Type 0 : BOX
	MeshGenerator::CreateBoxInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::BOX], instanceVertices)) {
		std::cout << "failed create box instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(m_instanceIndexBuffers[INSTANCE_TYPE::BOX], instanceIndices)) {
		std::cout << "failed create box instance index buffer in chunk manager" << std::endl;
		return false;
	}
	instanceVertices.clear();
	instanceIndices.clear();


	// Instance Type 1 : CROSS
	MeshGenerator::CreateCrossInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::CROSS], instanceVertices)) {
		std::cout << "failed create cross instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::CROSS], instanceIndices)) {
		std::cout << "failed create cross instance index buffer in chunk manager" << std::endl;
		return false;
	}
	instanceVertices.clear();
	instanceIndices.clear();


	// Instance Type 2 : FENCE
	MeshGenerator::CreateFenceInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::FENCE], instanceVertices)) {
		std::cout << "failed create fence instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::FENCE], instanceIndices)) {
		std::cout << "failed create fence instance index buffer in chunk manager" << std::endl;
		return false;
	}
	instanceVertices.clear();
	instanceIndices.clear();


	// Instance Type 3 : SQUARE
	MeshGenerator::CreateSquareInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::SQUARE], instanceVertices)) {
		std::cout << "failed create SQUARE instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::SQUARE], instanceIndices)) {
		std::cout << "failed create SQUARE instance index buffer in chunk manager" << std::endl;
		return false;
	}

	return true;
}

bool ChunkManager::MakeInstanceInfoBuffer()
{
	for (auto& instanceBuffer : m_instanceInfoBuffers) {
		if (!DXUtils::CreateInstanceBuffer(instanceBuffer, MAX_INSTANCE_BUFFER_COUNT)) { // 약 8MB
			std::cout << "failed create instance info buffer in chunk manager" << std::endl;
			return false;
		}
	}

	return true;
}