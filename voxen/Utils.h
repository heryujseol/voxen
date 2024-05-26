#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stb_image.h>

#include "Utils.h"
#include "Chunk.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

namespace Utils {
	static const float PI = 3.14159265f;
	static const float invPI = 1.0f / PI;

	static Vector3 CalcOffsetPos(Vector3 pos, int baseSize)
	{
		int floorX = (int)floor(pos.x);
		int floorY = (int)floor(pos.y);
		int floorZ = (int)floor(pos.z);

		int modX = ((floorX % baseSize) + baseSize) % baseSize;
		int modY = ((floorY % baseSize) + baseSize) % baseSize;
		int modZ = ((floorZ % baseSize) + baseSize) % baseSize;

		return Vector3((float)(floorX - modX), (float)(floorY - modY), (float)(floorZ - modZ));
	}

	template <typename T>
	static T Lerp(T a, T b, float w) 
	{ 
		w = std::clamp(w, 0.0f, 1.0f);
		return (1 - w) * a + w * b; 
	}

	static float CubicLerp(float a, float b, float w)
	{
		return (b - a) * (float)((3.0f - w * 2.0f) * w * w) + a;
	}

	static float Smootherstep(float a, float b, float w)
	{
		return (b - a) * (float)((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a;
	}

	static Vector2 RandomGradient(int ix, int iy)
	{
		const unsigned w = 8 * sizeof(unsigned);
		const unsigned s = w / 2;
		unsigned a = ix, b = iy;

		a *= 3284157443;
		b ^= a << s | a >> (w - s);
		b *= 1911520717;
		a ^= b << s | b >> (w - s);
		a *= 2048419325;

		float random = a * ((float)3.14159265f / ~(~0u >> 1)); // in [0, 2*Pi]

		Vector2 v(cos(random), sin(random));
		return v;
	}

	static Vector2 Hash(uint32_t x, uint32_t y) 
	{ 
		// https://www.shadertoy.com/view/3dVXDc
		uint32_t u0 = 1597334673U;
		uint32_t u1 = 3812015801U;
		float uf = (1.0f / float(0xffffffffU));

		uint32_t qi = x * u0;
		uint32_t qj = y * u1;

		uint32_t qx = (qi ^ qj) * u0;
		uint32_t qy = (qi ^ qj) * u1;

		float rx = -1.0f + 2.0f * qx * uf;
		float ry = -1.0f + 2.0f * qy * uf;

		return Vector2(rx, ry);
	}

	static float GetPerlinNoiseFbm(float x, float y) 
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;

		float n0 = Hash(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		float n1 = Hash(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		float n2 = Hash(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		float n3 = Hash(x1, y1).Dot(p - Vector2((float)x1, (float)y1));

		float inter_x0 = CubicLerp(n0, n1, p.x - (float)x0);
		float inter_x1 = CubicLerp(n2, n3, p.x - (float)x0);
		float inter_y = CubicLerp(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y;
	}

	static float PerlinFbm(float x, float y, float freq, int octave)
	{
		float amp = 1.0f;
		float noise = 0.0f;

		for (int i = 0; i < octave; ++i) {
			noise += amp * GetPerlinNoiseFbm(x * freq, y * freq);

			freq *= 2.0f;
			amp *= exp2(-0.85f);
			 //exp2(-0.85f);
		}

		return noise;
	}

	static float GetPerlinNoise(float x, float y)
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;

		float n0 = RandomGradient(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		float n1 = RandomGradient(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		float n2 = RandomGradient(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		float n3 = RandomGradient(x1, y1).Dot(p - Vector2((float)x1, (float)y1));

		float inter_x0 = CubicLerp(n0, n1, p.x - (float)x0);
		float inter_x1 = CubicLerp(n2, n3, p.x - (float)x0);
		float inter_y = CubicLerp(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y;
	}

	static int GetHeight(int x, int z)
	{
		float noise = GetPerlinNoise((float)x / 32.0f, (float)z / 32.0f); // [-1.0, 1.0]

		return (int)((noise + 1.0f) * 24.0f); // [0, 48]
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

	static float HenyeyGreensteinPhase(Vector3 L, Vector3 V, float aniso)
	{
		// L: toLight
		// V: eyeDir
		// https://www.shadertoy.com/view/7s3SRH
		float cosT = L.Dot(V);
		float g = aniso;
		return (1.0f - g * g) / (4.0f * PI * pow(abs(1.0f + g * g - 2.0f * g * cosT), 3.0f / 2.0f));
	}
};