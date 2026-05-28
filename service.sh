#!/system/bin/sh

# Script de servicio para Play Integrity Bypass

MODDIR=${0%/*}

# Esperar a que el sistema esté listo
sleep 10

# Ocultar suplentes de Magisk
resetprop ro.boot.verifiedbootstate green
resetprop ro.boot.vbmeta.device_state locked
resetprop ro.boot.veritymode enforcing
resetprop ro.secure 1
resetprop ro.debuggable 0

# Limpiar rastros de Magisk en propiedades
for prop in $(getprop | grep -E "magisk|zygisk|root" | cut -d'[' -f2 | cut -d']' -f1); do
    resetprop "$prop" ""
done

# Ocultar procesos sospechosos
for pid in $(ps -A | grep -E "magisk|su|zygisk" | awk '{print $1}'); do
    if [ -d "/proc/$pid" ]; then
        mount --bind /dev/null "/proc/$pid/cmdline" 2>/dev/null
    fi
done
