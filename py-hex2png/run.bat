@ECHO OFF
IF  "(env) "  NEQ  "%PROMPT:~0,6%"  ECHO Please run setup.bat first && EXIT /b

python hex2png.py img.hex

