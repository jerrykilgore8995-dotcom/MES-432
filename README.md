# MES-432 — clean digital mastering EQ

Target: original black-face MES-432C/9. This is a new independent project, not Tominator. Current state: reference research; no validated circuit or VST3 yet.

## Hardware reference
4U, 19-inch, independent L/R, three 24-position parametric bands, two shelves per channel, five shape positions (5/6/9/11/15). Gain steps: -9,-8,-7,-6,-5,-4,-3,-2.5,-2,-1.5,-1,-0.5,0,0.5,1,1.5,2,2.5,3,4,5,6,7,8,9. Low shelf 50/100 Hz. High shelf varies by revision: select physical reference before locking (fixed 10 kHz on older units, switchable 7/12 kHz on reviewed C/9). Do not merge revisions.

## References
- https://www.audiotechnology.net/reviews/iti-audio-mes-432c9 — C/9 controls and revision differences
- https://vintageking.com/blogs/around-the-shop/sontec-mes-432c9-parametric-eq — physical hardware and stepped controls
- https://vintageking.com/used-and-vintage/sontec-mes-432c-9-parametric-equalizer-vk01484-vintage — panel and knob photographs
- https://www.session-recall.com/en/devices/sontec-mes-432c9-custom — legible panel scales (custom variant, cross-check before use)
- https://gyraf.dk/schematics/audioboard432.gif — original audio board scan, listed as 430C; verify revision
- https://groupdiy.com/threads/sontec-432-inspired-equalizer.79346/ — five-stage LTspice project, alternate DOAs, not an original circuit authority
- https://groupdiy.com/threads/sontec-mes432-level-switch-values.21603/ — 50k center-tapped level switch discussion; shelf values uncertain
- https://gearspace.com/threads/the-sound-of-sontec.1323735/ — real hardware magnitude comparisons
- https://www.gb-mastering.com/equipment/sontec — reports shape-dependent peak gain in Sontec topology; modified hybrid hardware, not calibration reference.

## Engineering gates
1. Obtain legible audio board and actual switch wiring/tables. Record component values and source with revision; unresolved values must block reference claims.
2. Construct connected linear AC ngspice model including filter loading, summing and interactions. Approximating amplifiers as ideal linear devices is acceptable only if measured effect on the curves is negligible; no noise/harmonic simulation required for target.
3. Sweep each switch position and gain, export complex transfer data. Include multiple active bands. Verify against independent original hardware measurements.
4. Fit stable low-order digital sections/state-space equations to complex responses at 44.1, 48, 88.2, 96, 176.4, 192 kHz; preserve above-Nyquist center settings' in-band effects. Target 0.05 dB and 1 degree where practical, report actual worst case. Do not silently clamp center frequency to Nyquist.
5. Real-time DSP uses fitted coefficients only, no SPICE solver, no saturation, hum, hiss, intentional color or unnecessary latency. Neutral and bypass sample-identical; double-precision states, no allocation in process callback.
6. JUCE faceplate from actual reference: silver fluted knobs, black caps, white lines, red scale indexes, engraved separator lines, exact stepped legends and switch locations, green LEDs, rack ears. No fake meter or graph, no added controls on panel. No manufacturer logo. Capture actual VST3 in native host, validate nonblank PNG.
7. Build Windows x64 VST3 in CI only after reference data and fit pass; publish measurements and screenshot alongside binary.

## Blockers
Original GIF could not be retrieved by current browsing interface; GroupDIY attachments inaccessible. Full switch tables and circuit connectivity not yet verified. No generic EQ curves are to be represented as Sontec measurements. 
