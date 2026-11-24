Package: tinygltf:x64-windows@3.0.0

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.51.36260.0
- CMake Version: 4.4.2
-    vcpkg-tool version: 2026-07-27-98d7cb0cf1f4686a3e43aa5672b6230c1d56bce8
    vcpkg-readonly: true
    vcpkg-scripts version: 1460b31b08c42cc2e9ac2c79f45ec8707e2675e2

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/syoyo/tinygltf/archive/v3.0.0.tar.gz -> syoyo-tinygltf-v3.0.0.tar.gz
syoyo-tinygltf-v3.0.0.tar.gz.34888.part: error: download from https://github.com/syoyo/tinygltf/archive/v3.0.0.tar.gz had an unexpected hash
note: Expected: 3305c94aaa6f2b82ac2533bb17672b0ddd0239c413acc87b428be50dc0f9bcd4c300f6ed7f3077424ccc8237e4e75f1194d311d21f799809e0220eeb3f8900a4
note: Actual  : b9a689f25284206969f31ba382abd06cd4ed69ee6a118af3484aaa17b68aebefddc97a4e722e3bde8fa39b737677cb09cbd3d3e6f2a573ac4ba1fb718478b79a
CMake Error at scripts/cmake/vcpkg_download_distfile.cmake:134 (message):
  Download failed, halting portfile.
Call Stack (most recent call first):
  scripts/cmake/vcpkg_from_github.cmake:120 (z_vcpkg_download_distfile)
  C:/Users/yelac/AppData/Local/vcpkg/registries/git-trees/43cde6d4cfbe0817d997fb8374f6c89d4959601b/portfile.cmake:2 (vcpkg_from_github)
  scripts/ports.cmake:209 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "dependencies": [
    "tinygltf"
  ]
}

```
</details>
