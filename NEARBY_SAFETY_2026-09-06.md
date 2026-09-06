# 一般附近派遣：定位同步與 120 秒上限

## 最新實機進度：armed 遠距離追趕與 3 筆並行領取通過

2026-09-06 使用者同意直接操作驗證，目標仍192.168.50.202:5555 / Android14 / game PID10976。native1.4.21/code55、SO hash符合、遊戲在前景、附近原先off/0644、種類fruit seed；return=all，planting=off，批次／花田服務皆未運作。未操作其他ADB裝置。

本輪原始證據在手機root-only目錄`/data/local/tmp/pikmin-nearby-armed-jump-1788696777000`：baseline.txt、before-candidates.tsv、逐秒gate.tsv、started-ids.txt、dispatch-history.tsv、selection.tsv、return-history.tsv。未提交整份遊戲日誌或二進位。工作區tmp有本輪執行與唯讀稽核腳本，不影響產品程式。

- 開啟普通armed後，以既有JoyStick TELEPORT介面從24.3568325,124.1660690飛往35.7042269,139.4021074（實際float座標35.7042274,139.4021149）。目的地5筆花苗，本輪達3筆Start即關閉新派遣。沒有使用CC批次模式或畫面手勢。
- raw先到、processed經27.1936827,127.9750805 → 30.0305309,131.7840919 → 32.8673792,135.5931034追趕。`game-location-catching-up`及`location-settling`期間新增Start均0；1788696791082才ready，epoch49→60。3筆選隊全部晚於目的地ready，並各隔約5秒才跨tick重驗及Start。
- 三筆Start時間1788696799473、1788696799481、1788696799488，相差僅15ms，均早於任何一筆領取完成；不是循序等待收回。post-set隊員ID去重為7個，沒有跨任務重複；pre-start CanTryStart=1、power=1、原生duration均正值≤120000。

| 任務ID | 原生往返ms | 隊伍數 | 選隊至重驗ms | Start至領取確認ms |
| --- | ---: | ---: | ---: | ---: |
| EhZpOEtrT0pRcFJJbUNRZEdKV01OLVVR | 168 | 1 | 5036 | 3966 |
| EhZXMHN5bkJtNlIxLWJvVkpUR1dZci1R | 26242 | 1 | 5037 | 34234 |
| EhYxN05lb2o0NVFZaWxrVm81SGRhQ1Z3 | 28742 | 5 | 5039 | 36256 |

- 三筆都有start-rpc-completed及同ID返程batch-confirmed。最後一筆1788696835744確認後才還原GPS，期間沒有提早離開目的地。最終位置已回起點附近，ready/epoch70，附近off/0644、種類未改、PID10976持續運作；另外2筆花苗未派，保留後續樣本。
- 唯讀稽核已檢查：恰3筆Start、選隊和Start皆在目的地ready之後、duration與pre-start相同且在範圍內、跨tick≥1500ms、隊員數符合且7個ID無重複、每筆各一次返程確認、Start跨度≤1000ms。全部通過。
- 本輪首個測試腳本用Android shell算`epochSeconds*1000`溢位，錯把舊歷史算入新測試，安全中止且off；沒有新增Start。已先還原GPS，再將baseline改為字串`date +%s000`並驗13位數才正式重測。失敗證據目錄尾碼1990315864不可混入有效樣本，也不是native錯誤。

**結論：本次「遠距離分段期間不提前派遣、到點穩定＋跨tick重驗、最多3筆並行且隊伍不重複、成功自動領取」已在這組花苗樣本端到端通過。** 不擴大宣稱所有類型／所有GPS app已驗證：本輪未重測水果／禮物／忙碌owner，未實際製造>120秒原生候選（上限邊界仍由既有policy測試覆蓋），批次／花田完整回歸及先前UI待辦另列。没有新程式、APK或native build，無需重開。

## 前一輪：1.4.21 已載入，靜止單筆與 off 跳點觀察通過

- 第二次重開後，目標仍192.168.50.202:5555 / 24095PCADG / Android14；game PID10976。19:56:00 hooks正常載入、19:56:31 `runtime fields verified=1`，未重現前次120秒loader逾時。module1.4.21/code55與SO `b3d8ee978390c220e6cc5b068816206294646e53a4be77ea57fbb6ef19850807`一致。
- 新guard同PID、連續多次ready / 3個fix / epoch11，raw與processed約24.3568325,124.1660690。CLOCK_BOOTTIME freshness修正實機通過。
- 靜止時僅一筆花苗在200m內（約18.4m）。附近短暫armed 16秒後自動恢復off/0644，未改種類fruit seed、未移動GPS。任務`EhZYc0w5VktJaFMtLWNwVTlwVEdhUUFR`：1788696036899 nearby-selection-settling，1隻／4580ms；1788696041971 nearby-pre-start-settled，1隻／4581ms、CanTryStart=1、power=1；下一毫秒start-requested，1788696042908 start-rpc-completed。跨tick間隔5072ms，沒有同tick立刻Start。
- 返程領取紀錄1788696051010 batch-dispatched、1788696052125 batch-confirmed，內容seed:1、pikmin:1；後續候選列表已無該ID。這是原生選隊→Start→返程領取的單筆紀錄確認，不是僅RPC完成，也非使用者目視確認。
- 單筆已領取後，維持off，以現有JoyStick TELEPORT service介面（不是CC UI操作）從原位置飛往既有花苗座標35.7042269,139.4021074，再返回原位置；沒有啟動批次／花田，兩者服務皆未運作。浮點目的地實際為35.7042274,139.4021149。
- 真實逐秒觀察：1788696161450 raw已到目的地，processed仍在原位，gate=game-location-catching-up；之後processed依序27.1936812,127.9750805 → 30.0305300,131.7840919 → 32.8673787,135.5931034；1788696170524才到目的地並location-settling，1788696173535才ready。epoch11→20，追趕約9秒，再穩定約3秒。
- 返程也出現3個中繼位置；1788696187686開始catching-up，1788696197791進入settling，1788696200833才ready，epoch20→30。最終已恢復原GPS附近24.3568325,124.1660690，game PID10976未變、附近off/0644、沒有額外Start。
- 前一輪待辦中的armed跨跳點與3筆花苗並行領取，已由本文上方最新實測完成；批次／花田回歸與先前UI待辦仍保留。off觀察本身不能替代armed負向測試。

## 先前部署紀錄（以下保留時間順序，最新以上方為準）

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
