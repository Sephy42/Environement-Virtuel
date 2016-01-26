# INCLUDE = -I/usr/include -I/usr/local/glut/include
# LIBDIR  = -L/usr/lib -L/usr/local/glut/glut/lib
#INCLUDE = -I/usr/local/VRML/JAVA/glut/include
#LIBDIR  = -L/usr/local/VRML/JAVA/glut/lib/glut.n32/

COMPILERFLAGS = -g -L/usr/X11R6/lib
CCOMPILER = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lglut -lGLU -lGL -lX11 -lpng

all:  receiver sender server Tank

Tank : Tank.o
	$(CCOMPILER) $(CFLAGS) -o Tank $(LIBDIR) Tank.o $(LIBRARIES) 
    
Tank.o : Tank.cpp
	$(CCOMPILER) $(CFLAGS) -c Tank.cpp

receiver : receiver.o
	$(CCOMPILER) $(CFLAGS) -o receiver $(LIBDIR) receiver.o $(LIBRARIES) 

receiver.o : receiver.cpp
	$(CCOMPILER) $(CFLAGS) -c receiver.cpp

sender : sender.o
	$(CCOMPILER) $(CFLAGS) -o sender $(LIBDIR) sender.o $(LIBRARIES) 

sender.o : sender.cpp
	$(CCOMPILER) $(CFLAGS) -c sender.cpp

server : server.o
	$(CCOMPILER) $(CFLAGS) -o server $(LIBDIR) server.o $(LIBRARIES)

server.o : server.cpp server.h
	$(CCOMPILER) $(CFLAGS) -c server.cpp

clean:
	rm Tank sender receiver server *.o
