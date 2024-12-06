CC = cc
CFLAGS = -Wall -Werror -Wextra -g
RFLAGS = -lreadline

LIBFT_REPO = git@github.com:Zico-Moras/42-libft.git
LIBFT_DIR = 42-libft
LIBFT = $(LIBFT_DIR)/libft.a


NAME = minishell

SRC = src/main.c src/parse.c

OBJ = $(SRC:.c=.o)

all : $(NAME) 

$(LIBFT): 
	@if [ ! -d "$(LIBFT_DIR)" ]; then \
	echo "Cloning libft repository..."; \
	git clone $(LIBFT_REPO) $(LIBFT_DIR); \
	else \
	echo "libft already exists"; \
	fi
	@echo "Building libft"
	$(MAKE) -C $(LIBFT_DIR) all


$(NAME): $(OBJ) $(LIBFT) 
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBFT) $(RFLAGS)
 


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)
	make -C $(LIBFT_DIR) clean

libftdel :
	rm -rf $(LIBFT_DIR)
