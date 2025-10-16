CC = cc
CFLAGS = -Wall -Werror -Wextra -g -I./include
RFLAGS = -lreadline
LIBFT_REPO = git@github.com:Zico-Moras/42-libft.git
LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libft.a
NAME = minishell
TEMP_PATH = .temp

SRC = \
	./main.c \
	./signals.c \
	./init.c \
	./utils.c \
	./free.c \
	./input.c \
	./lexer.c \
	./lexer_utils.c \
	./lexer_utils2.c \
	./lexer_token_utils.c \
	./parser.c \
	./parser_utils.c \
	./parser_utils2.c \
	./parse_node_utils.c \
	./parser_syntax.c \
#	./expander.c \
#	./expander_utils.c \

OBJ = $(SRC:.c=.o)

# Lexer test configuration
TEST_LEXER_SRC = ./test_lexer_main.c
TEST_LEXER_OBJ = $(TEST_LEXER_SRC:.c=.o)
TEST_LEXER_NAME = test_lexer

# Parser test configuration
TEST_PARSER_SRC = ./test_parser_main.c
TEST_PARSER_OBJ = $(TEST_PARSER_SRC:.c=.o)
TEST_PARSER_NAME = test_parser

##@ Main Targets

all: $(NAME)					## Build minishell

libft: $(LIBFT)

$(LIBFT):
	@if [ ! -d "$(LIBFT_DIR)" ]; then \
		echo "Cloning libft repository..."; \
		git clone $(LIBFT_REPO) $(LIBFT_DIR); \
	fi
	@echo "Building libft..."
	@$(MAKE) -s -C $(LIBFT_DIR) all

$(NAME): $(LIBFT) $(OBJ)
	@echo "Compiling $(NAME)..."
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBFT) $(RFLAGS)

%.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning object files..."
	@rm -rf $(OBJ)
	@$(MAKE) -s -C $(LIBFT_DIR) clean || true

fclean: clean
	@echo "Performing full cleanup..."
	@$(MAKE) -s fclean -C $(LIBFT_DIR) || true
	@rm -f $(OBJ) $(NAME)
	@rm -rf $(TEMP_PATH)
	@rm -f sup gdb.txt

cleanlibft:
	@echo "Removing libft directory..."
	@rm -rf $(LIBFT_DIR)

re: fclean all

##@ Test Targets - Lexer

test_lexer: libft $(TEST_LEXER_OBJ) $(filter-out ./main.o,$(OBJ))		## Build and run lexer tests
	@echo "Compiling lexer test binary..."
	@$(CC) $(CFLAGS) $(TEST_LEXER_OBJ) $(filter-out ./main.o,$(OBJ)) -o $(TEST_LEXER_NAME) $(LIBFT) $(RFLAGS)

$(TEST_LEXER_OBJ): $(TEST_LEXER_SRC)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

test_lexer_clean:				## Clean lexer test files
	@rm -f $(TEST_LEXER_OBJ) $(TEST_LEXER_NAME)

test_lexer_re: test_lexer_clean test_lexer	## Rebuild lexer tests

##@ Test Targets - Parser

test_parser: libft $(TEST_PARSER_OBJ) $(filter-out ./main.o,$(OBJ))	## Build and run parser tests
	@echo "Compiling parser test binary..."
	@$(CC) $(CFLAGS) $(TEST_PARSER_OBJ) $(filter-out ./main.o,$(OBJ)) -o $(TEST_PARSER_NAME) $(LIBFT) $(RFLAGS)

$(TEST_PARSER_OBJ): $(TEST_PARSER_SRC)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

test_parser_clean:				## Clean parser test files
	@rm -f $(TEST_PARSER_OBJ) $(TEST_PARSER_NAME)

test_parser_re: test_parser_clean test_parser	## Rebuild parser tests

##@ Test Targets - All Tests

test_all: test_lexer test_parser		## Build and run all tests

test_clean: test_lexer_clean test_parser_clean	## Clean all test files

test_re: test_clean test_all			## Rebuild all tests

##@ Debug Rules

# Arguments for debugging (override with: make valgrind ARG="your args")
ARG =

$(TEMP_PATH):
	@mkdir -p $(TEMP_PATH)

# Valgrind suppressions for readline leaks
define SUP_BODY
{
	name
	Memcheck:Leak
	fun:*alloc
	...
	obj:*/libreadline.so.*
	...
}
{
	leak readline
	Memcheck:Leak
	...
	fun:readline
}
{
	leak add_history
	Memcheck:Leak
	...
	fun:add_history
}
endef

supfile:
	$(file > sup,$(SUP_BODY))

gdb: all $(NAME)					## Debug with gdb
	gdb --tui ./$(NAME)

get_log:
	@touch gdb.txt
	@if command -v lnav >/dev/null 2>&1; then \
		lnav gdb.txt; \
	else \
		tail -f gdb.txt; \
	fi

vgdb_cmd: $(NAME) $(TEMP_PATH)
	@printf "target remote | vgdb --pid=" > $(TEMP_PATH)/gdb_commands.txt
	@printf "$(shell pgrep -f valgrind)" >> $(TEMP_PATH)/gdb_commands.txt
	@printf "\n" >> $(TEMP_PATH)/gdb_commands.txt
	@if [ -f .vgdbinit ]; then \
		cat .vgdbinit >> $(TEMP_PATH)/gdb_commands.txt; \
	fi

valgrind: all supfile $(NAME)				## Debug with valgrind (memcheck)
	@echo "Running valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=sup --tool=memcheck -s ./$(NAME) $(ARG)

vgdb: all supfile $(NAME) $(TEMP_PATH)			## Debug with valgrind & gdb (advanced)
	@if ! command -v tmux >/dev/null 2>&1; then \
		echo "Error: tmux is required for vgdb target"; \
		exit 1; \
	fi
	tmux split-window -h "valgrind --vgdb-error=0 --log-file=gdb.txt ./$(NAME) $(ARG)"
	@sleep 1
	@$(MAKE) vgdb_cmd
	tmux split-window -v "gdb --tui -x $(TEMP_PATH)/gdb_commands.txt $(NAME)"
	tmux resize-pane -U 18
	@$(MAKE) get_log

helgrind: all $(NAME) $(TEMP_PATH)			## Debug threads with helgrind
	@echo "Running helgrind..."
	@if command -v tmux >/dev/null 2>&1; then \
		tmux set-option remain-on-exit on; \
		tmux split-window -h "valgrind --log-file=gdb.txt --tool=helgrind -s ./$(NAME) $(ARG)"; \
		tmux resize-pane -R 55; \
		$(MAKE) get_log; \
	else \
		valgrind --log-file=gdb.txt --tool=helgrind -s ./$(NAME) $(ARG); \
	fi

vgdb_helgrind: all $(NAME) $(TEMP_PATH)			## Debug threads with helgrind & gdb
	@if ! command -v tmux >/dev/null 2>&1; then \
		echo "Error: tmux is required for vgdb_helgrind target"; \
		exit 1; \
	fi
	tmux split-window -h "valgrind --vgdb-error=0 --log-file=gdb.txt --tool=helgrind ./$(NAME) $(ARG)"
	@sleep 1
	@$(MAKE) vgdb_cmd
	tmux split-window -v "gdb --tui -x $(TEMP_PATH)/gdb_commands.txt $(NAME)"
	tmux resize-pane -U 18
	@$(MAKE) get_log

help:						## Show this help message
	@echo "Usage: make [target] [ARG=\"arguments\"]"
	@echo ""
	@echo "Targets:"
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z_0-9-]+:.*##/ { printf "  %-20s %s\n", $$1, $$2 }' $(MAKEFILE_LIST)

.PHONY: all libft clean fclean cleanlibft re gdb valgrind vgdb helgrind vgdb_helgrind supfile get_log vgdb_cmd help \
	test_lexer test_lexer_clean test_lexer_re \
	test_parser test_parser_clean test_parser_re \
	test_all test_clean test_re
