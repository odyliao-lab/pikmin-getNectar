# Native P0 safety checkpoint: 1.4.11 bounded regression passed

**Do not install 1.4.10/code44.** Its first game-path test crashed in the newly exercised selected-team enumeration. Earlier standalone tests and installation verification below are historical, not acceptance. Version1.4.11/code45 fixes the specific MethodInfo call error and passed the bounded game tests documented at the end. Gift dispatch and the full improvement plan remain unaccepted.

Baseline: 1.4.9/code43, game 152.0 arm64. This checkpoint does not alter GPS controls, the controller's six-second post-start hop interval, or the maximum of three concurrent armed starts.

## Implemented

- `dispatch_safety.h`: native string-ID reservations shared by armed/batch. Reserve before applying a team; validate the entire proposed team against the filtered live inventory, not only its count. A sent expedition cannot reserve a different team for a duplicate start.
- Keep the exact batch team across the selection-settling tick. Read `ExpeditionItemData.get_Pikmins` and require the same IDs to remain eligible before Start; do not validate a newly calculated picker but send an old selection.
- The new getter is resolved from runtime metadata and must match the installed v152 RVA `0x5FAFBF8`; static disassembly confirms it delegates through ExpeditionPikminStats. No new hook or generic ABI is introduced. Existing enumeration is reused, with full-width scoped GC roots and write barriers for the outgoing string array.
- Batch Start requires a fresh native `TotalDurationMs > 0 && <= 2000` in addition to CanTryStart, carrying power, target/ready and distance. Unknown/zero/negative duration fails closed. Armed retains the independent 200m policy, not this two-second gate.
- A sent reservation survives timeout, RPC exception and mode changes. It releases on a live assigned-to-available transition or a later complete expedition projection in which its task has disappeared. Unsent selections can cancel. Conservative holds after ambiguous faults may reduce throughput; there is no arbitrary timeout release.
- Gifts require a validated AllowedPikminId, exactly that single picker ID, and eligibility in the live filtered projection. Missing restriction, unknown identity or unavailable owner blocks selection. No substitutions, cancellation of mushroom work, or trial RPCs to discover availability.
- `dispatch_selection_diagnostics.tsv` records `gift-owner-readonly` on evidence changes: designated ID, picker ID, projected status and assigned task where metadata validates it. This is projected inventory, NOT independently verified server state or the full native UI disabled-reason result. Rare-decoration gifts without an explicit AllowedPikminId fail closed pending separate research.

## Verification at installation checkpoint

- Full Android NDK r27d build passed.
- Independent arm64 `dispatch_safety_test.cpp` passed on the target Android14 device: duration boundaries, malformed/duplicate IDs, exact-team comparison, three independent teams, conflict rejection, same-owner duplicate starts, cancellation and evidence-based release.
- Existing `managed_gc_test.cpp` passed again on-device: full 64-bit handle, null guards and exactly-once release.
- Installed via Magisk; rebooted and verified active code44 and SO SHA256 `4afc5ecb745c97a076f7108f48047fb7c3235327338c9f7e6c6f72c4258479b3`.
- ZIP SHA256 `646538c33049260a836cb3eb3c1f9928c2824182d247e3966c7efd728b0b8332`.
- Dispatch and flower farm are off. Game opening, read-only gift evidence and dispatch regression are still pending user unlock/open. Installation and unit tests do not establish game-path acceptance.

## Next acceptance / rollback

First passive game startup and fresh task snapshots, then one seed/fruit and a small mixed batch using the real Control Center 0.6.3 service. Verify original team equals wire IDs, native duration, RPC outcome, exact-task receipt, >=6s hop spacing, GPS release and process stability. Separately test armed concurrent nearby tasks with disjoint wire IDs; no gift dispatch in this stage. Fault/mode-change/manual-team-change behavior needs additional controlled coverage.

Keep 1.4.9/code43 as rollback (SO `f4997fcafa1469bc96502e95b23fe836fd600a3edbe4efd320eff8606b8e9942`). Do not use rejected 1.4.6 or 1.4.8 builds. Module rollback needs another reboot; APK has not changed. Raw device task/Pikmin IDs, location captures and backup data must not be committed to this public repository.

## First game-path test: rejected 1.4.10

Passive startup and fresh candidate observations worked. Three current gifts had a different picker ID from their explicit AllowedPikminId; the new gate blocked all three. Their projected owner status was Available; this does not explain the earlier different six busy gifts or prove server/UI availability. No gift RPC was sent.

One fruit reached both GPS gates and SetPikmins returned native duration18ms, CanTryStart=true and carrying power=true. The added actual-team recheck then crashed UnityMain before any module Start/RPC. Crash PC libil2cpp+0xB0DD994 (WhereEnumerableIterator<object>.MoveNext+0x58) executes `ldr x8,[x20,#0x20]` with x20=0; x20 is incoming MethodInfo/x1. Module caller+0x81360 used x1=0. This is a proven null generic-context argument, not a busy-Pikmin or GPS failure.

Stopped the controller, verified stopped/gps_control_released and dispatch off, and restored its original queue with byte comparison. No new reward receipt or Start wire row. A boot-time calendar/usap64 crash also exists in the buffer and is separate from this reproduced Pikmin failure.

1.4.11 passes each resolved implementation MethodInfo to all five GetEnumerator/MoveNext/Current call sites. Added a static regression guard rejecting null iterator context; this guard does not simulate IL2CPP execution. No change to GPS timings or safety policy. Rejected SO/ZIP retained locally for disassembly, never published as a release.

## 1.4.11 live acceptance

Android14/Pikmin152/Control Center0.6.3; active SO `e6fc048bf08f818bda8014a0540be8c13175976f11dabe45a3b9e9532aaa37fc`, ZIP `d89d0514b5d02b6208dfeecbe5c28974934dad3dfe0d81bc5ae86731ba5342c4`.

- After user unlock, ADB launch produced fresh candidates without a manual expedition-page transition. An initial GPS JoyStick cold-start request failed to maintain the mock provider: the one-item controller timed out at arrival with no native Start. User subsequently started JoyStick; its cold-start integration remains follow-up work, not fixed here. Official [TELEPORT API](https://theappninjas.com/docs/pro-intents-api/teleport/) confirms the existing float lat/lng service contract; sending the intent is not proof of live positioning.
- Single fruit passed: native21ms, six Pikmin, RPC completed, exact-task reward receipt. Actual selected IDs equalled outbound wire IDs. This exercises the previously crashing selected-team iterator.
- Mixed three-item batch passed first attempt for every item: seed35ms/team1, fruit35ms/team6, seed80ms/team3. All exact-task receipts and selected/wire ID matches verified. GPS request to final release118405ms. Next hops occurred10340ms and9642ms after the previous starts, after their reward confirmations; both exceed the six-second floor.
- Armed test at a location with exactly three eligible nearby fruit/seeds: three wire requests within18ms, teams6+1+3 = ten unique IDs, no overlap, all RPCs completed. Native durations36851/19298/47560ms: armed deliberately retains200m policy, not the batch2s policy.
- Crucially, live inventory still reported busy121 after the first armed Start. The next two picks logged reserved6 and reserved7, with eligible counts reduced accordingly. This directly exercises reservations while game state has not yet caught up; it is not merely three sequential completions.
- All three armed return completions were observed. Two have exact-task TSV confirmed rows; the fruit has its exact-task return dispatch, subsequent log `batch confirmed completed=6`, inventory disappearance and later cumulative completed7, but its individual confirmed TSV row is missing. Do not claim seven complete durable receipts. Investigate history-write consistency in P1.
- Game PID18183 stayed unchanged throughout these1.4.11 tests; no new game crash observed. Original controller queue restored with byte comparison; original armed mode and type filter restored after bounded test completion. Gifts were not dispatched.

Still unverified: gift identity/availability reconciliation with server and UI, live negative duration-boundary/manual-team-change/fault cases, long runs, persistence, atomic snapshots and retry classification. Standalone policy tests cover boundary/conflict cases but do not substitute for all of these device tests.
