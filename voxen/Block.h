#pragma once
#include "Terrain.h"

#include <stdint.h>

enum BLOCK_TYPE : uint8_t {
	AIR = 0,
	WATER = 1,
};

enum INSTANCE_TYPE : uint8_t {
	BOX = 0,
	CROSS = 1,
	FENCE = 2,
	SQUARE = 3,
	NONE = 4,
};

class Block {
public:
	

	static const int BLOCK_TYPE_COUNT = 256;
	static const int INSTANCE_TYPE_COUNT = 4;

	static inline bool IsInstance(uint8_t type) { return type >= 128; }
	static inline INSTANCE_TYPE GetInstanceType(uint8_t type)
	{
		if (128 <= type && type < 128 + 16)
			return INSTANCE_TYPE::BOX;
		else if (type < 128 + 16 * 2)
			return INSTANCE_TYPE::CROSS;
		else if (type < 128 + 16 * 3)
			return INSTANCE_TYPE::FENCE;
		else if (type < 128 + 16 * 4)
			return INSTANCE_TYPE::SQUARE;
		else
			return INSTANCE_TYPE::NONE;
	}

	Block() : m_type(0) {}
	~Block() {}

	inline uint8_t GetType() { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }


private:
	uint8_t m_type;
};