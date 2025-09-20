ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

NAME        = galactic_miner$(EXE_EXT)
NAME_DEBUG  = galactic_miner_debug$(EXE_EXT)

SRC_DIR     = src
TEST_DIR    = tests

SRC_COMMON  = \
    $(SRC_DIR)/backend_client.cpp \
    $(SRC_DIR)/achievements.cpp \
    $(SRC_DIR)/planets.cpp \
    $(SRC_DIR)/fleets.cpp \
    $(SRC_DIR)/buildings.cpp \
    $(SRC_DIR)/buildings_catalog.cpp \
    $(SRC_DIR)/combat.cpp \
    $(SRC_DIR)/combat_support.cpp \
    $(SRC_DIR)/combat_tracks.cpp \
    $(SRC_DIR)/combat_formations.cpp \
    $(SRC_DIR)/combat_tick.cpp \
    $(SRC_DIR)/quests.cpp \
    $(SRC_DIR)/research.cpp \
    $(SRC_DIR)/game_research.cpp \
    $(SRC_DIR)/game_quests.cpp \
    $(SRC_DIR)/game_convoys.cpp \
    $(SRC_DIR)/game_fleets.cpp \
    $(SRC_DIR)/game_combat_interface.cpp \
    $(SRC_DIR)/game.cpp \
    $(SRC_DIR)/save_system.cpp
SRC_MAIN    = $(SRC_DIR)/main.cpp
SRC         = $(SRC_COMMON) $(SRC_MAIN)
SRC_TEST    = $(SRC_COMMON) \
    $(TEST_DIR)/game_test_main.cpp \
    $(TEST_DIR)/game_test_achievements.cpp \
    $(TEST_DIR)/game_test_backend.cpp \
    $(TEST_DIR)/game_test_campaign.cpp \
    $(TEST_DIR)/game_test_energy.cpp \
    $(TEST_DIR)/game_test_control.cpp \
    $(TEST_DIR)/game_test_combat.cpp \
    $(TEST_DIR)/game_test_support.cpp \
    $(TEST_DIR)/game_test_research.cpp \
    $(TEST_DIR)/game_test_difficulty.cpp \
    $(TEST_DIR)/game_test_save.cpp

CC          = g++

OPT_LEVEL ?= 0

ifeq ($(OPT_LEVEL),0)
    OPT_FLAGS = -O0 -g
else ifeq ($(OPT_LEVEL),1)
    OPT_FLAGS = -O1 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),2)
    OPT_FLAGS = -O2 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),3)
    OPT_FLAGS = -O3 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else
    $(error Unsupported OPT_LEVEL=$(OPT_LEVEL))
endif

COMPILE_FLAGS = -std=c++17 -Wall -Werror -Wextra -Wmissing-declarations -Wold-style-cast \
                -Wshadow -Wconversion -Wformat=2 -Wundef -Wfloat-equal -Wodr -Wuseless-cast \
                -Wzero-as-null-pointer-constant -Wmaybe-uninitialized $(OPT_FLAGS) \
                -I$(SRC_DIR) -I$(LIBFT_DIR)

LIBFT_COMPILE_FLAGS = -Wall -Werror -Wextra -std=c++17 -Wmissing-declarations \
                -Wold-style-cast -Wshadow -Wconversion -Wformat=2 -Wundef \
                -Wfloat-equal -Wconversion -Wodr -Wuseless-cast \
                -Wzero-as-null-pointer-constant -Wmaybe-uninitialized \
                -Wno-error=float-equal $(OPT_FLAGS)

CFLAGS = $(COMPILE_FLAGS)

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RMDIR   = rmdir /S /Q
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RMDIR   = rm -rf
    RM      = rm -f
endif

LIBFT_DIR   = ./libft

OBJ_DIR         = ./objs
OBJ_DIR_DEBUG   = ./objs_debug

ENABLE_LTO  ?= 0
ENABLE_PGO  ?= 0
export ENABLE_LTO ENABLE_PGO

ifeq ($(ENABLE_LTO),1)
    COMPILE_FLAGS   += -flto
    LDFLAGS  += -flto
endif

ifeq ($(ENABLE_PGO),1)
    COMPILE_FLAGS  += -fprofile-generate
endif

ifeq ($(DEBUG),1)
    CFLAGS    += -DDEBUG=1
    OBJ_DIR    = $(OBJ_DIR_DEBUG)
    TARGET     = $(NAME_DEBUG)
    LIBFT      = $(LIBFT_DIR)/Full_Libft_debug.a
else
    TARGET     = $(NAME)
    LIBFT      = $(LIBFT_DIR)/Full_Libft.a
endif

export COMPILE_FLAGS

ifeq ($(OS),Windows_NT)
    LDFLAGS     = $(LIBFT)
else
    LDFLAGS     = $(LIBFT) -lssl -lcrypto -ldl -lpthread -lreadline
endif

SFML_LIBS   = $(shell pkg-config --libs sfml-graphics sfml-window sfml-system sfml-audio 2>/dev/null)
SFML_CFLAGS = $(shell pkg-config --cflags sfml-graphics sfml-window sfml-system sfml-audio 2>/dev/null)

OBJS        = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS   = $(SRC_TEST:%.cpp=$(OBJ_DIR)/%.o)

all: check_sfml dirs $(TARGET) test

dirs:
	-$(MKDIR) $(OBJ_DIR)
	-$(MKDIR) $(OBJ_DIR_DEBUG)

debug:
	$(MAKE) all DEBUG=1

$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(SFML_CFLAGS) -c $< -o $@

$(TARGET): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(SFML_LIBS) $(LDFLAGS)

test: $(LIBFT) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ $(LDFLAGS)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) $(notdir $(LIBFT)) COMPILE_FLAGS="$(LIBFT_COMPILE_FLAGS)"

clean:
	$(RMDIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG)

fclean: clean
	$(RM) $(NAME) $(NAME_DEBUG) test

re: fclean all

check_sfml:
	@pkg-config --exists sfml-graphics || (echo "SFML not found, installing..." && sudo apt-get update && sudo apt-get install -y libsfml-dev)

.PHONY: all clean fclean re debug dirs test check_sfml
