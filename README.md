# vkpp

C++ wrappers for Vulkan.

## Build

1. Make sure all submodules are updated:

    ```powershell
    git submodule update --init --recursive
    ```

2. Setup [microsoft/vcpkg](https://github.com/microsoft/vcpkg):

    ```powershell
    # Clone vcpkg into a folder you like, which can also be shared by other projects:
    git clone https://github.com/microsoft/vcpkg.git
    # Run the bootstrap script:
    cd vcpkg
    .\bootstrap-vcpkg.bat # Linux: ./bootstrap-vcpkg.sh
    # Configure the VCPKG_ROOT environment variable for convenience:
    $env:VCPKG_ROOT="C:\path\to\vcpkg" # Linux: export VCPKG_ROOT="/path/to/vcpkg"
    ```

3. Setup [radsdk](https://github.com/majunlichn/radsdk) for additional dependencies:

    ```powershell
    # Clone radsdk into a folder you like (requires Git LFS support):
    git clone https://github.com/majunlichn/radsdk.git
    # Execute setup.py to download and build additional libraries:
    cd radsdk
    python setup.py
    # Configure the RADSDK_ROOT environment variable for convenience:
    $env:RADSDK_ROOT="C:\path\to\radsdk" # Linux: export RADSDK_ROOT="/path/to/radsdk"
    ```

3. Generate project files and build:

    ```powershell
    # At the root of the vkpp repository:
    cmake -S . -B build -D VCPKG_MANIFEST_DIR="$env:RADSDK_ROOT" -D VCPKG_INSTALLED_DIR="$env:RADSDK_ROOT/vcpkg_installed"
    ```
