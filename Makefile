ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

NAME        = galactic_miner$(EXE_EXT)
NAME_DEBUG  = galactic_miner_debug$(EXE_EXT)

TEST_DIR    = tests

SRC_COMMON  = \
    backend_client.cpp \
    achievements.cpp \
    planets.cpp \
    fleets.cpp \
    buildings.cpp \
    buildings_catalog.cpp \
    combat.cpp \
    combat_support.cpp \
    combat_tracks.cpp \
    combat_formations.cpp \
    combat_tick.cpp \
    game_bootstrap.cpp \
    player_profile.cpp \
    font_util.cpp \
    main_menu.cpp \
    quests.cpp \
    research.cpp \
    game_research.cpp \
    game_quests.cpp \
    game_convoys.cpp \
    game_fleets.cpp \
    game_combat_interface.cpp \
    game.cpp \
    save_system.cpp \
    ui_input.cpp \
    ui_menu.cpp \
    profile_entry_flow.cpp \
    profile_management_flow.cpp \
    profile_preferences.cpp
SRC_MAIN    = main.cpp
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
                -I. -I$(LIBFT_DIR)

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

SDL_LIBS    = $(shell pkg-config --libs sdl2 SDL2_ttf 2>/dev/null)
SDL_CFLAGS  = $(shell pkg-config --cflags sdl2 SDL2_ttf 2>/dev/null)

OBJS        = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS   = $(SRC_TEST:%.cpp=$(OBJ_DIR)/%.o)

all: check_sdl dirs $(TARGET) test

dirs:
	-$(MKDIR) $(OBJ_DIR)
	-$(MKDIR) $(OBJ_DIR_DEBUG)

debug:
	$(MAKE) all DEBUG=1

$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(TARGET): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(SDL_LIBS) $(LDFLAGS)

test: $(LIBFT) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ $(LDFLAGS)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) $(notdir $(LIBFT)) COMPILE_FLAGS="$(LIBFT_COMPILE_FLAGS)"

clean:
	$(RMDIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG)

fclean: clean
	$(RM) $(NAME) $(NAME_DEBUG) test

re: fclean all


check_sdl:
	@if ! pkg-config --exists sdl2 SDL2_ttf; then \
		printf "Error: SDL2 development libraries (SDL2 and SDL2_ttf) not found.\n" >&2; \
		printf "Please install the SDL2 and SDL2_ttf development packages for your platform and ensure pkg-config can locate them.\n" >&2; \
		exit 1; \
	fi
.PHONY: all clean fclean re debug dirs test check_sdl
