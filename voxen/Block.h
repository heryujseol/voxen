#pragma once
#include "Utils.h"

class Block 
{
public:
	Block() : m_isActive(false), m_type(0) {}
	~Block() {}

	inline bool IsActive() { return m_isActive; }
	inline void SetActive(bool active) { m_isActive = active; }
	void SetType(int x, int y, int z)
	{
		int e = 255;

		float nx = (float)x / 256.0f;
		float ny = (float)z / 256.0f;

		//float m = Utils::GetHeight(nx, ny);

		if (y <= e * 0.15)
			m_type = 1; // water
		else if (y <= e * 0.2)
			m_type = 2; // sand
		else if (y <= e * 0.4)
			m_type = 3;	// grass
		else if (y <= e * 0.6)
			m_type = 3; // dark grass
		else if (y <= e * 0.8)
			m_type = 4; // pale grass
		else
			m_type = 5; // snow
	}

private:
	bool m_isActive;
	int m_type;
};