set -e
cd "$(git rev-parse --show-toplevel)"

gtest=temp/googletest/googletest
program=Fresh-Random-Civ-Picker_CPPGUI
tests=UnitTesting_FRCP/tests
out=temp/tests

if [ ! -d "$gtest" ]; then
  git clone --quiet --depth 1 --branch v1.14.0 https://github.com/google/googletest temp/googletest
fi
mkdir -p "$out"
if [ ! -f "$out/gtest-all.o" ]; then
  g++ -std=c++17 -O1 -I"$gtest/include" -I"$gtest" -c "$gtest/src/gtest-all.cc" -o "$out/gtest-all.o"
fi

g++ -std=c++17 -O1 -DNOMINMAX -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 \
  -I"$gtest/include" -IUnitTesting_FRCP -I"$program" -I"$program/common" \
  -I"$program/model/catalog" -I"$program/model/picker" \
  -I"$program/parchment/graphics/layout" -I"$program/parchment/graphics/drawing" \
  "$tests/test.cpp" UnitTesting_FRCP/TestMain.cpp "$tests/ParchmentGeometryTest.cpp" "$tests/CivDataTest.cpp" \
  "$tests/ShareLinkTest.cpp" "$tests/HotkeyBindingsTest.cpp" "$out/gtest-all.o" -static -o "$out/tests.exe"

"$out/tests.exe" "$@"
