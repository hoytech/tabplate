testtool: *.h tool/main.cpp
	g++ -fsanitize=address -std=c++17 -DTESTTOOL tool/main.cpp -o testtool -g

.PHONY: verify upload

verify: *.h *.ino
	arduino --verify tabplate.ino

upload: *.h *.ino
	arduino --upload tabplate.ino
