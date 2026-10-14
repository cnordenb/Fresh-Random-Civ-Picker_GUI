#pragma once

// The most editions, DLCs, civs and store links data.json may list; CivCatalog refuses a file with more. The main
// window has room for this many: three edition rows (hotkeys Q, W and E), nine DLC toggles per edition (hotkeys A
// to L) and a civ grid of 13 rows of five.
constexpr int MAX_EDITIONS = 3;
constexpr int MAX_DLCS = 32;
constexpr int MAX_DLC_TOGGLES = 9;
constexpr int MAX_CIVS = 64;
constexpr int MAX_EDITION_LINKS = 4;
