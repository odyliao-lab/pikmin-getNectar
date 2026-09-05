# Native1.4.12 flower-field regression

With Control Center0.6.5 on rooted Android14/Pikmin152, a user-supplied fresh14-point field completed two rounds in approximately6minutes. No native rebuild or reboot.

- 17 distinct fruit task Start requests,17 RPC completions, no faults and17 matching completed reward receipts. Details identify bloomed-poi fruit/seasonal nectar. No duplicate task Start.
- Continuous planting: one Start and one Stop observed in live logs,147 sampled active-owned states, then the native result-dialog close handler. Final planting0/owned0, farmoff; controller GPS work lock released.
- Same game PID throughout, no crash observed. Last receipt arrived about50s after route completion while holding the final position, with planting already stopped.
- Initial Start log rolled out before the final saved logcat snapshot; it was captured in earlier live queries. No speed-warning branch activation was observed, so that branch has no new acceptance evidence.

Farm uses the existing200m armed rule, not the batch2s gate. Actual native Start durations were4.232–72.771s. Farm's unchanged point dwell/result settle does not guarantee receipt before the next hop. This run proves a productive bounded two-round path, not harvesting every possible bloom, a seed-in-field sample, whole-session speedup, or long-term endurance.

Source, native ABI and safety gates were unchanged in this validation stage. Exact coordinates, task IDs and raw logs remain private/device-local.
