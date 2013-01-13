CXX = g++
CXXFLAGS = -O2 -Wall -fomit-frame-pointer -funroll-loops -I.
LDFLAGS = -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf -lGL
SOURCES = main.cpp title.cpp game.cpp editor.cpp
OBJECTS=$(SOURCES:.cpp=.o)
NAME = tnr

all: $(SOURCES) $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f *.o $(NAME)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
