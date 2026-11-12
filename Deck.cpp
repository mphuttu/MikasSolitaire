#include "pch.h"
#include "Deck.h"
#include <algorithm>
#include <random>
#include <stdexcept>

CDeck::CDeck()
{
	Create();
}

CDeck::~CDeck()
{}

void CDeck::Create()
{
	m_cards.clear();
	for (int suit = 0; suit < 4; ++suit)
	{
		for (int rank = 1; rank <= 13; ++rank)
		{
			m_cards.emplace_back(static_cast<Suit>(suit), rank);
		}
	}
}

void CDeck::Shuffle()
{
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_cards.begin(), m_cards.end(), g);
}

CCard CDeck::Draw()
{
	if (m_cards.empty())
	{
		throw std::out_of_range("No more cards in the deck");
	}
	CCard card = m_cards.back();
	m_cards.pop_back();
	return card;
}

void CDeck::ResetDeck()
{
	Create();
	Shuffle();
}
