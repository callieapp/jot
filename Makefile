# Everyday commands. The CMake presets do the real work; this file gives them
# short names. Run `make` to list them.

# --- App-specific: everything above the next marker differs between apps. ---

APP := jot
GUI_TARGET := jot-gui
CLI_TARGET := jot
LOG_CATEGORIES := jot.*

FEDORA_DEPS := gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtdeclarative-devel \
	qt6-qtquickcontrols2-devel tomlplusplus-devel clang-tools-extra nodejs npm
DEBIAN_DEPS := build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev \
	qt6-declarative-dev-tools qml6-module-qtquick-controls qml6-module-qtquick-templates \
	qml6-module-qtquick-effects libtomlplusplus-dev clang-format nodejs npm

# --- Shared by every app in the suite below this line. ---

PRESET ?= dev
BUILD := build/$(PRESET)
GUI := $(BUILD)/src/gui/$(GUI_TARGET)
CLI := $(BUILD)/src/cli/$(CLI_TARGET)

# Fedora's Qt sends logs to the journal when stderr is not a terminal, and its
# qtlogging.ini turns debug output off. This puts everything back on stderr,
# timestamped, with the app's own categories fully enabled.
DEV_ENV := QT_FORCE_STDERR_LOGGING=1 \
	QT_LOGGING_RULES="qml.debug=true;$(LOG_CATEGORIES)=true" \
	QT_MESSAGE_PATTERN="%{time hh:mm:ss.zzz} %{if-category}%{category} %{endif}%{type}: %{message}"

.DEFAULT_GOAL := help
.PHONY: help deps setup configure build run cli test lint format check clean

help: ## List commands
	@awk 'BEGIN {FS = ":.*## "} /^[a-z-]+:.*## / {printf "  make %-14s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

deps: ## Install build and development dependencies (uses sudo)
	@if command -v dnf >/dev/null; then sudo dnf install -y $(FEDORA_DEPS); \
	elif command -v apt-get >/dev/null; then sudo apt-get update && sudo apt-get install -y $(DEBIAN_DEPS); \
	else echo "make deps: unsupported distro, see README.md for packages" >&2; exit 1; fi

setup: ## Install dev tooling, enable git hooks, and configure
	@npm ci
	@git config core.hooksPath .githooks
	@cmake --preset $(PRESET)
	@touch $(PRESET_STAMP)

# CMake re-runs itself with the cached settings, so a changed preset only reaches
# an existing build directory through this stamp.
PRESET_STAMP := $(BUILD)/.preset-applied

$(PRESET_STAMP): CMakePresets.json
	@cmake --preset $(PRESET)
	@touch $@

configure: ## Re-run CMake
	@cmake --preset $(PRESET)
	@touch $(PRESET_STAMP)

build: $(PRESET_STAMP) ## Build everything
	@cmake --build --preset $(PRESET)

run: build ## Run the app with logging on; ARGS="..." passes options
	@$(DEV_ENV) $(GUI) $(ARGS)

cli: build ## Run the CLI, e.g. make cli ARGS="--help"
	@$(DEV_ENV) $(CLI) $(ARGS)

test: build ## Run the tests
	@ctest --preset $(PRESET)

lint: build ## Lint QML and Markdown
	@cmake --build --preset $(PRESET) --target all_qmllint
	@npx markdownlint-cli2

format: ## Format C++, QML, Markdown, JSON and YAML in place
	@scripts/format.sh fix
	@npm run --silent format:md

check: ## Run everything CI checks: format, lint, a -Werror build, and tests
	@scripts/format.sh check
	@npx markdownlint-cli2
	@npm run --silent format:md:check
	@cmake --workflow --preset ci

clean: ## Remove build output
	@rm -rf build
