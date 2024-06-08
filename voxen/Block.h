#pragma once
#include "Terrain.h"

#include <stdint.h>

class Block 
{
public:
	enum Type : uint8_t {
		AIR = 0,
		WATER = 1,
	};

	static const int BLOCK_TYPE_COUNT = 256;
	static const int BLOCK_INSTANCE_COUNT = 64;

	static inline bool IsInstance(uint8_t type) { return type >= 128; }

	Block() : m_type(0) {}
	~Block() {}

	inline uint8_t GetType() { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }


private:
	uint8_t m_type;
};