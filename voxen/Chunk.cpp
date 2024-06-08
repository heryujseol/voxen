#include "Chunk.h"
#include "DXUtils.h"
#include "MeshGenerator.h"

#include <future>
#include <algorithm>
#include <unordered_map>

Chunk::Chunk(UINT id) : m_id(id), m_position(0.0, 0.0, 0.0), m_isLoaded(false) {}

Chunk::~Chunk() { Clear(); }

void Chunk::Initialize()
{
	// 0. initialize chunk data
	InitChunkData();

	// 1. intialize sprite vertcie data
	InitSpriteVerticesData();

	// 2. initialize mesh(basic & water) vertice data
	InitMeshVerticesData();

	// 3. initialize constant data
	m_constantData.world = Matrix::CreateTranslation(m_position);
}

void Chunk::Update(float dt)
{
	// m_constantData.world *= Matrix::CreateRotationY(dt);
}

void Chunk::Clear()
{
	m_basicVertices.clear();
	m_basicIndices.clear();

	m_waterVertices.clear();
	m_waterIndices.clear();

	m_spriteVertices.clear();
}

void Chunk::InitChunkData()
{
	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int z = 0; z < CHUNK_SIZE_P; ++z) {
			int nx = (int)m_position.x + x - 1;
			int nz = (int)m_position.z + z - 1;

			int height = Terrain::GetHeight(nx, nz);
			float t = Terrain::GetPerlinNoise2((float)nx / 182.0f, (float)nz / 182.0f);

			for (int y = 0; y < CHUNK_SIZE_P; ++y) {
				m_blocks[x][y][z].SetType(0);

				int ny = (int)m_position.y + y;
				if (-64 <= ny && (ny <= height || height <= 62)) {
					uint8_t type = Terrain::GetType(nx, ny, nz, height, t);

					m_blocks[x][y][z].SetType(type);
				}
				// for sprite testing
				if (ny == height + 1 && nx % 2 == 0 && nz % 2 == 0) {
					m_blocks[x][y][z].SetType(128);
				}
			}
		}
	}
}
void Chunk::InitSpriteVerticesData()
{
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_SIZE; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				uint8_t type = m_blocks[x + 1][y + 1][z + 1].GetType();

				if (Block ::IsSprite(type)) {}
			}
		}
	}
}

void Chunk::InitMeshVerticesData()
{
	// 1. make axis column bit data
	static uint64_t basicAxisColBit[CHUNK_SIZE_P2 * 3];
	static uint64_t waterAxisColBit[CHUNK_SIZE_P2 * 3];

	std::fill(basicAxisColBit, basicAxisColBit + CHUNK_SIZE_P2 * 3, 0);
	std::fill(waterAxisColBit, waterAxisColBit + CHUNK_SIZE_P2 * 3, 0);
	std::unordered_map<uint8_t, bool> typeMap;

	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int y = 0; y < CHUNK_SIZE_P; ++y) {
			for (int z = 0; z < CHUNK_SIZE_P; ++z) {
				uint8_t type = m_blocks[x][y][z].GetType();
				if (type == Block::Type::AIR || Block::IsSprite(type))
					continue;

				typeMap[type] = true;
				if (type == Block::Type::WATER) {
					// x dir column
					waterAxisColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					// y dir column
					waterAxisColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					// z dir column
					waterAxisColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
				else {
					// x dir column
					basicAxisColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					// y dir column
					basicAxisColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					// z dir column
					basicAxisColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
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
	static uint64_t cullColBit[CHUNK_SIZE_P2 * 6];
	std::fill(cullColBit, cullColBit + CHUNK_SIZE_P2 * 6, 0);

	for (int axis = 0; axis < 3; ++axis) {
		for (int h = 1; h < CHUNK_SIZE_P - 1; ++h) {
			for (int w = 1; w < CHUNK_SIZE_P - 1; ++w) {
				uint64_t waterColBit =
					waterAxisColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];
				uint64_t basicColBit =
					basicAxisColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];

				cullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					waterColBit & ~(waterColBit << 1) | basicColBit & ~(basicColBit << 1);
				cullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					waterColBit & ~(waterColBit >> 1) | basicColBit & ~(basicColBit >> 1);
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
	static uint64_t faceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6];
	for (const auto& p : typeMap)
		std::fill(faceColBit[p.first], faceColBit[p.first] + CHUNK_SIZE2 * 6, 0);

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

					uint8_t type = 0;
					if (face < 2) {
						type = m_blocks[bitPos + 1][h + 1][w + 1].GetType();
					}
					else if (face < 4) {
						type = m_blocks[w + 1][bitPos + 1][h + 1].GetType();
					}
					else { // face < 6
						type = m_blocks[w + 1][h + 1][bitPos + 1].GetType();
					}

					faceColBit[type][Utils::GetIndexFrom3D(face, bitPos, w, CHUNK_SIZE)] |=
						(1ULL << h);
				}
			}
		}
	}


	// 4. make vertices by bit slices column
	// face 0, 1 : left-right
	// face 2, 3 : top-bottom
	// face 4, 5 : front-back
	for (const auto& p : typeMap) {
		uint8_t type = p.first;

		for (int face = 0; face < 6; ++face) {
			for (int s = 0; s < CHUNK_SIZE; ++s) {
				for (int i = 0; i < CHUNK_SIZE; ++i) {
					uint64_t faceBit =
						faceColBit[type][Utils::GetIndexFrom3D(face, s, i, CHUNK_SIZE)];
					int step = 0;
					while (step < CHUNK_SIZE) {						   // 111100011100
						step += Utils::TrailingZeros(faceBit >> step); // 1111000111|00| -> 2
						if (step >= CHUNK_SIZE)
							break;

						int ones = Utils::TrailingOnes((faceBit >> step));	// 1111000|111|00 -> 3
						uint64_t submask = ((1ULL << ones) - 1ULL) << step; // 111 << 2 -> 11100

						int w = 1;
						while (i + w < CHUNK_SIZE) {
							uint64_t cb = faceColBit[type][Utils::GetIndexFrom3D(
											  face, s, i + w, CHUNK_SIZE)] &
										  submask;
							if (cb != submask)
								break;

							faceColBit[type][Utils::GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] &=
								(~submask);
							w++;
						}

						std::vector<VoxelVertex>& vertices =
							(type == Block::Type::WATER) ? m_waterVertices : m_basicVertices;
						std::vector<uint32_t>& indices =
							(type == Block::Type::WATER) ? m_waterIndices : m_basicIndices;

						if (face == 0)
							MeshGenerator::CreateQuadMesh(
								vertices, indices, s, step, i, w, ones, face, type);
						else if (face == 1)
							MeshGenerator::CreateQuadMesh(
								vertices, indices, s + 1, step, i, w, ones, face, type);
						else if (face == 2)
							MeshGenerator::CreateQuadMesh(
								vertices, indices, i, s, step, w, ones, face, type);
						else if (face == 3)
							MeshGenerator::CreateQuadMesh(
								vertices, indices, i, s + 1, step, w, ones, face, type);
						else if (face == 4)
							MeshGenerator::CreateQuadMesh(
								vertices, indices, i, step, s, w, ones, face, type);
						else // face == 5
							MeshGenerator::CreateQuadMesh(
								vertices, indices, i, step, s + 1, w, ones, face, type);

						step += ones;
					}
				}
			}
		}
	}
}