@ECHO OFF
SET DIR=C:\Users\maarten\AppData\Local\Programs\Python\Python39\
%DIR%python.exe -m venv env
CALL env\Scripts\activate.bat
python  -m pip  install  --upgrade pip setuptools wheel
IF EXIST requirements.txt (
   pip install -r requirements.txt
)
