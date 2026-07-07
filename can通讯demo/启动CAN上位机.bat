@echo off
cd /d "%~dp0pc_host"
python can_motor_gui.py
if errorlevel 1 pause
