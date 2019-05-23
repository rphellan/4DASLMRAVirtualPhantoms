#FLAGS= -g -O0 -Wall -D _DEBUG -Wno-unused-result -fPIC -fopenmp -pthread -pedantic
FLAGS= -O3 -Wall -Wno-unused-result -fPIC -fopenmp -pthread -pedantic 

all: libsvm.a
	@echo
	@echo "libsvm.a built..."
	@echo "DONE."

libsvm.a: svm.cpp
	gcc $(FLAGS) -c svm.cpp -o libsvm.o
	ar csr libsvm.a libsvm.o

clean:
	rm -f libsvm.o; rm -f libsvm.a;