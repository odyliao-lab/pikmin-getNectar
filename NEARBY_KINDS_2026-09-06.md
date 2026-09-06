# native 1.4.19：附近派遣種類篩選

2026-09-06補驗：Android14、game PID25787，先使用CC0.6.12的真實Gateway寫入0644控制檔，原生依序確認mask0/4/1/2/3，全部通過；測試期間dispatch off。返程UI默认水果＋花苗、禮物不勾已確認。候選0筆，所以這是設定串接證據，不是各類實際派出驗收。完成後恢復原armed/all/auto、nearby fruit seed；native binary未更換。

CC 0.6.12將附近200m開關移到返程頁，新增水果／花苗／禮物盒勾選。

`/data/local/tmp/pikmin-nearby-kinds.txt`：空白分隔fruit/seed/gift，none=0，缺檔=fruit+seed(mask3)。未知、空白、超長、讀取錯誤失敗關閉，不預設全選。0644，CC使用tmp+rename寫入。
bit水果1、花苗2、禮物4。只限一般armed；legacy kinds=farm仍收水果＋花苗，batch只看原本指定task與2秒/4m閘門。
舊kinds=seed/fruit/gift仍與新mask取交集。沒有改GPS，也没有新增hook、RVA或managed呼叫。

`files/nearby_dispatch_status.tsv`：v1、毫秒時間、PID、mask、mode、legacy_filter，以TAB分隔；在有效task列表掃描寫出（包含0筆），供CC驗證原生讀取，不等於實際派遣成功。
篩選接在原requested判斷，原有同隊ID重驗、CanTryStart、搬運力、忙碌排除、指定禮物owner、並行3筆限制、庫存確認／return處理全保留。

## 測試

`nearby_policy_test.cpp`：8種mask、格式錯誤、farm不受個人mask影響、legacy限制。
Android arm64真機執行成功；同輪dispatch_safety、managed_gc、return_policy24、planter_policy24、planter_steps20及generic MethodInfo靜態5處全部通過。
實機安裝並重開機到1.4.19；候選清單備份為0，本次不宣稱新的各類任務派遣實測。
CC repo `HANDOFF_2026-09-06_NEARBY_KINDS.md`記錄設定讀取與UI驗收。

回退包 `/data/local/tmp/pikmin-nearby-20260906/rollback-native-1.4.18.zip`；回退前停自動化、確認無pending RPC，再安裝並重開機。
