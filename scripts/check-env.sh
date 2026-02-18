#!/bin/bash

# Script to check the environment for SimulIDE on Linux
# Usage: ./scripts/check-env.sh

echo "=== LINUX SYSTEM CHECK START ==="

check_cmd() {
    local name=$1
    local cmd=$2
    local required=$3
    
    if eval "$cmd" > /dev/null 2>&1; then
        echo -e "[OK] $name"
        eval "$cmd" | head -n 1
        return 0
    else
        if [ "$required" = "true" ]; then
            echo -e "[FAIL] $name not found"
            return 1
        else
            echo -e "[WARN] $name not found (Optional)"
            return 0
        fi
    fi
}

check_cmd "OS version" "uname -a" "true"
check_cmd "Git" "git --version" "true"
check_cmd "Compiler (g++)" "g++ --version" "true"
check_cmd "Qt (qmake)" "qmake6 --version || qmake --version" "true"
check_cmd "Make" "make --version" "true"
check_cmd "CMake" "cmake --version" "false"

# Check for libelf
if ldconfig -p | grep libelf > /dev/null 2>&1; then
    echo "[OK] libelf is installed"
else
    echo "[FAIL] libelf not found. Run scripts/setup-linux.sh"
fi

echo "=== SYSTEM CHECK DONE ==="
