ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

MAKEFLAGS   += --no-print-directory

STYLE_MAGENTA := \033[1;35m
STYLE_RESET   := \033[0m

NAME        = galactic_miner$(EXE_EXT)
NAME_DEBUG  = galactic_miner_debug$(EXE_EXT)

TEST_DIR    = tests

SRC_COMMON  = \
    build_info.cpp \
    backend_client.cpp \
    achievements.cpp \
    achievements_definitions.cpp \
    planets.cpp \
    fleets.cpp \
    buildings.cpp \
    buildings_catalog.cpp \
    combat.cpp \
    combat_encounter_control.cpp \
    combat_encounter_launch.cpp \
    combat_encounter_state.cpp \
    combat_support_allies.cpp \
    combat_support_power.cpp \
    combat_support_raiders.cpp \
    combat_tracks.cpp \
    combat_formations.cpp \
    combat_tick.cpp \
    game_bootstrap.cpp \
    game_bootstrap_serialization.cpp \
    player_profile.cpp \
    font_util.cpp \
    menu_localization.cpp \
    main_menu.cpp \
    main_menu_autosave.cpp \
    main_menu_achievements.cpp \
    main_menu_connectivity.cpp \
    main_menu_sync.cpp \
    main_menu_crash.cpp \
    main_menu_content.cpp \
    main_menu_guides.cpp \
    main_menu_performance.cpp \
    main_menu_portraits.cpp \
    main_menu_palette.cpp \
    main_menu_render_navigation.cpp \
    main_menu_render_overlay.cpp \
    main_menu_render_tutorial.cpp \
    main_menu_render_achievements.cpp \
    main_menu_render_autosave.cpp \
    main_menu_render.cpp \
    menu_audio.cpp \
    quests.cpp \
    research.cpp \
    game_research.cpp \
    game_backend_status.cpp \
    game_quests_accessors.cpp \
    game_quests_choices.cpp \
    game_quests_completion.cpp \
    game_quests_context.cpp \
    game_quests_failure.cpp \
    game_quests_internal.cpp \
    game_quests_lore.cpp \
    game_quests_snapshot.cpp \
    game_buildings.cpp \
    game_convoys.cpp \
    game_fleets.cpp \
    game_combat_interface.cpp \
    game.cpp \
    save_system.cpp \
    save_system_background.cpp \
    ui_input.cpp \
    ui_menu.cpp \
    ui_menu_interaction.cpp \
    ui_menu_layout.cpp \
    new_game_flow_helpers.cpp \
    new_game_flow.cpp \
    load_game_flow.cpp \
    settings_flow.cpp \
    profile_entry_flow_helpers.cpp \
    profile_entry_flow.cpp \
    profile_management_flow.cpp \
    profile_preferences.cpp \
    string_table.cpp
SRC_MAIN    = main.cpp
SRC         = $(SRC_COMMON) $(SRC_MAIN)
TOTAL_SRC   := $(words $(SRC))
SRC_TEST    = $(SRC_COMMON) \
    $(TEST_DIR)/game_test_main.cpp \
    $(TEST_DIR)/game_test_menu_basics.cpp \
    $(TEST_DIR)/game_test_menu_preferences.cpp \
    $(TEST_DIR)/game_test_menu_connectivity.cpp \
    $(TEST_DIR)/game_test_menu_saves.cpp \
    $(TEST_DIR)/game_test_menu_assets.cpp \
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
SRC_TEST_ONLY = $(filter-out $(SRC), $(SRC_TEST))
TOTAL_TEST_SRC := $(words $(SRC_TEST_ONLY))

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

DEBUG ?= 0

BUILD_VERSION ?= dev
BUILD_BRANCH ?= local

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
    CFLAGS    += -DDEBUG=0
    TARGET     = $(NAME)
    LIBFT      = $(LIBFT_DIR)/Full_Libft.a
endif

CFLAGS += -DGALACTIC_BUILD_VERSION=\"$(BUILD_VERSION)\" -DGALACTIC_BUILD_BRANCH=\"$(BUILD_BRANCH)\"

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

all: build test

initialize:
	@printf '$(STYLE_MAGENTA)[GALACTIC SETUP] Initializing libft submodule$(STYLE_RESET)\n'
	@git submodule update --init --recursive libft

check_libft_initialized:
	@status=$$(git submodule status --recursive libft 2>/dev/null || echo "__missing__"); \
	if [ "$$status" = "__missing__" ] || \
	   printf "%s" "$$status" | grep -Eq '^[+-U]' || \
	   [ ! -f "$(LIBFT_DIR)/Makefile" ]; then \
		printf "Error: libft submodule is not initialized. Run 'make initialize' first.\n" >&2; \
		exit 1; \
	fi

build: check_sdl check_libft_initialized dirs $(TARGET)

dirs:
	@-$(MKDIR) $(OBJ_DIR)
	@-$(MKDIR) $(OBJ_DIR_DEBUG)

debug:
	@$(MAKE) build DEBUG=1

$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@progress_index_main=$$(printf '%s\n' "$(SRC)" | tr ' ' '\n' | nl -ba | awk -v target="$<" '$$2==target {print $$1}'); \
	progress_index_test=$$(printf '%s\n' "$(SRC_TEST_ONLY)" | tr ' ' '\n' | nl -ba | awk -v target="$<" '$$2==target {print $$1}'); \
	if [ -n "$$progress_index_main" ]; then \
		printf '$(STYLE_MAGENTA)[GALACTIC BUILD] (%d/%d) Compiling %s$(STYLE_RESET)\n' "$$progress_index_main" "$(TOTAL_SRC)" "$<"; \
	elif [ -n "$$progress_index_test" ]; then \
		printf '$(STYLE_MAGENTA)[GALACTIC BUILD] (%d/%d) Compiling %s$(STYLE_RESET)\n' "$$progress_index_test" "$(TOTAL_TEST_SRC)" "$<"; \
	else \
		printf '$(STYLE_MAGENTA)[GALACTIC BUILD] Compiling %s$(STYLE_RESET)\n' "$<"; \
	fi
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(TARGET): $(LIBFT) $(OBJS)
	@printf '$(STYLE_MAGENTA)[GALACTIC BUILD] Linking %s$(STYLE_RESET)\n' $@
	@$(CC) $(CFLAGS) $(OBJS) -o $@ $(SDL_LIBS) $(LDFLAGS)
	@printf '$(STYLE_MAGENTA)[GALACTIC BUILD] Output -> %s$(STYLE_RESET)\n' $@

test: check_libft_initialized $(LIBFT) $(TEST_OBJS)
	@printf '$(STYLE_MAGENTA)[GALACTIC BUILD] Linking %s$(STYLE_RESET)\n' $@
	@$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ $(SDL_LIBS) $(LDFLAGS)
	@printf '$(STYLE_MAGENTA)[GALACTIC BUILD] Output -> %s$(STYLE_RESET)\n' $@

$(LIBFT): check_libft_initialized
	@need_build=0; \
        if $(MAKE) -C $(LIBFT_DIR) -q $(notdir $(LIBFT)) COMPILE_FLAGS="$(LIBFT_COMPILE_FLAGS)"; then \
                :; \
        else \
                status=$$?; \
                if [ $$status -eq 1 ]; then \
                        need_build=1; \
                else \
                        exit $$status; \
                fi; \
        fi; \
        if [ $$need_build -eq 1 ] || [ ! -f "$@" ]; then \
                printf '$(STYLE_MAGENTA)[GALACTIC LIBFT] Updating %s$(STYLE_RESET)\n' '$(notdir $(LIBFT))'; \
                $(MAKE) -C $(LIBFT_DIR) $(notdir $(LIBFT)) COMPILE_FLAGS="$(LIBFT_COMPILE_FLAGS)"; \
        else \
                printf '$(STYLE_MAGENTA)[GALACTIC LIBFT] %s is up to date$(STYLE_RESET)\n' '$(notdir $(LIBFT))'; \
        fi

clean:
	@printf '$(STYLE_MAGENTA)[GALACTIC CLEAN] Removing build directories$(STYLE_RESET)\n'
	@$(RMDIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG)

fclean: clean
	@printf '$(STYLE_MAGENTA)[GALACTIC CLEAN] Removing binaries$(STYLE_RESET)\n'
	@$(RM) $(NAME) $(NAME_DEBUG) test

re: fclean all


check_sdl:
	@if pkg-config --exists sdl2 SDL2_ttf; then \
                printf '$(STYLE_MAGENTA)[GALACTIC CHECK] SDL2 development libraries detected$(STYLE_RESET)\n'; \
        else \
                printf '$(STYLE_MAGENTA)[GALACTIC CHECK] Warning: SDL2 development libraries (SDL2 and SDL2_ttf) not found$(STYLE_RESET)\n' >&2; \
                printf '$(STYLE_MAGENTA)[GALACTIC CHECK] Continuing without SDL2 UI support$(STYLE_RESET)\n' >&2; \
        fi
.PHONY: all build clean fclean re debug dirs test check_sdl initialize check_libft_initialized
