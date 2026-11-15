# EMOTION LAB 0.1 — Ableton Live VST3 prototype

**Status: source prototype, not yet compiled or tested in Ableton.**

This is a six-parameter real-time audio effect using conventional DSP. It deliberately **does not** claim to change actual speech emotions or words. Emotion-preserving speech-to-speech transformation needs a separate ML model and controlled listening tests. The initial code provides an immediately testable baseline to compare with future neural approaches.

## Six parameters (all 0–100, neutral=50)
- Emotion — spectral brightness/air (sad–happy perceptual proxy)
- Energy — level / harmonic density
- Tension — high-frequency emphasis / mild saturation
- Confidence — bass/body / level and dynamic reduction
- Intimacy — low/high spectral emphasis
- Expression — gentle amplitude movement

## Build without local Visual Studio
1. Create a new GitHub repository and upload the **contents** of this folder, including `.github/workflows/build-windows.yml`.
2. Open GitHub Actions → Build Windows VST3 → Run workflow (or push to main).
3. Download the artifact `EMOTION_LAB_Windows_x64_VST3` once the workflow is green.
4. Extract the `.vst3` bundle into `C:\Program Files\Common Files\VST3\`.
5. Restart Ableton Live, scan plugins, insert `EMOTION LAB` as an audio effect on a voice track.
6. Compare the original audio with all six controls at **50** (neutral), then automate each control separately.

Build requirements: CMake >=3.24, C++17, Visual Studio 2022 x64, internet access for FetchContent of JUCE 9.0.5. Check JUCE licence terms before commercial distribution.

## Known limitations
- The interface uses JUCE GenericAudioProcessorEditor, not custom knob graphics.
- No speech content change, syllable-level prosody modification, pitch contour conversion or real emotion transformation yet.
- Extremely loud audio may still clip in host chains; start with safe input gain.
- Needs actual CI compile and Ableton playback/automation testing.

## Planned next step
Benchmark a speech emotion-conversion engine (content retention, naturalness, latency, Windows feasibility, and portability to iOS AUv3) against this baseline before promising real-time operation.


## Interface update 0.2
Custom JUCE editor with six horizontal-arranged vertical faders based on the supplied sketch: white background, slim red line and hollow red circular handle. Values are mapped to the same six APVTS parameter IDs, so automation and existing presets remain compatible. Double-click a fader to reset it to 50. Built artifact is not compiled/tested in this environment.
