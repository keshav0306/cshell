

all: main.o bg.o ls.o echo.o cd.o discover.o pwd.o pinfo.o command.o token.o
		gcc  main.o bg.o ls.o echo.o cd.o discover.o pwd.o pinfo.o command.o token.o


main: main.c
		gcc  -c main.c

bg: bg.c
		gcc  -c bg.c

ls: ls.c
		gcc   -c ls.c

pwd: pwd.c
		gcc   -c pwd.c

echo: echo.c
		gcc   -c echo.c

discover: discover.c
		gcc   -c discover.c

cd: cd.c
		gcc  -c cd.c

pinfo: pinfo.c
		gcc   -c pinfo.c

command: command.c
		gcc   -c command.c

token: token.c
		gcc -c token.c
		
remove: main.o bg.o ls.o echo.o cd.o discover.o pwd.o pinfo.o
		rm main.o bg.o ls.o echo.o cd.o discover.o pwd.o pinfo.o command.o
