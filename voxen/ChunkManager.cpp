#include "ChunkManager.h"


ChunkManager::ChunkManager()
{	
	m_loadFuture = std::async(std::launch::async, []() {});
}

ChunkManager::~ChunkManager() {}

void ChunkManager::Initialize(ComPtr<ID3D11Device>& device, Vector3 cameraOffset)
{

	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int x = (int)cameraOffset.x + Chunk::BLOCK_SIZE * (i - CHUNK_SIZE / 2);
				int y = (int)cameraOffset.y + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraOffset.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				m_chunks[std::make_tuple(x, y, z)] = Chunk(x, y, z);
				m_chunks[std::make_tuple(x, y, z)].Initialize(device);
			}
		}
	}
}

void ChunkManager::Update(ComPtr<ID3D11Device>& device, Camera& camera)
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
			m_loadFuture = std::async(std::launch::async, &ChunkManager::LoadChunks, this, std::ref(device));
		}
	}
}

void ChunkManager::Render(ComPtr<ID3D11DeviceContext>& context)
{
	for (auto& c : m_chunks) {
		if (c.second.IsEmpty() || !c.second.IsLoaded())
			continue;

		c.second.Render(context);
	}
}

void ChunkManager::LoadChunks(ComPtr<ID3D11Device>& device)
{
	int count = 0;
	while (!m_loadChunkList.empty()) {
		Vector3 pos = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;
		m_chunks[std::make_tuple(x, y, z)] = Chunk(x, y, z);
		m_chunks[std::make_tuple(x, y, z)].Initialize(device);
		count++;

		if (count == 3) //  chunks loading per each frame
			return;
	}
}

void ChunkManager::UnloadChunks()
{
	for (int i = 0; i < m_unloadChunkList.size(); ++i) {
		Vector3 pos = m_unloadChunkList[i];
		auto position = std::make_tuple((int)pos.x, (int)pos.y, (int)pos.z);

		m_chunks[position].~Chunk();
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