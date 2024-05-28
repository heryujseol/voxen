#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stb_image.h>

#include "Utils.h"
#include "Chunk.h"

using namespace DirectX::SimpleMath;

class Utils {
public:
	static Vector3 CalcChunkPos(Vector3 pos)
	{
		int floorX = (int)floor(pos.x);
		int floorY = (int)floor(pos.y);
		int floorZ = (int)floor(pos.z);

		int modX = ((floorX % Chunk::CHUNK_SIZE) + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
		int modY = ((floorY % Chunk::CHUNK_SIZE) + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
		int modZ = ((floorZ % Chunk::CHUNK_SIZE) + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;

		return Vector3((float)(floorX - modX), (float)(floorY - modY), (float)(floorZ - modZ));
	}

	static void ReadImage(
		const std::string filename, std::vector<uint8_t>& image, int& width, int& height)
	{

		int channels = 4;
		unsigned char* img = stbi_load(filename.c_str(), &width, &height, &channels, 0);

		image.resize((size_t)width * height * 4);

		if (channels == 1) {
			for (size_t i = 0; i < (size_t)width * height; i++) {
				uint8_t g = img[i * channels + 0];
				for (size_t c = 0; c < 4; c++) {
					image[4 * i + c] = g;
				}
			}
		}
		else if (channels == 2) {
			for (size_t i = 0; i < (size_t)width * height; i++) {
				for (size_t c = 0; c < 2; c++) {
					image[4 * i + c] = img[i * channels + c];
				}
				image[4 * i + 2] = 255;
				image[4 * i + 3] = 255;
			}
		}
		else if (channels == 3) {
			for (size_t i = 0; i < (size_t)width * height; i++) {
				for (size_t c = 0; c < 3; c++) {
					image[4 * i + c] = img[i * channels + c];
				}
				image[4 * i + 3] = 255;
			}
		}
		else if (channels == 4) {
			for (size_t i = 0; i < (size_t)width * height; i++) {
				for (size_t c = 0; c < 4; c++) {
					image[4 * i + c] = img[i * channels + c];
				}
			}
		}
		else {
			std::cout << "Cannot read " << channels << " channels" << std::endl;
		}

		delete[] img;
	}

	static inline int GetIndexFrom3D(int axis, int y, int x, int length)
	{
		return (length * length) * axis + length * y + x;
	}

	static inline int TrailingZeros(uint64_t num)
	{
		if (num == 0)
			return 64;
		return (int)log2(num & ((~num) + 1)); // __builtin_ctzll or _BitScanForward64
	}

	static inline int TrailingOnes(uint64_t num)
	{
		return (int)log2((num & ~(num + 1)) + 1); // __builtin_ctzll or _BitScanForward64}
	}
};