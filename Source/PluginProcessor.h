// Synthe · AFTERHOURS
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>
#include "Dsp.h"

namespace sy
{
// knob ids in the order the prototype's P object used them
constexpr int NUM_P = 18;
enum PId { FORM, WIDTH, DRIVE, DETUNE, VOICES, SUB, UP, ZAP, KLIK, GLIDE, CUT, RES, OTT, PUMP, DELAY, RUM, VOL, GLITCH };
inline const char* PIDS[NUM_P] = { "form", "width", "drive", "detune", "voices", "sub", "up", "zap", "klik", "glide",
                                   "cut", "res", "ott", "pump", "delay", "rum", "vol", "glitch" };
inline const float BASE[NUM_P] = { .35f, .8f, .5f, .5f, .75f, .2f, .25f, .25f, .2f, .3f, .7f, .15f, .5f, .25f, .25f, .25f, .75f, 0 };

struct Preset { const char* name; int cat; bool mono; std::vector<std::pair<int, float>> v; };
extern const std::vector<Preset> PRESETS;
inline const char* CAT_NAMES[5] = { "ALLE", "LEAD", "STAB", "PLUCK", "PAD" };
} // namespace sy

class SynthProcessor : public juce::AudioProcessor
{
public:
    SynthProcessor();
    ~SynthProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Synthe"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return (int) sy::PRESETS.size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    int category = 0;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float p (int id) const { return prm[(size_t) id]->load(); }

    struct Osc { double ph = 0; float mult = 1, ratio = 1, gL = 1, gR = 1, gain = 0; bool sine = false; };
    struct Voice
    {
        bool active = false, released = false;
        int note = -1, age = 0, autoEnd = 0;
        float vel = 1;
        std::array<Osc, 12> osc; int nOsc = 0; int subIdx = -1;
        sy::Target freq;               // glide target (Hz of the root)
        float zap = 0, zapCoef = 0; int zapDelay = 0;
        // amp env
        float amp = 0, aInc = 0, sus = 0, dCoef = 0, rCoef = 0; int aN = 0;
        // filter env
        float fCut = 1000, fBase = 1000, fCoef = 0; int fDelay = 0; float q = 0.7f;
        sy::Biquad lpL, lpR;
        // klik layer
        int kN = 0; float kNoise = 0, kNoiseMul = 1, kBlip = 0, kBlipMul = 1, kF = 0, kFMul = 1, kPh = 0; sy::Biquad kHp;
        std::vector<int> held;         // mono mode: notes held on this voice
    };
    void noteOn (int note, float vel);
    void noteOff (int note);
    void startVoice (Voice& v, int note, float vel);
    void glideTo (Voice& v, int note);
    void releaseVoice (Voice& v);
    void maybeGlitch (int offset);
    void renderVoices (float* L, float* R, float* klik, int n);

    std::array<std::atomic<float>*, sy::NUM_P> prm {};
    std::atomic<float> *monoP = nullptr, *octP = nullptr;
    int currentPreset = 0;
    double sr = 44100.0;
    sy::Rng rng;
    std::array<Voice, 16> voices;
    Voice* monoVoice = nullptr;

    // bus
    juce::dsp::Oversampling<float> os { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    juce::AudioBuffer<float> bus, klikBuf;
    sy::Compressor ottComp, limiter;
    sy::Biquad ottShelf[2], lo[2], hi[2], dHp, dDamp, rHp[2], rLp[2];
    sy::Target preDrive, postDrive, ottWet, ottDry, dSend, rSend, master, dTime;
    float curveDrive = -1; std::vector<float> curve;
    std::vector<float> dL, dR; int dW = 0;
    juce::dsp::Convolution reverb;
    juce::AudioBuffer<float> revIn;
    float pumpG = 1, pumpTarget = 1, pumpCoef = 0; int pumpHold = 0;

    // glitch
    struct Glitch
    {
        int type = -1, t = 0, D = 0, L = 0, zapLen = 0; bool up = false, zapSquare = false;
        float zf0 = 0, zf1 = 0, zph = 0;
    } gl;
    double busyUntil = 0, gridNext = 0;
    int64_t now = 0;
    sy::Target gDry, stIn, stFb, stOut, tsIn, crG, bpG;
    std::vector<float> stBuf, tsBuf[2]; int stW = 0, tsW = 0;
    sy::Biquad crHp[2], bp[2];
    bool hostPlaying = false; double hostBpm = 140, lastPpq = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthProcessor)
};
