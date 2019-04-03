dec112lost: dec112lost.o lost.o sqlite.o xml.o helper.o mongoose.o
	gcc -o dec112lost dec112lost.o lost.o sqlite.o xml.o helper.o mongoose.o -lsqlite3 -lspatialite -llog4c -lxml2 -lpthread -ldl -lm

dec112lost.o: dec112lost.c dec112lost.h mongoose.h
	gcc -I/usr/include/libxml2/ -c dec112lost.c

lost.o: lost.c lost.h
	gcc -I/usr/include/libxml2/ -c lost.c

sqlite.o: sqlite.c sqlite.h
	gcc -I/usr/include/libxml2/ -c sqlite.c

xml.o: xml.c xml.h
	gcc -I/usr/include/libxml2/ -c xml.c

helper.o: helper.c helper.h
	gcc -I/usr/include/libxml2/ -c helper.c

mongoose.o: mongoose.c mongoose.h
	gcc -c mongoose.c

clean:
	rm *.o
	rm dec112lost
