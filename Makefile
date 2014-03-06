start:
	cd openspy && make && cd ..
#	cd qr && make && cd ..
	cd serverbrowsing && make && cd ..
	cd legacyms && make && cd ..
	cd peerchat && make && cd ..
	cd keyserver && make && cd ..
	cd natneg && make && cd ..
	cd playerspy && make && cd ..
	cd playersearch && make && cd ..
	cd legacystats && make && cd ..
	cd legacystatsprocessor && make && cd ..
debug:
	cd openspy && make debug && cd ..
#	cd qr && make debug && cd ..
	cd serverbrowsing && make debug && cd ..
	cd legacyms && make debug && cd ..
	cd peerchat && make debug && cd ..
	cd keyserver && make debug && cd ..
	cd natneg && make debug && cd ..
	cd playerspy && make debug && cd ..
	cd playersearch && make debug && cd ..
	cd legacystats && make debug && cd ..
	cd legacystatsprocessor && make debug && cd ..
clean:
	rm  openspy/modules/*.so
	rm openspy/openspy
