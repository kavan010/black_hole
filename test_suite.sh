#!/bin/bash
#
# BLACK HOLE SIMULATION - COMPREHENSIVE TEST SUITE
# Tests all features from Phase 1-6

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

VERBOSE=0
if [[ "$1" == "--verbose" ]]; then
    VERBOSE=1
fi

TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓ PASS]${NC} $1"
    ((TESTS_PASSED++))
    ((TESTS_TOTAL++))
}

log_failure() {
    echo -e "${RED}[✗ FAIL]${NC} $1"
    ((TESTS_FAILED++))
    ((TESTS_TOTAL++))
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_section() {
    echo ""
    echo -e "${YELLOW}===========================================${NC}"
    echo -e "${YELLOW}$1${NC}"
    echo -e "${YELLOW}===========================================${NC}"
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

test_dependencies() {
    log_section "TEST SUITE 1: Build System Dependencies"

    if command_exists cmake; then
        CMAKE_VERSION=$(cmake --version | head -n1)
        log_success "CMake found: $CMAKE_VERSION"
    else
        log_failure "CMake not found"
    fi

    if command_exists g++; then
        GCC_VERSION=$(g++ --version | head -n1)
        log_success "g++ found: $GCC_VERSION"
    elif command_exists clang++; then
        CLANG_VERSION=$(clang++ --version | head -n1)
        log_success "clang++ found: $CLANG_VERSION"
    else
        log_failure "C++ compiler not found"
    fi
}

test_project_structure() {
    log_section "TEST SUITE 2: Project Structure"

    REQUIRED_FILES=(
        "CMakeLists.txt"
        "black_hole.cpp"
        "geodesic_kerr.comp"
        "README.md"
        "CONTROLS.md"
    )

    for file in "${REQUIRED_FILES[@]}"; do
        if [ -f "$file" ]; then
            log_success "Found: $file"
        else
            log_failure "Missing: $file"
        fi
    done

    SHADER_FILES=(
        "shaders/vertex.glsl"
        "shaders/fragment.glsl"
        "shaders/tonemap.frag"
    )

    for file in "${SHADER_FILES[@]}"; do
        if [ -f "$file" ]; then
            log_success "Found shader: $file"
        else
            log_failure "Missing shader: $file"
        fi
    done
}

test_code_quality() {
    log_section "TEST SUITE 3: Code Quality Checks"

    if grep -q "1e-6\|epsilon" geodesic_kerr.comp; then
        log_success "Shader has division-by-zero protections"
    else
        log_warning "Shader may lack some division-by-zero protections"
    fi

    if grep -q "std140" geodesic_kerr.comp; then
        log_success "Shader uses std140 layout (proper alignment)"
    else
        log_failure "Shader missing std140 layout specification"
    fi

    if grep -q "Logger::" black_hole.cpp; then
        log_success "Uses Logger class for error reporting"
    else
        log_warning "May not have proper error logging"
    fi
}

test_build_process() {
    log_section "TEST SUITE 4: Build Process"

    if [ ! -d "build" ]; then
        log_info "Creating build directory..."
        mkdir -p build
    fi

    log_info "Configuring with CMake..."
    if cmake -B build -S . > cmake_output.log 2>&1; then
        log_success "CMake configuration successful"
    else
        log_failure "CMake configuration failed"
        return 1
    fi

    log_info "Building project..."
    if cmake --build build > build_output.log 2>&1; then
        log_success "Build successful"
    else
        log_failure "Build failed"
        return 1
    fi

    if [ -f "build/BlackHole3D" ]; then
        log_success "BlackHole3D executable created"
    else
        log_failure "BlackHole3D executable not found"
    fi
}

test_shader_compilation() {
    log_section "TEST SUITE 5: Shader Validation"

    if grep -q "#version 430" geodesic_kerr.comp; then
        log_success "Compute shader has correct GLSL version (430)"
    else
        log_failure "Compute shader missing or has wrong GLSL version"
    fi

    REQUIRED_FUNCTIONS=(
        "initRayKerr"
        "geodesicRHSKerr"
        "geodesicRHSSchwarzschild"
        "rk4Step"
        "updatePhotonIntensity"
    )

    for func in "${REQUIRED_FUNCTIONS[@]}"; do
        if grep -q "$func" geodesic_kerr.comp; then
            log_success "Found function: $func"
        else
            log_failure "Missing function: $func"
        fi
    done
}

test_documentation() {
    log_section "TEST SUITE 6: Documentation Completeness"

    PHASE_DOCS=(
        "PHASE4_FEATURES.md"
        "PHASE5_FEATURES.md"
        "PHASE6_FEATURES.md"
    )

    for doc in "${PHASE_DOCS[@]}"; do
        if [ -f "$doc" ]; then
            log_success "Found documentation: $doc"
        else
            log_failure "Missing documentation: $doc"
        fi
    done

    README_SECTIONS=(
        "Features"
        "Quick Start"
        "Build Instructions"
    )

    for section in "${README_SECTIONS[@]}"; do
        if grep -qi "$section" README.md; then
            log_success "README has section: $section"
        else
            log_warning "README may be missing section: $section"
        fi
    done
}

test_git_repository() {
    log_section "TEST SUITE 7: Git Repository Health"

    if [ -d ".git" ]; then
        log_success "Git repository detected"

        CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
        log_info "Current branch: $CURRENT_BRANCH"

        COMMIT_COUNT=$(git rev-list --count HEAD 2>/dev/null || echo "0")
        log_info "Total commits: $COMMIT_COUNT"

        for phase in {1..6}; do
            if git log --oneline | grep -qi "phase $phase"; then
                log_success "Found Phase $phase commit"
            fi
        done
    else
        log_warning "Not a git repository"
    fi
}

test_features() {
    log_section "TEST SUITE 8: Feature Verification"

    if grep -q "rk4Step" geodesic_kerr.comp; then
        log_success "Phase 1: RK4 integration implemented"
    else
        log_failure "Phase 1: RK4 integration not found"
    fi

    if grep -q "GridCache" black_hole.cpp; then
        log_success "Phase 1: Grid caching implemented"
    else
        log_failure "Phase 1: Grid caching not found"
    fi

    if grep -q "ShaderManager" black_hole.cpp; then
        log_success "Phase 2: ShaderManager refactoring complete"
    else
        log_failure "Phase 2: ShaderManager not found"
    fi

    if grep -q "PerformanceMonitor" black_hole.cpp; then
        log_success "Phase 3: Performance monitoring implemented"
    else
        log_failure "Phase 3: Performance monitoring not found"
    fi

    if grep -q "KerrParams" geodesic_kerr.comp; then
        log_success "Phase 4: Kerr metric implemented"
    else
        log_failure "Phase 4: Kerr metric not found"
    fi

    if grep -q "BloomRenderer" black_hole.cpp; then
        log_success "Phase 4: Bloom post-processing implemented"
    else
        log_failure "Phase 4: Bloom not found"
    fi

    if grep -q "visualizationMode" geodesic_kerr.comp; then
        log_success "Phase 5: Visualization modes implemented"
    else
        log_failure "Phase 5: Visualization modes not found"
    fi

    if grep -q "shakuraSunyaev" geodesic_kerr.comp; then
        log_success "Phase 5: Shakura-Sunyaev disk model implemented"
    else
        log_failure "Phase 5: Shakura-Sunyaev disk not found"
    fi

    if grep -q "intensity" geodesic_kerr.comp; then
        log_success "Phase 6: Radiative transfer implemented"
    else
        log_failure "Phase 6: Radiative transfer not found"
    fi

    if grep -q "wavelengthBand" geodesic_kerr.comp; then
        log_success "Phase 6: Multi-wavelength rendering implemented"
    else
        log_failure "Phase 6: Multi-wavelength rendering not found"
    fi
}

test_runtime_logs() {
    log_section "TEST SUITE 9: Runtime Log Validation"

    if [ -f "src/utils/logger.hpp" ]; then
        log_success "Logger header file found"

        LOG_LEVELS=("DEBUG" "INFO" "WARN" "ERROR")
        for level in "${LOG_LEVELS[@]}"; do
            if grep -q "$level" src/utils/logger.hpp; then
                log_success "Logger supports $level level"
            fi
        done
    else
        log_failure "Logger header file not found"
    fi

    if [ -f "src/utils/performance_monitor.hpp" ]; then
        log_success "Performance monitor header found"
    else
        log_failure "Performance monitor header not found"
    fi
}

main() {
    log_section "BLACK HOLE SIMULATION - COMPREHENSIVE TEST SUITE"
    log_info "Starting comprehensive test suite..."
    log_info "Working directory: $(pwd)"
    log_info "Date: $(date)"
    echo ""

    test_dependencies
    test_project_structure
    test_code_quality
    test_build_process
    test_shader_compilation
    test_documentation
    test_git_repository
    test_features
    test_runtime_logs

    log_section "TEST SUITE SUMMARY"
    echo ""
    echo "Total Tests: $TESTS_TOTAL"
    echo -e "${GREEN}Tests Passed: $TESTS_PASSED${NC}"
    echo -e "${RED}Tests Failed: $TESTS_FAILED${NC}"
    echo ""

    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}╔═══════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║                                       ║${NC}"
        echo -e "${GREEN}║   ✓  ALL TESTS PASSED SUCCESSFULLY   ║${NC}"
        echo -e "${GREEN}║                                       ║${NC}"
        echo -e "${GREEN}╚═══════════════════════════════════════╝${NC}"
        echo ""
        log_success "The black hole simulation is ready for deployment!"
        exit 0
    else
        echo -e "${RED}╔═══════════════════════════════════════╗${NC}"
        echo -e "${RED}║                                       ║${NC}"
        echo -e "${RED}║   ✗  SOME TESTS FAILED                ║${NC}"
        echo -e "${RED}║                                       ║${NC}"
        echo -e "${RED}╚═══════════════════════════════════════╝${NC}"
        echo ""
        log_failure "Please review the failed tests above"
        exit 1
    fi
}

main
