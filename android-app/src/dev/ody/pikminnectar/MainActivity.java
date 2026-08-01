package dev.ody.pikminnectar;

import android.app.Activity;
import android.app.AlertDialog;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class MainActivity extends Activity {
    private static final String FILES = "/data/user/0/com.nianticlabs.pikmin/files";
    private static final String MODE = FILES + "/nectar_rpc_mode.txt";
    private static final String STATUS = FILES + "/nectar_status.tsv";
    private static final String CLAIMS = FILES + "/nectar_claims.tsv";

    private final Handler handler = new Handler(Looper.getMainLooper());
    private final ExecutorService worker = Executors.newSingleThreadExecutor();
    private Switch autoSwitch;
    private TextView stateText;
    private TextView gpsText;
    private TextView detailText;
    private TextView logText;
    private boolean binding;
    private boolean destroyed;

    private final Runnable refreshLoop = new Runnable() {
        @Override public void run() {
            refresh();
            if (!destroyed) handler.postDelayed(this, 2000);
        }
    };

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setStatusBarColor(Color.rgb(38, 98, 57));
        setContentView(buildUi());
        handler.post(refreshLoop);
    }

    @Override protected void onDestroy() {
        destroyed = true;
        handler.removeCallbacksAndMessages(null);
        worker.shutdownNow();
        super.onDestroy();
    }

    private View buildUi() {
        int pad = dp(20);
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(pad, dp(18), pad, pad);
        root.setBackgroundColor(Color.rgb(246, 249, 245));
        scroll.addView(root);

        TextView title = text("Pikmin Bloom 大花精華", 26, Color.rgb(28, 70, 40), true);
        root.addView(title);
        TextView subtitle = text("直接讀取地圖資料並呼叫領取 RPC，不使用拉花手勢", 14, Color.DKGRAY, false);
        subtitle.setPadding(0, dp(5), 0, dp(18));
        root.addView(subtitle);

        LinearLayout control = card();
        autoSwitch = new Switch(this);
        autoSwitch.setText("自動領取範圍內大花精華");
        autoSwitch.setTextSize(18);
        autoSwitch.setPadding(0, 0, 0, dp(8));
        autoSwitch.setOnCheckedChangeListener(this::onToggle);
        control.addView(autoSwitch);
        stateText = text("正在讀取…", 17, Color.rgb(35, 90, 50), true);
        gpsText = text("GPS：—", 15, Color.DKGRAY, false);
        detailText = text("地圖資料：—", 14, Color.DKGRAY, false);
        control.addView(stateText);
        control.addView(gpsText);
        control.addView(detailText);
        root.addView(control);

        LinearLayout actions = new LinearLayout(this);
        actions.setOrientation(LinearLayout.HORIZONTAL);
        actions.setPadding(0, dp(12), 0, dp(12));
        Button refresh = button("重新整理");
        refresh.setOnClickListener(v -> refresh());
        Button clear = button("清除 LOG");
        clear.setOnClickListener(v -> confirmClear());
        actions.addView(refresh, new LinearLayout.LayoutParams(0, dp(48), 1));
        LinearLayout.LayoutParams clearParams = new LinearLayout.LayoutParams(0, dp(48), 1);
        clearParams.setMarginStart(dp(10));
        actions.addView(clear, clearParams);
        root.addView(actions);

        TextView logTitle = text("領取紀錄", 20, Color.rgb(28, 70, 40), true);
        root.addView(logTitle);
        logText = text("尚無紀錄", 14, Color.rgb(40, 40, 40), false);
        logText.setTypeface(Typeface.MONOSPACE);
        logText.setTextIsSelectable(true);
        logText.setPadding(0, dp(8), 0, dp(20));
        root.addView(logText);
        return scroll;
    }

    private void onToggle(CompoundButton ignored, boolean enabled) {
        if (binding) return;
        autoSwitch.setEnabled(false);
        worker.execute(() -> {
            try {
                String value = enabled ? "auto" : "diag";
                String command = "printf '" + value + "\\n' > " + MODE +
                        "; uid=$(stat -c %u " + FILES + "); chown $uid:$uid " + MODE +
                        "; chmod 0644 " + MODE + "; restorecon " + MODE;
                execRoot(command);
                runOnUiThread(() -> Toast.makeText(this,
                        enabled ? "自動領取已開啟" : "自動領取已關閉", Toast.LENGTH_SHORT).show());
            } catch (Exception error) {
                runOnUiThread(() -> Toast.makeText(this, "root 寫入失敗：" + error.getMessage(), Toast.LENGTH_LONG).show());
            } finally {
                runOnUiThread(() -> { autoSwitch.setEnabled(true); refresh(); });
            }
        });
    }

    private void refresh() {
        worker.execute(() -> {
            try {
                String raw = execRoot("printf '%s\\n' '---MODE---'; cat " + MODE +
                        " 2>/dev/null; printf '%s\\n' '---STATUS---'; cat " + STATUS +
                        " 2>/dev/null; printf '%s\\n' '---CLAIMS---'; tail -n 100 " + CLAIMS + " 2>/dev/null; true");
                Snapshot snapshot = Snapshot.parse(raw);
                runOnUiThread(() -> bind(snapshot));
            } catch (Exception error) {
                runOnUiThread(() -> {
                    stateText.setText("無法取得 root 資料");
                    stateText.setTextColor(Color.rgb(180, 45, 45));
                    detailText.setText(error.getMessage());
                });
            }
        });
    }

    private void bind(Snapshot value) {
        binding = true;
        autoSwitch.setChecked("auto".equals(value.mode));
        binding = false;
        if (value.status.length < 10) {
            stateText.setText("等待模組狀態（更新模組後需重新開機）");
            stateText.setTextColor(Color.rgb(180, 100, 20));
            gpsText.setText("GPS：—");
            detailText.setText("目前模式：" + value.mode);
        } else {
            boolean planting = "1".equals(value.status[2]);
            boolean online = "1".equals(value.status[3]);
            boolean hasGps = "1".equals(value.status[4]);
            boolean rpc = "1".equals(value.status[8]);
            boolean ready = planting && online && hasGps && rpc;
            stateText.setText(ready ? "● 自動化條件已就緒" : "● 等待條件");
            stateText.setTextColor(ready ? Color.rgb(35, 125, 60) : Color.rgb(190, 105, 15));
            gpsText.setText(hasGps ? "GPS：" + value.status[5] + ", " + value.status[6] : "GPS：等待中");
            detailText.setText("種花 " + yesNo(planting) + "　網路 " + yesNo(online) +
                    "　RPC " + yesNo(rpc) + "\n地圖大花：" + value.status[7] +
                    "　最後結果：" + translateResult(value.status[9]));
        }
        logText.setText(formatClaims(value.claimLines));
    }

    private void confirmClear() {
        new AlertDialog.Builder(this)
                .setTitle("清除領取紀錄？")
                .setMessage("只會清除 APP 顯示的 nectar_claims.tsv，不會變更遊戲資料。")
                .setNegativeButton("取消", null)
                .setPositiveButton("清除", (dialog, which) -> worker.execute(() -> {
                    try { execRoot(": > " + CLAIMS); } catch (Exception ignored) {}
                    runOnUiThread(this::refresh);
                })).show();
    }

    private String formatClaims(String[] lines) {
        if (lines.length == 0) return "尚無領取紀錄";
        StringBuilder out = new StringBuilder();
        for (int i = lines.length - 1; i >= 0; --i) {
            String[] x = lines[i].split("\\t", -1);
            if (x.length < 12) continue;
            String time;
            try { time = new SimpleDateFormat("MM/dd HH:mm:ss", Locale.TAIWAN).format(new Date(Long.parseLong(x[0]))); }
            catch (Exception error) { time = x[0]; }
            String reward = "獎勵類型 " + x[8] + " × " + x[9];
            if ("3".equals(x[8])) reward = honeyName(x[10]) + flowerName(x[11]) + "精華 × " + x[9];
            if (!"SUCCESS".equals(x[1])) reward = translateResult(x[1] + ":" + x[7]);
            out.append(time).append("  ").append(reward).append('\n')
                    .append("GPS ").append(x[3]).append(", ").append(x[4])
                    .append("　").append(x[5]).append("m\n")
                    .append("花 ID ").append(x[2]).append("\n\n");
        }
        return out.length() == 0 ? "尚無領取紀錄" : out.toString().trim();
    }

    private static String honeyName(String code) {
        switch (code) { case "1": return "白色"; case "2": return "紅色"; case "3": return "藍色";
            case "4": return "黃色"; case "5": return "愛心"; default: return "未知顏色"; }
    }

    private static String flowerName(String code) {
        String[] names = {"未知", "向日葵", "鬱金香", "三色堇", "玫瑰", "一般花", "聖誕紅", "山茶花",
                "梅花", "水仙", "櫻花", "粉蝶花", "康乃馨", "海芋", "繡球花", "百合", "扶桑花",
                "雞蛋花", "彼岸花", "波斯菊", "仙客來", "銀蓮花", "石竹", "龍膽", "菊花",
                "聖誕玫瑰", "嘉德麗雅蘭", "風信子", "香豌豆", "鈴蘭", "牡丹", "睡蓮", "牽牛花",
                "九重葛", "大理花", "鐵線蓮", "雪花蓮", "小蒼蘭", "油菜花", "玫瑰", "鳶尾花",
                "天堂鳥", "雞冠花", "萬壽菊", "鼠尾草", "報春花", "蝴蝶蘭", "金魚草", "矮牽牛",
                "鬱金香", "勿忘草", "罌粟花", "桔梗", "曇花", "美人蕉", "洋桔梗", "薊花",
                "天竺葵", "酢漿草"};
        try { int index = Integer.parseInt(code); return index >= 0 && index < names.length ? names[index] : "花種" + code; }
        catch (Exception error) { return "花種" + code; }
    }

    private static String translateResult(String value) {
        if (value == null) return "—";
        if (value.startsWith("SUCCESS")) return "成功";
        if (value.contains("INVENTORY_FULL")) return "精華庫存已滿";
        if (value.contains("ALREADY_REWARDED")) return "已領取";
        if (value.contains("NOT_BLOOMING")) return "大花未盛開";
        if (value.contains("PENDING")) return "處理中";
        if (value.contains("RPC_FAULTED")) return "RPC 失敗";
        return value;
    }

    private String execRoot(String command) throws Exception {
        Process process = new ProcessBuilder("su", "-mm", "-c", command).redirectErrorStream(true).start();
        StringBuilder output = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
            String line; while ((line = reader.readLine()) != null) output.append(line).append('\n');
        }
        int code = process.waitFor();
        if (code != 0) throw new IllegalStateException("su exit " + code + ": " + output.toString().trim());
        return output.toString();
    }

    private LinearLayout card() {
        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(18), dp(16), dp(18), dp(16));
        card.setBackgroundColor(Color.WHITE);
        return card;
    }

    private TextView text(String value, int sp, int color, boolean bold) {
        TextView view = new TextView(this); view.setText(value); view.setTextSize(sp); view.setTextColor(color);
        if (bold) view.setTypeface(Typeface.DEFAULT, Typeface.BOLD); return view;
    }

    private Button button(String label) { Button button = new Button(this); button.setText(label); button.setAllCaps(false); return button; }
    private int dp(int value) { return Math.round(value * getResources().getDisplayMetrics().density); }
    private static String yesNo(boolean value) { return value ? "✓" : "✗"; }

    private static final class Snapshot {
        String mode = "diag";
        String[] status = new String[0];
        String[] claimLines = new String[0];

        static Snapshot parse(String raw) {
            Snapshot result = new Snapshot();
            String section = "";
            StringBuilder claims = new StringBuilder();
            for (String line : raw.split("\\r?\\n")) {
                if ("---MODE---".equals(line)) { section = "mode"; continue; }
                if ("---STATUS---".equals(line)) { section = "status"; continue; }
                if ("---CLAIMS---".equals(line)) { section = "claims"; continue; }
                if ("mode".equals(section) && !line.trim().isEmpty()) result.mode = line.trim();
                else if ("status".equals(section) && !line.trim().isEmpty()) result.status = line.split("\\t", -1);
                else if ("claims".equals(section) && !line.trim().isEmpty()) claims.append(line).append('\n');
            }
            String value = claims.toString().trim();
            result.claimLines = TextUtils.isEmpty(value) ? new String[0] : value.split("\\n");
            return result;
        }
    }
}
