#pragma once

class Block 
{
public:
	Block() : m_isActive(false) {}
	~Block() {}

	inline bool IsActive() { return m_isActive; }
	inline void SetActive(bool active) { m_isActive = active; }

private:
	bool m_isActive;
};