# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/20 11:10:34 by ichpakov          #+#    #+#              #
#    Updated: 2025/07/28 16:18:40 by njeanbou         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: all clean fclean re

NAME = webserv

#//////////////////////////////////////////////////////////////////////////////
#		ALL FILES
#//////////////////////////////////////////////////////////////////////////////

SRC_DIR = src
OBJ_DIR = obj
FILE_DIR = file
UPLOADS_DIR = uploads

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))


#//////////////////////////////////////////////////////////////////////////////
#		COMMAND SHORTCUTS
#//////////////////////////////////////////////////////////////////////////////

CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -g -fsanitize=address -static-libasan
RM = rm -rf

#//////////////////////////////////////////////////////////////////////////////
#		RULES
#//////////////////////////////////////////////////////////////////////////////

all: ${NAME}

# Binary creation

$(NAME): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

# Mandatory rules

clean:
	$(RM) $(OBJ_DIR) 

fclean: clean
	$(RM) $(NAME) $(FILE_DIR)/* $(UPLOADS_DIR)/*

re: fclean all