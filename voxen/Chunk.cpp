#include "Chunk.h"
#include "DXUtils.h"

#include <future>

Chunk::Chunk()
	: m_position(0.0, 0.0, 0.0), m_stride(sizeof(Vertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr), m_constantBuffer(nullptr), m_isLoaded(false)
{
}

Chunk::~Chunk() { Clear(); }

bool Chunk::Initialize()
{
	static long long sum = 0;
	static long long count = 0;
	//auto start_time = std::chrono::steady_clock::now();

	// 1. make axis column bit data
	std::vector<uint64_t> axisColBit(CHUNK_SIZE_P2 * 3, 0);
	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int z = 0; z < CHUNK_SIZE_P; ++z) {
			int height = Utils::GetHeight((int)m_position.x + x - 1, (int)m_position.z + z - 1);
			for (int y = 0; y < CHUNK_SIZE_P; ++y) {
				m_blocks[x][y][z].SetActive(1 <= m_position.y + y && m_position.y + y <= height);
				m_blocks[x][y][z].SetType(x, y, z);
				if (m_blocks[x][y][z].IsActive()) {
					// x dir column
					axisColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					// y dir column
					axisColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					// z dir column
					axisColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
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
				uint64_t colBit = axisColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];

				cullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					colBit & ~(colBit << 1);
				cullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
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
			for (int w = 0; w < CHUNK_SIZE; ++w) {
				uint64_t culledBit = // 34bit: P,CHUNK_SIZE,P
					cullColBit[Utils::GetIndexFrom3D(face, h + 1, w + 1, CHUNK_SIZE_P)];
				culledBit = culledBit >> 1;					   // 33bit: P,CHUNK_SIZE
				culledBit = culledBit & ~(1ULL << CHUNK_SIZE); // 32bit: CHUNK_SIZE

				while (culledBit) {
					int bitPos = Utils::TrailingZeros(culledBit); // 1110001000 -> trailing zero : 3
					culledBit = culledBit & (culledBit - 1ULL);	  // 1110000000

					faceColbit[Utils::GetIndexFrom3D(face, bitPos, w, CHUNK_SIZE)] |= (1ULL << h);
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
				uint64_t faceBit = faceColbit[Utils::GetIndexFrom3D(face, s, i, CHUNK_SIZE)];
				int step = 0;
				while (step < CHUNK_SIZE) {						   // 111100011100
					step += Utils::TrailingZeros(faceBit >> step); // 1111000111|00| -> 2
					if (step >= CHUNK_SIZE)
						break;

					int ones = Utils::TrailingOnes((faceBit >> step)); // 1111000|111|00 -> 3
					uint64_t submask = ((1ULL << ones) - 1ULL) << step;		// 111 << 2 -> 11100

					int w = 1;
					while (i + w < CHUNK_SIZE) {
						uint64_t cb =
							faceColbit[Utils::GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] & submask;
						if (cb != submask)
							break;

						faceColbit[Utils::GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] &= (~submask);
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
	if (!IsEmpty()) {
		if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
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

	//auto end_time = std::chrono::steady_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	//sum += duration.count();
	//count++;
	//std::cout << "Function Average duration: " << (double)sum / (double)count << " microseconds"
	//		  << std::endl;

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

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
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

void Chunk::CreateQuad(int ix, int iy, int iz, int merged, int length, int face)
{
	int originVertexSize = (int)m_vertices.size();

	float x = (float)ix;
	float y = (float)iy;
	float z = (float)iz;

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
	else if (face == 2) { // bottom -> top
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
	else if (face == 3) { // top -> bottom
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
	else if (face == 4) { // front -> back
		v.normal = Vector3(0.0f, 0.0f, -1.0f);

		v.pos = Vector3(x, y, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x, y + length, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y + length, z);
		m_vertices.push_back(v);

		v.pos = Vector3(x + merged, y, z);
		m_vertices.push_back(v);
	}
	else if (face == 5) { // back -> front
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