cocos0 : cocos0.c winsuport.o winsuport.h
	gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses


cocos1 : cocos1.c winsuport.o winsuport.h
	gcc -Wall cocos1.c winsuport.o -o cocos1 -lcurses -lpthread


cocos2 : cocos2.c winsuport.o winsuport.h
	gcc cocos2.c winsuport.o -o cocos2 -lcurses -lpthread
	
cocos3 : cocos3.c memoria.o memoria.h winsuport2.o winsuport2.h
	gcc -Wall cocos3.c memoria.o winsuport2.o -o cocos3 -lcurses -lpthread
	
cocos4 : cocos4.c memoria.o memoria.h winsuport2.o winsuport2.h missatge.o missatge.h semafor.o semafor.h
	gcc -Wall cocos4.c memoria.o winsuport2.o missatge.o semafor.o -o cocos4 -lcurses -lpthread
	
fantasma3 : fantasma3.c memoria.o memoria.h winsuport2.o winsuport2.h 
	gcc -Wall fantasma3.c memoria.o winsuport2.o -o fantasma3 -lcurses

fantasma4 : fantasma4.c memoria.o memoria.h winsuport2.o winsuport2.h missatge.o missatge.h semafor.o semafor.h
	gcc -Wall fantasma4.c memoria.o winsuport2.o missatge.o semafor.o  -o fantasma4 -lcurses

winsuport.o : winsuport.c winsuport.h
	gcc -c winsuport.c -o winsuport.o 
	
winsuport2.o : winsuport2.c winsuport2.h
	gcc -c winsuport2.c -o  winsuport2.o 


memoria.o : memoria.c memoria.h
	gcc -c memoria.c -o memoria.o
	
missatge.o : missatge.c missatge.h
	gcc -c missatge.c -o missatge.o
	
semafor.o : semafor.c semafor.h
	gcc -c semafor.c -o semafor.o

clean:
	rm -f cocos0 cocos1 cocos2 winsuport.o
