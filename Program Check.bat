@echo off
set /a count = 1
:start
v_rand.exe -t 50 3000
set /a count += 1
if %count% LEQ 20 (
	goto start
)
echo.
echo.
echo "1) 해시 데이터가 잘 출력되었는지 확인해주세요. 2번째 줄의 64자리 영문+숫자 조합의 문자열입니다."
echo "2) 해시 데이터의 정수형 출력이 잘 되었는지 확인해주세요. 3번째 줄 부터 이후 끝 줄 까지 입니다."
pause >nul
exit /b