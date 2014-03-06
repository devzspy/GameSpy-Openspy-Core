start:
	g++ *.cpp -lmysqlclient -lGeoIP -Wno-write-strings -fPIC -shared -o ../openspy/modules/qr.so -I.. ../common/*.cpp ../common/*.c
debug:
	g++ *.cpp -lmysqlclient -lGeoIP -Wno-write-strings -ggdb -fPIC -shared -o ../openspy/modules/qr.so -I.. ../common/*.cpp ../common/*.c
