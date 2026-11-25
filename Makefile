CC			= clang
NAME		= memory_inspector

VICTIM		= victim/victim

SRC			= $(shell find ./src -name "*.c")
ASM			= $(shell find ./src -name "*.s")
OBJ			= $(SRC:.c=.o) $(ASM:.s=.o)

ARCH		= -arch arm64
CFLAGS		= -Wall -Wextra -Werror -I./include $(ARCH)
ASFLAGS		= $(ARCH)

all: $(NAME)

# Cible de test : un process qu'on possede, signe avec get-task-allow
# pour que task_for_pid reussisse sans root et SIP active.
victim: $(VICTIM)

$(INTERPRETER): interpreter/interpreter.c
	$(CC) -Wall $(ARCH) $< -o $@

$(VICTIM): victim/victim.c
	$(CC) -Wall $(ARCH) $< -o $@
	codesign -s - -f --entitlements victim/ent.plist $@

$(NAME): $(OBJ)
	$(CC) $(ARCH) $(OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(ASFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME) $(VICTIM)

re: fclean all

.PHONY: all victim clean fclean re
