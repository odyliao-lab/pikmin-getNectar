# Native1.4.13 preflight checkpoint — game acceptance pending

This commit checkpoints the already built/installed advisory availability projection. It is NOT a claim of successful new batch-game regression or a general release recommendation. The separate Control Center GPS boot restoration was tested independently and does not validate this native addition.

- Version1.4.13/code47. No new hooks/RVAs/enumerators: reuse the existing fully validated availability/assignment projection.
- `dispatch_availability.tsv` uses a version1 timestamp/PID header, seven-column advisory rows and row-count footer, published by same-directory temp file/flush/chmod0644/rename. Existing12-column candidates remain unchanged.
- Explicit busy gift owner requires status2 and a nonempty assigned task different from that gift. Unknown/incomplete data does not infer busy. Ordinary no-team requires a complete nonempty inventory with known status values and zero eligible IDs.
- Advisory data never authorises Start. Existing native2s batch duration gate, exact available team/unique gift owner/reservations, GPS arrival evidence, and200m armed policy remain unchanged. Control Center requires two distinct fresh matching observations before skipping a GPS hop; missing evidence falls back to the guarded flow.
- Pure arm64 dispatch-safety tests and iterator MethodInfo static guard passed; native build passed. SO SHA256 `3299219cd3573ba882610058aea9bf835ded6b3a395c3cc9e518335382e8e845`; module ZIP `b998fd866772a497f7629e0d006d6fa93983cf3a3bc9663e5612002786c131c4`.
- Still required: live available gift + fruit/seed regression and a naturally busy-owner negative sample proving no GPS/Start. Do not fabricate busy state or send unrelated mushroom tasks to create a test sample.
- Roll back to the verified1.4.12 build if needed, never the rejected/crashing1.4.10. A native rollback/install requires reboot. No native changes were necessary for the separate APK0.6.8 GPS foreground startup fix.
