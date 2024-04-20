#include "ChunkManager.h"
#include "Graphics.h"

#include <iostream>

ChunkManager::ChunkManager()
{	
	m_loadFuture = std::async(std::launch::async, []() {});
}

ChunkManager::~ChunkManager() {}

bool ChunkManager::Initialize(Vector3 cameraOffset)
{
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int x = (int)cameraOffset.x + Chunk::BLOCK_SIZE * (i - CHUNK_SIZE / 2);
				int y = (int)cameraOffset.y + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraOffset.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				m_chunks[std::make_tuple(x, y, z)] = Chunk(x, y, z);
				if (!m_chunks[std::make_tuple(x, y, z)].Initialize())
					return false;
			}
		}
	}

	return true;
}

void ChunkManager::Update(Camera& camera)
{
	if (camera.IsOnChunkDirtyFlag()) {
		UpdateChunkList(camera.GetChunkPosition());
		camera.OffChunkDirtyFlag();

		if (!m_unloadChunkList.empty()) {
			UnloadChunks();
		}
	}

	if (!m_loadChunkList.empty()) {
		if (m_loadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			m_loadFuture = std::async(std::launch::async, &ChunkManager::LoadChunks, this);
		}
	}
}

void ChunkManager::Render(Camera& camera)
{
	for (auto& c : m_chunks) {
		if (c.second.IsEmpty() || !c.second.IsLoaded())
			continue;

		if (!FrustumCulling(c.second.GetPosition(), camera))
			continue;
		c.second.Render();
	}
}

void ChunkManager::LoadChunks()
{
	int count = 0;
	while (!m_loadChunkList.empty()) {
		Vector3 pos = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;
		m_chunks[std::make_tuple(x, y, z)] = Chunk(x, y, z);
		m_chunks[std::make_tuple(x, y, z)].Initialize();
		count++;

		if (count == 1) //  chunks loading per each frame
			return;
	}
}

void ChunkManager::UnloadChunks()
{
	for (int i = 0; i < m_unloadChunkList.size(); ++i) {
		Vector3 pos = m_unloadChunkList[i];
		auto position = std::make_tuple((int)pos.x, (int)pos.y, (int)pos.z);

		m_chunks.erase(position);
	}
	m_unloadChunkList.clear();
}

void ChunkManager::UpdateChunkList(Vector3 cameraOffset)
{
	std::map<std::tuple<int, int, int>, bool> loadedChunkMap;
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int x = (int)cameraOffset.x + Chunk::BLOCK_SIZE * (i - CHUNK_SIZE / 2);
				int y = (int)cameraOffset.y + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraOffset.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				if (m_chunks.find(std::make_tuple(x, y, z)) == m_chunks.end())
					m_loadChunkList.push_back(
						Vector3((float)x, (float)y, (float)z)); // will be loaded
				else
					loadedChunkMap[std::make_tuple(x, y, z)] = true;
			}
		}
	}

	for (auto& p : m_chunks) { // {1 , 2, 3} -> {1, 2}
		if (loadedChunkMap.find(p.first) == loadedChunkMap.end() && m_chunks[p.first].IsLoaded()) {
			int x = std::get<0>(p.first);
			int y = std::get<1>(p.first);
			int z = std::get<2>(p.first);

			m_unloadChunkList.push_back(Vector3((float)x, (float)y, (float)z));
		}
	}
}

bool ChunkManager::FrustumCulling(Vector3 position, Camera& camera) {
	
	Matrix invMat = camera.GetProjectionMatrix().Invert() * camera.GetViewMatrix().Invert();

	std::vector<Vector3> worldPos = {
		Vector3::Transform(Vector3(-1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 1.0f), invMat)
	};

	std::vector<Vector4> planes = { 
		DirectX::XMPlaneFromPoints(worldPos[0], worldPos[1], worldPos[2]),
		DirectX::XMPlaneFromPoints(worldPos[7], worldPos[6], worldPos[5]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[5], worldPos[1]),
		DirectX::XMPlaneFromPoints(worldPos[3], worldPos[2], worldPos[6]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[0], worldPos[3]),
		DirectX::XMPlaneFromPoints(worldPos[1], worldPos[5], worldPos[6])
	};

	for (int i = 0; i < 6; ++i) {
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position)) < 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 0.0f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 32.f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 32.0f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 0.0f, 32.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 0.0f, 32.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 32.f, 32.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(32.0f, 32.0f, 32.0f))) <= 0.0f)
			continue;
		return false;
	}
	return true;
}