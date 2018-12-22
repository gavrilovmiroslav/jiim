
CXX=g++ -std=c++11 -g
BOOST=-I/usr/local/include/boost -lboost_system -lboost_thread
JVM_HOME=/usr/lib/jvm/java-11-oracle
JVMTI_HEADER=-I$(JVM_HOME)/include/ -I$(JVM_HOME)/include/linux
EXPORT_LIB_NAME=libjiim.so

jiim: bin/jiim.o bin/jeeves.o
	$(CXX) -shared -fPIC -o libjiim.so $^ -lc

sjiim: bin/sjiim.o
	$(CXX) -o sjiim $^ -lc -lpthread

bin/%.o: src/%.cc
	$(CXX) $(JVMTI_HEADER) $(BOOST) -Wall -c -fPIC $< -o $@ -lc

clean:
	rm -rf $(EXPORT_LIB_NAME) bin/*.o
