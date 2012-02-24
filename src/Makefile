CC=g++ -O3
LIBS=-lsndfile -lpng -lfftw3

../bin/plot-wave: plot_wave.o
	$(CC) $(LIBS) png_tools.o pcm.o plot_wave.o $(LIBS) -o ../bin/plot-wave

plot_wave.o: plot_wave.cpp pcm.o png_tools.o
	$(CC) -c plot_wave.cpp 

pcm.o: png_tools.o pcm.cpp pcm.h
	$(CC) -c pcm.cpp

png_tools.o: png_tools.cpp png_tools.h
	$(CC) -c png_tools.cpp

clean:
	rm -vf *.o ../bin/*

