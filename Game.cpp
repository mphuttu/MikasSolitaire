#include "pch.h"
#include "Game.h"
#include <utility>

namespace
{
	bool IsRed(const CCard& card)
	{
		return card.GetSuit() == Suit::Hearts || card.GetSuit() == Suit::Diamonds;
	}
}

bool CGame::Initialize()
{
	m_deck.ResetDeck();
	m_stock.clear();
	m_waste.clear();
	for (auto& pile : m_foundations) pile.clear();
	for (auto& pile : m_tableau) pile.clear();

	for (int column = 0; column < 7; ++column)
	{
		for (int row = 0; row <= column; ++row)
		{
			CCard card = m_deck.Draw();
			if (row == column) card.Flip();
			m_tableau[column].push_back(card);
		}
	}
	for (int i = 0; i < 24; ++i) m_stock.push_back(m_deck.Draw());
	m_initialStock = m_stock;
	m_initialTableau = m_tableau;
	return true;
}

bool CGame::DrawStock()
{
	if (m_stock.empty() && m_waste.empty()) return false;
	if (!m_stock.empty())
	{
		CCard card = m_stock.back();
		m_stock.pop_back();
		card.Flip();
		m_waste.push_back(card);
	}
	else
	{
		while (!m_waste.empty())
		{
			CCard card = m_waste.back();
			m_waste.pop_back();
			card.Flip();
			m_stock.push_back(card);
		}
	}
	return true;
}

void CGame::Restart()
{
	m_stock = m_initialStock;
	m_tableau = m_initialTableau;
	m_waste.clear();
	for (auto& pile : m_foundations) pile.clear();
}

namespace
{
	struct GameState
	{
		std::vector<CCard> stock, waste;
		std::array<std::vector<CCard>, 4> foundations;
		std::array<std::vector<CCard>, 7> tableau;
	};

	void WritePile(CArchive& archive, const std::vector<CCard>& pile)
	{
		archive << static_cast<BYTE>(pile.size());
		for (const auto& card : pile)
		{
			archive << static_cast<BYTE>(card.GetSuit());
			archive << static_cast<BYTE>(card.GetRank());
			archive << static_cast<BYTE>(card.IsFaceUp() ? 1 : 0);
		}
	}

	void ReadPile(CArchive& archive, std::vector<CCard>& pile)
	{
		BYTE count;
		archive >> count;
		if (count > 52) AfxThrowArchiveException(CArchiveException::badSchema);
		for (int i = 0; i < count; ++i)
		{
			BYTE suit, rank, faceUp;
			archive >> suit >> rank >> faceUp;
			if (suit > 3 || rank < 1 || rank > 13 || faceUp > 1)
				AfxThrowArchiveException(CArchiveException::badSchema);
			CCard card(static_cast<Suit>(suit), rank);
			if (faceUp) card.Flip();
			pile.push_back(card);
		}
	}

	void WriteState(CArchive& archive, const GameState& state)
	{
		WritePile(archive, state.stock);
		WritePile(archive, state.waste);
		for (const auto& pile : state.foundations) WritePile(archive, pile);
		for (const auto& pile : state.tableau) WritePile(archive, pile);
	}

	void ReadState(CArchive& archive, GameState& state)
	{
		ReadPile(archive, state.stock);
		ReadPile(archive, state.waste);
		for (auto& pile : state.foundations) ReadPile(archive, pile);
		for (auto& pile : state.tableau) ReadPile(archive, pile);
	}

	bool ValidateState(const GameState& state, bool initial)
	{
		bool seen[4][13] = {};
		int count = 0;
		auto unique = [&](const CCard& card)
		{
			int suit = static_cast<int>(card.GetSuit());
			int rank = card.GetRank() - 1;
			if (seen[suit][rank]) return false;
			seen[suit][rank] = true;
			++count;
			return true;
		};
		for (const auto& card : state.stock)
			if (card.IsFaceUp() || !unique(card)) return false;
		for (const auto& card : state.waste)
			if (!card.IsFaceUp() || !unique(card)) return false;
		for (const auto& pile : state.foundations)
			for (size_t i = 0; i < pile.size(); ++i)
				if (!pile[i].IsFaceUp() || pile[i].GetRank() != static_cast<int>(i + 1) ||
					(i > 0 && pile[i].GetSuit() != pile[0].GetSuit()) || !unique(pile[i]))
					return false;
		for (size_t col = 0; col < state.tableau.size(); ++col)
		{
			const auto& pile = state.tableau[col];
			if (initial && pile.size() != col + 1) return false;
			bool faceUp = false;
			for (size_t i = 0; i < pile.size(); ++i)
			{
				const CCard& card = pile[i];
				if (!unique(card) || (faceUp && !card.IsFaceUp())) return false;
				if (faceUp && (pile[i - 1].GetRank() != card.GetRank() + 1 ||
					IsRed(pile[i - 1]) == IsRed(card))) return false;
				faceUp = card.IsFaceUp();
				if (initial && faceUp != (i + 1 == pile.size())) return false;
			}
			if (!pile.empty() && !faceUp) return false;
		}
		if (initial)
		{
			if (state.stock.size() != 24 || !state.waste.empty()) return false;
			for (const auto& pile : state.foundations) if (!pile.empty()) return false;
		}
		return count == 52;
	}
}

void CGame::Serialize(CArchive& archive)
{
	const DWORD magic = 0x4c4f534d; // MSOL
	const DWORD version = 1;
	if (archive.IsStoring())
	{
		archive << magic << version;
		WriteState(archive, { m_stock, m_waste, m_foundations, m_tableau });
		WriteState(archive, { m_initialStock, {}, {}, m_initialTableau });
	}
	else
	{
		DWORD fileMagic, fileVersion;
		archive >> fileMagic >> fileVersion;
		if (fileMagic != magic || fileVersion != version)
			AfxThrowArchiveException(CArchiveException::badSchema);
		GameState current, initial;
		ReadState(archive, current);
		ReadState(archive, initial);
		BYTE extra;
		if (archive.Read(&extra, 1) != 0 || !ValidateState(current, false) || !ValidateState(initial, true))
			AfxThrowArchiveException(CArchiveException::badSchema);
		m_stock = std::move(current.stock);
		m_waste = std::move(current.waste);
		m_foundations = std::move(current.foundations);
		m_tableau = std::move(current.tableau);
		m_initialStock = std::move(initial.stock);
		m_initialTableau = std::move(initial.tableau);
	}
}
bool CGame::MoveToFoundation(Location from)
{
	if (from.pile == Pile::Tableau)
	{
		if (from.column < 0 || from.column >= 7 || from.card < 0 ||
			static_cast<size_t>(from.card) + 1 != m_tableau[from.column].size())
			return false;
	}
	else if (from.pile != Pile::Waste)
		return false;

	for (int column = 0; column < 4; ++column)
		if (Move(from, { Pile::Foundation, column, 0 })) return true;
	return false;
}

bool CGame::Move(Location from, Location to)
{
	if (from.pile == Pile::Foundation && to.pile == Pile::Foundation) return false;
	std::vector<CCard>* source = nullptr;
	std::vector<CCard>* destination = nullptr;
	if (from.pile == Pile::Waste) source = &m_waste;
	else if (from.pile == Pile::Foundation && from.column >= 0 && from.column < 4)
		source = &m_foundations[from.column];
	else if (from.pile == Pile::Tableau && from.column >= 0 && from.column < 7)
		source = &m_tableau[from.column];
	if (to.pile == Pile::Foundation && to.column >= 0 && to.column < 4)
		destination = &m_foundations[to.column];
	else if (to.pile == Pile::Tableau && to.column >= 0 && to.column < 7)
		destination = &m_tableau[to.column];

	if (!source || !destination || source == destination || from.card < 0 ||
		static_cast<size_t>(from.card) >= source->size() || !(*source)[from.card].IsFaceUp())
		return false;
	if (from.pile != Pile::Tableau && static_cast<size_t>(from.card) != source->size() - 1)
		return false;

	const CCard& card = (*source)[from.card];
	for (size_t i = static_cast<size_t>(from.card) + 1; i < source->size(); ++i)
	{
		const CCard& previous = (*source)[i - 1];
		const CCard& next = (*source)[i];
		if (!next.IsFaceUp() || previous.GetRank() != next.GetRank() + 1 ||
			IsRed(previous) == IsRed(next)) return false;
	}
	if (to.pile == Pile::Foundation)
	{
		if (static_cast<size_t>(from.card) != source->size() - 1) return false;
		if (destination->empty())
		{
			if (card.GetRank() != 1) return false;
		}
		else if (destination->back().GetSuit() != card.GetSuit() ||
			destination->back().GetRank() + 1 != card.GetRank()) return false;
	}
	else if (destination->empty())
	{
		if (card.GetRank() != 13) return false;
	}
	else if (!destination->back().IsFaceUp() ||
		destination->back().GetRank() != card.GetRank() + 1 ||
		IsRed(destination->back()) == IsRed(card)) return false;

	destination->insert(destination->end(), source->begin() + from.card, source->end());
	source->erase(source->begin() + from.card, source->end());
	if (from.pile == Pile::Tableau && !source->empty() && !source->back().IsFaceUp())
		source->back().Flip();
	return true;
}

bool CGame::IsWon() const
{
	for (const auto& pile : m_foundations)
		if (pile.size() != 13) return false;
	return true;
}
