#!/system/bin/sh
# Root-owned IPC bridge.  The Zygisk library itself runs as Pikmin Bloom and
# can only access that app's private files; the control-center APK can safely
# access this module directory through Magisk root instead.
MODDIR=${0%/*}
GAME_FILES=/data/user/0/com.nianticlabs.pikmin/files
CONTROL_MODE="$MODDIR/nectar_control_mode.txt"
RETURN_CONTROL_MODE="$MODDIR/return_control_mode.txt"
ADB_RETURN_CONTROL="/data/local/tmp/pikmin-return-mode.txt"
PUBLIC_STATUS="$MODDIR/nectar_status.tsv"
GAME_MODE="$GAME_FILES/nectar_rpc_mode.txt"
GAME_RETURN_MODE="$GAME_FILES/return_rpc_mode.txt"
GAME_STATUS="$GAME_FILES/nectar_status.tsv"
GAME_GPS="$GAME_FILES/nectar_system_gps.tsv"
GAME_RETURN_TRACE="$GAME_FILES/return_rpc_trace.tsv"
PUBLIC_RETURN_TRACE="$MODDIR/return_rpc_trace.tsv"

[ -f "$CONTROL_MODE" ] || printf 'diag\n' >"$CONTROL_MODE"
[ -f "$RETURN_CONTROL_MODE" ] || printf 'dry-run\n' >"$RETURN_CONTROL_MODE"
chmod 0644 "$CONTROL_MODE"
chmod 0644 "$RETURN_CONTROL_MODE"

(
  while true; do
    # The game process only needs to read this file; keep root ownership so a
    # compromised game process cannot silently enable automatic collection.
    if [ -r "$CONTROL_MODE" ]; then
      cp "$CONTROL_MODE" "$GAME_MODE" 2>/dev/null
      chmod 0644 "$GAME_MODE" 2>/dev/null
    fi
    if [ -r "$RETURN_CONTROL_MODE" ]; then
      # ADB/root cannot access Magisk's module label on this device.  Accept a
      # small allowlisted control file and copy it into the protected module
      # channel before exposing it to the game process.
      if [ -r "$ADB_RETURN_CONTROL" ]; then
        case "$(cat "$ADB_RETURN_CONTROL" 2>/dev/null)" in
          dry-run|one|batch|off) cp "$ADB_RETURN_CONTROL" "$RETURN_CONTROL_MODE" 2>/dev/null ;;
        esac
      fi
      cp "$RETURN_CONTROL_MODE" "$GAME_RETURN_MODE" 2>/dev/null
      chmod 0644 "$GAME_RETURN_MODE" 2>/dev/null
    fi
    # Publish diagnostic data for the controller without granting it access to
    # Pikmin Bloom's private directory.
    if [ -r "$GAME_STATUS" ]; then
      cp "$GAME_STATUS" "$PUBLIC_STATUS" 2>/dev/null
      chmod 0644 "$PUBLIC_STATUS" 2>/dev/null
    fi
    if [ -r "$GAME_RETURN_TRACE" ]; then
      cp "$GAME_RETURN_TRACE" "$PUBLIC_RETURN_TRACE" 2>/dev/null
      chmod 0644 "$PUBLIC_RETURN_TRACE" 2>/dev/null
    fi
    # Supply the active system location as a read-only fallback for game
    # versions whose internal LocationController layout has changed.
    SYSTEM_GPS="$(dumpsys location 2>/dev/null | sed -n 's|.*Location\[gps \([0-9.-]*\),\([0-9.-]*\).*|\1\t\2|p' | head -n 1)"
    if [ -n "$SYSTEM_GPS" ]; then
      printf '%s\n' "$SYSTEM_GPS" >"$GAME_GPS"
      chmod 0644 "$GAME_GPS" 2>/dev/null
    fi
    sleep 2
  done
) &
