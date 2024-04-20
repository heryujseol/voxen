#pragma once

class Block 
{
public:
	Block();
	~Block();
	bool IsActive();
	void SetActive(bool active);

private:
	bool m_isActive;
};