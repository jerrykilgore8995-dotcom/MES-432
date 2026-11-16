# Curve-first modeling

Source of measured-q.csv: Phillip Dempster's measurements of a serviced MES-432C, https://gearspace.com/threads/the-sound-of-sontec.1323735/page-2 . Values are approximate Q in DMG EQuilibrium/TDR Nova convention, not universally interchangeable. Boost causes bandwidth expansion. No measured cut extrapolation is justified yet.

https://phillipdempster.com/post/the-sound-of-sontec/ provides 11 measured settings and a blind-test archive, including dry console and matched digital/hardware audio at 48 kHz. Its analysis used parallel bands, analog phase mode. Hardware has small broadband insertion/gain shifts. Normalize flat insertion gain for this clean plugin, but retain measured setting-dependent shifts if substantiated.

https://www.gb-mastering.com/equipment/sontec reports approximately 1.5 dB peak loss at narrow Q on a Sontec-style circuit and broadband shifts with gain; this is secondary evidence, not a calibration table.

https://stamaudio.com/shop/equalizers/sa-432c/ describes bridged-T feedback filtering and a fixed 10 kHz shelf on its clone. Do not conflate this with the 8/12 kHz D9 revision documented in https://www.makebelievestudio.com/_files/ugd/c30d27_29255481d37044ef86cecd6c5e390f80.pdf .

https://www.massenburg.com/wp-content/uploads/2019/03/ParaEQ.pdf original parametric EQ paper; historical topology reference, not a complete C/9 circuit.

https://mhsecure.com/products/software/SontecMES-432D9D.html offers an authorized trial with a no-analog mode. If available in a licensed local host, measure transfer responses as an independent D9 reference, clearly labeled as a different revision.

The GroupDIY 432 thread remains blocked by HTTP 403 on direct browsing. Do not invent unseen schematic components.

Priority: measured magnitude and control interactions over simulation method. Use stable digital IIR fitted to measured curves where possible. Native direct or parallel state-space models acceptable. No analog nonlinearities required. Missing shelf and negative gain data remain blockers to claiming exact model.
