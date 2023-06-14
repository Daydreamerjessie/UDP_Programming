all: servermain serverA serverB

servermain: servermain.cpp
	g++ -std=c++11 servermain.cpp -g -o servermain

serverA: serverA.cpp
	g++ -std=c++11 serverA.cpp -g -o serverA
serverB: serverB.cpp
	g++ -std=c++11 serverB.cpp -g -o serverB

clean:
	rm serverA serverB servermain
