@ECHO OFF
IF  "(env) "  NEQ  "%PROMPT:~0,6%"  ECHO Please run setup.bat first && EXIT /b

python hexs2pngs.py none.log
python hexs2pngs.py rock.log
python hexs2pngs.py paper.log
python hexs2pngs.py scissors.log

