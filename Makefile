CC = gcc
CFLAGS = -Wall -W

OBJLIST = genutil.o wordlist.o words.o cube.o score.o timer.o input.o	\
	  output.o play.o batch.o dbuild.o boggle.o

boggle : $(OBJLIST)
	gcc $(CFLAGS) -o boggle $(OBJLIST) -lncurses

words.o    : words.c words.h boggle.h cube.h wordlist.h
cube.o     : cube.c cube.h boggle.h
score.o    : score.c score.h boggle.h words.h output.h
timer.o    : timer.c timer.h boggle.h output.h
input.o    : input.c input.h boggle.h words.h output.h timer.h
output.o   : output.c output.h boggle.h cube.h
play.o     : play.c play.h boggle.h words.h cube.h score.h timer.h	\
	     input.h output.h
batch.o    : batch.c batch.h genutil.h cube.h words.h
dbuild.o   : dbuild.c dbuild.h boggle.h
genutil.o  : genutil.h
wordlist.o : wordlist.c wordlist.h boggle.h
boggle.o   : boggle.c boggle.h dbuild.h

clean :
	rm $(OBJLIST)
