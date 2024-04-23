#include "ChunkManager.h"
#include "Graphics.h"
#include "Utils.h"

#include <iostream>

ChunkManager::ChunkManager()
{
	m_loadFuture = std::async(std::launch::async, []() {});
}

ChunkManager::~ChunkManager() {}

bool ChunkManager::Initialize(Vector3 cameraChunkPos)
{
	for (int i = 0; i < MAX_HEIGHT_CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int y = Chunk::BLOCK_SIZE * i;
				int x = (int)cameraChunkPos.x + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraChunkPos.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				m_chunks[std::make_tuple(x, y, z)] = new Chunk(x, y, z);
				if (!m_chunks[std::make_tuple(x, y, z)]->Initialize())
					return false;
			}
		}
	}

	return true;
}

void ChunkManager::Update(Camera& camera)
{
	Vector3 diffChunkPos = camera.GetDiffChunkPos();
	int moveDirX = (int)diffChunkPos.x / Chunk::BLOCK_SIZE;
	int moveDirZ = (int)diffChunkPos.z / Chunk::BLOCK_SIZE;
	if (moveDirX != 0 || moveDirZ != 0) {
		UpdateChunkList(camera.GetChunkPosition(), moveDirX, moveDirZ);
	}

	if (!m_loadChunkList.empty()) {
		if (m_loadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			m_loadFuture = std::async(std::launch::async, &ChunkManager::LoadChunks, this);
		}
	}

	if (!m_unloadChunkList.empty()) {
		UnloadChunks();
	}
}

void ChunkManager::Render(Camera& camera)
{
	for (auto& c : m_chunks) {
		if (!c.second->IsLoaded())
			continue;

		if (c.second->IsEmpty())
			continue;

		if (!FrustumCulling(c.second->GetPosition(), camera))
			continue;

		c.second->Render();
	}
}

void ChunkManager::UpdateChunkList(Vector3 cameraChunkPos, int moveDirX, int moveDirZ)
{
	if (moveDirX != 0) {
		// unload
		int prevChunkPosX = (int)(cameraChunkPos.x - moveDirX * Chunk::BLOCK_SIZE);
		int nx = prevChunkPosX - moveDirX * Chunk::BLOCK_SIZE * (CHUNK_SIZE / 2);
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			int nz = (int)cameraChunkPos.z + Chunk::BLOCK_SIZE * (z - CHUNK_SIZE / 2);
			for (int y = 0; y < MAX_HEIGHT_CHUNK_SIZE; ++y) {
				int ny = Chunk::BLOCK_SIZE * y;

				m_unloadChunkList.push(Vector3((float)nx, (float)ny, (float)nz));
			}
		}

		// load
		nx = (int)cameraChunkPos.x + moveDirX * Chunk::BLOCK_SIZE * (CHUNK_SIZE / 2);
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			int nz = (int)cameraChunkPos.z + Chunk::BLOCK_SIZE * (z - CHUNK_SIZE / 2);
			
			int maxHeight = 0;
			for (int i = 0; i < Chunk::BLOCK_SIZE; ++i) {
				for (int j = 0; j < Chunk::BLOCK_SIZE; ++j) {
					maxHeight = max(Utils::GetHeight(nx + i, nz + j), maxHeight);
				}
			}

			int maxChunkHeight = maxHeight / Chunk::BLOCK_SIZE;
			for (int y = 0; y <= maxChunkHeight; ++y)
			{
				int ny = Chunk::BLOCK_SIZE * y;

				m_loadChunkList.push(Vector3((float)nx, (float)ny, (float)nz));
				m_chunks[std::make_tuple(nx, ny, nz)] = new Chunk(nx, ny, nz);
			}
		}
	}


	if (moveDirZ != 0) {
		// unload
		int prevChunkPosZ = (int)(cameraChunkPos.z - moveDirZ * Chunk::BLOCK_SIZE);
		int nz = prevChunkPosZ - moveDirZ * Chunk::BLOCK_SIZE * (CHUNK_SIZE / 2);
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			int nx = (int)cameraChunkPos.x + Chunk::BLOCK_SIZE * (x - CHUNK_SIZE / 2);
			for (int y = 0; y < MAX_HEIGHT_CHUNK_SIZE; ++y) {
				int ny = Chunk::BLOCK_SIZE * y;

				m_unloadChunkList.push(Vector3((float)nx, (float)ny, (float)nz));
			}
		}

		// load
		nz = (int)cameraChunkPos.z + moveDirZ * Chunk::BLOCK_SIZE * (CHUNK_SIZE / 2);
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			int nx = (int)cameraChunkPos.x + Chunk::BLOCK_SIZE * (x - CHUNK_SIZE / 2);
			int maxHeight = 0;
			for (int i = 0; i < Chunk::BLOCK_SIZE; ++i) {
				for (int j = 0; j < Chunk::BLOCK_SIZE; ++j) {
					maxHeight = max(Utils::GetHeight(nx + i, nz + j), maxHeight);
				}
			}

			int maxChunkHeight = maxHeight / Chunk::BLOCK_SIZE;
			for (int y = 0; y <= maxChunkHeight; ++y) {
				int ny = Chunk::BLOCK_SIZE * y;

				m_chunks[std::make_tuple(nx, ny, nz)] = new Chunk(nx, ny, nz);
				m_loadChunkList.push(Vector3((float)nx, (float)ny, (float)nz));
			}
		}
	}
}

void ChunkManager::LoadChunks()
{
	while (!m_loadChunkList.empty()) {
		Vector3 pos = m_loadChunkList.front();
		m_loadChunkList.pop();

		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;

		auto it = m_chunks.find(std::make_tuple(x, y, z));
		if (it != m_chunks.end() && !it->second->IsLoaded()) {
			m_chunks[std::make_tuple(x, y, z)]->Initialize();
		}
	}
}

void ChunkManager::UnloadChunks()
{
	int count = 0;
	while (!m_unloadChunkList.empty()) {
		Vector3 pos = m_unloadChunkList.front();
		m_unloadChunkList.pop();

		auto it = m_chunks.find(std::make_tuple((int)pos.x, (int)pos.y, (int)pos.z));
		if (it != m_chunks.end()) {
			if (it->second->IsLoaded()) {
				delete it->second;
			}

			m_chunks.erase(it);
			count++;
			if (count == 8)
				return;
		}
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