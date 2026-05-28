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

# Crear módulo Magisk
echo "Creando módulo Magisk..."
mkdir -p "out/$MODULE_NAME/zygisk"

# Copiar archivos del módulo
cp module.prop "out/$MODULE_NAME/"
cp customize.sh "out/$MODULE_NAME/"
cp system.prop "out/$MODULE_NAME/"
cp service.sh "out/$MODULE_NAME/"

# Copiar bibliotecas compiladas
for ABI in arm64-v8a armeabi-v7a x86 x86_64; do
    SRC="libs/$ABI/libplayintegrity.so"
    if [ -f "$SRC" ]; then
        cp "$SRC" "out/$MODULE_NAME/zygisk/$ABI.so"
        echo "  + zygisk/$ABI.so"
    else
        echo "  ! Falta $SRC"
    fi
done

# customize.sh espera libs/$ARCH/libplayintegrity.so durante la instalación,
# así que también incluimos esa estructura en el zip final.
mkdir -p "out/$MODULE_NAME/libs"
cp -R libs/* "out/$MODULE_NAME/libs/" 2>/dev/null || true

# Empaquetar
cd out
rm -f "$MODULE_NAME-$MODULE_VERSION.zip"
zip -r "$MODULE_NAME-$MODULE_VERSION.zip" "$MODULE_NAME"
cd ..

echo "=========================================="
echo "  Compilación completada"
echo "  Módulo: out/$MODULE_NAME-$MODULE_VERSION.zip"
echo "=========================================="
