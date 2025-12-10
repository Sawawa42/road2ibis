NAME = tinyPaint
CXX = c++
CXXFLAGS = -Isrc -Iexternal/lodepng
LIBS = -lglfw -lGLESv2
SRCS = src/main.cpp src/Shader.cpp src/Mesh.cpp src/FrameBuffer.cpp src/Brush.cpp src/App.cpp src/UndoSystem.cpp external/lodepng/lodepng.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LIBS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME) output.png history.bin

re: fclean all

.PHONY: all clean fclean re
