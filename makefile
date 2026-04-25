# ============================================================
# Compiler settings
# ============================================================
CC      = gcc
CFLAGS  = -Wall -Wextra -g -lpthread


# ============================================================
# Paths
# ============================================================
ENGINE_DIR  = DB_Service/DB_Engine
CLIENT_DIR  = DB_Service/DB_Client
DATA_DIR    = DB_Service/Data


# ============================================================
# Targets
# ============================================================

# Default — build everything
all: db_engine demo


# --- DB Engine ---
db_engine: $(ENGINE_DIR)/DB_engine.c $(ENGINE_DIR)/DB_File_IO_Handler.c
	@echo ">>> Building DB Engine..."
	@mkdir -p $(DATA_DIR)
	$(CC) $(CFLAGS) \
		$(ENGINE_DIR)/DB_engine.c \
		$(ENGINE_DIR)/DB_File_IO_Handler.c \
		-I$(ENGINE_DIR) \
		-o $(ENGINE_DIR)/db_engine


# --- Demo ---
demo: demo.c $(CLIENT_DIR)/DB_Client.c
	@echo ">>> Building demo..."
	$(CC) $(CFLAGS) \
		demo.c \
		$(CLIENT_DIR)/DB_Client.c \
		-I$(CLIENT_DIR) \
		-I$(ENGINE_DIR) \
		-I. \
		-o demo


# ============================================================
# RUN
# ============================================================
run: all stop
	@echo ">>> Starting DB Engine in background..."
	$(ENGINE_DIR)/db_engine & echo $$! > db_engine.pid
	@sleep 1
	@echo ">>> Running demo..."
	./demo
	@echo ">>> Done."


# ============================================================
# STOP (SAFE)
# ============================================================
stop:
	@echo ">>> Stopping DB Engine..."
	@if [ -f db_engine.pid ]; then \
		kill `cat db_engine.pid` 2>/dev/null || true; \
		rm -f db_engine.pid; \
	else \
		echo "No PID file found"; \
	fi
	@echo ">>> Cleaning IPC message queues..."
	@ipcs -q | awk 'NR>3 {print $$2}' | xargs -r -I {} ipcrm -q {}


# ============================================================
# FULL RESET (DB + IPC)
# ============================================================
reset:
	@echo ">>> Full reset (DB + IPC)..."
	rm -f $(DATA_DIR)/*.db
	rm -f $(DATA_DIR)/*.idx
	@ipcs -q | awk 'NR>3 {print $$2}' | xargs -r -I {} ipcrm -q {}


# ============================================================
# CLEAN
# ============================================================
clean:
	@echo ">>> Cleaning..."
	rm -f $(ENGINE_DIR)/db_engine
	rm -f demo
	rm -f $(DATA_DIR)/*.db
	rm -f $(DATA_DIR)/*.idx
	rm -f db_engine.pid


clean_bin:
	rm -f $(ENGINE_DIR)/db_engine
	rm -f demo


.PHONY: all run stop clean clean_bin reset