BIN=../../bin

#FLAGS= -ggdb -O0 -Wall -Wno-unused-result -fopenmp -pthread -std=gnu99 -pedantic
FLAGS= -O3 -Wall -Wno-unused-result -fopenmp -pthread -std=gnu99 -pedantic
IFTLIB    = -L$(NEWIFT_DIR)/lib -lift -llapack -lblas  -lm 
IFTFLAGS  = -I$(NEWIFT_DIR)/include 

libift:
	cd ../../; $(MAKE) -C $(NEWIFT_DIR); cd -\


$@.c: libift $@.c 	
	gcc $(FLAGS) $@.c -o $(BIN)/$@ $(IFTFLAGS) $(IFTLIB) ; \

clean:
	rm $(BIN)/*;










