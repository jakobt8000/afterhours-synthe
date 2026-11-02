// Synthe · AFTERHOURS
// Layout of the final browser test: presets (with category tabs, MONO and OKT) on the left,
// BREDDE / DRIVE / GLITCH as big knobs on top and 14 small knobs in three rows. 940 x 504 drawn at 80%.
#include "PluginEditor.h"
#include "Look.h"

namespace syui
{
using namespace ahui;
using namespace sy;
static juce::String U (const char* s) { return juce::String::fromUTF8 (s); }
static juce::String pct (float v) { return juce::String (juce::roundToInt (v * 100)) + "%"; }

// ---------- Knob ----------
Knob::Knob (juce::RangedAudioParameter& p, juce::String l, std::function<juce::String (float)> f, float s)
    : param (p), attach (p, [this] (float v) { value = param.convertTo0to1 (v); repaint(); }), label (l), fmt (std::move (f)), size (s)
{
    attach.sendInitialUpdate();
}
void Knob::paint (juce::Graphics& g)
{
    g.fillAll (panel);
    const bool big = size > 100;
    const float w = (float) getWidth(), h = (float) getHeight();
    const float lblH = big ? 17.0f : 14.0f, content = size + 2 + lblH + 2 + 14, top = (h - content) * 0.5f;
    drawKnob (g, w * 0.5f, top + size * 0.5f, size, value);
    g.setColour (ink); g.setFont (mono (big ? 13.0f : 11.0f, big ? 3.0f : 1.0f));
    g.drawText (label, juce::Rectangle<float> (0, top + size + 2, w, lblH), juce::Justification::centred);
    g.setColour (muted); g.setFont (mono (11));
    g.drawText (fmt (value), juce::Rectangle<float> (0, top + size + 4 + lblH, w, 14), juce::Justification::centred);
}
void Knob::mouseDown (const juce::MouseEvent&) { start = value; attach.beginGesture(); }
void Knob::mouseDrag (const juce::MouseEvent& e)
{
    value = juce::jlimit (0.0f, 1.0f, start - e.getDistanceFromDragStartY() / (e.mods.isShiftDown() ? 600.0f : 180.0f));
    attach.setValueAsPartOfGesture (param.convertFrom0to1 (value));
    repaint();
}
void Knob::mouseUp (const juce::MouseEvent&) { attach.endGesture(); }
void Knob::mouseDoubleClick (const juce::MouseEvent&) { attach.setValueAsCompleteGesture (param.convertFrom0to1 (param.getDefaultValue())); }
void Knob::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& d)
{
    value = juce::jlimit (0.0f, 1.0f, value + (d.deltaY > 0 ? 0.02f : -0.02f));
    attach.setValueAsCompleteGesture (param.convertFrom0to1 (value));
}

// ---------- PresetList ----------
void PresetList::paint (juce::Graphics& g)
{
    g.fillAll (panel);
    for (size_t r = 0; r < shown.size(); ++r)
    {
        const int i = shown[r];
        const juce::Rectangle<float> row (0, (float) r * 35, (float) getWidth(), 34);
        if (i == owner.proc.getCurrentProgram()) fillDots (g, row, Dots::light);
        g.setColour (line); g.fillRect (row.withY (row.getBottom()).withHeight (1));
        g.setColour (ink); g.setFont (mono (12, 1));
        g.drawText (U (PRESETS[(size_t) i].name), row.withTrimmedLeft (12), juce::Justification::centredLeft, true);
    }
}
void PresetList::mouseUp (const juce::MouseEvent& e)
{
    const int r = (int) (e.position.y / 35);
    if (r < 0 || r >= (int) shown.size()) return;
    owner.proc.setCurrentProgram (shown[(size_t) r]);
    owner.proc.updateHostDisplay (juce::AudioProcessor::ChangeDetails().withProgramChanged (true));
    repaint();
}

// ---------- Content ----------
Content::Content (SynthProcessor& p) : proc (p)
{
    auto& ap = proc.apvts;
    const int mw = W - LEFT - 1;
    auto add = [&] (int id, const char* label, std::function<juce::String (float)> f, float size, int x0, int y, int w, int h) {
        auto* k = knobs.add (new Knob (*ap.getParameter (PIDS[id]), U (label), std::move (f), size));
        addAndMakeVisible (k); k->setBounds (x0, y, w, h);
    };
    // big row
    const float bw = (mw - 2) / 3.0f;
    const int bigH = 165;
    const int bigIds[3] = { WIDTH, DRIVE, GLITCH };
    const char* bigLbl[3] = { "BREDDE", "DRIVE", "GLITCH" };
    for (int i = 0; i < 3; ++i)
    {
        const int x0 = LEFT + 1 + juce::roundToInt (i * (bw + 1)), x1 = LEFT + 1 + juce::roundToInt (i * (bw + 1) + bw);
        add (bigIds[i], bigLbl[i], i == 2 ? std::function<juce::String (float)> ([] (float v) { return v < 0.01f ? juce::String ("FRA") : pct (v); }) : std::function<juce::String (float)> (pct),
             108, x0, 0, x1 - x0, bigH);
    }
    // small rows
    struct K { int id; const char* label; std::function<juce::String (float)> f; };
    auto cut = [] (float v) { const float h = 150 * std::pow (80.0f, v); return h >= 1000 ? juce::String (h / 1000, 1) + " KHZ" : juce::String (juce::roundToInt (h)) + " HZ"; };
    const std::vector<std::vector<K>> rows = {
        { { DETUNE, "DETUNE", pct }, { VOICES, "STEMMER", [] (float v) { return juce::String (1 + juce::roundToInt (v * 8)); } }, { SUB, "SUB", pct }, { UP, "OKT ↑", pct },
          { GLIDE, "GLIDE", [] (float v) { return juce::String (juce::roundToInt (v * 300)) + " MS"; } } },
        { { ZAP, "ZAP", [] (float v) { return "+" + juce::String (juce::roundToInt (v * 24)) + " HT"; } }, { KLIK, "KLIK", pct }, { CUT, "FILTER", cut }, { RES, "RESONANS", pct }, { OTT, "OTT", pct } },
        { { PUMP, "PUMP", pct }, { DELAY, "DELAY", pct }, { RUM, "RUM", pct },
          { VOL, "VOLUME", [] (float v) { const int d = juce::roundToInt (-24 + v * 30); return (d > 0 ? "+" : "") + juce::String (d) + " DB"; } } }
    };
    int y = bigH + 1;
    for (auto& row : rows)
    {
        const int cols = (int) row.size();
        const float cw = (mw - (cols - 1)) / (float) cols;
        for (int i = 0; i < cols; ++i)
        {
            const int x0 = LEFT + 1 + juce::roundToInt (i * (cw + 1)), x1 = LEFT + 1 + juce::roundToInt (i * (cw + 1) + cw);
            add (row[(size_t) i].id, row[(size_t) i].label, row[(size_t) i].f, 66, x0, y, x1 - x0, 112);
        }
        y += 113;
    }

    view.setViewedComponent (&list, false);
    view.setScrollBarsShown (true, false); view.setScrollBarThickness (6);
    view.getVerticalScrollBar().setColour (juce::ScrollBar::thumbColourId, ink.withAlpha (0.35f));
    view.getVerticalScrollBar().setColour (juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (view);
    view.setBounds (0, 60, LEFT, H - 60 - 41);
    refreshList();
    setSize (W, H);
    startTimerHz (15);
}

void Content::refreshList()
{
    list.shown.clear();
    for (size_t i = 0; i < PRESETS.size(); ++i)
        if (proc.category == 0 || PRESETS[i].cat == proc.category) list.shown.push_back ((int) i);
    list.setSize (LEFT - 6, (int) list.shown.size() * 35);
    list.repaint();
    repaint (0, 0, LEFT, H);
}

void Content::timerCallback()
{
    const int st = proc.getCurrentProgram() * 1000 + (proc.apvts.getRawParameterValue ("mono")->load() > 0.5f ? 100 : 0)
                 + juce::roundToInt (proc.apvts.getRawParameterValue ("oct")->load());
    if (st != lastState) { lastState = st; list.repaint(); repaint (0, 0, LEFT, H); }
}

void Content::paint (juce::Graphics& g)
{
    g.fillAll (line);
    // left column: header, category tabs, list (viewport), MONO / OKT bar
    g.setColour (alt); g.fillRect (0, 0, LEFT, 28);
    g.setColour (muted); g.setFont (mono (11, 1));
    g.drawText ("PRESETS", juce::Rectangle<float> (12, 0, LEFT - 12, 28), juce::Justification::centredLeft);
    const float tw = (LEFT - 4) / 5.0f;
    for (int i = 0; i < 5; ++i)
    {
        const juce::Rectangle<float> r (i * (tw + 1), 29, tw, 30);
        g.setColour (panel); g.fillRect (r);
        if (i == proc.category) fillDots (g, r, Dots::light);
        g.setColour (ink); g.setFont (mono (9.5f));
        g.drawText (CAT_NAMES[i], r, juce::Justification::centred);
    }
    g.setColour (panel); g.fillRect (0, 60, LEFT, H - 60 - 41);                 // behind the preset list
    const juce::Rectangle<float> monoR (0, (float) H - 40, 94, 40), octR (95, (float) H - 40, LEFT - 95, 40);
    const bool isMono = proc.apvts.getRawParameterValue ("mono")->load() > 0.5f;
    g.setColour (panel); g.fillRect (monoR); g.fillRect (octR);
    if (isMono) fillDots (g, monoR, Dots::light);
    g.setColour (ink); g.setFont (mono (12, 1));
    g.drawText ("MONO", monoR, juce::Justification::centred);
    g.setFont (mono (14));
    g.drawText (U ("−"), octR.withWidth (32), juce::Justification::centred);
    g.drawText ("+", octR.withTrimmedLeft (octR.getWidth() - 32), juce::Justification::centred);
    g.setFont (mono (11));
    g.drawText ("OKT " + juce::String (juce::roundToInt (proc.apvts.getRawParameterValue ("oct")->load())), octR.reduced (24, 0), juce::Justification::centred);
}

void Content::mouseUp (const juce::MouseEvent& e)
{
    const auto p = e.position;
    if (p.x >= LEFT) return;
    if (p.y >= 29 && p.y < 59)
    {
        proc.category = juce::jlimit (0, 4, (int) (p.x / ((LEFT - 4) / 5.0f + 1)));
        refreshList(); return;
    }
    if (p.y >= H - 40)
    {
        if (p.x < 94)
        {
            auto* m = proc.apvts.getParameter ("mono");
            m->beginChangeGesture(); m->setValueNotifyingHost (m->getValue() > 0.5f ? 0.0f : 1.0f); m->endChangeGesture();
        }
        else
        {
            auto* o = proc.apvts.getParameter ("oct");
            const int cur = juce::roundToInt (o->convertFrom0to1 (o->getValue()));
            const int nv = juce::jlimit (1, 6, cur + (p.x < 95 + 32 ? -1 : (p.x > LEFT - 32 ? 1 : 0)));
            o->beginChangeGesture(); o->setValueNotifyingHost (o->convertTo0to1 ((float) nv)); o->endChangeGesture();
        }
        repaint (0, H - 40, LEFT, 40);
    }
}
} // namespace syui

SynthEditor::SynthEditor (SynthProcessor& p) : AudioProcessorEditor (p), content (p)
{
    addAndMakeVisible (content);
    setResizable (true, true);
    setResizeLimits (470, 252, 1410, 756);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) syui::Content::W / syui::Content::H);
    setSize (752, juce::roundToInt (syui::Content::H * 0.8f));
}

void SynthEditor::resized()
{
    content.setBounds (0, 0, syui::Content::W, syui::Content::H);
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) syui::Content::W));
}
