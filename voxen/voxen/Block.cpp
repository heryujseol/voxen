#include "Block.h"

Block::Block() : m_isActive(true) {}

bool Block::IsActive() { return m_isActive; }

void Block::SetActive(bool active) { m_isActive = active; }

Block::~Block() {}
