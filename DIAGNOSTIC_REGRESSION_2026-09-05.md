# v152 native diagnostics and Android 14 validation (2026-09-05)

This public note omits live coordinates, task/Pikmin IDs and private app data. Detailed per-task evidence is retained in the private Control Center handoff and local diagnostic backups. Do not publish runtime TSV captures without redaction.

## Current checkpoint

- Native v1.4.9 / versionCode 43, Android 14 POCO 24095PCADG, Pikmin Bloom 152.0.
- Active SO SHA256: f4997fcafa1469bc96502e95b23fe836fd600a3edbe4efd320eff8606b8e9942.
- The previously failing single seed succeeded on its first retry test after the availability correction, with RPC completion, inventory removal and automatic reward receipt.
- A subsequent requested four-item test collected its first seed but exposed a Control Center false confirmation timeout. After the controller correction, the remaining three items completed without retries in 188916 ms. This was NOT one uninterrupted four-item run.
- Control Center 0.6.3 + native 1.4.9 completed the remaining fruit/seed/fruit sequence with native durations 343/36/31 ms and teams 6/1/6. All had reward receipts; next hops occurred 12.510 and 12.743 seconds after the preceding starts.
- The game PID remained unchanged throughout this validation. No new game crash was observed. A separate system dex2oat64 crash during APK signing migration was not a game crash.
- These are bounded fruit/seed tests, not proof of large-batch, long-duration or gift reliability.

## IMPORTANT: gifts remain unvalidated

The user manually confirmed that the designated Pikmin for ALL SIX remaining gifts are on mushroom tasks that cannot be interrupted. Each gift belongs to exactly one designated Pikmin; no substitute can collect it. The module's candidate projection nevertheless shows a picked count of one for each gift. This is an unresolved contradiction, not proof that those gifts are dispatchable.

Do not dispatch gifts to validate this hypothesis until the availability source and exact designated-Pikmin identity are reconciled. Candidate count must not be labeled server-ready. Status/restriction filtering fixed the reproduced fruit/seed failures, but cannot yet be declared a complete eligibility predicate. Investigate predicted versus server-backed inventory state and the native UI's other disabled-reason checks. User requested no further gift testing at this checkpoint.

## Rejected builds and rollback

- v1.4.6/code40, SO cdfcb2ee882201f194a3621145ebed80cf5596f8a962cabf0ba795d23304e50d: startup SIGSEGV. It hooked RVA 0x704C994 from a stale dump, which is an instruction inside a function in this APK, not the RPC entry. Game PC 0x49d0dc0 and caller trampoline +0x40c match this regression. Never redeploy REJECTED-startup-crash-v1.4.6.zip.
- v1.4.5/code39 rollback SO: 36313ac7d73a289dd2a938c1ba3de6630154f84b887f3f22201fe2dd6eb511b1.
- v1.4.7/code41 stable diagnostic rollback SO: 37282645e0bcf7e76e1d0644e162a277ff29c7ea794302e5f79fc65d9e83d038.
- v1.4.8/code42, SO 70a47d8e563ead036445469175a2f2b6dfcd3b176714f1efa241b8826f51f741: new scoped-root cleanup crashed at libil2cpp+0x534d08c, caller module+0x7f08c. Never redeploy REJECTED-startup-crash-v1.4.8.zip. A separate Thread-107/libNianticLabsPlugin crash also occurred; its cause is unproven.

## Use the correct APK-derived dump

The actual v152 dump exists under the local temporary directory pikmin-v152-static/dump-output, alongside script.json, extracted libil2cpp.so and global-metadata.dat. dump.cs is 87223547 bytes; script.json is 250950645 bytes; libil2cpp.so is 254505288 bytes.

reference/dump.cs is stale for this installed APK. It places RegisterMapObject at 0x58B0B80; the actual APK and working module agree on 0x5A50188. Correct v152 entry points:
- SendStartExpeditionRpcAsync: 0x7263270
- ExpeditionItemData.SetPikmins: 0x5FB2120
- ExpeditionItemData.StartExpeditionAsync: 0x5FB222C; MoveNext: 0x5FB3740
- ExpeditionDetailPageV2.StartExpeditionAsync: 0x5F9A56C; MoveNext: 0x5FA03DC

## Passive request observer

v1.4.7 resolves the RPC through runtime metadata after login and requires the actual RVA and five-word prologue before installing the observer. Request and point field offsets, repeated-field storage and bounds are validated. It does not call managed enumerators or mutate requests.

dispatch_wire_v152.tsv retains the last 24 hours, at most 256 requests, and distinguishes game/module sources. Three pre-fix module requests had nonempty distinct IDs and request coordinates within one metre of target, yet all faulted with INVALID_ARGUMENT. This weakens empty-ID/wrong-wire-coordinate explanations but does not prove server-side location freshness.

## Availability filtering and GC handle correction

The native UI AutoPick (0x5F9C10C) uses PikminSelector.AvailablePikmin (0x6E3159C) before selection. GetIsBusyReason (0x5F550A8) checks status Available=1 or Entourage=32 and calls ShouldRestrictActionsUnlessIsTask (0x6D456FC) with ServerClock.CurrentTimeMs. ExpeditionItemData.Allows (0x5FB1F10) checks expedition restrictions, not the whole UI availability predicate.

The module previously passed full inventory to the fastest picker. The correction filters a concrete GetPikminList snapshot first, verifies six runtime methods against actual v152 RVAs, validates bounded list fields, and uses no new generic enumerator. Logs observed total174/eligible36/busy138/restricted0. See the gift contradiction above before generalizing this success.

v152 il2cpp_gchandle_free at 0x53565f8 branches to 0x534d070, masks full x0 by 0xffffffffffffe000, then reads page+0x20 at the crash PC. The old uint32_t handle declaration truncated that address. v1.4.9 uses uintptr_t for creation, storage and release through managed_gc.h. Existing long-lived handle lifetime policy is unchanged.

tests/managed_gc_test.cpp uses the production scoped-root wrapper and mock callbacks. It passed as an independent arm64 ADB shell executable: >32-bit round-trip, exactly-once release, null object/callbacks and zero handle. Full native build and git diff --check passed; this wrapper test alone is not live IL2CPP proof.

## Next work

1. Preserve this verified checkpoint; keep raw diagnostics and signing keys out of public commits.
2. Reconcile gift designated-Pikmin availability with actual task assignments; classify unknown as unverified, never ready.
3. Audit the historical two-second batch duration requirement: current native starts use distance plus CanTryStart, not a hard two-second or five-minute duration bound. Recent tested durations were all below two seconds, which does not prove the bound exists.
4. Investigate excessive arrival-to-start delay and TSV publication/proof races before shortening safety waits.
5. Extend regression tests for stale/malformed snapshots, late/duplicate receipts, cancellation, and GPS ownership release; then perform staged device validation.
