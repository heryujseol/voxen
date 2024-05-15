#pragma once

#include <stdint.h>

class Block 
{
public:
	Block() : m_type(0) {}
	~Block() {}

	inline uint8_t GetType() { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }

	static const int BLOCK_TYPE_COUNT = 256;

private:
	uint8_t m_type;
};