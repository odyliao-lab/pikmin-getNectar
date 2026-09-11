# pikmin-getNectar

2026-09-12 1.4.33/code67隨Control Center0.12.7/code65交付：重開後實際載入及種花reason13收集按鈕呼叫／提示消失已驗；沿路49點的5果1苗派遣及領取亦有原生證據。跨帳號、失聯及多輪耐久仍待補驗。下方「待重開」為開發當時紀錄。

2026-09-12 開發版1.4.33/code67：新增種花獎勵視窗處理（GotItemOverlay reason13）。僅在return=all或本核心停花後60秒內授權，等待collectButton可操作且非loading，再走原生InvokeClick；其他獎勵不操作，60秒逾時／例外不重送，跨frame物件使用有界GC root。11組隔離native測試（新視窗測試為mock runtime）及建置通過，實際關閉仍待新核心重開驗證。配套APK0.12.7/code65，未發布。

2026-09-11 開發版1.4.31/code65：補登入／登出帳號雜湊回報，v2路線租約綁定帳號並撤銷登出前token。原生閒置證據尚缺，session維持busy，不開放跨程序接續。10組隔離policy及編譯通過，登入時序／切帳／沿路採果仍待真機驗證；配套Control Center0.12.1/code59，詳相鄰repo的HANDOFF_2026-09-11_SESSION_IDENTITY.md。未發布。

2026-09-11 **1.4.30/code64 沿路開發版**：新增獨立route租約／延遲軌跡門檻及本次工作明確授權的停花／結算回報，既有armed與batch/farm條件保留。9組隔離Android policy、MethodInfo靜態及模組建置通過；已排入192.168.50.202 Magisk，待重開載入，未公開發布、未完成新模式花田驗收。配套APK0.12.0/code58。協定、備份與待驗項目見相鄰控制中心 `HANDOFF_2026-09-11_ROUTE_HARVEST.md`；不要將後續的歷史版本快照當作現況。

2026-09-11 本機 **1.4.29/code63**：新增`core_runtime.tsv`每秒同PID版本回報，供Control Center0.11.1/code56內附配對核心更新及重開核對。原1.4.28散步修正保留；餵食仍唯讀探查。模組編譯與MethodInfo／probe靜態檢查通過，未安裝／發布，實際載入待驗。完整流程見相鄰Control Center的 `HANDOFF_2026-09-11_CORE_UPDATE.md`。

2026-09-11 本機 **1.4.28/code62**：一般armed支援18km/h連續定位，候選每秒檢查、選隊至少隔1秒重驗；保留8m原始／遊戲定位一致性與5分鐘原生上限。批次／花田頻率不變。靜態檢查及模組編譯完成，policy執行與手機驗收尚未進行，未安裝／發布。[移動派遣交接](NEARBY_WALKING_2026-09-11.md)。

2026-09-10 本機 **1.4.27/code61**：加入獨立opt-in `feeding_probe.inc` 精華庫存唯讀探查，永遠observe-only，沒有餵食／收瓣RPC。配合Control Center0.11.0前置介面；尚未安裝／重開／實機核對，不是可用的自動餵食版本。開發邊界、協定與後續工作見相鄰Control Center repo的 `HANDOFF_2026-09-10_FEEDING.md`。1.4.26既有未提交改動保留，不屬於本次新增餵食功能。

最新開發 **1.4.26/code60**：配合Control Center0.10.5，批次水果／花苗距離上限100m，禮物仍4m，實際選定隊伍原生往返仍須>0且<=2000ms、CanStart及隊員重驗不變。新增唯讀`batch_area_capability.tsv`同PID／12秒能力回報，APK無新鮮回報時保持4m相容路徑。一般armed5分鐘與花田規則不變。8組裝置policy／MethodInfo靜態與建置通過；已排入指定Android14手機Magisk，**待重開載入及相近任務驗收**，未公開發布。回退備份與完整協定位於Control Center `HANDOFF_2026-09-09_AUTOMATION_PHASE1.md` 的0.10.5段落。

最新開發 **1.4.25/code59**：供Control Center新手機部署首版使用，service在沒有既有模式檔時預設nectar／return皆off，不再預設auto。已有模式檔不覆寫；C++操作規則不變。五處MethodInfo靜態檢查、重新建置與四項隔離host shell預設／保留值測試通過；尚未安裝／重開或Android真機驗收。需要新模組重開才使用此服務版本。

最新開發 **1.4.24/code58** 配合Control Center0.9.1：新增育苗手動恢復協定，同PID／90秒有效期／確切pending綁定，兩次完整庫存核對後封存舊pending才解除暫停。恢復不呼叫遊戲RPC，結果不明不放行。原5分鐘不限距離與批次／花田規則保留。本機靜態檢查與編譯完成，native policy裝置執行及真正pending恢复仍待驗證，尚未安裝。[育苗介面](PLANTER_2026-09-06.md)。

最新開發1.4.23/code57：一般armed取消200m半徑，只保留原生往返5分鐘及既有定位／隊員安全保護；farm200m、batch4m/2秒不變。7組arm64政策與5處MethodInfo檢查通過，已排入待載入模組，尚待重開及>200m真實派遣。guard v2需CC0.8.2+。完整回退和證據見Control Center的HANDOFF_2026-09-08_TIME_ONLY.md；下方1.4.22及更早規則為歷史。

在已 root、啟用 Magisk Zygisk 的 Android 手機上，透過 Pikmin Bloom 的 IL2CPP 原生 API 提供本機自動化能力。它不使用螢幕手勢或畫面座標點擊。

目前 module 提供四個彼此獨立的控制面：

- 路邊大花精華的掃描與原生領取 RPC；
- 返程獎勵的原生處理與狀態紀錄；
- 探險候選掃描、最快隊伍選擇，以及受 Control Center 嚴格限制的原生派遣。
- 成熟花苗原生拔出與永久空槽補種；獨立開關、庫存確認及24小時操作歷史。

> 遊戲更新會改變 IL2CPP RVA。使用 root、定位模擬或遊戲自動化可能違反服務條款，帳號與裝置風險由使用者自行承擔。

## 相容性

**1.4.22/code56 + Control Center0.7.1/code37**：一般附近派遣放寬為遊戲距離≤200m且原生往返≤300秒（5分鐘），兩條件皆需符合；定位同步穩定、跨tick同隊／忙碌排除及批次2秒上限不變。16組APK JVM及手機7組native policy測試通過；已排入Magisk更新，**待重開後載入與120～300秒真實候選驗證**。`nearby_gate_status.tsv`最後一欄改300000，須配套新版APK。[修改與安全界線](NEARBY_5MIN_2026-09-07.md)。

1.4.21/code55搭配CC0.6.15/code32：一般附近派遣須符合遊戲處理後位置200m、定位同步穩定、跨tick同隊重驗及原生往返≤120秒。**Android14實機已通過armed遠距離分段追趕期間0派遣、到點後3筆花苗並行送出及自動領取確認**：3筆在15ms內送出，原生往返0.168／26.242／28.742秒，7隻隊員互不重複。確認全部領取後才還原GPS，附近目前off。這是花苗樣本驗收，水果／禮物與批次／花田完整回歸仍另列，未宣稱全面驗收。[安全規則、證據及待驗證項目](NEARBY_SAFETY_2026-09-06.md)。

1.4.19/code53搭配CC0.6.12/code29新增一般200m自動派遣的水果／花苗／禮物盒篩選，預設水果＋花苗；CC入口移到返程頁。控制檔獨立於批次／花田，全部不勾不新增派遣；保留忙碌隊員排除與禮物專屬ID限制。此舊版缺少上述時間與定位同步保護，不宜在遠距離跳點時啟用附近派遣。詳見[控制協定與驗證邊界](NEARBY_KINDS_2026-09-06.md)。

1.4.18/code52搭配CC0.6.11/code28支援補種步數篩選，預設1000/3000/5000，100/10000/其他不勾。Android14已驗證原生讀取設定，並實際拔出既有100步金苗後，跳過100步候選補入1000步紅苗；不影響已種花苗的成熟拔出。[協定與證據](PLANTER_2026-09-06.md)。

1.4.17/code51 + CC0.6.10/code27新增自動育苗，Android14實測成熟藍苗拔出／新Pikmin入庫＋100步金苗補種成功，岩石未成熟株與其他47筆seed保留、無閃退。不使用額外槽位，不修改成長步數，不盲目重送未知結果。dist更新為1.4.17；多輪成長與滿倉／斷線等例外尚待實機驗證。[育苗介面、驗收與回退](PLANTER_2026-09-06.md)。1.4.16只讀測試因命名空間核對而停止，未執行操作，不作發布版本。

1.4.15/code49已補齊零結束時間的搬運水果（含散步拾取／蘑菇獎勵），沿用原生領取與控制檔。Android14搭配CC0.6.9實測9筆一般拾取＋1筆蘑菇水果＋1筆既有探險返程全部有逐筆領取確認，花園待領水果清空、遊戲未閃退；13筆未出發探險保留。滿倉與所有變體仍非完整驗收。dist已更新，1.4.14只作診斷、不應當作修復版。[條件、實測與還原](CARRY_CLAIM_2026-09-06.md)。

- Pikmin Bloom `152.0`
- Android `arm64-v8a`（已測 Android 13；native 1.4.9 已完成 Android 14 小批次驗證）
- Magisk 與 Zygisk

module 會驗證 `libil2cpp.so` 檔案大小；不相符時不安裝 hooks。其他遊戲版本不得直接沿用此建置。

## 探險原生派遣

在 `armed` 模式下，module 從 `InventoryManager.GetPikminTaskList()` 讀取任務、以 `ExpeditionDataStore.ByIndex()` 取得資料，並以遊戲的 `PickFastestUpToItemLimitsReservingForTroop()` 選擇最快隊伍。

無論是附近自動派遣或 Control Center 批次模式，真正呼叫 `StartExpeditionAsync()` 前都會重新檢查：

- `CanTryStart == true`；
- `armed` 模式必須以遊戲內部位置確認候選在 200 m 內；
- 批次模式必須有指定 task 的新鮮到點證明，且遊戲內部位置在目標 4 m 內。
- native 1.4.10 起，批次另要求真正送出時的原生往返時間大於 0 且不超過 2 秒，並核對原本選中的同一批 ID 仍可用。200 m armed 不套用這個 2 秒限制。

`armed` 會在同一輪 live task-list 掃描中，對仍在 200 m 內的候選最多送出三筆原生派遣；每筆都有獨立的 RPC 與庫存確認，避免 GPS 持續移動時後面的候選先離開範圍。三筆是保守的初始上限，不是無限制 burst；已送出的 task 不會在確認前重複送出。`batch` 完全不使用這個平行路徑，仍是指定目標、到點後一次一筆。

批次模式在 `SetPikmins()` 後會等待至少 1.5 秒，下一個 live inventory tick 再重新確認同一任務、GPS 距離、`CanTryStart` 與搬運力，才呼叫 `StartExpeditionAsync()`。這避免在同一個遊戲 update turn 選隊後立刻送出；若任何條件改變，該次安全略過並由控制端的既有重試流程決定後續動作。

已在 v152 實機驗證花苗／水果：選擇最快隊伍、原生派遣和後續庫存確認皆成功。派遣歷史 TSV 保留最近 24 小時，事件包括候選、套用隊伍、送出派遣與庫存確認。

native **1.4.9 / code 43** 在最快選隊前加入狀態與動作限制篩選，並修正 v152 GC handle 必須保留 64 位元的問題。搭配 Control Center 0.6.3，在 Android 14 完成連續三筆水果／花苗首次派遣與領取，無遊戲閃退。這是有界小批次驗證；診斷方法、回退版本與限制見 [2026-09-05 紀錄](DIAGNOSTIC_REGRESSION_2026-09-05.md)。

**1.4.11 / code 45 已通過 Android14 有界回歸**：單筆水果、連續三筆花苗／水果，以及200m內三筆同時派遣皆成功；同時三筆的十隻皮克敏ID不重複，請求相隔18ms。新增跨派遣隊伍預留、實際選隊ID核對、批次2秒硬閘門與禮物指定ID唯讀診斷。**不可使用1.4.10/code44**，它新增的泛型列舉缺少MethodInfo而崩潰，已在1.4.11修正。禮物盒完整派遣、大量／長時間作業及紀錄一致性仍待驗證。[P0 實測證據與回退方式](DISPATCH_SAFETY_2026-09-05.md)。

若 `StartExpeditionAsync()` 回報 server fault，`files/dispatch_selection_diagnostics.tsv` 會保留最近 24 小時的診斷列：時間、任務種類與 ID、`SetPikmins` 後的遊戲狀態、picker 數量、實際送入的 ID 數量，以及精確的皮克敏 ID 清單。這只用來比對 native 與遊戲 UI 的選隊差異；不會改變派遣策略或任何閘門。

**1.4.12 / code 46 已完成 Android 14 單筆禮物盒端到端驗證**：最快選隊前先限制為禮物指定 ID，並通過原生 `ExpeditionItemData.Allows`。實際派遣封包只有該指定皮克敏，原生往返 45 ms，RPC 完成、同 task 獎勵領取與庫存消失皆確認，遊戲 PID 未變。這是單筆成功，不代表所有禮物狀態或大量作業都已驗收。

禮物盒接受相同的 GPS 到點閘門，只允許遊戲指定的皮克敏。缺少指定 ID、忙碌、受動作限制、已預留或原生 Allows 拒絕時安全略過；不改派其他皮克敏，也不中斷蘑菇任務。選隊後與真正送出前的 ID 核對仍保留。

**1.4.13 / code47（2026-09-06）** 新增版本化、原子發布的 `dispatch_availability.tsv`，提供飛行前不可用預檢，不新增 hooks/RVA，也不以 advisory 資料授權派遣。搭配 Control Center0.6.8 已通過 Android14 一筆可用禮物＋連續兩筆花苗：三筆皆首試成功並領回，原生往返472/125/40ms，遊戲未閃退；下一跳距前筆Start9.776秒且在領回之後。此版本水果、自然忙碌／無可用隊伍時的不飛行略過及大量作業仍待驗證。回退使用已驗證1.4.12，勿用1.4.10。[預檢與實測紀錄](PREFLIGHT_CHECKPOINT_2026-09-06.md)。

**已知限制（2026-09-05）：** 1.4.9 曾在使用者確認禮物專屬皮克敏全忙於蘑菇時仍顯示選隊數1；候選數不是可派遣證明。1.4.11 實測拒絕指定ID不符；1.4.12 再限制picker輸入，已完成單筆完整禮物派遣與領取，但自然忙碌樣本和所有禮物變體仍待驗證，不能單凭庫存投影判定真正閒置。1.4.9 沒有批次2秒硬閘門，勿回退至該版本。見 [1.4.12驗證](GIFT_SELECTION_2026-09-05.md)。

### 控制檔

- `/data/local/tmp/pikmin-dispatch-mode.txt`：`off`、`armed` 或 `batch`。
- `/data/local/tmp/pikmin-dispatch-kinds.txt`：可選的 armed 篩選器：`seed`、`fruit`、`gift`，或花田用的 `farm`（水果＋花苗、排除禮物盒）；缺少或未知值等同 `all`，不改變既有 armed 行為。批次模式不使用此檔。
- 批次模式另使用 target 與 ready 檔，只允許 Control Center 指定的一筆任務。
- 模式檔權限必須是 `0644`。`0600 root` 會使遊戲注入行程讀不到檔案，module 會安全視為 `off`。

`off` 會釋放尚未完成的確認鎖；Control Center 會在完成、停止或逾時時寫回 `off`。

1.4.10 的已送出隊伍預留不因 `off` 或 RPC 逾時直接釋放，以免模式切換後重新使用仍可能在途的皮克敏；需等待庫存狀態轉換或任務消失的證據。

## 花田種花控制

native1.4.12 + Control Center0.6.5 已於 Android14 完成新鮮花田14點×2輪：17筆水果全部原生派遣並領取、無RPC失敗或遊戲閃退，全程連續種花及停止後原生關閉結算視窗流程正常。這不是同路線速度比較，也不代表所有可能花朵皆產果。[實測與限制](FLOWER_FARM_2026-09-05.md)。

原生 module 另讀取 `/data/local/tmp/pikmin-planting-mode.txt`：`on` 會使用遊戲目前選取的花瓣，呼叫遊戲自身的 `StartPlantingWithConfirmationAsync(..., false)`；`off` 只會停止由 module 自己啟動的種花，絕不停止玩家原本手動開始的 session。缺少或未知值一律只是觀察，不會改變種花狀態。狀態寫到 `files/planting_control_status.tsv`。

開始／停止已於 Pikmin Bloom v152 實機驗證：沿用目前選取的花瓣進入種花中，`off` 後回報已停止。對 module 自己停止的 session，會等待並以遊戲原生的結算視窗關閉方法收尾；不使用 Accessibility 或畫面座標。

Control Center 的花田兩輪服務會在整段工作期間額外寫入 `/data/local/tmp/pikmin-flower-farm-mode.txt=on`。只有這個顯式 session 存在時，module 才會以遊戲自己的 `Task.CompletedTask` 抑制 `SpeedMonitor` 的速度警告**視窗**；它不改變 GPS、`IsPlayerSpeeding` 或遊戲／伺服器的速度限制判定。此 hook 已在 v152 build 安裝，仍待下一輪有效花田確認實際提示不再出現。GPS 跳點、停留時間與兩輪排程仍屬於 Control Center，不是本 module 單獨提供的功能。

## 安裝

1. 從 `zygisk-module/pikmin-nectar-rpc-v152.zip`（或以原始碼建置的同名 zip）安裝 Magisk module。
2. 重新開機。
3. 安裝並在 Magisk 授權相容的控制 App，例如私有的 Pikmin Control Center。
4. 使用前確認遊戲版本、module 狀態與控制檔權限。

安裝前先確認實際裝置，不要假設 Wi-Fi ADB 位址固定：

```powershell
adb devices
adb -s <device-serial> push .\zygisk-module\pikmin-nectar-rpc-v152.zip /data/local/tmp/pikmin-nectar-rpc-v152.zip
adb -s <device-serial> exec-out su -c 'chmod 644 /data/local/tmp/pikmin-nectar-rpc-v152.zip && magisk --install-module /data/local/tmp/pikmin-nectar-rpc-v152.zip'
adb -s <device-serial> reboot
```

## 路邊大花精華

只有同時符合下列條件才會送出領取：有網路介面、遊戲正在種花、已取得遊戲內部 GPS、目標大花正在盛開且尚未領取、距離角色不超過 100 m。

原始檔案位於 Pikmin Bloom app data 的 `files` 目錄：

- `nectar_rpc_mode.txt`：`test_once` 或 `auto`
- `nectar_status.tsv`：即時 GPS、種花、網路、花數、RPC 與最後結果
- `nectar_claims.tsv`：領取結果

## 從原始碼建置

需要 Android NDK r27d、CMake 與 Ninja：

```powershell
powershell -ExecutionPolicy Bypass -File .\zygisk-module\build.ps1 `
  -NdkPath C:\Android\android-ndk-r27d `
  -CmakePath C:\Tools\cmake.exe `
  -NinjaPath C:\Tools\ninja.exe
```

## 專案結構

- `zygisk-module/`：native hooks、RPC、返程處理、探險派遣與 TSV。
- `android-app/`：傳統大花精華控制 App。
- `dist/`：已建置安裝檔。

第三方元件與授權資訊請見 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
