#include "Chunk.h"
#include "DXUtils.h"

Chunk::Chunk(int x, int y, int z)
	: m_position((float)x, (float)y, (float)z), m_stride(0), m_offset(0), m_indexCount(0),
	  m_vertexBuffer(nullptr), m_indexBuffer(nullptr), m_constantBuffer(nullptr), m_isLoaded(false)
{
}

Chunk::~Chunk()
{
	m_blocks.clear();
	m_vertices.clear();
	m_indices.clear();

	if (m_vertexBuffer) {
		m_vertexBuffer.Reset();
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer) {
		m_vertexBuffer.Reset();
		m_indexBuffer = nullptr;
	}
	if (m_constantBuffer) {
		m_vertexBuffer.Reset();
		m_constantBuffer = nullptr;
	}
}

bool Chunk::Initialize()
{
	m_blocks.resize(BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE; ++i) {
		m_blocks[i].resize(BLOCK_SIZE);
		for (int j = 0; j < BLOCK_SIZE; ++j) {
			m_blocks[i][j].resize(BLOCK_SIZE);
		}
	}

	for (int x = 0; x < BLOCK_SIZE; ++x) {
		for (int z = 0; z < BLOCK_SIZE; ++z) {
			int height = Utils::GetHeight((int)m_position.x + x, (int)m_position.z + z);
			for (int y = 0; y < BLOCK_SIZE && m_position.y + y <= height; ++y) {
				m_blocks[x][y][z].SetActive(true);
			}
		}
	}

	for (int x = 0; x < BLOCK_SIZE; ++x) {
		for (int y = 0; y < BLOCK_SIZE; ++y) {
			for (int z = 0; z < BLOCK_SIZE; ++z) {
				if (m_blocks[x][y][z].IsActive()) {
					bool x_n = true, x_p = true, y_n = true, y_p = true, z_n = true, z_p = true;
					if (x > 0 && m_blocks[x - 1][y][z].IsActive())
						x_n = false;
					if (x < BLOCK_SIZE - 1 && m_blocks[x + 1][y][z].IsActive())
						x_p = false;
					if (y - 1 >= 0 && m_blocks[x][y - 1][z].IsActive())
						y_n = false;
					if (y < BLOCK_SIZE - 1 && m_blocks[x][y + 1][z].IsActive())
						y_p = false;
					if (z > 0 && m_blocks[x][y][z - 1].IsActive())
						z_n = false;
					if (z < BLOCK_SIZE - 1 && m_blocks[x][y][z + 1].IsActive())
						z_p = false;
					CreateBlock(x, y, z, x_n, x_p, y_n, y_p, z_n, z_p);
				}
			}
		}
	}


	m_indexCount = m_indices.size();
	if (m_indexCount != 0) {
		if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices, m_stride, m_offset)) {
			std::cout << "failed create vertex buffer in chunk" << std::endl;
			return false;
		}

		if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
			std::cout << "failed create index buffer in chunk" << std::endl;
			return false;
		}

		m_constantData.world = Matrix::CreateTranslation(m_position).Transpose();
		if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
			std::cout << "failed create constant buffer in chunk" << std::endl;
			return false;
		}
		m_constantData.world = m_constantData.world.Transpose();
	}

	m_isLoaded = true;
	return true;
}

void Chunk::Update(float dt)
{
	m_constantData.world *= Matrix::CreateRotationY(dt);

	ChunkConstantData transposedConstantData = m_constantData;
	transposedConstantData.world = transposedConstantData.world.Transpose();
	DXUtils::UpdateConstantBuffer(m_constantBuffer, transposedConstantData);
}

void Chunk::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
	Graphics::context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	Graphics::context->DrawIndexed((UINT)m_indexCount, 0, 0);
}

void Chunk::CreateBlock(
	int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p)
{
	// À­¸é
	int originVertexSize = (int)m_vertices.size();

	if (y_p) {
		Vertex v;
		v.normal = Vector3(0.0f, 1.0f, 0.0f);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);
	}


	// ¾Æ·§¸é
	if (y_n) {
		Vertex v;
		v.normal = Vector3(0.0f, -1.0f, 0.0f);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);
	}


	// ¾Õ¸é
	if (z_n) {
		Vertex v;
		v.normal = Vector3(0.0f, 0.0f, -1.0f);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);
	}


	// µÞ¸é
	if (z_p) {
		Vertex v;
		v.normal = Vector3(0.0f, 0.0f, 1.0f);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);
	}


	// ¿ÞÂÊ
	if (x_n) {
		Vertex v;
		v.normal = Vector3(-1.0f, 0.0f, 0.0f);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 0.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);
	}


	// ¿À¸¥ÂÊ
	if (x_p) {
		Vertex v;
		v.normal = Vector3(1.0f, 0.0f, 0.0f);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 0.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 1.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 1.0f);
		m_vertices.push_back(v);

		v.pos = Vector3(x + 1.0f, y + 0.0f, z + 0.0f);
		m_vertices.push_back(v);
	}

	int newVertexCount = (int)m_vertices.size() - originVertexSize;
	std::vector<uint32_t> indices;
	for (int i = 0; i < newVertexCount; i += 4) {
		m_indices.push_back(originVertexSize + i);
		m_indices.push_back(originVertexSize + i + 1);
		m_indices.push_back(originVertexSize + i + 2);

		m_indices.push_back(originVertexSize + i);
		m_indices.push_back(originVertexSize + i + 2);
		m_indices.push_back(originVertexSize + i + 3);
	}
}

bool Chunk::IsLoaded() { return m_isLoaded; }

bool Chunk::IsEmpty() { return m_indexCount == 0; }

Vector3 Chunk::GetPosition() { return m_position; }
