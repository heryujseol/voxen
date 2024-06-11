#pragma once
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Terrain {

	static int32_t p[512] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
        8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203,
        117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
        165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220,
        105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,
        3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59,
        227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70,
        221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178,
        185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176,
        115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195,
        78, 66, 215, 61, 156, 180,

        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
        8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203,
        117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
        165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220,
        105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,
        3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59,
        227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70,
        221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178,
        185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176,
        115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195,
        78, 66, 215, 61, 156, 180,
    };
	
	static float Smootherstep(float a, float b, float w)
	{
		return (b - a) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a;
	}

	static float CubicLerp(float a, float b, float w)
	{
		return (b - a) * (float)((3.0f - w * 2.0f) * w * w) + a;
	}

	static float Lerp(float a, float b, float w) { return a + w * (b - a); }

	static Vector2 RandomGradient2(int ix, int iy)
	{
		const unsigned w = 8 * sizeof(unsigned);
		const unsigned s = w / 2;
		unsigned a = ix, b = iy;

		a *= 1789256896;
		b ^= a << s | a >> (w - s);
		b *= 3588430632;
		a ^= b << s | b >> (w - s);
		a *= 1568468904;

		float random = a * ((float)3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

		Vector2 v(cos(random), sin(random));
		return v;
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

	static float GetPerlinNoise2(float x, float y)
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;

		float n0 = RandomGradient2(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		if ((p - Vector2((float)x0, (float)y0)).Length() == 0)
			n0 = 0;
		float n1 = RandomGradient2(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		if ((p - Vector2((float)x1, (float)y0)).Length() == 0)
			n1 = 0;
		float n2 = RandomGradient2(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		if ((p - Vector2((float)x0, (float)y1)).Length() == 0)
			n2 = 0;
		float n3 = RandomGradient2(x1, y1).Dot(p - Vector2((float)x1, (float)y1));
		if ((p - Vector2((float)x1, (float)y1)).Length() == 0)
			n3 = 0;

		float inter_x0 = Smootherstep(n0, n1, p.x - (float)x0);
		float inter_x1 = Smootherstep(n2, n3, p.x - (float)x0);
		float inter_y = Smootherstep(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y * 0.5f + 0.5f;
	}

	static float GetPerlinNoise(float x, float y)
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;

		float n0 = RandomGradient(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		if ((p - Vector2((float)x0, (float)y0)).Length() == 0)
			n0 = 0;
		float n1 = RandomGradient(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		if ((p - Vector2((float)x1, (float)y0)).Length() == 0)
			n1 = 0;
		float n2 = RandomGradient(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		if ((p - Vector2((float)x0, (float)y1)).Length() == 0)
			n2 = 0;
		float n3 = RandomGradient(x1, y1).Dot(p - Vector2((float)x1, (float)y1));
		if ((p - Vector2((float)x1, (float)y1)).Length() == 0)
			n3 = 0;

		float inter_x0 = CubicLerp(n0, n1, p.x - (float)x0);
		float inter_x1 = CubicLerp(n2, n3, p.x - (float)x0);
		float inter_y = CubicLerp(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y * 0.5f + 0.5f;
	}

	static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

	static float lerp(float t, float a, float b) { return a + t * (b - a); }

	static float grad(int32_t hash, float x, float y, float z)
	{
		int32_t h = hash & 15;
		float u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	static float Get3DPerlinNoise(float x, float y, float z)
	{
		int X = (int)floor(x) & 255, Y = (int)floor(y) & 255, Z = (int)floor(z) & 255;
		x -= floor(x);
		y -= floor(y);
		z -= floor(z);
		float u = fade(x), v = fade(y), w = fade(z);
		int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z, B = p[X + 1] + Y, BA = p[B] + Z,
			BB = p[B + 1] + Z;

		return lerp(w,
			lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
				lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
			lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
				lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
	}

	static int GetHeight(int x, int z)
	{
		float nx = (float)x / 256.0f;
		float ny = (float)z / 256.0f;

		float result = 0.0f;
		float amp = 0.9f;
		float freq = 1.0f;
		//int octave = 5;

		for (int i = 0; i < 5; ++i) {
			result += amp * GetPerlinNoise(nx * freq, ny * freq);

			amp *= 0.5f;
			freq *= 2.0f;
		}

		result = pow(result, 2.8f);
		result = round(result * 80) / 80;

		return (int)(result * 100.0f) + 16;
	}
	

	/*	type
	0 = air
	1 = water
	2 = grass
	3 = sand
	4 = stone
	5 = snow grass
	6 = snow
	7 = dirt
	8 = swamp grass
	9 = swamp grass2
	*/
	static uint8_t GetType(int x, int y, int z, int h)
	{
		float thick = Get3DPerlinNoise((float)x / 96.0f, (float)y / 96.0f, (float)z / 96.0f);
		float t = GetPerlinNoise2((float)x / 182.0f, (float)z / 182.0f);

		uint8_t type = 0;

		if (y == h) { // Áö¸é
			if (y > 140) {
				if (t > 0.6f)
					type = 6;
				else
					type = 5;
			}
			else if (y > 108) {
				if (t > 0.48f)
					type = 2;
				else if (t > 0.42f)
					type = 4;
				else
					type = 5;
			}
			else if (y > 96) {
				if (t > 0.6f)
					type = 9;
				else
					type = 8;
			}
			else if (y > 62) {
				if (t > 0.6f)
					type = 3;
				else if (t > 0.4f)
					type = 8;
				else
					type = 9;
			}
			else
				type = 3;

			if (thick > 0.6f)
				type = 0;
		}
		else if (y < h) {
			if (h - 10 >= y)
				type = 4;
			else {
				if (y > 140) {
					if (t > 0.6f) {
						if (y == h - 1)
							type = 5;
						else
							type = 7;
					}
					else
						type = 5;
				}
				else if (y > 108) {
					if (t > 0.55f)
						type = 7;
					else if (t > 0.4f)
						type = 4;
					else
						type = 7;
				}
				else if (y > 96) {
					type = 7;
				}
				else if (y > 62) {
					if (t > 0.6f)
						type = 3;
					else
						type = 7;
				}
				else
					type = 3;
			}

			if (thick > 0.6f)
				type = 0;
		}

		if (y > h && y < 62)
			type = 1;

		return type;
	}
}