#include "pch.h"
#include "Card.h"

CCard::CCard()
{
	m_suit = Suit::Clubs;
	m_rank = 1;
	m_faceUp = false;
}

CCard::CCard(Suit suit, int rank)
{
	this->m_suit = suit;
	this->m_rank = rank;
	this->m_faceUp = false;
}	

Suit CCard::GetSuit() const
{
	return m_suit;
}

int CCard::GetRank() const
{
	return m_rank;
}

bool CCard::IsFaceUp() const
{
	return m_faceUp;
}

void CCard::Flip()
{
	m_faceUp = !m_faceUp;
}
