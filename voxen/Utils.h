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

		int modX = ((floorX % Chunk::BLOCK_SIZE) + Chunk::BLOCK_SIZE) % Chunk::BLOCK_SIZE;
		int modY = ((floorY % Chunk::BLOCK_SIZE) + Chunk::BLOCK_SIZE) % Chunk::BLOCK_SIZE;
		int modZ = ((floorZ % Chunk::BLOCK_SIZE) + Chunk::BLOCK_SIZE) % Chunk::BLOCK_SIZE;

		return Vector3((float)(floorX - modX), (float)(floorY - modY), (float)(floorZ - modZ));
	}

	static float Lerp(float a, float b, float w) { return (1 - w) * a + w * b; }

	static float CubicLerp(float a, float b, float w)
	{
		return (b - a) * (float)((3.0f - w * 2.0f) * w * w) + a;
	}

	static float Smootherstep(float a, float b, float w)
	{
		return (b - a) * (float)((w * (w * 6.0f - 15.0f) + 10.0f) * w * w * w) + a;
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

		float random = a * ((float)3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

		Vector2 v(cos(random), sin(random));
		return v;
	}


	//static float DotGreidGradient(int ix, int iy, float x, float y)
	//{
	//	Vector2 gradient = RandomGradient(ix, iy);

	//	float dx = x - (float)ix;
	//	float dy = y - (float)iy;


	//	return (dx * gradient.x + dy * gradient.y);
	//}

	static float GetPerlinNoise(float x, float y)
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;


		float n0 = RandomGradient(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		if (Vector2((float)x0, (float)y0).Length() == 0)
			n0 = 0;
		float n1 = RandomGradient(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		if (Vector2((float)x1, (float)y0).Length() == 0)
			n0 = 0;
		float n2 = RandomGradient(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		if (Vector2((float)x0, (float)y1).Length() == 0)
			n0 = 0;
		float n3 = RandomGradient(x1, y1).Dot(p - Vector2((float)x1, (float)y1));
		if (Vector2((float)x1, (float)y1).Length() == 0)
			n0 = 0;

		float inter_x0 = CubicLerp(n0, n1, p.x - (float)x0);
		float inter_x1 = CubicLerp(n2, n3, p.x - (float)x0);
		float inter_y = CubicLerp(inter_x0, inter_x1, p.y - (float)y0);

		//float inter_x0 = Lerp(n0, n1, p.x - (float)x0);
		//float inter_x1 = Lerp(n2, n3, p.x - (float)x0);
		//float inter_y = Lerp(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y * 0.5f + 0.5f;	//[0, 1]
		//return inter_y; //[-1, 1]

		//int x0 = (int)floor(x);
		//int x1 = x0 + 1;
		//int y0 = (int)floor(y);
		//int y1 = y0 + 1;

		//float sx = x - (float)x0;
		//float sy = y - (float)y0;

		//float n0, n1, ix0, ix1, value;

		//n0 = DotGreidGradient(x0, y0, x, y);
		//n1 = DotGreidGradient(x1, y0, x, y);
		//ix0 = Smootherstep(n0, n1, sx);

		//n1 = DotGreidGradient(x0, y1, x, y);
		//n1 = DotGreidGradient(x1, y1, x, y);
		//ix1 = Smootherstep(n0, n1, sx);
		//
		//value = Smootherstep(ix0, ix1, sy);

		//return value * 0.5f + 0.5f;
	}

	//static float reigenoise(float x, float y)
	//{
	//	return 2.0f * (0.5f - abs(0.5f - GetPerlinNoise(x, y)));
	//}

	//static float Noise(int x, int y)
	//{
	//	int n = x + y * 57;
	//	n = (n << 13) ^ n;

	//	return (1.0f -
	//			((n * (n * n * 15731 + 789221) + 1376312589) & 2, 147, 483, 647) / 1073741824.0f);
	//}

	//static float SmoothedNoise(float x, float y)
	//{
	//	float corners = (Noise(x - 1.0f, y - 1.0f) + Noise(x + 1.0f, y - 1.0f) +
	//						Noise(x - 1.0f, y + 1.0f) + Noise(x + 1.0f, y + 1.0f)) /
	//					16.0f;
	//	float sides = (Noise(x - 1.0f, y) + Noise(x + 1.0f, y) + Noise(x, y - 1.0f) +
	//					  Noise(x - 1.0f, y + 1.0f)) /
	//				  8.0f;
	//	float center = Noise(x, y) / 4.0f;

	//	return corners + sides + center;
	//}

	//static float InterpolatedNoise(float x, float y)
	//{
	//	int x0 = (int)x;
	//	int x1 = x - x0;
	//	int y0 = (int)y;
	//	int y1 = y - y0;

	//	float v0 = SmoothedNoise(x0, y0);
	//	float v1 = SmoothedNoise(x0 + 1.0f, y0);
	//	float v2 = SmoothedNoise(x0, y0 + 1.0f);
	//	float v3 = SmoothedNoise(x0 + 1.0f, y0 + 1.0f);

	//	float i0 = Lerp(v0, v1, x1);
	//	float i1 = Lerp(v2, v3, x1);

	//	return Lerp(i0, i1, y1);
	//}

	static int GetHeight(int x, int z)
	{
		//float nx = (float)x / 128.0f;
		//float ny = (float)z / 128.0f;

		//float noise = GetPerlinNoise(nx, ny) + 0.25f * GetPerlinNoise(4 * nx, 4 * ny);

		//noise = noise / (1.0f + 0.25f);

		//noise = pow(noise, 2.5f);

		//return (int)((noise)*160.0f); // [0, 48]

		float nx = (float)x / 32.0f - 0.5f;		// - 0.5f??
		float ny = (float)z / 32.0f - 0.5f;

		float e = (1.0f * GetPerlinNoise(1.0f * nx, 1.0f * ny) + 0.5f * GetPerlinNoise(2.0f * nx, 2.0f * ny) +
				0.25f * GetPerlinNoise(4.0f * nx, 4.0f * ny) + 0.13f * GetPerlinNoise(8.0f * nx, 8.0f * ny) +
				0.06f * GetPerlinNoise(16.0f * nx, 16.0f * ny));
		e = e / (1.0f + 0.5f + 0.25f + 0.13f + 0.06f);
		e = pow(e, 3.0f);

		return (int)(e * 64.0f);

		//float total = 0.0f;
		//float p = 1.0f;
		//float n = 5;
		//
		//for (int i = 0; i < n; ++i)
		//{
		//	float freq = 2.0f * i;
		//	float amp = p * i;

		//	total = total + InterpolatedNoise(x * freq, z * freq) * amp;
		//}

		//return (int)(total * 64.0f);
	}

	//static int GetHeight(int x, int z)
	//{
	//	float nx = (float)x / 64.0f;
	//	float ny = (float)z / 64.0f;

	//	float result = 0.0f;
	//	float amp = 1.0f;
	//	float freq = 2.0f;
	//	int octave = 5;

	//	for (int i = 0; i < octave; ++i) {
	//		float n = amp * GetPerlinNoise(nx * freq, ny * freq);
	//		result += n;

	//		amp *= i;
	//		freq *= i;
	//	}
	//	return (int)result;
	//}


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
};