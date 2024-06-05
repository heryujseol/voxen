#pragma once
#include "Terrain.h"

#include <stdint.h>

class Block 
{
public:
	Block() : m_type(0) {}
	~Block() {}

	inline uint8_t GetType() { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }

	static inline bool IsSprite(uint8_t type) { return type >= 128; }

	static const int BLOCK_TYPE_COUNT = 256;

	
	enum Type : uint8_t { 
		AIR = 0, 
		WATER = 1,
	};

private:
	uint8_t m_type;
};