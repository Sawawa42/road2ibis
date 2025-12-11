NAME = tinyPaint
CXX = c++
CXXFLAGS = -Isrc -Iexternal/lodepng
LIBS = -lglfw -lGLESv2
SRCS = src/main.cpp \
	src/Core/App.cpp \
	src/Core/InputManager.cpp \
	src/Core/Window.cpp \
	src/History/HistoryStorage.cpp \
	src/History/HistoryWorker.cpp \
	src/History/HistoryManager.cpp \
	src/Graphics/FrameBuffer.cpp \
	src/Graphics/Mesh.cpp \
	src/Graphics/Shader.cpp \
	src/Tools/Brush.cpp \
	src/LayerTexture.cpp \
	src/TileSystem.cpp \
	src/Canvas.cpp \
	src/Renderer.cpp \
	external/lodepng/lodepng.cpp
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
