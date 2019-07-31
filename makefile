test:*.cpp *.hpp 
	g++ $^ -std=c++11 -o $@
.PHONY:clean
clean:
	rm test
