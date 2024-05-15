#pragma once
#include "Terrain.h"

class Block 
{
public:
	Block() : m_type(0) {}
	~Block() {}
	
	inline void SetType(unsigned char type) { m_type = type; }
	inline unsigned char GetType() { return m_type; }

private:
	unsigned char m_type;
};