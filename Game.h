#pragma once
#include "Deck.h"
#include <array>
#include <vector>

class CGame
{
public:
	enum class Pile { None, Stock, Waste, Foundation, Tableau };
	struct Location
	{
		Pile pile = Pile::None;
		int column = 0;
		int card = 0;
	};

	bool Initialize();
	bool DrawStock();
	bool Move(Location from, Location to);
	bool MoveToFoundation(Location from);
	bool IsWon() const;
	void Restart();
	void Serialize(CArchive& archive);

	const std::vector<CCard>& Stock() const { return m_stock; }
	const std::vector<CCard>& Waste() const { return m_waste; }
	const std::vector<CCard>& Foundation(int column) const { return m_foundations[column]; }
	const std::vector<CCard>& Tableau(int column) const { return m_tableau[column]; }

private:
	CDeck m_deck;
	std::vector<CCard> m_stock;
	std::vector<CCard> m_waste;
	std::array<std::vector<CCard>, 4> m_foundations;
	std::array<std::vector<CCard>, 7> m_tableau;
	std::vector<CCard> m_initialStock;
	std::array<std::vector<CCard>, 7> m_initialTableau;
};
