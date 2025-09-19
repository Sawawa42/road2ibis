NAME = tinyPaint

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++11

SRCS = main.cpp tinyPaint.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) -lglfw -lGL

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
