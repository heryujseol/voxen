#pragma once
#include "Terrain.h"

class Block 
{
public:
	Block() : m_isActive(false), m_type(0) {}
	~Block() {}

	inline bool IsActive() { return m_isActive; }
	inline void SetActive(bool active) { m_isActive = active; }
	
	int m_type;

private:
	bool m_isActive;
};