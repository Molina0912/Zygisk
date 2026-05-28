#!/bin/bash

set -e

# Configuración
NDK_PATH="${ANDROID_NDK_HOME:-$ANDROID_HOME/ndk/25.2.9519653}"
MODULE_NAME="PlayIntegrityBypass"
MODULE_VERSION="v1.2.0"

echo "=========================================="
echo "  Compilando $MODULE_NAME $MODULE_VERSION"
echo "=========================================="

# Verificar NDK
if [ ! -d "$NDK_PATH" ]; then
    echo "Error: NDK no encontrado en $NDK_PATH"
    echo "Configura ANDROID_NDK_HOME o ANDROID_HOME."
    exit 1
fi

NDK_BUILD="$NDK_PATH/ndk-build"
if [ ! -x "$NDK_BUILD" ]; then
    echo "Error: ndk-build no encontrado en $NDK_BUILD"
    exit 1
fi

# Limpiar compilaciones anteriores
rm -rf build out libs obj
mkdir -p build out libs

# Compilar todas las arquitecturas usando ndk-build
echo "Compilando con ndk-build..."
"$NDK_BUILD" \
    NDK_PROJECT_PATH=. \
    APP_BUILD_SCRIPT=src/Android.mk \
    NDK_APPLICATION_MK=src/Application.mk \
    NDK_LIBS_OUT=libs \
    NDK_OUT=obj \
    -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# Crear estructura del módulo Magisk en una carpeta de staging
STAGING="out/staging"
echo "Creando módulo Magisk..."
rm -rf "$STAGING"
mkdir -p "$STAGING/zygisk"

# Copiar archivos del módulo a la raíz del staging
cp module.prop   "$STAGING/"
cp customize.sh  "$STAGING/"
cp system.prop   "$STAGING/"
cp service.sh    "$STAGING/"

# Copiar META-INF (necesario para que Magisk reconozca el zip como módulo)
cp -R META-INF   "$STAGING/"

# Copiar bibliotecas compiladas
for ABI in arm64-v8a armeabi-v7a x86 x86_64; do
    SRC="libs/$ABI/libplayintegrity.so"
    if [ -f "$SRC" ]; then
        cp "$SRC" "$STAGING/zygisk/$ABI.so"
        echo "  + zygisk/$ABI.so"
    else
        echo "  ! Falta $SRC"
    fi
done

# Asegurar permisos correctos
chmod 0755 "$STAGING/customize.sh" "$STAGING/service.sh" 2>/dev/null || true
chmod 0755 "$STAGING/META-INF/com/google/android/update-binary" 2>/dev/null || true
find "$STAGING/zygisk" -type f -name "*.so" -exec chmod 0755 {} \; 2>/dev/null || true

# Empaquetar: los archivos van en la RAÍZ del zip, no dentro de una subcarpeta.
# Magisk requiere que module.prop esté en la raíz del zip.
ZIP_OUT="$PWD/out/$MODULE_NAME-$MODULE_VERSION.zip"
rm -f "$ZIP_OUT"
( cd "$STAGING" && zip -r "$ZIP_OUT" . -x "*.DS_Store" )

# Limpiar staging
rm -rf "$STAGING"

echo "=========================================="
echo "  Compilación completada"
echo "  Módulo: $ZIP_OUT"
echo "=========================================="

# Mostrar contenido del zip para verificar (sin abortar el script)
echo ""
echo "Listado de out/:"
ls -la out/ || true
echo ""
echo "Contenido del ZIP:"
unzip -l "$ZIP_OUT" || true
