NAME = tinyPaint
CXX = c++
CXXFLAGS = -std=c++11
LIBS = -lglfw -lGLEW -lGL
SRCS = main.cpp Shader.cpp Mesh.cpp FrameBuffer.cpp
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
