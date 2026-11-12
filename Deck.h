#pragma once
#include <vector>
#include "Card.h"

// Deck vastaa 52 kortin pakasta
class CDeck
{
public:
	CDeck();
	~CDeck();

	void Create();
	void Shuffle();
	CCard Draw();
	void ResetDeck();
private:
	std::vector<CCard> m_cards;
};

