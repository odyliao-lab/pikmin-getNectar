$ErrorActionPreference = 'Stop'
$src = Join-Path (Split-Path $PSScriptRoot -Parent) 'cpp'
$recovery = Get-Content -LiteralPath (Join-Path $src 'planter_recovery.inc') -Raw
$engine = Get-Content -LiteralPath (Join-Path $src 'planter_automation.inc') -Raw
$checks = @(
    @($recovery.Contains('fresh_recovery(request,getpid(),now_ms())'), 'current PID and expiry gate'),
    @($recovery.Contains('if(pending.task)'), 'in-flight request cannot be bypassed'),
    @($recovery.Contains('same_ledger(ledger,request.expected)'), 'exact expected pending operation'),
    @($recovery.Contains('now_ms()-recovery_seen_at<5000'), 'two stable observations before release'),
    @($recovery.Contains('recovery_seen_at=0;recovery_waiting=true'), 'incomplete inventory resets observation'),
    @(($recovery.IndexOf('std::rename(pending_path') -lt $recovery.IndexOf('restart_blocked=false')), 'archive before unblocking'),
    @($recovery.Contains('if(read_bounded(pending_path,512)!=raw)'), 'pending bytes rechecked before archival'),
    @((!$recovery.Contains('set_expedition') -and !$recovery.Contains('call(') -and !$recovery.Contains('start("')), 'recovery submits no managed mutation'),
    @($engine.Contains('if (recovery_waiting)'), 'pause normal automation while reviewing'),
    @($recovery.Contains('next_action=now_ms()+5000'), 'normal automation delayed after release')
)
foreach($check in $checks){if(!$check[0]){throw "FAIL: $($check[1])"};Write-Output "PASS: $($check[1])"}
