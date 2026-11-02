// Synthe · AFTERHOURS
// The rage synth engine ported from the browser prototype:
// voice: unison saws (+ octave layer, sub, klik, zap) -> per-voice lowpass with envelope -> amp envelope
// bus:   drive (4x oversampled) -> OTT -> mono low end -> delay + reverb -> pump -> GLITCH -> volume -> limiter
#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace sy;

namespace sy
{
using P = std::pair<int, float>;
const std::vector<Preset> PRESETS = {
    { "RAGE LEAD", 1, true, { P (GLITCH, .25f), P (FORM, .35f), P (WIDTH, .85f), P (DRIVE, .6f), P (DETUNE, .5f), P (SUB, .2f), P (UP, .3f), P (ZAP, .25f), P (KLIK, .2f), P (GLIDE, .35f), P (CUT, .78f), P (RES, .15f), P (OTT, .6f), P (PUMP, .3f), P (DELAY, .25f), P (RUM, .25f) } },
    { "STAB", 2, false, { P (FORM, .02f), P (WIDTH, .75f), P (DRIVE, .7f), P (DETUNE, .45f), P (SUB, .3f), P (UP, .2f), P (ZAP, .5f), P (KLIK, .5f), P (GLIDE, 0), P (CUT, .58f), P (RES, .3f), P (OTT, .7f), P (PUMP, .2f), P (DELAY, .2f), P (RUM, .15f) } },
    { "PLUCK", 3, false, { P (FORM, .17f), P (WIDTH, .7f), P (DRIVE, .35f), P (DETUNE, .35f), P (SUB, .1f), P (UP, .15f), P (ZAP, .2f), P (KLIK, .3f), P (GLIDE, 0), P (CUT, .5f), P (RES, .25f), P (OTT, .5f), P (PUMP, .2f), P (DELAY, .4f), P (RUM, .3f) } },
    { "SUPERSAW PAD", 4, false, { P (FORM, .95f), P (WIDTH, 1), P (DRIVE, .25f), P (DETUNE, .7f), P (VOICES, 1), P (SUB, .1f), P (UP, .3f), P (ZAP, 0), P (KLIK, 0), P (GLIDE, 0), P (CUT, .62f), P (RES, .1f), P (OTT, .4f), P (PUMP, .6f), P (DELAY, .3f), P (RUM, .6f) } },
    { "DIGI LEAD", 1, true, { P (FORM, .28f), P (WIDTH, .4f), P (DRIVE, .9f), P (DETUNE, .2f), P (VOICES, .25f), P (SUB, .25f), P (UP, 0), P (ZAP, .6f), P (KLIK, .3f), P (GLIDE, .5f), P (CUT, .95f), P (RES, .4f), P (OTT, .8f), P (PUMP, .2f), P (DELAY, .3f), P (RUM, .2f) } },
    { "NAT STAB", 2, false, { P (FORM, .06f), P (WIDTH, .6f), P (DRIVE, .5f), P (DETUNE, .5f), P (SUB, .35f), P (UP, 0), P (ZAP, .35f), P (KLIK, .25f), P (CUT, .36f), P (RES, .5f), P (OTT, .55f), P (PUMP, .3f), P (DELAY, .5f), P (RUM, .45f) } },
    { "HYPER", 3, false, { P (GLITCH, .3f), P (FORM, .5f), P (WIDTH, 1), P (DRIVE, .4f), P (DETUNE, .8f), P (SUB, .05f), P (UP, .7f), P (ZAP, .15f), P (KLIK, .15f), P (CUT, .72f), P (RES, .2f), P (OTT, .9f), P (PUMP, .45f), P (DELAY, .35f), P (RUM, .35f) } },
    { "MØRK PAD", 4, false, { P (FORM, .85f), P (WIDTH, .85f), P (DRIVE, .35f), P (DETUNE, .55f), P (SUB, .4f), P (UP, 0), P (ZAP, 0), P (KLIK, 0), P (CUT, .3f), P (RES, .2f), P (OTT, .45f), P (PUMP, .3f), P (DELAY, .2f), P (RUM, .7f) } },
    { "GLIDE SIRENE", 1, true, { P (FORM, .4f), P (WIDTH, .7f), P (DRIVE, .5f), P (DETUNE, .35f), P (SUB, .25f), P (UP, .2f), P (ZAP, .1f), P (KLIK, .1f), P (GLIDE, .8f), P (CUT, .7f), P (RES, .25f), P (OTT, .55f), P (PUMP, .25f), P (DELAY, .45f), P (RUM, .3f) } },
    { "OPIUM LEAD", 1, true, { P (FORM, .42f), P (WIDTH, .7f), P (DRIVE, .55f), P (DETUNE, .45f), P (SUB, .3f), P (UP, .1f), P (ZAP, .2f), P (KLIK, .15f), P (GLIDE, .4f), P (CUT, .45f), P (RES, .35f), P (OTT, .6f), P (PUMP, .3f), P (DELAY, .4f), P (RUM, .5f) } },
    { "SKRIG", 1, true, { P (GLITCH, .4f), P (FORM, .25f), P (WIDTH, .3f), P (DRIVE, 1), P (DETUNE, .1f), P (VOICES, .12f), P (SUB, .2f), P (UP, .3f), P (ZAP, .8f), P (KLIK, .3f), P (GLIDE, .2f), P (CUT, 1), P (RES, .65f), P (OTT, .9f), P (PUMP, .2f), P (DELAY, .3f), P (RUM, .2f) } },
    { "VAMP LEAD", 1, true, { P (GLITCH, .2f), P (FORM, .32f), P (WIDTH, 1), P (DRIVE, .75f), P (DETUNE, .65f), P (SUB, .15f), P (UP, .6f), P (ZAP, .3f), P (KLIK, .2f), P (GLIDE, .25f), P (CUT, .8f), P (RES, .2f), P (OTT, .8f), P (PUMP, .35f), P (DELAY, .3f), P (RUM, .3f) } },
    { "HÅRD STAB", 2, false, { P (GLITCH, .35f), P (FORM, 0), P (WIDTH, .7f), P (DRIVE, 1), P (DETUNE, .4f), P (SUB, .35f), P (UP, .25f), P (ZAP, .7f), P (KLIK, .8f), P (GLIDE, 0), P (CUT, .7f), P (RES, .25f), P (OTT, 1), P (PUMP, .25f), P (DELAY, .15f), P (RUM, .1f) } },
    { "KOR STAB", 2, false, { P (FORM, .1f), P (WIDTH, 1), P (DRIVE, .35f), P (DETUNE, .85f), P (SUB, .1f), P (UP, .5f), P (ZAP, .2f), P (KLIK, .2f), P (GLIDE, 0), P (CUT, .5f), P (RES, .15f), P (OTT, .6f), P (PUMP, .3f), P (DELAY, .3f), P (RUM, .55f) } },
    { "ORGEL STAB", 2, false, { P (FORM, .08f), P (WIDTH, .5f), P (DRIVE, .3f), P (DETUNE, .1f), P (VOICES, .12f), P (SUB, .6f), P (UP, .6f), P (ZAP, .15f), P (KLIK, .35f), P (GLIDE, 0), P (CUT, .45f), P (RES, .1f), P (OTT, .5f), P (PUMP, .2f), P (DELAY, .25f), P (RUM, .25f) } },
    { "GLAS PLUCK", 3, false, { P (FORM, .12f), P (WIDTH, .8f), P (DRIVE, .2f), P (DETUNE, .3f), P (SUB, 0), P (UP, .6f), P (ZAP, .15f), P (KLIK, .4f), P (GLIDE, 0), P (CUT, .75f), P (RES, .45f), P (OTT, .5f), P (PUMP, .2f), P (DELAY, .55f), P (RUM, .4f) } },
    { "DIGICORE ARP", 3, false, { P (GLITCH, .55f), P (FORM, .2f), P (WIDTH, .6f), P (DRIVE, .8f), P (DETUNE, .3f), P (VOICES, .38f), P (SUB, .15f), P (UP, .3f), P (ZAP, .4f), P (KLIK, .3f), P (GLIDE, 0), P (CUT, .85f), P (RES, .5f), P (OTT, .9f), P (PUMP, .3f), P (DELAY, .35f), P (RUM, .2f) } },
    { "MØRK PLUCK", 3, false, { P (FORM, .16f), P (WIDTH, .7f), P (DRIVE, .4f), P (DETUNE, .4f), P (SUB, .4f), P (UP, 0), P (ZAP, .25f), P (KLIK, .25f), P (GLIDE, 0), P (CUT, .3f), P (RES, .55f), P (OTT, .55f), P (PUMP, .25f), P (DELAY, .5f), P (RUM, .4f) } },
    { "NATTE PAD", 4, false, { P (FORM, .78f), P (WIDTH, 1), P (DRIVE, .2f), P (DETUNE, .6f), P (SUB, .2f), P (UP, .2f), P (ZAP, 0), P (KLIK, 0), P (GLIDE, 0), P (CUT, .45f), P (RES, .15f), P (OTT, .45f), P (PUMP, .5f), P (DELAY, .35f), P (RUM, .8f) } },
    { "STØV PAD", 4, false, { P (FORM, .9f), P (WIDTH, .9f), P (DRIVE, .6f), P (DETUNE, .9f), P (SUB, .15f), P (UP, .35f), P (ZAP, 0), P (KLIK, 0), P (GLIDE, 0), P (CUT, .5f), P (RES, .2f), P (OTT, .6f), P (PUMP, .35f), P (DELAY, .25f), P (RUM, .65f) } },
    { "STADION PAD", 4, false, { P (FORM, .7f), P (WIDTH, 1), P (DRIVE, .3f), P (DETUNE, .75f), P (SUB, .2f), P (UP, .5f), P (ZAP, 0), P (KLIK, 0), P (GLIDE, 0), P (CUT, .8f), P (RES, .1f), P (OTT, .7f), P (PUMP, .7f), P (DELAY, .2f), P (RUM, .5f) } }
};
} // namespace sy

static float mtof (float m) { return 440.0f * std::pow (2.0f, (m - 69) / 12.0f); }
static float cutHz (float v) { return 150.0f * std::pow (80.0f, v); }
struct FormEnv { float a, d, s, r, fOct, fd; };
static FormEnv formEnv (float f)
{
    return { 0.001f + f * f * 0.7f, 0.07f + f * 1.4f, f < 0.3f ? 0.0f : (f - 0.3f) / 0.7f * 0.85f, 0.05f + f * 1.4f, 4.2f * (1 - f) + 0.4f, 0.035f + f * 0.9f };
}

juce::AudioProcessorValueTreeState::ParameterLayout SynthProcessor::createLayout()
{
    static const char* names[NUM_P] = { "Form", "Width", "Drive", "Detune", "Voices", "Sub", "Octave up", "Zap", "Click", "Glide",
                                        "Filter", "Resonance", "OTT", "Pump", "Delay", "Reverb", "Volume", "Glitch" };
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    // the first preset is the default sound
    float def[NUM_P]; for (int i = 0; i < NUM_P; ++i) def[i] = BASE[i];
    for (auto [id, v] : PRESETS[0].v) def[id] = v;
    for (int i = 0; i < NUM_P; ++i)
        l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { PIDS[i], 1 }, names[i], juce::NormalisableRange<float> (0, 1), def[i]));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "mono", 1 }, "Mono", PRESETS[0].mono));
    l.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "oct", 1 }, "Octave", 1, 6, 3));
    return l;
}

SynthProcessor::SynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SYNTHE", createLayout())
{
    for (int i = 0; i < NUM_P; ++i) prm[(size_t) i] = apvts.getRawParameterValue (PIDS[i]);
    monoP = apvts.getRawParameterValue ("mono");
    octP = apvts.getRawParameterValue ("oct");
}

const juce::String SynthProcessor::getProgramName (int index)
{
    return juce::String::fromUTF8 (PRESETS[(size_t) juce::jlimit (0, (int) PRESETS.size() - 1, index)].name);
}

void SynthProcessor::setCurrentProgram (int index)
{
    currentPreset = juce::jlimit (0, (int) PRESETS.size() - 1, index);
    const auto& pr = PRESETS[(size_t) currentPreset];
    float vals[NUM_P]; for (int i = 0; i < NUM_P; ++i) vals[i] = BASE[i];
    for (auto [id, v] : pr.v) vals[id] = v;
    vals[VOL] = p (VOL);                                  // the preset keeps your volume, like the prototype
    for (int i = 0; i < NUM_P; ++i)
        if (auto* q = apvts.getParameter (PIDS[i])) q->setValueNotifyingHost (vals[i]);
    if (auto* q = apvts.getParameter ("mono")) q->setValueNotifyingHost (pr.mono ? 1.0f : 0.0f);
}

bool SynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    const float fs = (float) sr;
    bus.setSize (2, samplesPerBlock); klikBuf.setSize (1, samplesPerBlock); revIn.setSize (2, samplesPerBlock);
    os.reset(); os.initProcessing ((size_t) samplesPerBlock);
    ottComp.prepare (fs, -42, 10, 4, 0.002f, 0.09f);
    limiter.prepare (fs, -3, 20, 0, 0.001f, 0.1f);
    setLatencySamples (limiter.len + (int) std::round (os.getLatencyInSamples()));
    for (int c = 0; c < 2; ++c)
    {
        lo[c].set (LP, 160, 0.5f, fs); hi[c].set (HP, 160, 0.5f, fs);
        rHp[c].set (HP, 250, 0.7f, fs); rLp[c].set (LP, 7000, 0.5f, fs);
        crHp[c].set (HP, 400, 0.7f, fs);
        lo[c].reset(); hi[c].reset(); rHp[c].reset(); rLp[c].reset(); crHp[c].reset(); ottShelf[c].reset(); bp[c].reset();
    }
    dHp.set (HP, 300, 0.7f, fs); dDamp.set (LP, 4500, 0.5f, fs); dHp.reset(); dDamp.reset();
    dL.assign ((size_t) (sr * 2) + 8, 0.0f); dR.assign ((size_t) (sr * 2) + 8, 0.0f); dW = 0;
    stBuf.assign ((size_t) (sr * 1) * 2 + 16, 0.0f); stW = 0;
    for (auto& b : tsBuf) b.assign ((size_t) (sr * 1.6) + 8, 0.0f);
    tsW = 0;

    // reverb: 3.2 s noise impulse (decay ^2.8), normalised like Web Audio's ConvolverNode
    const int len = (int) (sr * 3.2);
    juce::AudioBuffer<float> ir (2, len);
    Rng r; double pw = 0;
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < len; ++i) { const float x = r.next() * std::pow (1.0f - (float) i / (float) len, 2.8f); ir.setSample (ch, i, x); pw += x * x; }
    const float power = (float) std::max (0.000125, std::sqrt (pw / (2.0 * len)));
    ir.applyGain (0.00125f / power * (float) (44100.0 / sr));
    reverb.reset();
    reverb.prepare ({ sr, (juce::uint32) samplesPerBlock, 2 });
    reverb.loadImpulseResponse (std::move (ir), sr, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);

    for (auto& v : voices) v.active = false;
    monoVoice = nullptr;
    for (auto* t : { &gDry }) t->jump (1);
    for (auto* t : { &stIn, &stFb, &stOut, &tsIn, &crG, &bpG }) t->jump (0);
    gl.type = -1; pumpG = pumpTarget = 1; busyUntil = 0; gridNext = 0; now = 0;
    master.jump (std::pow (10.0f, (-24 + p (VOL) * 30) / 20));
    curveDrive = -1;
}

// ---------- voices ----------
void SynthProcessor::startVoice (Voice& v, int note, float vel)
{
    const float fs = (float) sr;
    if (p (GLITCH) > 0.01f && rng.uni() < p (GLITCH) * 0.22f) note += rng.uni() < 0.6f ? 12 : -12;
    const float form = p (FORM), fr = mtof ((float) note);
    const auto e = formEnv (form);
    const int n = 1 + juce::roundToInt (p (VOICES) * 8);
    const float det = p (DETUNE) * (0.7f + 0.6f * form) * 55, width = p (WIDTH);
    const bool spread = form > 0.45f;
    v = Voice();
    v.active = true; v.note = note; v.vel = vel;
    v.freq.jump (fr);
    auto add = [&] (float mult, float cents, float pan, float gain, bool sine) {
        if (v.nOsc >= (int) v.osc.size()) return;
        auto& o = v.osc[(size_t) v.nOsc++];
        o.mult = mult; o.ratio = std::pow (2.0f, cents / 1200.0f); o.gain = gain; o.sine = sine;
        o.ph = spread ? rng.uni() : 0.0;
        const float x = (pan + 1) * 0.5f; o.gL = std::cos (x * kPi * 0.5f); o.gR = std::sin (x * kPi * 0.5f); // StereoPanner, mono input
    };
    const float ug = 0.3f / std::sqrt ((float) n);
    for (int i = 0; i < n; ++i)
    {
        const float x = n == 1 ? 0.0f : (float) i / (n - 1) * 2 - 1, sh = (x < 0 ? -1.0f : 1.0f) * std::pow (std::abs (x), 1.4f);
        add (1, sh * det, n == 1 ? 0.0f : x * width * (i % 2 ? 1.0f : -1.0f), ug, false);
    }
    if (p (UP) > 0.01f) { add (2, -det * 0.6f, -width * 0.8f, p (UP) * 0.22f, false); add (2, det * 0.6f, width * 0.8f, p (UP) * 0.22f, false); }
    if (p (SUB) > 0.01f) { v.subIdx = v.nOsc; add (0.5f, 0, 0, p (SUB) * 0.7f, true); }
    // zap: pitch starts high and drops into place
    v.zap = p (ZAP) * 2400 * (1 - form * 0.8f);
    v.zapCoef = std::exp (-1.0f / ((0.006f + form * 0.02f) * fs)); v.zapDelay = (int) (0.001 * sr);
    // filter envelope
    v.q = 0.7f + p (RES) * 11;
    v.fBase = cutHz (p (CUT)); v.fCut = std::min (18000.0f, v.fBase * std::pow (2.0f, e.fOct));
    v.fCoef = std::exp (-1.0f / (e.fd / 3 * fs)); v.fDelay = (int) (0.002 * sr);
    // amp envelope
    v.aN = std::max (1, (int) (e.a * sr)); v.aInc = vel / (float) v.aN; v.amp = 0;
    v.sus = vel * e.s; v.dCoef = std::exp (-1.0f / (e.d / 3 * fs)); v.rCoef = std::exp (-1.0f / (e.r / 4 * fs));
    v.autoEnd = e.s == 0 ? (int) ((e.a + e.d * 1.6f + 0.1f) * sr) : 0;
    // klik: noise burst + falling blip
    const float k = p (KLIK);
    if (k > 0.01f)
    {
        v.kN = (int) (0.03 * sr);
        v.kNoise = k * 0.6f * vel; v.kNoiseMul = std::pow (0.0005f / v.kNoise, 1.0f / (0.025f * fs));
        v.kBlip = k * 0.3f * vel; v.kBlipMul = std::pow (0.0005f / v.kBlip, 1.0f / (0.02f * fs));
        v.kF = fr * 4; v.kFMul = std::pow (0.25f, 1.0f / (0.012f * fs));
        v.kHp.set (HP, 2500, 1, fs); v.kHp.reset();
    }
    v.lpL.reset(); v.lpR.reset();
}

void SynthProcessor::glideTo (Voice& v, int note)
{
    const float gt = std::max (0.005f, p (GLIDE) * 0.3f);
    v.freq.to (mtof ((float) note), gt / 3, (float) sr);
    v.note = note;
}

void SynthProcessor::releaseVoice (Voice& v)
{
    if (v.released) return;
    v.released = true;
    if (monoVoice == &v) monoVoice = nullptr;
}

void SynthProcessor::noteOn (int note, float vel)
{
    const bool mono = monoP->load() > 0.5f;
    if (mono && monoVoice != nullptr && monoVoice->active && ! monoVoice->released)
    {
        glideTo (*monoVoice, note); monoVoice->held.push_back (note); return;
    }
    Voice* slot = nullptr;
    for (auto& v : voices) if (! v.active) { slot = &v; break; }
    if (slot == nullptr) { slot = &voices[0]; for (auto& v : voices) if (v.age > slot->age) slot = &v; }
    startVoice (*slot, note, vel);
    if (mono) { monoVoice = slot; slot->held = { note }; }
}

void SynthProcessor::noteOff (int note)
{
    for (auto& v : voices)
    {
        if (! v.active || v.released) continue;
        if (! v.held.empty())
        {
            auto it = std::find (v.held.begin(), v.held.end(), note);
            if (it == v.held.end()) continue;
            v.held.erase (it);
            if (! v.held.empty()) glideTo (v, v.held.back());
            else releaseVoice (v);
        }
        else if (v.note == note) releaseVoice (v);
    }
}

void SynthProcessor::renderVoices (float* L, float* R, float* kl, int n)
{
    const float fs = (float) sr, dt = 1.0f / fs;
    for (auto& v : voices)
    {
        if (! v.active) continue;
        for (int i = 0; i < n; ++i)
        {
            // envelopes
            if (v.released) v.amp *= v.rCoef;
            else if (v.aN > 0) { v.amp += v.aInc; if (--v.aN == 0) v.amp = v.vel; }
            else v.amp = v.sus + (v.amp - v.sus) * v.dCoef;
            ++v.age;
            if ((v.released && v.amp < 1e-4f) || (v.autoEnd > 0 && v.age > v.autoEnd)) { v.active = false; if (monoVoice == &v) monoVoice = nullptr; break; }
            if (v.fDelay > 0) --v.fDelay; else v.fCut = v.fBase + (v.fCut - v.fBase) * v.fCoef;
            if ((i & 15) == 0) { v.lpL.set (LP, v.fCut, v.q, fs); v.lpR.set (LP, v.fCut, v.q, fs); }
            if (v.zapDelay > 0) --v.zapDelay; else v.zap *= v.zapCoef;
            const float root = v.freq.next(), zr = v.zap > 0.01f ? std::pow (2.0f, v.zap / 1200.0f) : 1.0f;

            float sl = 0, sr2 = 0, sub = 0;
            for (int k = 0; k < v.nOsc; ++k)
            {
                auto& o = v.osc[(size_t) k];
                const float f = root * o.mult * o.ratio * zr, inc = f * dt;
                const float s = o.sine ? std::sin (2 * kPi * (float) o.ph) : osc (SAW, (float) o.ph, inc);
                o.ph += inc; if (o.ph >= 1) o.ph -= 1;
                if (k == v.subIdx) sub += s * o.gain;
                else { sl += s * o.gain * o.gL; sr2 += s * o.gain * o.gR; }
            }
            // saws through the filter, sub joins after it; both follow the amp envelope
            const float l = (v.lpL.process (sl) + sub) * v.amp, r = (v.lpR.process (sr2) + sub) * v.amp;
            L[i] += l; R[i] += r;

            if (v.kN > 0)
            {
                --v.kN;
                const float noise = v.kHp.process (rng.next()) * v.kNoise;
                const float blip = std::sin (2 * kPi * v.kPh) * v.kBlip;
                v.kPh += v.kF * dt; if (v.kPh >= 1) v.kPh -= 1;
                v.kNoise *= v.kNoiseMul; v.kBlip *= v.kBlipMul; if (v.kF > root) v.kF *= v.kFMul;
                if (v.kN < (int) (0.005 * sr)) v.kBlip = 0;
                kl[i] += noise + blip;
            }
        }
    }
}

// ---------- glitch ----------
void SynthProcessor::maybeGlitch (int offset)
{
    const double t = (double) (now + offset);
    const float gv = p (GLITCH);
    if (gv < 0.01f || t < busyUntil) return;
    if (rng.uni() > std::pow (gv, 1.4f) * 0.38f) return;
    const double sd = 60.0 / hostBpm / 4 * sr;
    static const float W[6] = { 3, 1.4f, 1.6f, 2, 2, 1.5f };
    float r = rng.uni() * 11.5f; int type = 0;
    for (int k = 0; k < 6; ++k) { if ((r -= W[k]) < 0) { type = k; break; } }
    gl = Glitch();
    gl.type = type; gl.t = -offset; gl.D = (int) (sd * (rng.uni() < 0.5f ? 2 : 4));
    if (type == 0) gl.L = std::max (16, (int) (sd / (rng.uni() < 0.5f ? 2 : 4)));
    if (type == 3)
    {
        gl.zapSquare = rng.uni() < 0.5f; gl.up = rng.uni() < 0.35f;
        gl.zapLen = (int) ((0.06 + rng.uni() * 0.1) * sr);
        gl.zf0 = gl.up ? 90.0f : 3200.0f + rng.uni() * 2000.0f; gl.zf1 = gl.up ? 3500.0f : 70.0f;
        busyUntil = t + sd; return;
    }
    if (type == 5) gl.up = rng.uni() < 0.5f;
    busyUntil = t + gl.D + sd;
}

// ---------- block ----------
void SynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (bus.getNumSamples() < n) prepareToPlay (sr, n);
    const float fs = (float) sr;

    double ppq = 0; hostPlaying = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) hostBpm = juce::jlimit (30.0, 300.0, *b);
            if (auto q = pos->getPpqPosition()) { ppq = *q; hostPlaying = pos->getIsPlaying(); }
        }

    bus.clear(); klikBuf.clear();
    float* L = bus.getWritePointer (0); float* R = bus.getWritePointer (1); float* K = klikBuf.getWritePointer (0);

    // MIDI, sample accurate: render up to each event
    const int transpose = (juce::roundToInt (octP->load()) - 3) * 12;
    int done = 0;
    for (const auto meta : midi)
    {
        const int at = juce::jlimit (0, n, meta.samplePosition);
        if (at > done) { renderVoices (L + done, R + done, K + done, at - done); done = at; }
        const auto m = meta.getMessage();
        if (m.isNoteOn()) noteOn (m.getNoteNumber() + transpose, m.getFloatVelocity());
        else if (m.isNoteOff()) noteOff (m.getNoteNumber() + transpose);
        else if (m.isAllNotesOff() || m.isAllSoundOff()) for (auto& v : voices) releaseVoice (v);
    }
    if (done < n) renderVoices (L + done, R + done, K + done, n - done);
    for (int i = 0; i < n; ++i) { L[i] += K[i]; R[i] += K[i]; }   // klik goes straight to the bus

    // grid: 16ths for GLITCH, quarters for PUMP (host transport; GLITCH also runs on a free clock while notes sound)
    const double spb = 60.0 / hostBpm * sr;     // samples per beat
    std::vector<int> quarters;
    if (hostPlaying)
    {
        for (int64_t k = (int64_t) std::ceil (ppq * 4 - 1e-9);; ++k)
        {
            const double off = (k / 4.0 - ppq) * spb;
            if (off >= n) break;
            if (off < 0) continue;
            maybeGlitch ((int) off);
            if (k % 4 == 0) quarters.push_back ((int) off);
        }
        gridNext = 0;
    }
    else
    {
        bool sounding = false; for (auto& v : voices) sounding |= v.active && ! v.released;
        if (! sounding) gridNext = 0;
        else
        {
            if (gridNext <= 0) gridNext = (double) now + 0.03 * sr;
            while (gridNext < (double) (now + n)) { maybeGlitch ((int) (gridNext - (double) now)); gridNext += spb / 4; }
        }
    }

    // drive (4x oversampled waveshaper, same curve as the prototype)
    const float drive = p (DRIVE);
    if (std::abs (drive - curveDrive) > 1e-4f)
    {
        curveDrive = drive; curve.resize (4096);
        const float k = 0.5f + drive * 14, off = 0.04f * drive, n0 = std::tanh (k * off);
        float mx = 0;
        for (int i = 0; i < 4096; ++i) { const float x = i / 2047.5f - 1; curve[(size_t) i] = std::tanh (k * (x + off)) - n0; mx = std::max (mx, std::abs (curve[(size_t) i])); }
        for (auto& c : curve) c /= mx;
    }
    preDrive.to (0.6f + drive * 2.2f, 0.02f, fs); postDrive.to (0.9f / (0.6f + drive * 1.6f), 0.02f, fs);
    for (int i = 0; i < n; ++i) { const float g = preDrive.next(); L[i] *= g; R[i] *= g; }
    {
        juce::dsp::AudioBlock<float> blk (bus.getArrayOfWritePointers(), 2, (size_t) n);
        auto up = os.processSamplesUp (blk);
        for (size_t ch = 0; ch < 2; ++ch)
        {
            float* d = up.getChannelPointer (ch);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
            {
                const float x = juce::jlimit (-1.0f, 1.0f, d[i]), pos = (x + 1) * 2047.5f;
                const int i0 = std::min (4094, (int) pos); const float fr = pos - i0;
                d[i] = curve[(size_t) i0] + (curve[(size_t) i0 + 1] - curve[(size_t) i0]) * fr;
            }
        }
        os.processSamplesDown (blk);
    }

    // OTT, mono low end, delay + reverb sends
    ottWet.to (p (OTT) * 0.28f, 0.02f, fs); ottDry.to (1 - p (OTT) * 0.45f, 0.02f, fs);
    for (auto& s : ottShelf) s.set (HSHELF, 3500, 0, fs, p (OTT) * 5);
    dSend.to (p (DELAY) * 0.7f, 0.02f, fs); rSend.to (p (RUM) * 0.9f, 0.02f, fs);
    dTime.to ((float) (0.75 * 60.0 / hostBpm * sr), 0.02f, fs);
    float* rl = revIn.getWritePointer (0); float* rr = revIn.getWritePointer (1);
    const int dLen = (int) dL.size();
    for (int i = 0; i < n; ++i)
    {
        const float pd = postDrive.next();
        float l = L[i] * pd, r = R[i] * pd;
        float cl = l, cr = r; ottComp.process (cl, cr);
        const float ow = ottWet.next(), od = ottDry.next();
        l = l * od + ottShelf[0].process (cl) * ow; r = r * od + ottShelf[1].process (cr) * ow;
        const float m = (lo[0].process (l) + lo[1].process (r)) * 0.5f;
        l = m + hi[0].process (l); r = m + hi[1].process (r);
        // ping-pong delay (1/8 dotted): send -> L -> R -> damp -> feedback -> L
        const float dtS = dTime.next();
        float rp = (float) dW - dtS; while (rp < 0) rp += (float) dLen;
        const int i0 = (int) rp; const float fr = rp - i0, i1f = (float) ((i0 + 1) % dLen);
        const float yl = dL[(size_t) i0] + (dL[(size_t) i1f] - dL[(size_t) i0]) * fr;
        const float yr = dR[(size_t) i0] + (dR[(size_t) i1f] - dR[(size_t) i0]) * fr;
        const float send = dHp.process ((l + r) * 0.5f * dSend.next() * 2.0f) * 0.5f;
        dL[(size_t) dW] = send + dDamp.process (yr) * 0.42f;
        dR[(size_t) dW] = yl;
        dW = (dW + 1) % dLen;
        const float rs = rSend.next();
        rl[i] = rHp[0].process (l * rs); rr[i] = rHp[1].process (r * rs);
        L[i] = l + yl; R[i] = r + yr;
    }
    {
        juce::dsp::AudioBlock<float> blk (revIn.getArrayOfWritePointers(), 2, (size_t) n);
        reverb.process (juce::dsp::ProcessContextReplacing<float> (blk));
    }

    // pump, glitch, volume, limiter
    master.to (std::pow (10.0f, (-24 + p (VOL) * 30) / 20), 0.02f, fs);
    const float pump = p (PUMP);
    size_t qi = 0;
    const int stLen = (int) stBuf.size() / 2, tsLen = (int) tsBuf[0].size();
    const float tau3 = std::exp (-1.0f / (0.003f * fs));
    for (auto* t : { &gDry, &stIn, &stFb, &stOut, &tsIn, &crG, &bpG }) t->coef = tau3;
    float* oL = buffer.getWritePointer (0); float* oR = buffer.getWritePointer (1);
    for (int i = 0; i < n; ++i)
    {
        float l = L[i] + rLp[0].process (rl[i]), r = R[i] + rLp[1].process (rr[i]);
        // pump on every beat while the host plays
        if (qi < quarters.size() && quarters[qi] == i)
        {
            ++qi;
            if (pump > 0.01f) { pumpG = 1 - pump * 0.85f; pumpTarget = 1; pumpHold = (int) (0.012 * sr); pumpCoef = std::exp (-1.0f / ((float) spb * 0.22f)); }
        }
        if (pumpHold > 0) --pumpHold; else pumpG = pumpTarget + (pumpG - pumpTarget) * pumpCoef;
        l *= pumpG; r *= pumpG;

        // GLITCH stage
        float dryT = 1, stInT = 0, stFbT = 0, stOutT = 0, tsInT = 0, crT = 0, bpT = 0, add = 0;
        int stDelay = std::max (1, gl.L);
        float tsDelay = 0.005f * fs;
        if (gl.type >= 0)
        {
            const int k = gl.t++;
            const int D = gl.D;
            if (k >= 0)
            {
                switch (gl.type)
                {
                    case 0: stInT = k < gl.L ? 1.0f : 0.0f; stFbT = stOutT = (k >= gl.L && k < D) ? 1.0f : 0.0f; stFbT *= 0.97f; dryT = (k >= gl.L && k < D) ? 0.0f : 1.0f; break;
                    case 1:
                    {
                        const float x = std::min (1.0f, (float) k / (0.98f * D));
                        tsDelay = (0.005f + (float) D / fs * 0.5f * x * x) * fs;
                        if (k < (int) (0.75f * D)) tsInT = 1; else { tsInT = 0; tsIn.coef = std::exp (-1.0f / (0.1f * D)); }
                        dryT = k < D ? 0.0f : 1.0f;
                        break;
                    }
                    case 2: crT = k < D ? 0.9f : 0.0f; dryT = k < D ? 0.0f : 1.0f; break;
                    case 3:
                        if (k < gl.zapLen + (int) (0.02 * sr))
                        {
                            const float x = (float) k / (float) gl.zapLen;
                            const float f = gl.zf0 * std::pow (gl.zf1 / gl.zf0, std::min (1.0f, x));
                            const float g = 0.13f * std::pow (0.001f / 0.13f, std::min (1.0f, (float) k / (gl.zapLen + 0.02f * fs)));
                            add = (gl.zapSquare ? osc (SQUARE, gl.zph, f / fs) : osc (SAW, gl.zph, f / fs)) * g;
                            gl.zph += f / fs; if (gl.zph >= 1) gl.zph -= 1;
                        }
                        break;
                    case 4:
                        if (k < D) { const int sl = std::max (1, (int) (60.0 / hostBpm / 8 * sr)); gDry.v = ((k / sl) % 2) ? 0.0f : 1.0f; dryT = gDry.v; }
                        break;
                    case 5:
                    {
                        if (k < D && (k & 15) == 0)
                        {
                            const float x = (float) k / (float) D;
                            const float f = gl.up ? 250.0f * std::pow (24.0f, x) : 6000.0f * std::pow (1.0f / 24.0f, x);
                            for (auto& b : bp) { const float zz1 = b.z1, zz2 = b.z2; b.set (BP, f, 6, fs); b.z1 = zz1; b.z2 = zz2; }
                        }
                        bpT = k < D ? 2.2f : 0.0f; dryT = k < D ? 0.0f : 1.0f;
                        break;
                    }
                    default: break;
                }
                if (k > D + (int) (0.3 * sr)) gl.type = -1;
            }
        }
        gDry.target = dryT; stIn.target = stInT; stFb.target = stFbT; stOut.target = stOutT; tsIn.target = tsInT; crG.target = crT; bpG.target = bpT;
        const float gd = gl.type == 4 ? gDry.v : gDry.next();
        const float si = stIn.next(), sf = stFb.next(), so = stOut.next(), ti = tsIn.next(), cg = crG.next(), bg = bpG.next();
        // stutter loop
        const int rIdx = ((stW - stDelay) % stLen + stLen) % stLen;
        const float sdl = stBuf[(size_t) rIdx * 2], sdr = stBuf[(size_t) rIdx * 2 + 1];
        stBuf[(size_t) stW * 2] = l * si + sdl * sf; stBuf[(size_t) stW * 2 + 1] = r * si + sdr * sf;
        stW = (stW + 1) % stLen;
        // tape stop (delay line whose time grows: pitch dives)
        tsBuf[0][(size_t) tsW] = l * ti; tsBuf[1][(size_t) tsW] = r * ti;
        float tp = (float) tsW - tsDelay; while (tp < 0) tp += (float) tsLen;
        const int t0 = (int) tp; const float tf = tp - t0, t1 = (float) ((t0 + 1) % tsLen);
        const float tl = tsBuf[0][(size_t) t0] + (tsBuf[0][(size_t) t1] - tsBuf[0][(size_t) t0]) * tf;
        const float tr = tsBuf[1][(size_t) t0] + (tsBuf[1][(size_t) t1] - tsBuf[1][(size_t) t0]) * tf;
        tsW = (tsW + 1) % tsLen;
        // bitcrush (3-step staircase) and band-pass sweep
        auto crush = [] (float x) { return std::round (juce::jlimit (-1.0f, 1.0f, x) * 3.0f) / 3.0f; };
        const float cl = crHp[0].process (crush (l * cg)), cr = crHp[1].process (crush (r * cg));
        const float bl = bp[0].process (l * bg), br = bp[1].process (r * bg);
        l = l * gd + sdl * so + tl + cl + bl + add;
        r = r * gd + sdr * so + tr + cr + br + add;

        const float mg = master.next();
        l *= mg; r *= mg;
        limiter.process (l, r);
        auto ceilf = [] (float x) { const float a = std::abs (x); return a <= 0.9f ? x : std::copysign (0.9f + 0.1f * std::tanh ((a - 0.9f) / 0.1f), x); };
        oL[i] = ceilf (l); oR[i] = ceilf (r);
    }
    now += n;
}

void SynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto st = apvts.copyState();
    st.setProperty ("preset", currentPreset, nullptr);
    st.setProperty ("category", category, nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, destData);
}

void SynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto st = juce::ValueTree::fromXml (*xml);
        if (! st.isValid()) return;
        currentPreset = juce::jlimit (0, (int) PRESETS.size() - 1, (int) st.getProperty ("preset", 0));
        category = juce::jlimit (0, 4, (int) st.getProperty ("category", 0));
        apvts.replaceState (st);
    }
}

juce::AudioProcessorEditor* SynthProcessor::createEditor() { return new SynthEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SynthProcessor(); }
