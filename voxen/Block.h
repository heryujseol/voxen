#pragma once
#include "Terrain.h"

#include <stdint.h>

enum BLOCK_TYPE : uint8_t {
	AIR = 0,
	WATER = 1,
	LEAF = 10,
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

	static inline bool IsOpaqua(uint8_t type) { return (1 < type && type < 10);  } // 임시 데이터
	static inline bool IsSemiAlpha(uint8_t type) { return (10 <= type && type < 20); } // 임시 데이터
	static inline bool IsTransparency(uint8_t type) { return (type <= 1); } // 임시 데이터

	static inline bool IsInstance(uint8_t type) { return type >= 128; }
	static inline INSTANCE_TYPE GetInstanceType(uint8_t type)
	{
		if (128 <= type && type < 128 + 16)
			return INSTANCE_TYPE::BOX;
		else if (128 + 16 <= type && type < 128 + 16 * 2)
			return INSTANCE_TYPE::CROSS;
		else if (128 + 16 * 2 <= type && type < 128 + 16 * 3)
			return INSTANCE_TYPE::FENCE;
		else if (128 + 16 * 3 <= type && type < 128 + 16 * 4)
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