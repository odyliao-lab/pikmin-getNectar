#!/system/bin/sh
# Root-owned IPC bridge.  The Zygisk library itself runs as Pikmin Bloom and
# can only access that app's private files; the control-center APK can safely
# access this module directory through Magisk root instead.
MODDIR=${0%/*}
GAME_FILES=/data/user/0/com.nianticlabs.pikmin/files
CONTROL_MODE="$MODDIR/nectar_control_mode.txt"
PUBLIC_STATUS="$MODDIR/nectar_status.tsv"
GAME_MODE="$GAME_FILES/nectar_rpc_mode.txt"
GAME_STATUS="$GAME_FILES/nectar_status.tsv"

[ -f "$CONTROL_MODE" ] || printf 'diag\n' >"$CONTROL_MODE"
chmod 0644 "$CONTROL_MODE"

(
  while true; do
    # The game process only needs to read this file; keep root ownership so a
    # compromised game process cannot silently enable automatic collection.
    if [ -r "$CONTROL_MODE" ]; then
      cp "$CONTROL_MODE" "$GAME_MODE" 2>/dev/null
      chmod 0644 "$GAME_MODE" 2>/dev/null
    fi
    # Publish diagnostic data for the controller without granting it access to
    # Pikmin Bloom's private directory.
    if [ -r "$GAME_STATUS" ]; then
      cp "$GAME_STATUS" "$PUBLIC_STATUS" 2>/dev/null
      chmod 0644 "$PUBLIC_STATUS" 2>/dev/null
    fi
    sleep 2
  done
) &
