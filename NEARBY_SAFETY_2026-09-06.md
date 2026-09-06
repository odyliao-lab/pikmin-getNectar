# 一般附近派遣：定位同步與 120 秒上限

## 部署狀態（實機驗收前檢查點）

- **19:45 實機續驗後，最新候選為 Native `1.4.21 / 55`**：已編譯、7組原生policy與5處MethodInfo檢查通過，並由 Magisk 安裝到 `modules_update`，**仍須再次重開機才載入**。APK維持0.6.15，不必重装。
- Control Center `0.6.15 / 32` 已同簽章覆蓋安裝。
- 目標 `192.168.50.202:5555`、Android14、24095PCADG。操作前 `adb devices` 已確認；未操作其它裝置。
- 原遊戲 PID4203、native1.4.19/SO hash 已備份。現在 `/data/local/tmp/pikmin-dispatch-mode.txt` 為 **off / 0644**，只暫停附近派遣；沒有改 JoyStick、GPS座標、返程／育苗或精華設定。
- **已請使用者重開機／解鎖／開啟遊戲，先保持附近派遣關閉、不移動 GPS。尚未宣稱新原生 runtime 欄位或實際派遣驗收成功。**

### 19:45 重開後實測：欄位成功，時鐘需修正

- 首次遊戲 PID11292 有Zygisk `detect game`，但120秒等待耗盡，記錄 `libil2cpp.so was not loaded`；沒有新heartbeat。後來核心庫已出現在maps，不能拿module.prop或舊TSV當成功。只重啟遊戲後PID19519於19:45:41成功安裝hooks、19:45:50記錄 `[NEARBY-GPS] runtime fields verified=1`。冷啟動等待逾時獨立列為待觀察，本次未修改loader或新增hook。
- 新guard同PID持續更新，但始終`stale-game-location`。實際raw與processed同為24.1663475,120.6338120；raw時間755828，而Unix時間1788695172167、系統uptime約757秒。1.4.20錯把遊戲欄位名稱中的WallTime當Unix時間，導致安全拒派，並非新誤派。
- 唯讀拉取該手機的`libichigonative.so`（SHA256 `896d46b45c7f2d63778220070ca63601f24a750f7e5b95e77df04207b7715d0e`），反組譯匯出`WallTime_GetElapsedWallTimeNanos` 0x32a340→0x32a29c，確認0x32a2bc設定clock id7，再呼叫clock_gettime，即CLOCK_BOOTTIME（包含休眠）。沒有提交遊戲二進位。
- 1.4.21只把raw freshness比較改用CLOCK_BOOTTIME；讀取失敗仍拒派，不自動校正offset、不退回raw/system GPS。跨tick計時仍用CLOCK_MONOTONIC；TSV第2欄仍是Unix heartbeat，第11欄保留遊戲原始boot-time ms。CC只用第2欄檢查heartbeat，APK協定無須改版。
- 新增實機時間數值、boot與monotonic不同、休眠後舊fix、clock失敗、誤用Unix／monotonic的測試。7組arm64測試通過是policy驗證，**1.4.21靜止ready、分段跳點阻擋及實際送出／收回仍待重開後驗收**。
- 新ZIP SHA256 `a885b86120b086a9ab42428feb67e7829da28cb473e014244be618f6df067343`；新SO `b3d8ee978390c220e6cc5b068816206294646e53a4be77ea57fbb6ef19850807`，已比對modules_update內容。原備份目錄新增`rollback-native-1.4.20.zip`，1.4.19備份保留。現在遊戲仍跑1.4.20，附近off/0644；未移動GPS、未送出測試派遣。

## 原因與使用者確認

使用者手動從台灣遠距離飛行，遊戲看到分3～4段移動。先前 CC UI 測試觀察到的 GPS 位移也都是使用者手動操作，不是 CC 自動飛行驗收。

1.4.19 普通 armed 以 `current_location()` 的 raw location（必要時 system GPS fallback）判斷200m，沒有原生時間上限、也沒有跨 tick 的隊伍穩定期。
2026-09-06 14:39:54 的三筆水果，`post-set` 與 `start-requested` 原生時間分別是519535566、519560925、519568240ms（約144小時），CanTryStart仍為true，因此舊程式放行。使用者同意位置穩定／跨tick重驗，明確將時間條件訂為**不超過2分鐘**，不是5分鐘或2秒。

## 本次規則

- **一般附近 armed（legacy kinds不是farm）**：遊戲處理後位置距任務≤200m，且真正選中隊伍的 `0 < TotalDurationMs <= 120000`，缺值／0／負值都不派。
- 不再用原始座標或系統GPS fallback授權附近派遣。
- 透過現有 IL2CPP C API 讀取 `LocationController.latestRawLocation`、`latestRawLocationWallTimeMs`、`<LatestDeviceLocation>k__BackingField`；依runtime欄位型別、value size與巢狀座標layout檢查，失敗關閉。参考dump只用來查欄位名稱，沒有套用舊dump偏移值或新增RVA/hook。
- raw／處理後遊戲位置差≤8m；raw timestamp≤5秒且不在未來；連續至少3個不同raw timestamp、觀察≥3秒、處理後位置保持於8m錨點內才ready。跳點／不同步／時鐘回退／心跳間隔>5秒會更換epoch，舊選隊全部作廢；心跳超過2秒不得使用ready。
- SetPikmins後只保存原生ID與epoch、不新增managed物件指標。下次task-list更新且經過至少1500ms，重新invalidate／讀取原生時間、CanTryStart、搬運力、同隊ID、隊員可用性、禮物owner，再立即讀取位置／模式／種類才能Start。
- 選隊超過20秒、任務消失、模式切換、位置epoch改變，都釋放尚未送出的預約；已送出的預約仍沿用原有安全確認機制，不能提前釋放。
- 保留最多3筆待選隊＋已送出工作，可在同一更新送出多筆，沒有改回一筆完成再下一筆。
- **批次保持4m／原生≤2秒與原有晚一tick啟動；花田保持其controller到點／停留流程**。新一般附近同步等待不套到短暫farm armed區間，避免破壞5秒停留。沒有改種花、返程、育苗、JoyStick service或getGPS。

這是派出前的保護；使用者在皮克敏已出發後再手動遠距離跳點，仍可能影響遊戲計算的返程，不能以本修正保證實際回收時間永遠≤2分鐘。

## 相容介面与診斷

既有 control file、6欄 `nearby_dispatch_status.tsv`、12欄候選TSV及派遣歷史欄位保持不變。

新 `files/nearby_gate_status.tsv`：12欄

`v1 / unix_wall_ms / pid / state / epoch / distinct_samples / raw_lat / raw_lng / processed_lat / processed_lng / raw_boot_ms / 120000`

state包含`waiting-game-location`、`stale-game-location`、`game-location-catching-up`、`location-settling`、`ready`等。每秒原子更新0644；來源只能是遊戲欄位，不是JoyStick目標或system fallback。

派遣歷史新增`nearby-selection-settling`、`nearby-selection-reset`、`nearby-duration-blocked`、`nearby-start-blocked`；blocked每task／reason限10秒一次，沿用24小時滾動。送出前selection diagnostics為`nearby-pre-start-settled`。沒有新的傳送行為或遊戲請求改寫。

CC只接受同PID、10秒內、v1／12欄、limit120000的guard heartbeat；缺少它會禁止開啟一般附近派遣，防止僅更新APK卻仍跑舊native。取得heartbeat不等於位置ready，位置追趕期間UI會顯示暫不派遣。

## 已完成的測試

- NDK r27d / Android28 arm64 Release 編譯、既有POSIX Magisk封裝成功。
- `tests/run-native-tests.ps1 -Device 192.168.50.202:5555`：7個獨立arm64 policy執行檔在手機通過（不載入遊戲，不是實際派遣）：dispatch_safety、managed_gc、nearby_policy、nearby_safety、planter_policy、planter_steps、return_policy。
- nearby_safety回放三筆實際超長duration、120000/120001邊界、3～4段遠距離跳點、直接跳點、相同fix重讀、舊／未來／NaN／缺欄位、時鐘回退、心跳中斷與經緯度0值。這是合成定位樣本測試，不是使用者實際飛行。
- generic MethodInfo靜態5處檢查通過。
- CC 12個JVM測試入口通過，新增NearbyGuard11項版本／PID／freshness／上限／狀態檢查；APK v3簽章驗證通過。

## 必須接續的實機驗收

1. 重開機後 `adb devices`、型號、module.prop1.4.21及實際SO hash；重新開遊戲，先保持dispatch off。若再遇loader120秒逾時，先檢查核心庫／登入狀態，不可把舊TSV當新heartbeat。
2. 檢查`[NEARBY-GPS] runtime fields verified=1`及新guard TSV同PID。若verified=0、讀不到或時間不合，**不得退回raw/system授權**，保持off研究欄位／時鐘。
3. 靜止時raw與processed對齊，distinct_samples≥3、state=ready；仍不代表Start已驗證。
4. 使用者操作遠距離跳點時先off觀察：raw先變、processed分段追趕，ready應失效／epoch改變，最後才恢復。記錄真實raw／processed／原生時間；不要拿先前手動位移冒稱新測試。
5. 保留可用水果／花苗、近點穩定後開armed；確認`nearby-selection-settling`→稍後`nearby-pre-start-settled`→`start-requested`全部正值≤120000，後續RPC／庫存確認及真正收回。
6. 有多筆時確認最多3筆並行且隊員不重複；無忙碌可用隊員就等，禮物僅專屬owner可用才派。長時間／分段中沒有Start，不能只看RPC完成就當領取成功。
7. 批次、花田完整回歸，以及上一階段0.6.14末尾補強的筆數保存／完整UI視覺矩陣仍待適當樣本驗收。

## 產物与回退

- 以下是1.4.20原始產物；**最新1.4.21 hash與續驗狀態見本文上方**。
- ZIP SHA256 `0baadd539a48556b08f9822cbef791bc0076ad3f37687d8b5d0134689df1abb0`
- SO SHA256 `03980696b07f7b8db81bbfc19361ae226f1d1031ac1cfa0710f76ae43e0b4a46`
- APK SHA256 `18116948ca7684872d682af558a49fce97dc33db3825d0dbc83118b284fe9014`
- root-only備份 `/data/local/tmp/pikmin-nearby-safety-20260906`：原mode/kinds、事前history/selection、rollback-1.4.19.so、rollback-native-1.4.19.zip、rollback-0.6.14.apk、prefs。
- 舊ZIP `2cb8308040de1635e4bc7c783fc28240a2acc1a2b3012a173114d6d55c17ebae`；舊SO `1c3d068110b26de5430e6b66248f92289b03233637717ed004fef30b4867cd8e`。
- 回退先關附近／批次／花田，確認無待送請求後裝舊ZIP並重開；回退到1.4.19時維持附近off，舊版缺口仍存在。不要只把舊SO覆蓋上去冒稱已載入。
