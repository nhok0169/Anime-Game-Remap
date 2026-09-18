#!/bin/bash
# Run the Integration Tester on Linux / WSL, the platform its goldens are produced on.
#
#   bash integrationTest.sh runSuite
#   bash integrationTest.sh produceOutputs
#   bash integrationTest.sh runSuite ApiDocTests.test_fullFix_modFixed
#
# From Windows (Git Bash tool), run it through an LF copy -- a checkout gives this file CRLF endings:
#   MSYS_NO_PATHCONV=1 wsl -d Ubuntu-22.04 bash -c 'tr -d "\r" < "/mnt/e/.../integrationTest.sh" > /tmp/it.sh && bash /tmp/it.sh runSuite'
# or copy it into a scratch folder with LF endings first.
#
# Needs: the Linux core module built and copied into the package (Tools/Misc/Linux/linuxBuild.sh), the
# venv at AG_REMAP_WSL_VENV (default ~/agremap-venv), and directory-tree installed OUTSIDE that venv:
#   python -m pip install --target ~/itlib directory-tree==0.0.4
# A full run takes ~10-13 minutes with the checkout on /mnt/e. A one-off OSError "[Errno 22] Invalid
# argument" or a "Bus error" there has been /mnt/e I/O, not the code: re-run the affected tests once
# before investigating.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="${AG_REMAP_REPO:-$(cd "$HERE/../../.." 2>/dev/null && pwd)}"
[ -d "$REPO/Testing/Integration Tester" ] || { echo "no repo at $REPO; set AG_REMAP_REPO"; exit 1; }
source "${AG_REMAP_WSL_VENV:-$HOME/agremap-venv}/bin/activate"
export PYTHONPATH="${ITLIB:-$HOME/itlib}"
cd "$REPO/Testing/Integration Tester" || exit 1
echo "== core: $(ls -la --time-style=long-iso "$REPO/Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2/"core.cpython-*.so)"
start=$(date +%s)
python main.py "$@" > /tmp/itest_stdout.txt 2>&1
code=$?
echo "EXIT=$code  SECONDS=$(( $(date +%s) - start ))"
grep -E "^Ran|^OK|^FAILED|^(ERROR|FAIL): " integrationTestResults.txt
exit $code
