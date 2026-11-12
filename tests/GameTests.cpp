#include "..\pch.h"
#include "..\Game.h"
#include <cassert>
#include <set>
#include <utility>
#include <vector>

using Pile = CGame::Pile;
using Location = CGame::Location;

static std::pair<int, int> Identity(const CCard& card)
{
	return { static_cast<int>(card.GetSuit()), card.GetRank() };
}

static std::vector<BYTE> Save(CGame& game)
{
	CMemFile file;
	CArchive archive(&file, CArchive::store);
	game.Serialize(archive);
	archive.Close();
	std::vector<BYTE> data(static_cast<size_t>(file.GetLength()));
	file.SeekToBegin();
	assert(file.Read(data.data(), static_cast<UINT>(data.size())) == data.size());
	return data;
}

static bool Load(CGame& game, std::vector<BYTE>& data)
{
	CMemFile file(data.data(), static_cast<UINT>(data.size()));
	try
	{
		CArchive archive(&file, CArchive::load);
		game.Serialize(archive);
		archive.Close();
		return true;
	}
	catch (CException* error)
	{
		error->Delete();
		return false;
	}
}

static void TestFoundationShortcut()
{
	CGame seed;
	seed.Initialize();
	const auto original = Save(seed);
	std::vector<BYTE> fixture(original.begin(), original.begin() + 8);
	auto pile = [&](const std::vector<CCard>& cards)
	{
		fixture.push_back(static_cast<BYTE>(cards.size()));
		for (const auto& card : cards)
		{
			fixture.push_back(static_cast<BYTE>(card.GetSuit()));
			fixture.push_back(static_cast<BYTE>(card.GetRank()));
			fixture.push_back(card.IsFaceUp() ? 1 : 0);
		}
	};
	auto faceUp = [](Suit suit, int rank)
	{
		CCard card(suit, rank);
		card.Flip();
		return card;
	};
	const std::set<std::pair<int, int>> used = {
		{0, 1}, {0, 2}, {0, 3}, {1, 1}, {2, 2}, {2, 4}
	};
	std::vector<CCard> stock;
	for (int suit = 0; suit < 4; ++suit)
		for (int rank = 1; rank <= 13; ++rank)
			if (used.count({suit, rank}) == 0) stock.emplace_back(static_cast<Suit>(suit), rank);
	pile(stock);
	pile({ faceUp(Suit::Hearts, 2), faceUp(Suit::Hearts, 1) });
	for (int i = 0; i < 4; ++i) pile({});
	pile({ CCard(Suit::Clubs, 2), faceUp(Suit::Diamonds, 1) });
	pile({ faceUp(Suit::Clubs, 4), faceUp(Suit::Hearts, 3) });
	for (int i = 0; i < 5; ++i) pile({});
	fixture.insert(fixture.end(), original.begin() + 8 + 13 + 52 * 3, original.end());

	CGame game;
	assert(Load(game, fixture));
	assert(!game.MoveToFoundation({ Pile::Stock, 0, 0 }));
	assert(!game.MoveToFoundation({ Pile::Tableau, 0, 0 }));
	assert(!game.MoveToFoundation({ Pile::Tableau, 1, 0 }));
	assert(!game.MoveToFoundation({ Pile::Tableau, 1, 1 }));
	assert(!game.MoveToFoundation({ Pile::Waste, 0, 0 }));
	assert(Save(game) == fixture);
	assert(game.MoveToFoundation({ Pile::Waste, 0, 1 }));
	assert(game.Foundation(0).back().GetRank() == 1);
	assert(game.MoveToFoundation({ Pile::Waste, 0, 0 }));
	assert(game.Foundation(0).back().GetRank() == 2);
	assert(game.MoveToFoundation({ Pile::Tableau, 1, 1 }));
	assert(game.Foundation(0).back().GetRank() == 3);
	assert(game.MoveToFoundation({ Pile::Tableau, 0, 1 }));
	assert(game.Foundation(1).back().GetSuit() == Suit::Diamonds);
	assert(game.Tableau(0).back().IsFaceUp());
	assert(!game.MoveToFoundation({ Pile::Waste, 0, -1 }));
	assert(!game.MoveToFoundation({ Pile::Foundation, 0, 2 }));
}

int main()
{
	TestFoundationShortcut();
	CGame game;
	game.Initialize();
	std::set<std::pair<int, int>> cards;
	for (const auto& card : game.Stock())
	{
		assert(!card.IsFaceUp());
		cards.insert(Identity(card));
	}
	assert(game.Stock().size() == 24);
	for (int col = 0; col < 7; ++col)
	{
		const auto& pile = game.Tableau(col);
		assert(pile.size() == static_cast<size_t>(col + 1));
		for (size_t i = 0; i < pile.size(); ++i)
		{
			assert(pile[i].IsFaceUp() == (i + 1 == pile.size()));
			cards.insert(Identity(pile[i]));
		}
	}
	assert(cards.size() == 52 && !game.IsWon());
	std::vector<std::pair<int, int>> firstPass;
	for (int i = 0; i < 24; ++i)
	{
		game.DrawStock();
		assert(game.Waste().back().IsFaceUp());
		firstPass.push_back(Identity(game.Waste().back()));
	}
	assert(game.Stock().empty());
	game.DrawStock();
	assert(game.Waste().empty() && game.Stock().size() == 24);
	for (const auto& card : game.Stock()) assert(!card.IsFaceUp());
	for (int i = 0; i < 24; ++i)
	{
		game.DrawStock();
		assert(Identity(game.Waste().back()) == firstPass[i]);
	}

	game.DrawStock();
	bool foundAce = false;
	for (int i = 0; i < 24; ++i)
	{
		game.DrawStock();
		const CCard card = game.Waste().back();
		const Location source{ Pile::Waste, 0, static_cast<int>(game.Waste().size()) - 1 };
		if (card.GetRank() == 1)
		{
			assert(game.Move(source, { Pile::Foundation, 0, 0 }));
			assert(game.Foundation(0).size() == 1);
			assert(!game.Move({ Pile::Foundation, 0, 0 }, { Pile::Foundation, 1, 0 }));
			auto foundationState = Save(game);
			CGame loaded;
			assert(Load(loaded, foundationState));
			assert(Save(loaded) == foundationState);
			CGame beginning = game;
			beginning.Restart();
			loaded.Restart();
			assert(Save(loaded) == Save(beginning));
			foundAce = true;
			break;
		}
		assert(!game.Move(source, { Pile::Foundation, 0, 0 }));
	}
	// If all aces were dealt to the tableau, the draw and rejection checks still apply.
	if (foundAce)
		assert(game.Foundation(0).back().GetRank() == 1);
	assert(!game.Move({ Pile::Tableau, 7, 0 }, { Pile::Foundation, 0, 0 }));
	assert(!game.Move({ Pile::Stock, 0, 0 }, { Pile::Tableau, 0, 0 }));

	bool foundTableauMove = false;
	bool foundReveal = false;
	for (int deal = 0; deal < 100 && !foundReveal; ++deal)
	{
		game.Initialize();
		for (int from = 1; from < 7 && !foundReveal; ++from)
		{
			const auto& source = game.Tableau(from);
			assert(!game.Move({ Pile::Tableau, from, 0 }, { Pile::Tableau, 0, 0 }));
			CCard card = source.back();
			for (int to = 0; to < 7; ++to)
			{
				if (from == to) continue;
				const CCard target = game.Tableau(to).back();
				const bool cardRed = card.GetSuit() == Suit::Hearts || card.GetSuit() == Suit::Diamonds;
				const bool targetRed = target.GetSuit() == Suit::Hearts || target.GetSuit() == Suit::Diamonds;
				const bool legal = target.GetRank() == card.GetRank() + 1 && cardRed != targetRed;
				CGame copy = game;
				const size_t sourceSize = copy.Tableau(from).size();
				assert(copy.Move({ Pile::Tableau, from, static_cast<int>(sourceSize - 1) },
					{ Pile::Tableau, to, 0 }) == legal);
				if (legal)
				{
					foundTableauMove = true;
					assert(copy.Tableau(from).size() == sourceSize - 1);
					assert(copy.Tableau(from).back().IsFaceUp());
					assert(Identity(copy.Tableau(to).back()) == Identity(card));
					auto tableauState = Save(copy);
					CGame loaded;
					assert(Load(loaded, tableauState));
					assert(Save(loaded) == tableauState);
					loaded.Restart();
					assert(Save(loaded) == Save(game));
					foundReveal = true;
				}
			}
		}
	}
	assert(foundTableauMove && foundReveal);
	game.Initialize();
	assert(game.Stock().size() == 24 && game.Waste().empty());
	for (int i = 0; i < 4; ++i) assert(game.Foundation(i).empty());

	const auto initial = Save(game);
	assert(game.DrawStock());
	assert(game.DrawStock());
	const auto progress = Save(game);
	assert(progress != initial);
	CGame restored;
	restored.Initialize();
	auto saved = progress;
	assert(Load(restored, saved));
	assert(Save(restored) == progress);
	restored.Restart();
	assert(Save(restored) == initial);

	auto invalid = initial;
	invalid[0] ^= 1;
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid[9] = 4;
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid[12] = invalid[9];
	invalid[13] = invalid[10];
	invalid[14] = invalid[11];
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid[11] = 1;
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid[8 + 13 + 52 * 3 + 1] = 4;
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid.pop_back();
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid = initial;
	invalid.push_back(0);
	assert(!Load(game, invalid) && Save(game) == progress);
	invalid.clear();
	assert(!Load(game, invalid) && Save(game) == progress);
}
