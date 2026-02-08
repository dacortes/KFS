#!/bin/bash
# install-toolchain.sh - Installs i686-elf cross-compiler toolchain
# Usage: ./install-toolchain.sh [install_dir]

################################################################################
#                               CONFIGURATION                                  #
################################################################################
set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default installation directory
DEFAULT_PREFIX="$HOME/opt/cross"
PREFIX="${1:-$DEFAULT_PREFIX}"
TARGET="i686-elf"
SRC_DIR="$HOME/src"

# Version numbers
BINUTILS_VERSION="2.42"
GCC_VERSION="13.2.0"

################################################################################
#                               FUNCTIONS                                      #
################################################################################
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "$1 is not installed"
        return 1
    fi
    return 0
}

################################################################################
#                               DEPENDENCY CHECK                               #
################################################################################
check_dependencies() {
    print_info "Checking system dependencies..."

    local deps=(
        "build-essential"
        "bison"
        "flex"
        "libgmp3-dev"
        "libmpc-dev"
        "libmpfr-dev"
        "texinfo"
        "libisl-dev"
        "wget"
        "make"
        "gcc"
        "g++"
        "git"
    )

    local missing=()

    for dep in "${deps[@]}"; do
        if ! dpkg -l | grep -q "^ii.*$dep"; then
            missing+=("$dep")
        fi
    done

    if [ ${#missing[@]} -gt 0 ]; then
        print_warning "Missing dependencies: ${missing[*]}"
        print_info "Installing missing dependencies..."

        if [ "$EUID" -ne 0 ]; then
            print_error "Please run with sudo to install dependencies"
            exit 1
        fi

        apt update
        apt install -y "${missing[@]}"
        print_success "Dependencies installed"
    else
        print_success "All dependencies are installed"
    fi
}

################################################################################
#                               SETUP ENVIRONMENT                              #
################################################################################
setup_environment() {
    print_info "Setting up environment..."

    # Create directories
    mkdir -p "$PREFIX"
    mkdir -p "$SRC_DIR"

    # Export environment variables
    export PREFIX="$PREFIX"
    export TARGET="$TARGET"
    export PATH="$PREFIX/bin:$PATH"

    # Create bashrc entry
    BASH_ENTRIES=(
        "# i686-elf cross-compiler toolchain"
        "export PREFIX=\"$PREFIX\""
        "export TARGET=\"$TARGET\""
        "export PATH=\"\$PREFIX/bin:\$PATH\""
    )

    # Check if entries already exist
    for entry in "${BASH_ENTRIES[@]}"; do
        if ! grep -Fxq "$entry" ~/.bashrc 2>/dev/null; then
            echo "$entry" >> ~/.bashrc
        fi
    done

    print_success "Environment setup complete"
    print_info "Install directory: $PREFIX"
    print_info "Source directory: $SRC_DIR"
}

################################################################################
#                               DOWNLOAD SOURCES                               #
################################################################################
download_sources() {
    print_info "Downloading source code..."

    cd "$SRC_DIR" || exit 1

    # Download binutils
    if [ ! -f "binutils-$BINUTILS_VERSION.tar.gz" ]; then
        print_info "Downloading binutils $BINUTILS_VERSION..."
        wget "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"
    else
        print_info "Binutils already downloaded"
    fi

    # Download GCC
    if [ ! -f "gcc-$GCC_VERSION.tar.gz" ]; then
        print_info "Downloading GCC $GCC_VERSION..."
        wget "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"
    else
        print_info "GCC already downloaded"
    fi

    # Extract sources
    if [ ! -d "binutils-$BINUTILS_VERSION" ]; then
        print_info "Extracting binutils..."
        tar xf "binutils-$BINUTILS_VERSION.tar.gz"
    fi

    if [ ! -d "gcc-$GCC_VERSION" ]; then
        print_info "Extracting GCC..."
        tar xf "gcc-$GCC_VERSION.tar.gz"
    fi

    print_success "Sources downloaded and extracted"
}

################################################################################
#                               BUILD BINUTILS                                 #
################################################################################
build_binutils() {
    print_info "Building binutils..."

    cd "$SRC_DIR" || exit 1

    # Create and enter build directory
    rm -rf build-binutils
    mkdir build-binutils
    cd build-binutils || exit 1

    # Configure
    print_info "Configuring binutils..."
    ../binutils-$BINUTILS_VERSION/configure \
        --target="$TARGET" \
        --prefix="$PREFIX" \
        --with-sysroot \
        --disable-nls \
        --disable-werror \
        --disable-multilib

    # Build and install
    print_info "Building binutils (this may take a while)..."
    make -j$(nproc)
    make install

    # Verify installation
    if [ -f "$PREFIX/bin/$TARGET-as" ]; then
        print_success "Binutils installed successfully"
    else
        print_error "Binutils installation failed"
        exit 1
    fi
}

################################################################################
#                               BUILD GCC                                      #
################################################################################
build_gcc() {
    print_info "Building GCC..."

    cd "$SRC_DIR" || exit 1

    # Create and enter build directory
    rm -rf build-gcc
    mkdir build-gcc
    cd build-gcc || exit 1

    # Configure
    print_info "Configuring GCC..."
    ../gcc-$GCC_VERSION/configure \
        --target="$TARGET" \
        --prefix="$PREFIX" \
        --disable-nls \
        --enable-languages=c,c++ \
        --without-headers \
        --disable-multilib \
        --disable-libssp \
        --disable-libgomp \
        --disable-libmudflap

    # Build and install
    print_info "Building GCC (this will take a LONG time)..."
    make -j$(nproc) all-gcc
    make all-target-libgcc
    make install-gcc
    make install-target-libgcc

    # Verify installation
    if [ -f "$PREFIX/bin/$TARGET-gcc" ]; then
        print_success "GCC installed successfully"
    else
        print_error "GCC installation failed"
        exit 1
    fi
}

################################################################################
#                               VERIFY INSTALLATION                            #
################################################################################
verify_installation() {
    print_info "Verifying installation..."

    # Check if commands exist
    local commands=("$TARGET-gcc" "$TARGET-as" "$TARGET-ld" "$TARGET-objcopy")

    for cmd in "${commands[@]}"; do
        if [ -f "$PREFIX/bin/$cmd" ]; then
            print_success "$cmd found"

            # Test version
            if "$PREFIX/bin/$cmd" --version &>/dev/null; then
                print_info "  $($PREFIX/bin/$cmd --version | head -1)"
            fi
        else
            print_error "$cmd not found"
        fi
    done

    print_info "Toolchain installed to: $PREFIX"
    print_info "Add to current shell: source ~/.bashrc"
    print_info "Or run: export PATH=\"$PREFIX/bin:\$PATH\""
}

################################################################################
#                               MAIN SCRIPT                                    #
################################################################################
main() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  i686-elf Cross-Compiler Installer     ${NC}"
    echo -e "${BLUE}========================================${NC}"

    # Check if running as root (for dependencies)
    if [ "$EUID" -eq 0 ]; then
        print_warning "Running as root. Some steps may not work properly."
        read -p "Continue anyway? (y/n): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi

    # Parse arguments
    if [ -n "$1" ]; then
        PREFIX="$1"
        print_info "Custom installation directory: $PREFIX"
    fi

    # Run installation steps
    check_dependencies
    setup_environment
    download_sources
    build_binutils
    build_gcc
    verify_installation

    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  Installation completed successfully!  ${NC}"
    echo -e "${GREEN}========================================${NC}"
}

# Run main function
main "$@"
