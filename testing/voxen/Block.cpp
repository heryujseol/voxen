#include "Block.h"

Block::Block() : m_isActive(false) {}

bool Block::IsActive() { return m_isActive; }

void Block::SetActive(bool active) { m_isActive = active; }
 
Block::~Block() {}
