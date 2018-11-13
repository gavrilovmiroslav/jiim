
CXX=g++ -std=c++11 -g
JVM_HOME=/usr/lib/jvm/java-1.11.0-openjdk-amd64/
JVMTI_HEADER=-I$(JVM_HOME)/include/ -I$(JVM_HOME)/include/linux
EXPORT_LIB_NAME=libjiim.so

jiim: bin/jiim.o bin/jeeves.o
	g++ -shared -fPIC -o libjiim.so $^ -lc

bin/%.o: src/%.cc
	$(CXX) $(JVMTI_HEADER) -Wall -c -fPIC $< -o $@ -lc

clean:
	rm -rf $(EXPORT_LIB_NAME) *.o
