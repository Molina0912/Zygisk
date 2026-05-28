# Play Integrity Bypass

Módulo Magisk + Zygisk que intenta bypassear Google Play Integrity API spoofeando propiedades del dispositivo y ocultando rastros de root/Magisk.

## Estructura

```
PlayIntegrityBypass/
├── module.prop          # Metadatos del módulo Magisk
├── customize.sh         # Script de instalación
├── service.sh           # Servicio post-boot (resetprop, hide procs)
├── system.prop          # Propiedades del sistema
├── build.sh             # Script de compilación (ndk-build)
├── src/                 # Código fuente C++
│   ├── main.cpp
│   ├── hooks.cpp / hooks.h
│   ├── integrity_spoof.cpp / integrity_spoof.h
│   ├── device_spoof.cpp / device_spoof.h
│   ├── utils.cpp / utils.h
│   ├── zygisk.hpp
│   ├── Android.mk
│   └── Application.mk
└── .github/workflows/build.yml   # CI: compila el ZIP en cada push
```

## Compilar localmente

Requiere Android NDK (r25 o superior recomendado).

```bash
export ANDROID_NDK_HOME=/ruta/al/android-ndk
chmod +x build.sh
./build.sh
```

Resultado: `out/PlayIntegrityBypass-v1.2.0.zip`.

## Compilación automática (GitHub Actions)

Cada push a `main` dispara el workflow `.github/workflows/build.yml`, que:

1. Configura el NDK.
2. Ejecuta `build.sh`.
3. Sube el ZIP del módulo como artefacto descargable desde la pestaña **Actions**.

## Instalación

1. Descarga `PlayIntegrityBypass-v1.2.0.zip` (artefacto de Actions o tu build local).
2. Magisk → Modules → Install from storage → selecciona el ZIP.
3. Reinicia.

## Requisitos

- Magisk v24.0+ con Zygisk activado.
- Android 11–15.
- Arquitecturas soportadas: `arm64-v8a`, `armeabi-v7a`, `x86`, `x86_64`.

## Aviso

Este código es para fines educativos y de investigación. El uso para evadir verificaciones de integridad en aplicaciones de producción puede violar sus términos de servicio.
