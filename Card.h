#pragma once

enum class Suit {
	Hearts,
	Diamonds,
	Clubs,
	Spades

};

class CCard
{
public:
	CCard();
	CCard(Suit suit, int rank);
	Suit GetSuit() const;
	int GetRank() const;
	bool IsFaceUp() const;
	void Flip();

private:
	Suit m_suit;
	int m_rank;
	bool m_faceUp;
};

