# pikmin-getNectar

在已 root、啟用 Magisk Zygisk 的 Android 手機上，透過 Pikmin Bloom 的 IL2CPP 原生 API 提供本機自動化能力。它不使用螢幕手勢或畫面座標點擊。

目前 module 提供三個彼此獨立的控制面：

- 路邊大花精華的掃描與原生領取 RPC；
- 返程獎勵的原生處理與狀態紀錄；
- 探險候選掃描、最快隊伍選擇，以及受 Control Center 嚴格限制的原生派遣。

> 遊戲更新會改變 IL2CPP RVA。使用 root、定位模擬或遊戲自動化可能違反服務條款，帳號與裝置風險由使用者自行承擔。

## 相容性

- Pikmin Bloom `152.0`
- Android `arm64-v8a`（已測 Android 13 / SDK 33）
- Magisk 與 Zygisk

module 會驗證 `libil2cpp.so` 檔案大小；不相符時不安裝 hooks。其他遊戲版本不得直接沿用此建置。

## 探險原生派遣

在 `armed` 模式下，module 從 `InventoryManager.GetPikminTaskList()` 讀取任務、以 `ExpeditionDataStore.ByIndex()` 取得資料，並以遊戲的 `PickFastestUpToItemLimitsReservingForTroop()` 選擇最快隊伍。

無論是附近自動派遣或 Control Center 批次模式，真正呼叫 `StartExpeditionAsync()` 前都會重新檢查：

- `CanTryStart == true`；
- `armed` 模式必須以遊戲內部位置確認候選在 200 m 內；
- 批次模式必須有指定 task 的新鮮到點證明，且遊戲內部位置在目標 4 m 內。

`armed` 會在同一輪 live task-list 掃描中，對仍在 200 m 內的候選最多送出三筆原生派遣；每筆都有獨立的 RPC 與庫存確認，避免 GPS 持續移動時後面的候選先離開範圍。三筆是保守的初始上限，不是無限制 burst；已送出的 task 不會在確認前重複送出。`batch` 完全不使用這個平行路徑，仍是指定目標、到點後一次一筆。

已在 v152 實機驗證花苗／水果：選擇最快隊伍、原生派遣和後續庫存確認皆成功。派遣歷史 TSV 保留最近 24 小時，事件包括候選、套用隊伍、送出派遣與庫存確認。

禮物盒會輸出候選並接受相同的 GPS 到點閘門，但只允許遊戲指定的皮克敏。若遊戲回報不可派遣，module 記錄略過且不會改派其他皮克敏。可派遣禮物盒尚未完成驗證；不要把它視為完成的功能。

### 控制檔

- `/data/local/tmp/pikmin-dispatch-mode.txt`：`off`、`armed` 或 `batch`。
- `/data/local/tmp/pikmin-dispatch-kinds.txt`：可選的 armed 篩選器：`seed`、`fruit`、`gift`，或花田用的 `farm`（水果＋花苗、排除禮物盒）；缺少或未知值等同 `all`，不改變既有 armed 行為。批次模式不使用此檔。
- 批次模式另使用 target 與 ready 檔，只允許 Control Center 指定的一筆任務。
- 模式檔權限必須是 `0644`。`0600 root` 會使遊戲注入行程讀不到檔案，module 會安全視為 `off`。

`off` 會釋放尚未完成的確認鎖；Control Center 會在完成、停止或逾時時寫回 `off`。

## 花田種花控制（第一階段）

原生 module 另讀取 `/data/local/tmp/pikmin-planting-mode.txt`：`on` 會使用遊戲目前選取的花瓣，呼叫遊戲自身的 `StartPlantingWithConfirmationAsync(..., false)`；`off` 只會停止由 module 自己啟動的種花，絕不停止玩家原本手動開始的 session。缺少或未知值一律只是觀察，不會改變種花狀態。狀態寫到 `files/planting_control_status.tsv`。

開始／停止已於 Pikmin Bloom v152 實機驗證：沿用目前選取的花瓣進入種花中，`off` 後回報已停止。它尚不包含 GPS 跳點、停留時間或兩輪排程；不要將它視為已完成的花田採果。

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
