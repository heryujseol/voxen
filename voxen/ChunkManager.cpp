#include "ChunkManager.h"
#include "Graphics.h"
#include "Utils.h"

#include <iostream>

ChunkManager::ChunkManager() {}

ChunkManager::~ChunkManager() {}

bool ChunkManager::Initialize(Vector3 cameraChunkPos)
{
	for (int i = 0; i < (CHUNK_COUNT + 1) * (CHUNK_COUNT + 1) * (MAX_HEIGHT_CHUNK_COUNT + 1); ++i) {
		m_chunkPool.push_back(new Chunk());
	}

	UpdateChunkList(cameraChunkPos);

	return true;
}

void ChunkManager::Update(Camera& camera)
{
	if (camera.m_isOnChunkDirtyFlag) {
		UpdateChunkList(camera.GetChunkPosition());
		camera.m_isOnChunkDirtyFlag = false;
	}

	UpdateLoadChunks();
	UpdateUnloadChunks();
}

void ChunkManager::Render(Camera& camera)
{
	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::atlasMapSRV.Get(),
		Graphics::grassColorMapSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	for (auto& c : m_chunkMap) {
		if (!c.second->IsLoaded())
			continue;

		if (c.second->IsEmpty())
			continue;

		if (!FrustumCulling(c.second->GetPosition(), camera))
			continue;

		c.second->Render();
	}
}

void ChunkManager::UpdateChunkList(Vector3 cameraChunkPos)
{
	std::map<std::tuple<int, int, int>, bool> loadedChunkMap;
	for (int i = 0; i < MAX_HEIGHT_CHUNK_COUNT; ++i) {
		for (int j = 0; j < CHUNK_COUNT; ++j) {
			for (int k = 0; k < CHUNK_COUNT; ++k) {
				int y = CHUNK_SIZE * i;
				int x = (int)cameraChunkPos.x + CHUNK_SIZE * (j - CHUNK_COUNT / 2);
				int z = (int)cameraChunkPos.z + CHUNK_SIZE * (k - CHUNK_COUNT / 2);

				if (m_chunkMap.find(std::make_tuple(x, y, z)) == m_chunkMap.end()) { // loading
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

	for (auto& p : m_chunkMap) { // {1 , 2, 3} -> {1, 2}
		if (loadedChunkMap.find(p.first) == loadedChunkMap.end() &&
			m_chunkMap[p.first]->IsLoaded()) {

			m_unloadChunkList.push_back(p.second);
		}
	}
}

void ChunkManager::UpdateLoadChunks()
{
	for (auto it = m_loadFutures.begin(); it != m_loadFutures.end();) {
		if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			it = m_loadFutures.erase(it);
		else
			++it;
	}

	while (!m_loadChunkList.empty() && m_loadFutures.size() < MAX_ASYNC_LOAD_COUNT) {
		Chunk* chunk = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		m_loadFutures.push_back(std::async(std::launch::async, &Chunk::Initialize, chunk));
	}
}

void ChunkManager::UpdateUnloadChunks()
{
	while (!m_unloadChunkList.empty()) {
		Chunk* chunk = m_unloadChunkList.back();
		m_unloadChunkList.pop_back();

		Vector3 pos = chunk->GetPosition();
		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;

		chunk->Clear();
		m_chunkMap.erase(std::make_tuple(x, y, z));
		ReleaseChunkToPool(chunk);
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