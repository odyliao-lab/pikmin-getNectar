# Gift selection checkpoint: native 1.4.12 / code 46

Baseline: 1.4.11 / code 45 (`96a3a53`). Do not deploy rejected 1.4.10.

The fastest-picker helper expects eligible input. Previously its availability-filtered input still contained non-owner Pikmin for gifts; the post-picker ID gate correctly blocked these selections, causing repeated safe refusals. Gift input is now restricted to the explicit designated ID and accepted by runtime-validated v152 `ExpeditionItemData.Allows` (RVA `0x5FB1F10`, with MethodInfo). Missing owner fails closed. Fruit/seed input is unchanged.

Busy/action restrictions, cross-dispatch reservations, post-selection identity checks, exact-team revalidation before Start, fresh GPS proof, CanTryStart/carrying and batch `0 < TotalDurationMs <= 2000` remain unchanged. No new hook or generic enumeration was introduced.

## Evidence and limits

Follow-up with Control Center0.6.5: a second designated-owner gift completed at123ms/team1, matching wire owner and exact-task reward receipt1.969s after Start. A consecutive fruit/seed pair also completed on first attempts (46ms/team6 and41ms/team5); next hop was11.727s after the fruit Start and after its receipt. Same game PID throughout. Native build unchanged; no reboot. This strengthens bounded regression coverage, not large-batch/long-session or busy-owner negative acceptance.

- Arm64 policy tests (owner/other/missing/native-rejected), managed GC regression and five-call MethodInfo source guard passed; native build passed.
- Installed module hash verified after reboot. Three passive gift projections selected their respective owner, one Pikmin each.
- One real Control Center batch gift completed on rooted Android 14 / Pikmin 152.0: GPS arrival proved, selection 32 ms, actual Start 45 ms/team 1, wire ID exactly matched owner, RPC completed in 966 ms, exact-task reward receipt after 9.051 s and inventory absence confirmed. Same game PID throughout; no crash observed.
- GPS request to controller release: 51.363 s. The 6 s minimum after Start was preserved. Test queue restored byte-identically.
- This does not validate all gift variants, a newly observed busy-owner negative sample, or a long gift batch. Never infer real availability only from a projected count of one.
- APK boot-service scheduling remains a separate Android thermal JobScheduler issue. This test used the validated direct service startup path without a new destination; it is not boot automation acceptance.

SO SHA256: `71c66f2ff7183e3729aab8138ae7156e427f8aa63bbf5a3eae478fa72f134193`

ZIP SHA256: `9d3dd1ce1bad914b3d579774ffd208b2d95704a2ee00cfbf884077690e1c14c4`

Rollback is the previously verified 1.4.11 build. Source changes are gift-specific; the next efficiency stage should first reduce controller root-process overhead without shortening safety gates or changing native hooks.
