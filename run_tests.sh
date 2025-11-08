#!/bin/bash
echo "======================================"
echo "BLACK HOLE SIMULATION - QUICK TEST"
echo "======================================"

PASS=0
FAIL=0

echo "[1] Checking project files..."
[ -f "black_hole.cpp" ] && [ -f "geodesic_kerr.comp" ] && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[2] Checking shaders..."
[ -f "shaders/vertex.glsl" ] && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[3] Checking documentation..."
[ -f "README.md" ] && [ -f "CONTROLS.md" ] && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[4] Checking Phase docs..."
[ -f "PHASE6_FEATURES.md" ] && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[5] Checking source structure..."
[ -d "src/utils" ] && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[6] Validating shader..."
grep -q "#version 430" geodesic_kerr.comp && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[7] Checking protections..."
grep -q "cotTheta" geodesic_kerr.comp && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[8] Verifying features..."
grep -q "wavelengthBand" geodesic_kerr.comp && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[9] CMake configuration..."
cmake -B build -S . > /dev/null 2>&1 && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo "[10] Building project..."
cmake --build build > /dev/null 2>&1 && echo "  PASS" && ((PASS++)) || echo "  FAIL" && ((FAIL++))

echo ""
echo "Tests Passed: $PASS/10"
[ $FAIL -eq 0 ] && echo "ALL TESTS PASSED!" || echo "SOME TESTS FAILED"
