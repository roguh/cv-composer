build:
	mkdir -p build ; cd build ; cmake .. ; make
readme:
	pandoc -V margin:0.8in -o README.pdf README.markdown
dist:
	tar czf ../implementations.tar.gz ../implementations
