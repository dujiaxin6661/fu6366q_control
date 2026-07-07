@echo off
cd /d "%~dp0"
python -m pip install --user -r ".\pc_host\requirements.txt"
pause
