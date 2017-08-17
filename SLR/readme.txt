Augmented rule로 S>E가 추가되므로

반드시!! 입력의 Starting symbol은 E가 되어야 합니다.

또한 rule.txt에서 S로 시작되는 symbol이 없어야합니다. ex) S>aBA 를 E>aBA로 바꿔야 

정상적으로 작동합니다. 

실행 방법은 window cygwin 에서 gcc 컴파일, 
혹은 linux, unix, mac ox의 terminal 에서 gcc 컴파일로 가능합니다.

명령어 예시)

gcc [directory]slr.c

a.exe 파일 생성되면

./a.exe rule.txt 

로 실행할 수 있습니다.


프로그램 실행시

명령어는 FIRST, FOLLOW , GOTO, ACTION, TABLE 모두 대문자만 인식가능합니다.
exit은 소문자만 인식가능합니다.

그외의 입력은 모두 input string으로 간주합니다.