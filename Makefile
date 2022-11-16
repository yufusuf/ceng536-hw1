reporter:
	gcc *.h reporter.c -o build/reporter 
analyzer:
	gcc *.h analyzer.c -o build/analyzer
clean:
	rm -f build/*
all:
	gcc *.h reporter.c -o build/reporter 
	gcc *.h analyzer.c -o build/analyzer
	
