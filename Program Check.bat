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
echo "1) �ؽ� �����Ͱ� �� ��µǾ����� Ȯ�����ּ���. 2��° ���� 64�ڸ� ����+���� ������ ���ڿ��Դϴ�."
echo "2) �ؽ� �������� ������ ����� �� �Ǿ����� Ȯ�����ּ���. 3��° �� ���� ���� �� �� ���� �Դϴ�."
pause >nul
exit /b