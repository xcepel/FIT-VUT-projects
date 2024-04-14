# Makefile
# Reseni ITY - 2. projekt
# Autor: Katerina Cepelkova, xcepel03, VUT FIT

EXECUTABLE = proj2

pdf:
	latex ${EXECUTABLE}.tex
	latex ${EXECUTABLE}.tex
	dvips -t a4 ${EXECUTABLE}.dvi
	ps2pdf ${EXECUTABLE}.ps

clean:
	rm -f ${EXECUTABLE}.dvi ${EXECUTABLE}.ps ${EXECUTABLE}.pdf ${EXECUTABLE}.aux ${EXECUTABLE}.log
