all: task-ok task-err-p task-err-a

task-ok: executive.o task-ok.o busy_wait.o
	gcc -Wall -o task-ok executive.o busy_wait.o task-ok.o -lpthread -lrt
task-err-p: executive.o task-err-p.o busy_wait.o
	gcc -Wall -o task-err-p executive.o busy_wait.o task-err-p.o -lpthread -lrt

task-err-a: executive.o task-err-a.o busy_wait.o
	gcc -Wall -o task-err-a executive.o busy_wait.o task-err-a.o -lpthread -lrt

executive.o: executive.c task.h
	gcc -Wall -c executive.c -DMULTIPROC

task.o: ${SCHEDULE}.c task.h busy_wait.h
	gcc -Wall -c task.c

busy_wait.o: busy_wait.c busy_wait.h 
	gcc -Wall -c busy_wait.c

clean:
	rm -f *.o *~ task-ok task-err-p task-err-a

