#include <windows.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#pragma comment(lib, "user32.lib")

static std::vector<BYTE> Resource(HMODULE module, int type, int id, WORD language)
{
	HRSRC info = FindResourceExW(module, MAKEINTRESOURCEW(type), MAKEINTRESOURCEW(id), language);
	assert(info);
	HGLOBAL loaded = LoadResource(module, info);
	assert(loaded);
	const BYTE* data = static_cast<const BYTE*>(LockResource(loaded));
	assert(data);
	return std::vector<BYTE>(data, data + SizeofResource(module, info));
}

static bool ContainsCommand(HMENU menu, UINT id)
{
	for (int i = 0; i < GetMenuItemCount(menu); ++i)
	{
		if (GetMenuItemID(menu, i) == id) return true;
		HMENU child = GetSubMenu(menu, i);
		if (child && ContainsCommand(child, id)) return true;
	}
	return false;
}

static void CheckLanguage(HMODULE module, WORD language, const wchar_t* expectedMenu,
	const wchar_t* expectedWin, const wchar_t* expectedFilter, const wchar_t* expectedSavePrompt)
{
	assert(!Resource(module, 4, 128, language).empty());
	assert(!Resource(module, 5, 100, language).empty());
	assert(SetThreadUILanguage(language) == language);
	for (UINT id = 400; id <= 421; ++id)
	{
		wchar_t value[2048];
		assert(LoadStringW(module, id, value, _countof(value)) > 0);
		if (id == 404) assert(std::wstring(value) == expectedWin);
	}
	wchar_t text[2048];
	assert(LoadStringW(module, 128, text, _countof(text)) > 0);
	assert(std::wstring(text).find(expectedFilter) != std::wstring::npos);
	assert(LoadStringW(module, 0xF103, text, _countof(text)) > 0);
	assert(std::wstring(text).find(expectedSavePrompt) != std::wstring::npos);
	HMENU menu = LoadMenuW(module, MAKEINTRESOURCEW(128));
	assert(menu);
	wchar_t title[256];
	assert(GetMenuStringW(menu, 0, title, _countof(title), MF_BYPOSITION) > 0);
	assert(std::wstring(title) == expectedMenu);
	assert(ContainsCommand(menu, 32779) && ContainsCommand(menu, 32780));
	if (language == 0x40b)
	{
		assert(GetMenuStringW(menu, 1, title, _countof(title), MF_BYPOSITION) > 0);
		assert(std::wstring(title) == L"&N\u00e4yt\u00e4");
		HMENU view = GetSubMenu(menu, 1);
		assert(view && GetMenuStringW(view, 0, title, _countof(title), MF_BYPOSITION) > 0);
		assert(std::wstring(title) == L"&Ty\u00f6kalurivit ja telakointi-ikkunat");
		HMENU help = GetSubMenu(menu, 2);
		assert(help && GetMenuStringW(help, 0, title, _countof(title), MF_BYPOSITION) > 0);
		assert(std::wstring(title) == L"&Sis\u00e4lt\u00f6\tF1");
		HMENU game = GetSubMenu(menu, 0);
		assert(game && GetMenuStringW(game, 6, title, _countof(title), MF_BYPOSITION) > 0);
		assert(std::wstring(title) == L"Tallenna &nimell\u00e4...");
		assert(LoadStringW(module, 409, text, _countof(text)) > 0);
		assert(std::wstring(text) == L"Peli\u00e4 ei voi piirt\u00e4\u00e4 (Direct2D-virhe 0x%08X).");
		const auto dialog = Resource(module, 5, 100, language);
		const std::wstring phrase = L"niit\u00e4 k\u00e4ytet\u00e4\u00e4n";
		const BYTE* first = reinterpret_cast<const BYTE*>(phrase.data());
		assert(std::search(dialog.begin(), dialog.end(), first, first + phrase.size() * sizeof(wchar_t)) != dialog.end());
		HMENU editMenu = LoadMenuW(module, MAKEINTRESOURCEW(119));
		assert(editMenu);
		HMENU edit = GetSubMenu(editMenu, 0);
		assert(edit && GetMenuStringW(edit, 2, title, _countof(title), MF_BYPOSITION) > 0);
		assert(std::wstring(title) == L"L&iit\u00e4\tCtrl+V");
		assert(DestroyMenu(editMenu));
	}
	assert(DestroyMenu(menu));
}

static unsigned Read16(const std::vector<BYTE>& data, size_t offset)
{
	assert(offset + 2 <= data.size());
	return data[offset] | (static_cast<unsigned>(data[offset + 1]) << 8);
}

static unsigned Read32(const std::vector<BYTE>& data, size_t offset)
{
	return Read16(data, offset) | (Read16(data, offset + 2) << 16);
}

static void CheckIcon(HMODULE module, const wchar_t* path)
{
	std::ifstream input(path, std::ios::binary);
	assert(input);
	std::vector<BYTE> ico((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
	auto group = Resource(module, 14, 128, 0x409);
	unsigned count = Read16(ico, 4);
	assert(count > 0 && count == Read16(group, 4));
	for (unsigned i = 0; i < count; ++i)
	{
		size_t entry = 6 + i * 16;
		size_t groupEntry = 6 + i * 14;
		assert(entry + 16 <= ico.size() && groupEntry + 14 <= group.size());
		for (size_t j = 0; j < 12; ++j)
			if (j != 4 && j != 5) assert(ico[entry + j] == group[groupEntry + j]);
		unsigned length = Read32(ico, entry + 8);
		unsigned offset = Read32(ico, entry + 12);
		auto image = Resource(module, 3, Read16(group, groupEntry + 12), 0x409);
		assert(length == image.size() && offset <= ico.size() && length <= ico.size() - offset);
		for (size_t j = 0; j < length; ++j) assert(ico[offset + j] == image[j]);
	}
}

int wmain(int argc, wchar_t* argv[])
{
	assert(argc == 3);
	HMODULE module = LoadLibraryExW(argv[1], nullptr, LOAD_LIBRARY_AS_DATAFILE);
	assert(module);
	WORD original = GetThreadUILanguage();
	CheckLanguage(module, 0x409, L"&Game", L"You won! Congratulations!", L"Solitaire games", L"Save changes");
	CheckLanguage(module, 0x40b, L"&Peli", L"Voitit pelin! Onneksi olkoon!", L"Pasianssipelit", L"Tallennetaanko");
	CheckIcon(module, argv[2]);
	SetThreadUILanguage(original);
	assert(FreeLibrary(module));
}
