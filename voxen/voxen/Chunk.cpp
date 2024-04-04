#include "Chunk.h"

Chunk::Chunk() : m_position(0.0f, 0.0f, 0.0f), m_stride(0), m_offset(0), m_indexCount(0) {}

Chunk::Chunk(int x, int y, int z) : m_position((float)x, (float)y, (float)z), m_stride(0), m_offset(0), m_indexCount(0)
{
}

Chunk::~Chunk() {}

bool Chunk::Initialize(ComPtr<ID3D11Device>& device)
{
	m_constantData.world = Matrix::CreateTranslation(m_position);
	m_constantData.invTrans = Matrix();

	for (int i = 0; i < BLOCK_SIZE; ++i) {
		for (int j = 0; j < BLOCK_SIZE; ++j) {
			for (int k = 0; k < BLOCK_SIZE; ++k) {
				CreateBlock(i, j, k, m_vertices, m_indices);
			}
		}
	}

	if (!Utils::CreateVertexBuffer(device, m_vertexBuffer, m_vertices, m_stride, m_offset)) {
		std::cout << "failed create vertex buffer in chunk" << std::endl;
		return false;
	}

	m_indexCount = m_indices.size();
	if (!Utils::CreateIndexBuffer(device, m_indexBuffer, m_indices)) {
		std::cout << "failed create index buffer in chunk" << std::endl;
		return false;
	}

	if (!Utils::CreateConstantBuffer(device, m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in chunk" << std::endl;
		return false;
	}

	return true;
}

void Chunk::Update(ComPtr<ID3D11DeviceContext>& context, float dt)
{ 
	Vector3 originTranslation = m_constantData.world.Translation();

	m_constantData.world *= Matrix::CreateTranslation(-originTranslation);
	m_constantData.world *= Matrix::CreateRotationY(dt);
	m_constantData.world *= Matrix::CreateTranslation(originTranslation);
	
	ChunkConstantData transConstantData;
	transConstantData.world = m_constantData.world.Transpose();
	Utils::UpdateConstantBuffer(context, m_constantBuffer, transConstantData);
}

void Chunk::Render(ComPtr<ID3D11DeviceContext>& context)
{
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
	context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	context->DrawIndexed(m_indexCount, 0, 0);
}

void Chunk::CreateBlock(
	int x, int y, int z, std::vector<Vertex>& out_vertices, std::vector<uint32_t>& out_indices)
{
	std::vector<Vector3> positions;
	std::vector<Vector3> colors;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords; // ÅØ½ºÃç ÁÂÇ¥

	// À­¸é
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 0.0f));
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ¾Æ·§¸é
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 0.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 0.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 1.0f));
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ¾Õ¸é
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 0.0f));
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 0.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 0.0f));
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 0.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// µÞ¸é
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 1.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ¿ÞÂÊ
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 0.0f, y + 1.0f, z + 0.0f));
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 0.0f));
	positions.push_back(Vector3(x + 0.0f, y + 0.0f, z + 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ¿À¸¥ÂÊ
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 0.0f));
	positions.push_back(Vector3(x + 1.0f, y + 1.0f, z + 1.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 1.0f));
	positions.push_back(Vector3(x + 1.0f, y + 0.0f, z + 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	uint32_t m_indices[36] = {
		0, 1, 2, 0, 2, 3,		// À­¸é
		4, 5, 6, 4, 6, 7,		// ¾Æ·§¸é
		8, 9, 10, 8, 10, 11,	// ¾Õ¸é
		12, 13, 14, 12, 14, 15, // µÞ¸é
		16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
		20, 21, 22, 20, 22, 23	// ¿À¸¥ÂÊ
	};

	uint32_t offsetIndex = out_vertices.size();
	for (size_t i = 0; i < 36; ++i) {
		out_indices.push_back(m_indices[i] + offsetIndex);
	}

	for (size_t i = 0; i < positions.size(); ++i) {
		Vertex v;
		v.pos = positions[i];
		v.color = colors[i];
		v.normal = normals[i];
		v.texcoord = texcoords[i];
		out_vertices.push_back(v);
	}
}

/*
*




*/