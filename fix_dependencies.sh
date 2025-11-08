#!/bin/bash
# =========================================
# 依赖修复脚本 - Fix Missing Dependencies
# =========================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo ""
echo "=========================================="
echo "  黑洞模拟 - 依赖检查和修复工具"
echo "=========================================="
echo ""

# 1. 检查 third_party 目录
echo -e "${BLUE}[1/4]${NC} 检查 third_party 目录..."

if [ ! -d "third_party" ]; then
    echo -e "${RED}[ERROR]${NC} third_party 目录不存在！"
    echo "这个仓库可能损坏，建议重新克隆。"
    exit 1
fi

# 2. 检查 ImGui
echo -e "${BLUE}[2/4]${NC} 检查 ImGui..."
IMGUI_MISSING=0

REQUIRED_IMGUI_FILES=(
    "third_party/imgui/imgui.cpp"
    "third_party/imgui/imgui.h"
    "third_party/imgui/backends/imgui_impl_glfw.cpp"
    "third_party/imgui/backends/imgui_impl_opengl3.cpp"
)

for file in "${REQUIRED_IMGUI_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}[MISSING]${NC} $file"
        IMGUI_MISSING=1
    fi
done

if [ $IMGUI_MISSING -eq 0 ]; then
    echo -e "${GREEN}[OK]${NC} ImGui 文件完整"
else
    echo -e "${RED}[ERROR]${NC} ImGui 文件缺失"
fi

# 3. 检查 ImPlot
echo -e "${BLUE}[3/4]${NC} 检查 ImPlot..."
IMPLOT_MISSING=0

REQUIRED_IMPLOT_FILES=(
    "third_party/implot/implot.cpp"
    "third_party/implot/implot.h"
    "third_party/implot/implot_items.cpp"
)

for file in "${REQUIRED_IMPLOT_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}[MISSING]${NC} $file"
        IMPLOT_MISSING=1
    else
        echo -e "${GREEN}[OK]${NC} $file"
    fi
done

if [ $IMPLOT_MISSING -eq 0 ]; then
    echo -e "${GREEN}[OK]${NC} ImPlot 文件完整"
else
    echo -e "${RED}[ERROR]${NC} ImPlot 文件缺失"
fi

# 4. 总结和建议
echo ""
echo -e "${BLUE}[4/4]${NC} 诊断总结"
echo "=========================================="

if [ $IMGUI_MISSING -eq 1 ] || [ $IMPLOT_MISSING -eq 1 ]; then
    echo -e "${RED}检测到缺失文件！${NC}"
    echo ""
    echo "可能的原因："
    echo "  1. 你的本地仓库版本太旧"
    echo "  2. Git 克隆时出现问题"
    echo "  3. 文件被意外删除"
    echo ""
    echo "推荐解决方案："
    echo ""
    echo -e "${GREEN}方案1: 拉取最新代码（推荐）${NC}"
    echo "  git fetch --all"
    echo "  git checkout claude/hdr-rendering-pipeline-011CUpV1fbrgVioJP7ySj6g6"
    echo "  git pull origin claude/hdr-rendering-pipeline-011CUpV1fbrgVioJP7ySj6g6"
    echo ""
    echo -e "${GREEN}方案2: 手动下载依赖${NC}"
    echo "  # 下载 ImPlot (如果只缺这个)"
    echo "  cd third_party/implot"
    echo "  git clone https://github.com/epezent/implot.git ."
    echo ""
    echo -e "${GREEN}方案3: 重新克隆仓库${NC}"
    echo "  cd .."
    echo "  git clone [仓库URL] black_hole_new"
    echo "  cd black_hole_new"
    echo "  git checkout claude/hdr-rendering-pipeline-011CUpV1fbrgVioJP7ySj6g6"
    echo ""
    exit 1
else
    echo -e "${GREEN}✓ 所有依赖文件完整！${NC}"
    echo ""
    echo "你可以继续构建："
    echo "  ./build_macos.sh"
    echo ""
    exit 0
fi
