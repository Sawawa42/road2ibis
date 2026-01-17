NAME = tinyPaint
CXX = c++
CXXFLAGS = 
LIBS = -lglfw -lGLEW -lGL
SRCS = src/main.cpp src/Shader.cpp src/Mesh.cpp src/FrameBuffer.cpp src/Brush.cpp src/App.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LIBS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
