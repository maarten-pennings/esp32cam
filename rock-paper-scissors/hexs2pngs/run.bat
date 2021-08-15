@ECHO OFF
IF  "(env) "  NEQ  "%PROMPT:~0,6%"  ECHO Please run setup.bat first && EXIT /b

python hexs2pngs.py ../logs/none.log
python hexs2pngs.py ../logs/rock.log
python hexs2pngs.py ../logs/paper.log
python hexs2pngs.py ../logs/scissors.log

