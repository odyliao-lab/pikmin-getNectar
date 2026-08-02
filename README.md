# pikmin-getNectar

在已 root、啟用 Magisk Zygisk 的 Android 手機上，直接讀取 Pikmin Bloom 地圖中的大花資料，並在符合條件時呼叫遊戲內部的領取 RPC。此實作不使用螢幕手勢。

## 相容性

- Pikmin Bloom `150.0`
- Android `arm64-v8a`（實機測試：Android 13 / SDK 33）
- Magisk 與 Zygisk
- 控制 App 最低 Android 9（API 28）

遊戲更新通常會改變 IL2CPP RVA；版本不是 150.0 時請不要安裝或啟用。使用 root、定位模擬或遊戲自動化可能違反服務條款，帳號與裝置風險由使用者自行承擔。

## 安裝

1. 從 [`dist/pikmin-nectar-rpc-v150.zip`](dist/pikmin-nectar-rpc-v150.zip) 安裝 Magisk 模組並重新開機。
2. 安裝 [`dist/pikmin-nectar-control.apk`](dist/pikmin-nectar-control.apk)。
3. 第一次開啟控制 App 時允許 Magisk root 權限。App 會維持單一 root shell，正常使用期間不會因每次狀態刷新而重複顯示授權提示。
4. 開啟 Pikmin Bloom，停留在散步地圖並開始種花。
5. 回到控制 App，確認 GPS、種花、網路及 RPC 均顯示就緒，再打開「自動領取範圍內大花精華」。

開關預設為關閉。只有同時符合下列條件才會送出領取：

- 裝置有網路介面；
- 遊戲正在種花；
- 已取得遊戲內部 GPS；
- 大花正在盛開、尚未領取，且與角色距離不超過 100 公尺。

## LOG

控制 App 會顯示最近 100 筆領取紀錄，包括：

- 日期與時間
- GPS 經緯度與距離
- 大花 ID
- 精華顏色、花種與數量
- 成功、已領取、未盛開、背包已滿或 RPC 錯誤

原始狀態檔位於 Pikmin Bloom app data 的 `files` 目錄：

- `nectar_rpc_mode.txt`：`diag` 或 `auto`
- `nectar_status.tsv`：即時 GPS、種花、網路、花數、RPC 與最後結果
- `nectar_claims.tsv`：領取結果

## 從原始碼建置

### Zygisk 模組

需要 Android NDK r27d、CMake 與 Ninja。預設工具路徑可在命令列覆寫：

```powershell
powershell -ExecutionPolicy Bypass -File .\zygisk-module\build.ps1 `
  -NdkPath C:\Android\android-ndk-r27d `
  -CmakePath C:\Tools\cmake.exe `
  -NinjaPath C:\Tools\ninja.exe
```

### 控制 App

需要 JDK 17、Android SDK Platform 35 與 Build Tools 35.0.0：

```powershell
powershell -ExecutionPolicy Bypass -File .\android-app\build.ps1 `
  -SdkPath C:\Android\sdk `
  -JavaHome C:\Java\jdk-17
```

建置腳本會建立本機 debug keystore；正式散布時應改用自己的簽署金鑰。

## 專案結構

- `zygisk-module/`：大花掃描、條件判斷、RPC 與結構化 LOG
- `android-app/`：root 控制開關與 LOG 檢視器
- `dist/`：已建置的 v1.0.0 安裝檔

第三方元件與授權資訊請見 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
