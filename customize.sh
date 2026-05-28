#!/system/bin/sh

# Play Integrity Bypass - Script de instalación

ui_print "========================================"
ui_print "  Play Integrity Bypass"
ui_print "  Versión: v1.2.0"
ui_print "========================================"

# Verificar versión mínima de Magisk
if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
    ui_print "Error: Se requiere Magisk v24.0+"
    abort
fi

# Magisk expone $ARCH en formato corto: arm, arm64, x86, x64.
# Mapearlo al nombre de ABI (carpeta zygisk/<abi>.so).
case "$ARCH" in
    arm64)
        ABI=arm64-v8a
        ;;
    arm)
        ABI=armeabi-v7a
        ;;
    x86)
        ABI=x86
        ;;
    x64|x86_64)
        ABI=x86_64
        ;;
    arm64-v8a|armeabi-v7a)
        ABI="$ARCH"
        ;;
    *)
        ui_print "Arquitectura no soportada: $ARCH"
        abort
        ;;
esac

ui_print "Arquitectura detectada: $ARCH (ABI=$ABI)"

ZYGISK_LIB="$MODPATH/zygisk/$ABI.so"
if [ ! -f "$ZYGISK_LIB" ]; then
    ui_print "Error: Biblioteca no encontrada: zygisk/$ABI.so"
    abort
fi

# Eliminar las .so de otras arquitecturas para ahorrar espacio
for f in "$MODPATH"/zygisk/*.so; do
    [ "$f" = "$ZYGISK_LIB" ] && continue
    rm -f "$f"
done

# Configurar permisos
set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm "$ZYGISK_LIB" 0 0 0755

ui_print "========================================"
ui_print "Instalación completada"
ui_print "Reinicia el dispositivo para activar"
ui_print "========================================"
