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

# Verificar arquitectura
case "$ARCH" in
    arm64-v8a|armeabi-v7a|x86|x86_64)
        ui_print "Arquitectura detectada: $ARCH"
        ;;
    *)
        ui_print "Arquitectura no soportada: $ARCH"
        abort
        ;;
esac

# Verificar que la biblioteca para esta arquitectura exista
ZYGISK_LIB="$MODPATH/zygisk/$ARCH.so"
if [ ! -f "$ZYGISK_LIB" ]; then
    # Compatibilidad: si el zip trae libs/$ARCH/libplayintegrity.so, copiarlo
    ALT_LIB="$MODPATH/libs/$ARCH/libplayintegrity.so"
    if [ -f "$ALT_LIB" ]; then
        ui_print "Copiando biblioteca desde libs/$ARCH..."
        mkdir -p "$MODPATH/zygisk"
        cp "$ALT_LIB" "$ZYGISK_LIB"
    else
        ui_print "Error: Biblioteca no encontrada para $ARCH"
        abort
    fi
fi

# Configurar permisos
set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm "$ZYGISK_LIB" 0 0 0755

# Limpiar archivos temporales
rm -rf "$MODPATH/libs"
rm -rf "$MODPATH/src"
rm -rf "$MODPATH/obj"
rm -rf "$MODPATH/build"

ui_print "========================================"
ui_print "Instalación completada"
ui_print "Reinicia el dispositivo para activar"
ui_print "========================================"
