start:
	g++ *.cpp -lmysqlclient -Wno-write-strings -fPIC -shared -o ../openspy/modules/legacystatsprocessor.so -I.. ../common/*.cpp ../common/*.c ../openspy/config.cpp
debug:
	g++ *.cpp -lmysqlclient -ggdb -fPIC -shared -o ../openspy/modules/legacystatsprocessor.so -I.. ../common/*.cpp ../common/*.c ../openspy/config.cpp
