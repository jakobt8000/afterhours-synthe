// AFTERHOURS shared look: grey panel, white ink, everything drawn as dots ("ink on paper", inverted).
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace ahui
{
// palette (same values as the browser test versions)
inline const juce::Colour panel { 0xff505053 }, line { 0xff2e2e31 }, ink { 0xffffffff }, muted { 0xffc8c8cb }, dimArc { 0xff848487 }, alt { 0xff5a5a5d }, head { 0xff5e5e61 };

inline juce::Font mono (float px, float spacingPx = 0)
{
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor (BinaryData::IBMPlexMonoRegular_ttf, BinaryData::IBMPlexMonoRegular_ttfSize);
    juce::Font f { juce::FontOptions (tf).withPointHeight (px) };
    if (spacingPx != 0) f.setExtraKerningFactor (spacingPx / px);
    return f;
}

// Stroke a path as a row of round dots (SVG: stroke-dasharray "0.01 gap" with round caps)
inline void dotPath (juce::Graphics& g, const juce::Path& path, float spacing, float diameter)
{
    juce::PathFlatteningIterator it (path, {}, 0.15f);
    float carry = 0; int sub = -1;
    const float r = diameter * 0.5f;
    while (it.next())
    {
        if (it.subPathIndex != sub) { sub = it.subPathIndex; carry = 0; }
        const float dx = it.x2 - it.x1, dy = it.y2 - it.y1, len = std::sqrt (dx * dx + dy * dy);
        if (len <= 0) continue;
        while (carry <= len)
        {
            const float t = carry / len;
            g.fillEllipse (it.x1 + dx * t - r, it.y1 + dy * t - r, diameter, diameter);
            carry += spacing;
        }
        carry -= len;
    }
}

// Light dot texture used for "selected" areas (CSS: radial-gradient .55px on a 3px grid, 60% ink)
inline void stipple (juce::Graphics& g, juce::Rectangle<float> r, float alpha = 0.6f, float step = 3.0f, float d = 1.1f)
{
    g.setColour (ink.withAlpha (alpha));
    for (float y = r.getY() + step * 0.5f; y < r.getBottom(); y += step)
        for (float x = r.getX() + step * 0.5f; x < r.getRight(); x += step)
            g.fillEllipse (x - d * 0.5f, y - d * 0.5f, d, d);
}


// Dot textures as tiled images (fast enough to fill many cells every frame)
enum class Dots { light, grain, dense, accent };
inline const juce::Image& dotTile (Dots kind)
{
    struct Spec { float step, d, a; int periods; };
    static const Spec specs[4] = { { 3.0f, 1.1f, 0.6f, 1 }, { 4.0f, 1.4f, 0.85f, 1 }, { 2.2f, 1.9f, 1.0f, 5 }, { 1.7f, 2.1f, 1.0f, 10 } };
    static juce::Image tiles[4];
    auto& img = tiles[(int) kind];
    if (img.isNull())
    {
        const auto& s = specs[(int) kind];
        const float S = 4.0f, size = s.step * (float) s.periods;
        const int px = juce::roundToInt (size * S);
        img = juce::Image (juce::Image::ARGB, px, px, true);
        juce::Graphics g (img);
        g.addTransform (juce::AffineTransform::scale ((float) px / size));
        g.setColour (ink.withAlpha (s.a));
        for (int y = 0; y < s.periods; ++y)
            for (int x = 0; x < s.periods; ++x)
            {
                const float cx = (x + 0.5f) * s.step, cy = (y + 0.5f) * s.step;
                g.fillEllipse (cx - s.d * 0.5f, cy - s.d * 0.5f, s.d, s.d);
            }
    }
    return img;
}
inline float tileSize (Dots kind) { static const float sz[4] = { 3.0f, 4.0f, 11.0f, 17.0f }; return sz[(int) kind]; }
inline void fillDots (juce::Graphics& g, juce::Rectangle<float> r, Dots kind)
{
    const auto& img = dotTile (kind);
    const float sc = tileSize (kind) / (float) img.getWidth();
    g.setFillType (juce::FillType (img, juce::AffineTransform::scale (sc).translated (r.getX(), r.getY())));
    g.fillRect (r);
    g.setFillType (juce::FillType());
}

// SVG angle (0 = 3 o'clock, clockwise) -> JUCE arc angle (0 = 12 o'clock, clockwise)
inline float rad (float svgDeg) { return (svgDeg + 90.0f) * juce::MathConstants<float>::pi / 180.0f; }

// The dotted AFTERHOURS knob: dim outer track, lit arc, dotted ring, centre dot and dotted needle
inline void drawKnob (juce::Graphics& g, float cx, float cy, float size, float v)
{
    // geometry of the Synthe test: r = 0.29 size, lr = 0.41 size, "big" styling above 100 px with an inner dim ring
    const bool big = size > 100;
    const float r = size * 0.29f, lr = size * 0.41f;
    juce::Path track; track.addCentredArc (cx, cy, lr, lr, 0, rad (135), rad (405), true);
    g.setColour (dimArc); dotPath (g, track, big ? 3.6f : 3.2f, big ? 2.0f : 1.5f);
    g.setColour (ink);
    if (v > 0.004f)
    {
        juce::Path lit; lit.addCentredArc (cx, cy, lr, lr, 0, rad (135), rad (135 + 270 * v), true);
        dotPath (g, lit, big ? 2.6f : 2.1f, big ? 3.0f : 2.4f);
    }
    juce::Path ring; ring.addCentredArc (cx, cy, r, r, 0, rad (0), rad (360), true);
    dotPath (g, ring, big ? 3.0f : 2.6f, big ? 2.0f : 1.5f);
    if (big)
    {
        juce::Path inner; inner.addCentredArc (cx, cy, r - 8, r - 8, 0, rad (0), rad (360), true);
        g.setColour (juce::Colour (0xff78787b)); dotPath (g, inner, 3.0f, 1.2f); g.setColour (ink);
    }
    const float cd = big ? 4.4f : 3.2f;
    g.fillEllipse (cx - cd * 0.5f, cy - cd * 0.5f, cd, cd);
    const float a = (135 + 270 * v) * juce::MathConstants<float>::pi / 180.0f;
    juce::Path nd; nd.startNewSubPath (cx, cy); nd.lineTo (cx + (r - 4) * std::cos (a), cy + (r - 4) * std::sin (a));
    dotPath (g, nd, big ? 3.0f : 2.4f, big ? 3.0f : 2.4f);
}
} // namespace ahui
