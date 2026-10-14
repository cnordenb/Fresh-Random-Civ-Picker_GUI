#include "pch.h"

#include "../Fresh-Random-Civ-Picker_CPPGUI/Assertions.cpp"
#include "../Fresh-Random-Civ-Picker_CPPGUI/ParchmentGeometry.cpp"

// The web version's CSS is the specification: each test names the rule it reproduces.

TEST(Rects, OverlapWhenTheyShareAPixel)
{
    EXPECT_TRUE(RectsOverlap({ 0, 0, 10, 10 }, { 9, 9, 20, 20 }));
    EXPECT_TRUE(RectsOverlap({ 0, 0, 10, 10 }, { 2, 2, 4, 4 }));
}

TEST(Rects, DoNotOverlapWhenTheyOnlyTouch)
{
    EXPECT_FALSE(RectsOverlap({ 0, 0, 10, 10 }, { 10, 0, 20, 10 }));
    EXPECT_FALSE(RectsOverlap({ 0, 0, 10, 10 }, { 0, 10, 10, 20 }));
}

TEST(Rects, HaveAnAreaOnlyWhenWideAndTall)
{
    EXPECT_TRUE(HasArea(PixelRect{ 0, 0, 1, 1 }));
    EXPECT_FALSE(HasArea(PixelRect{ 0, 0, 0, 5 }));
    EXPECT_FALSE(HasArea(PixelRect{ 5, 5, 4, 9 }));
    EXPECT_TRUE(HasArea(FloatRect{ 0.0f, 0.0f, 0.5f, 0.5f }));
    EXPECT_FALSE(HasArea(FloatRect{ 2.0f, 0.0f, 2.0f, 3.0f }));
}

TEST(Rects, BoundingRectCoversBoth)
{
    const PixelRect bounds = BoundingRect({ 5, 20, 10, 30 }, { 0, 25, 8, 40 });
    EXPECT_EQ(bounds.left, 0);
    EXPECT_EQ(bounds.top, 20);
    EXPECT_EQ(bounds.right, 10);
    EXPECT_EQ(bounds.bottom, 40);
}

TEST(DottedLine, OddLengthDotsEveryOtherPixelFromBothEnds)
{
    // outline: 1px dotted along 5 px: Chrome puts dots on both ends.
    EXPECT_TRUE(DottedLineHasDot(0, 5));
    EXPECT_FALSE(DottedLineHasDot(1, 5));
    EXPECT_TRUE(DottedLineHasDot(2, 5));
    EXPECT_FALSE(DottedLineHasDot(3, 5));
    EXPECT_TRUE(DottedLineHasDot(4, 5));
}

TEST(DottedLine, EvenLengthDoublesTheFirstDotToReachTheLastPixel)
{
    // Measured on the web version's focused lobby field: dots at 0, 1, 3, 5, ... of a 424 px side.
    EXPECT_TRUE(DottedLineHasDot(0, 424));
    EXPECT_TRUE(DottedLineHasDot(1, 424));
    EXPECT_FALSE(DottedLineHasDot(2, 424));
    EXPECT_TRUE(DottedLineHasDot(3, 424));
    EXPECT_FALSE(DottedLineHasDot(422, 424));
    EXPECT_TRUE(DottedLineHasDot(423, 424));
}

TEST(DottedLine, NoDotsOutsideTheLine)
{
    EXPECT_FALSE(DottedLineHasDot(-1, 10));
    EXPECT_FALSE(DottedLineHasDot(10, 10));
}

TEST(TileLayout, StretchCoversTheAreaWithOneTile)
{
    const TileSpan span = LayOutTiles({ 40, 6.0f, TileRule::stretch });
    EXPECT_FLOAT_EQ(span.origin, 0.0f);
    EXPECT_FLOAT_EQ(span.length, 40.0f);
}

TEST(TileLayout, RepeatCentresWholeTilesInTheArea)
{
    // border-image-repeat: repeat centres the tiling, so 40 px of 6 px tiles start 2 px before the area.
    const TileSpan span = LayOutTiles({ 40, 6.0f, TileRule::repeat });
    EXPECT_FLOAT_EQ(span.length, 6.0f);
    EXPECT_FLOAT_EQ(span.origin, -1.0f);
}

TEST(TileLayout, RoundScalesTilesToFitAWholeNumber)
{
    // border-image-repeat: round fits round(100 / 38) = 3 tiles of 33.33 px.
    const TileSpan span = LayOutTiles({ 100, 38.0f, TileRule::round });
    EXPECT_FLOAT_EQ(span.origin, 0.0f);
    EXPECT_NEAR(span.length, 100.0f / 3.0f, 0.001f);
}

TEST(TileLayout, RoundKeepsAtLeastOneTile)
{
    const TileSpan span = LayOutTiles({ 10, 38.0f, TileRule::round });
    EXPECT_FLOAT_EQ(span.length, 10.0f);
}

TEST(NineSlice, SplitsTheTargetIntoCornersEdgesAndMiddle)
{
    const NineSliceAreas areas = SplitNineSlice({ 10, 20, 110, 70 }, 6);
    EXPECT_EQ(areas.area[0].right, 16);
    EXPECT_EQ(areas.area[0].bottom, 26);
    EXPECT_EQ(areas.area[4].left, 16);
    EXPECT_EQ(areas.area[4].top, 26);
    EXPECT_EQ(areas.area[4].right, 104);
    EXPECT_EQ(areas.area[4].bottom, 64);
    EXPECT_EQ(areas.area[8].left, 104);
    EXPECT_EQ(areas.area[8].bottom, 70);
}

TEST(Scrollbar, KnobStartsAtTheTopAndEndsAtTheBottomOfTheTrack)
{
    // The knob runs the whole track: top at scroll 0, track length minus knob length at the end.
    EXPECT_EQ(KnobOffset({ 1000, 400, 0 }, 375), 0);
    EXPECT_EQ(KnobOffset({ 1000, 400, 600 }, 375), 354);
    EXPECT_EQ(KnobOffset({ 1000, 400, 300 }, 375), 177);
}

TEST(Scrollbar, NoScrollingWhenTheContentFits)
{
    EXPECT_EQ(MaxScrollTop({ 300, 400, 0 }), 0);
    EXPECT_FALSE(NeedsScrollbar({ 300, 400, 0 }));
    EXPECT_TRUE(NeedsScrollbar({ 401, 400, 0 }));
}

TEST(Scrollbar, ScrollTopIsClampedToTheContent)
{
    EXPECT_EQ(ClampScrollTop({ 1000, 400, 900 }), 600);
    EXPECT_EQ(ClampScrollTop({ 1000, 400, -5 }), 0);
}

TEST(Scrollbar, DraggingTheKnobScrollsInProportion)
{
    // ratio = hidden part / free track length = 600 / 354
    EXPECT_EQ(ScrollTopForKnobDrag({ 0, 177, 600, 354 }), 300);
    EXPECT_EQ(ScrollTopForKnobDrag({ 300, 1000, 600, 354 }), 600);
}

TEST(Scrollbar, HitTestFindsButtonsTrackAndKnob)
{
    // A 429 px bar: 27 px buttons, the track between them, a 21 px knob at track offset 100.
    EXPECT_EQ(HitTestScrollbar({ 5, 429, 100 }), ScrollbarPart::up_button);
    EXPECT_EQ(HitTestScrollbar({ 420, 429, 100 }), ScrollbarPart::down_button);
    EXPECT_EQ(HitTestScrollbar({ 60, 429, 100 }), ScrollbarPart::track_above_knob);
    EXPECT_EQ(HitTestScrollbar({ 130, 429, 100 }), ScrollbarPart::knob);
    EXPECT_EQ(HitTestScrollbar({ 200, 429, 100 }), ScrollbarPart::track_below_knob);
}

TEST(Slider, FillRunsFromTheMinimumToTheMaximum)
{
    EXPECT_FLOAT_EQ(SliderFill({ 1, 10, 1 }), 0.0f);
    EXPECT_FLOAT_EQ(SliderFill({ 1, 10, 10 }), 1.0f);
    EXPECT_FLOAT_EQ(SliderFill({ 1, 10, 4 }), 1.0f / 3.0f);
}

TEST(Slider, KnobTravelsBetweenTheArrowheads)
{
    // The groove runs from 32 to 308 px of the 340 px track: 9.41% to 90.59% of the slider width.
    EXPECT_NEAR(SliderKnobCentre(200, 0.0f), 18.82f, 0.01f);
    EXPECT_NEAR(SliderKnobCentre(200, 1.0f), 181.18f, 0.01f);
}

TEST(Slider, ValueAtFollowsThePointerAndIsClamped)
{
    EXPECT_EQ(SliderValueAt({ 200, { 1, 10, 5 } }, 110.0f), 6);
    EXPECT_EQ(SliderValueAt({ 200, { 1, 10, 5 } }, 0.0f), 1);
    EXPECT_EQ(SliderValueAt({ 200, { 1, 10, 5 } }, 199.0f), 10);
}

namespace
{
    // Every character 10 px wide, so the expected breaks are easy to see.
    struct EvenText
    {
        const wchar_t* text;
        float advances[64];
        int length;
    };

    EvenText MakeEvenText(const wchar_t* text, int length)
    {
        EvenText even = { text, {}, length };
        for (int i = 0; i < length && i < 64; i++) even.advances[i] = 10.0f;
        return even;
    }
}

TEST(WordWrap, BreaksAtTheLastSpaceThatFits)
{
    const EvenText even = MakeEvenText(L"aaa bbb ccc", 11);
    WrappedLines lines;
    ASSERT_TRUE(lines.Wrap({ even.text, even.advances, even.length, 75.0f }));
    ASSERT_EQ(lines.Count(), 2);
    EXPECT_EQ(lines.Line(0).start, 0);
    EXPECT_EQ(lines.Line(0).length, 7);
    EXPECT_EQ(lines.Line(1).start, 8);
    EXPECT_EQ(lines.Line(1).length, 3);
}

TEST(WordWrap, KeepsLineBreaksOfTheText)
{
    const EvenText even = MakeEvenText(L"ab\ncd", 5);
    WrappedLines lines;
    ASSERT_TRUE(lines.Wrap({ even.text, even.advances, even.length, 500.0f }));
    ASSERT_EQ(lines.Count(), 2);
    EXPECT_EQ(lines.Line(0).length, 2);
    EXPECT_EQ(lines.Line(1).start, 3);
}

TEST(WordWrap, AWordLongerThanTheLineOverflowsOnItsOwnLine)
{
    const EvenText even = MakeEvenText(L"abcdefgh ij", 11);
    WrappedLines lines;
    ASSERT_TRUE(lines.Wrap({ even.text, even.advances, even.length, 50.0f }));
    ASSERT_EQ(lines.Count(), 2);
    EXPECT_EQ(lines.Line(0).length, 8);
    EXPECT_EQ(lines.Line(1).start, 9);
}

TEST(WordWrap, AnEmptyTextHasOneEmptyLine)
{
    WrappedLines lines;
    ASSERT_TRUE(lines.Wrap({ L"", nullptr, 0, 50.0f }));
    EXPECT_EQ(lines.Count(), 1);
    EXPECT_EQ(lines.Line(0).length, 0);
}

TEST(WordWrap, TheWidestLineIsMeasuredWithoutTrailingSpaces)
{
    const EvenText even = MakeEvenText(L"aaa bbb ccc", 11);
    WrappedLines lines;
    ASSERT_TRUE(lines.Wrap({ even.text, even.advances, even.length, 75.0f }));
    EXPECT_FLOAT_EQ(lines.WidestLine(), 70.0f);
}

TEST(OpaqueRuns, FindsTheFullyOpaqueStretchesOfARow)
{
    const uint8_t alpha[] = { 0, 255, 255, 128, 255, 255, 255, 0 };
    OpaqueRuns runs;
    ASSERT_TRUE(runs.Scan(alpha, 8));
    ASSERT_EQ(runs.Count(), 2);
    EXPECT_EQ(runs.Run(0).start, 1);
    EXPECT_EQ(runs.Run(0).end, 3);
    EXPECT_EQ(runs.Run(1).start, 4);
    EXPECT_EQ(runs.Run(1).end, 7);
}

TEST(ColourFilter, GrayscaleUsesTheCssLuminanceWeights)
{
    const Rgb grey = Grayscale({ 255, 0, 0 });
    EXPECT_EQ(grey.red, 54);
    EXPECT_EQ(grey.green, 54);
    EXPECT_EQ(grey.blue, 54);
}

TEST(ColourFilter, BrightnessScalesAndClamps)
{
    const Rgb dim = Brightness({ 200, 100, 10 }, 0.88f);
    EXPECT_EQ(dim.red, 176);
    EXPECT_EQ(dim.green, 88);
    EXPECT_EQ(dim.blue, 9);
    EXPECT_EQ(Brightness({ 200, 0, 0 }, 2.0f).red, 255);
}

TEST(ColourFilter, HueRotateByZeroAndSaturateByOneChangeNothing)
{
    const Rgb same = Saturate(HueRotate({ 120, 60, 30 }, 0.0f), 1.0f);
    EXPECT_EQ(same.red, 120);
    EXPECT_EQ(same.green, 60);
    EXPECT_EQ(same.blue, 30);
}

TEST(Tooltip, SitsBelowRightOfTheCursor)
{
    const PixelPoint spot = PlaceTooltip({ { 100, 100 }, { 80, 40 }, { 0, 0, 1000, 800 } });
    EXPECT_EQ(spot.x, 110);
    EXPECT_EQ(spot.y, 120);
}

TEST(Tooltip, MovesLeftAndAboveAtTheEdges)
{
    const PixelPoint spot = PlaceTooltip({ { 980, 790 }, { 80, 40 }, { 0, 0, 1000, 800 } });
    EXPECT_EQ(spot.x, 916);
    EXPECT_EQ(spot.y, 742);
}

TEST(Blur, SpreadsAnOpaqueBlockAndKeepsItsTotal)
{
    static uint8_t mask[32 * 32] = {};
    for (int y = 12; y < 20; y++)
        for (int x = 12; x < 20; x++) mask[y * 32 + x] = 255;
    static ShadowBlur blur;
    ASSERT_TRUE(blur.Blur({ mask, 32, 32, 3.0f }));
    EXPECT_LT(mask[16 * 32 + 16], 255);
    EXPECT_GT(mask[16 * 32 + 16], 0);
    EXPECT_GT(mask[16 * 32 + 9], 0);
    EXPECT_EQ(mask[0], 0);
}

TEST(BoxShadow, KeepsTheBoxOpaqueAndShadesOnlyOutsideIt)
{
    static uint32_t pixels[20 * 20] = {};
    for (uint32_t& pixel : pixels) pixel = 0x00336699u;
    static BoxShadow shadow;
    ASSERT_TRUE(shadow.Apply({ pixels, 20, { 20, 20 }, { 5, 5, 15, 15 }, { 2, 2 }, 1.5f, 0.25f }));
    EXPECT_EQ(pixels[10 * 20 + 10], 0xff336699u);
    EXPECT_GT(pixels[16 * 20 + 16] >> 24, 0u);
    EXPECT_EQ(pixels[16 * 20 + 16] & 0x00ffffffu, 0u);
    EXPECT_EQ(pixels[0], 0u);
}
