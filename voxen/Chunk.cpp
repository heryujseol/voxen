#include "Chunk.h"
#include "DXUtils.h"

#include <future>

Chunk::Chunk()
	: m_position(0.0, 0.0, 0.0), m_stride(0), m_offset(0), m_indexCount(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr), m_constantBuffer(nullptr), m_isLoaded(false)
{
}

Chunk::~Chunk() { Clear(); }

inline int GetIndexFrom3D(int axis, int y, int x, int length)
{
	return (length * length) * axis + length * y + x;
}

inline int TrailingZeros(uint64_t num)
{
	if (num == 0)
		return 64;
	return (int)log2(num & ((~num) + 1)); // __builtin_ctzll or _BitScanForward64
}

inline int TrailingOnes(uint64_t num)
{
	return (int)log2((num & ~(num + 1)) + 1); // __builtin_ctzll or _BitScanForward64}
}

bool Chunk::Initialize_NO_OPT()
{
	auto start_time = std::chrono::steady_clock::now();
	std::vector<Vector3> activeBlocks;
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			int height = Utils::GetHeight((int)m_position.x + x, (int)m_position.z + z);
			for (int y = 0; y < CHUNK_SIZE; ++y) {
				if (0 <= m_position.y + y && m_position.y + y <= height) {
					m_blocks[x][y][z].SetActive(true);
					activeBlocks.push_back(Vector3((float)x, (float)y, (float)z));
				}
				else {
					m_blocks[x][y][z].SetActive(false);
				}
			}
		}
	}

	for (auto it = activeBlocks.begin(); it != activeBlocks.end(); ++it) {
		int x = (int)it->x;
		int y = (int)it->y;
		int z = (int)it->z;

		bool x_n = true, x_p = true, y_n = true, y_p = true, z_n = true, z_p = true;
		if (x > 0 && m_blocks[x - 1][y][z].IsActive())
			x_n = false;
		if (x < CHUNK_SIZE - 1 && m_blocks[x + 1][y][z].IsActive())
			x_p = false;
		if (y - 1 >= 0 && m_blocks[x][y - 1][z].IsActive())
			y_n = false;
		if (y < CHUNK_SIZE - 1 && m_blocks[x][y + 1][z].IsActive())
			y_p = false;
		if (z > 0 && m_blocks[x][y][z - 1].IsActive())
			z_n = false;
		if (z < CHUNK_SIZE - 1 && m_blocks[x][y][z + 1].IsActive())
			z_p = false;
		CreateBlock(x, y, z, x_n, x_p, y_n, y_p, z_n, z_p);
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
	auto end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	std::cout << "Function duration: " << duration.count() << " microseconds" << std::endl;
	return true;
}

bool Chunk::Initialize()
{
	auto start_time = std::chrono::steady_clock::now();

	// 1. make axis column bit data
	std::vector<uint64_t> axisColBit(CHUNK_SIZE_P2 * 3, 0);
	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int z = 0; z < CHUNK_SIZE_P; ++z) {
			int height = Utils::GetHeight((int)m_position.x + x - 1, (int)m_position.z + z - 1);

			for (int y = 0; y < CHUNK_SIZE_P; ++y) {
				m_blocks[x][y][z].SetActive(1 <= m_position.y + y && m_position.y + y <= height);

				if (m_blocks[x][y][z].IsActive()) {
					// x dir column
					axisColBit[GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					// y dir column
					axisColBit[GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					// z dir column
					axisColBit[GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
			}
		}
	}


	// 2. cull face column bit
	// 0: x axis & left->right side (- => + : dir +)
	// 1: x axis & right->left side (+ => - : dir -)
	// 2: y axis & bottom->top side (- => + : dir +)
	// 3: y axis & top->bottom side (+ => - : dir -)
	// 4: z axis & front->back side (- => + : dir +)
	// 5: z axis & back->front side (+ => - : dir -)
	std::vector<uint64_t> cullColBit(CHUNK_SIZE_P2 * 6, 0);
	for (int axis = 0; axis < 3; ++axis) {
		for (int h = 1; h < CHUNK_SIZE_P - 1; ++h) {
			for (int w = 1; w < CHUNK_SIZE_P - 1; ++w) {
				uint64_t colBit = axisColBit[GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];

				cullColBit[GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					colBit & ~(colBit << 1);
				cullColBit[GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					colBit & ~(colBit >> 1);
			}
		}
	}


	// 3. build face culled bit slices column
	/*
	 *     ---------------
	 *    / x    y    z /| <= 5:face, h:1, w:2 bits
	 *   / d    e    f / |
	 *  | 4    5    6 |  |
	 *  |			  |  |	     xi   jy  zk (3rd z slice - => +)
	 *  |   i    j    |k /  =>   da   eb  fc (2nd z slice - => +)   => bitPos->slice, h->shift, w->w
	 *  |  a    b    c| /        41   52  63 (1st z slice - => +)
	 *  | 1    2    3 |/
	 *  ---------------
	 */
	std::vector<uint64_t> faceColbit(CHUNK_SIZE2 * 6, 0);
	for (int face = 0; face < 6; ++face) {
		for (int h = 0; h < CHUNK_SIZE; ++h) {
			for (int w = 0; w < CHUNK_SIZE; ++w) { // 34bit: P,CHUNK_SIZE,P
				uint64_t culledBit = cullColBit[GetIndexFrom3D(face, h + 1, w + 1, CHUNK_SIZE_P)];
				culledBit = culledBit >> 1;					   // 33bit: P,CHUNK_SIZE
				culledBit = culledBit & ~(1ULL << CHUNK_SIZE); // 32bit: CHUNK_SIZE

				while (culledBit) {
					int bitPos = TrailingZeros(culledBit);		// 1110001000 -> trailing zero : 3
					culledBit = culledBit & (culledBit - 1ULL); // 1110000000

					faceColbit[GetIndexFrom3D(face, bitPos, w, CHUNK_SIZE)] |= (1ULL << h);
				}
			}
		}
	}


	// 4. make vertices by bit slices column
	// face 0, 1 : left-right
	// face 2, 3 : top-bottom
	// face 4, 5 : front-back
	for (int face = 0; face < 6; ++face) {
		for (int s = 0; s < CHUNK_SIZE; ++s) {
			for (int i = 0; i < CHUNK_SIZE; ++i) {
				uint64_t faceBit = faceColbit[GetIndexFrom3D(face, s, i, CHUNK_SIZE)];
				int step = 0;
				while (step < CHUNK_SIZE) {			  // 111100011100
					step += TrailingZeros(faceBit >> step); // 1111000111|00| -> 2
					if (step >= CHUNK_SIZE)
						break ;
					
					uint64_t ones = TrailingOnes((faceBit >> step)); // 1111000|111|00 -> 3
					uint64_t submask = ((1ULL << ones) - 1ULL) << step;	 // 111 << 2 -> 11100
					uint64_t w = 1;
					while (i + w < CHUNK_SIZE) {
						uint64_t cb = faceColbit[GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] & submask;
						if (cb != submask)
							break;

						faceColbit[GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] &= (~submask);
						w++;
					}

					if (face == 0)
						CreateQuad(s, step, i, w, ones, face);
					else if (face == 1)
						CreateQuad(s + 1, step, i, w, ones, face);
					else if (face == 2)
						CreateQuad(i, s, step, w, ones, face);
					else if (face == 3)
						CreateQuad(i, s + 1, step, w, ones, face);
					else if (face == 4)
						CreateQuad(i, step, s, w, ones, face);
					else // face == 5 
						CreateQuad(i, step, s + 1, w, ones, face);

					step += ones;
				}
			}
		}
	}
	

	// 5. make GPU buffer from CPU buffer
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

	auto end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	std::cout << "Function duration: " << duration.count() << " microseconds" << std::endl;

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

void Chunk::Clear()
{
	m_isLoaded = false;

	std::vector<Vertex>().swap(m_vertices); // clear container reallocating
	std::vector<uint32_t>().swap(m_indices);

	if (m_vertexBuffer) {
		m_vertexBuffer.ReleaseAndGetAddressOf();
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer) {
		m_vertexBuffer.ReleaseAndGetAddressOf();
		m_indexBuffer = nullptr;
	}
	if (m_constantBuffer) {
		m_vertexBuffer.ReleaseAndGetAddressOf();
		m_constantBuffer = nullptr;
	}
}

void Chunk::CreateQuad(int x, int y, int z, int merged, int length, int face)
{
	int originVertexSize = (int)m_vertices.size();

	Vertex v;
	if (face == 0) { // left -> right
		v.normal = Vector3(-1.0f, 0.0f, 0.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y, z + merged);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z + merged);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z);
		m_vertices.push_back(v);
	}
	else if (face == 1) { // right -> left
		v.normal = Vector3(1.0f, 0.0f, 0.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z + merged);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y, z + merged);
		m_vertices.push_back(v);
	}
	else if (face == 2) {
		v.normal = Vector3(0.0f, -1.0f, 0.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z + length);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y, z + length);
		m_vertices.push_back(v);
	}
	else if (face == 3) {
		v.normal = Vector3(0.0f, 1.0f, 0.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y, z + length);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z + length);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z);
		m_vertices.push_back(v);
	}
	else if (face == 4) {
		v.normal = Vector3(0.0f, 0.0f, -1.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z); // merged -> merged
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y + length, z); // h -> length
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z);
		m_vertices.push_back(v);
	}
	else if (face == 5) {
		v.normal = Vector3(0.0f, 0.0f, 1.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y + length, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z);
		m_vertices.push_back(v);
	}

	m_indices.push_back(originVertexSize);
	m_indices.push_back(originVertexSize + 1);
	m_indices.push_back(originVertexSize + 2);

	m_indices.push_back(originVertexSize);
	m_indices.push_back(originVertexSize + 2);
	m_indices.push_back(originVertexSize + 3);
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
	for (int i = 0; i < newVertexCount; i += 4) {
		m_indices.push_back(originVertexSize + i);
		m_indices.push_back(originVertexSize + i + 1);
		m_indices.push_back(originVertexSize + i + 2);

		m_indices.push_back(originVertexSize + i);
		m_indices.push_back(originVertexSize + i + 2);
		m_indices.push_back(originVertexSize + i + 3);
	}
}
