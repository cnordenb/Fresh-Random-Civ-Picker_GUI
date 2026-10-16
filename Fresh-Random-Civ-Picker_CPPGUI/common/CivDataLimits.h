#pragma once

// The most editions, DLCs, civs and store links data.json may list; CivCatalog refuses a file with more. The main
// window has room for three edition rows (hotkeys Q, W and E) and a civ grid of 13 rows of five. An edition's DLC
// toggles scroll once they don't fit above the civ grid, so any DLC can have one; the first nine of each edition
// have the hotkeys A to L.
constexpr int MAX_EDITIONS = 3;
constexpr int MAX_DLCS = 128;
constexpr int MAX_DLC_TOGGLES = MAX_DLCS;
constexpr int MAX_CIVS = 64;
constexpr int MAX_EDITION_LINKS = 4;
