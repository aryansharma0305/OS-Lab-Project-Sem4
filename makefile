CC      = gcc
CFLAGS  = -Wall -Wextra -g -lpthread


ENGINE_DIR  = DB_Service/DB_Engine
CLIENT_DIR  = DB_Service/DB_Client
DATA_DIR    = DB_Service/Data
SERVER_DIR = Server
UTILS_DIR  = Server/Utils

HANDLERS_DIR  = Server/Handlers


all: db_engine demo server


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


server: $(SERVER_DIR)/Server.c $(UTILS_DIR)/Auth.c $(CLIENT_DIR)/DB_Client.c $(HANDLERS_DIR)/guestHandler.c $(HANDLERS_DIR)/adminHandler.c $(HANDLERS_DIR)/chefHandler.c
	@echo ">>> Building Server..."
	$(CC) $(CFLAGS) \
		$(SERVER_DIR)/Server.c \
		$(UTILS_DIR)/Auth.c \
		$(CLIENT_DIR)/DB_Client.c \
		$(HANDLERS_DIR)/guestHandler.c \
		$(HANDLERS_DIR)/adminHandler.c \
		$(HANDLERS_DIR)/chefHandler.c \
		-I$(SERVER_DIR) \
		-I$(UTILS_DIR) \
		-I$(CLIENT_DIR) \
		-I$(ENGINE_DIR) \
		-I. \
		-o server \
		$(LDFLAGS)
		
		
		
		

run: all stop
	@echo ">>> Starting DB Engine in background..."
	$(ENGINE_DIR)/db_engine & echo $$! > db_engine.pid
	@sleep 1

	@echo ">>> Starting Socket Server..."
	./server & echo $$! > server.pid
	@sleep 1

	@echo ">>> System running (DB + Server)"

stop:
	@echo ">>> Stopping DB Engine..."
	@if [ -f db_engine.pid ]; then \
		kill `cat db_engine.pid` 2>/dev/null || true; \
		rm -f db_engine.pid; \
	else \
		echo "No DB Engine PID file found"; \
	fi

	@echo ">>> Stopping Server..."
	@if [ -f server.pid ]; then \
		kill `cat server.pid` 2>/dev/null || true; \
		rm -f server.pid; \
	else \
		echo "No Server PID file found"; \
	fi

	@echo ">>> Cleaning IPC message queues..."
	@ipcs -q | awk 'NR>3 {print $$2}' | xargs -r -I {} ipcrm -q {}





run_server: all stop
	@echo ">>> Starting DB Engine in background..."
	$(ENGINE_DIR)/db_engine & echo $$! > db_engine.pid
	@sleep 1

	@echo ">>> Starting Socket Server..."
	./server & echo $$! > server.pid
	@sleep 1

	@echo ">>> System running (DB + Server)"


run_db: db_engine stop
	@echo ">>> Starting DB Engine in background..."
	$(ENGINE_DIR)/db_engine & echo $$! > db_engine.pid
	@sleep 1

	@echo ">>> DB Engine running"





reset:
	@echo ">>> Full reset (DB + IPC)..."
	rm -f $(DATA_DIR)/*.db
	rm -f $(DATA_DIR)/*.idx
	@ipcs -q | awk 'NR>3 {print $$2}' | xargs -r -I {} ipcrm -q {}



clean:
	@echo ">>> Cleaning..."
	rm -f $(ENGINE_DIR)/db_engine
	rm -f server
	rm -f demo
	rm -f $(DATA_DIR)/*.db
	rm -f $(DATA_DIR)/*.idx
	rm -f db_engine.pid server.pid


clean_bin:
	rm -f $(ENGINE_DIR)/db_engine
	rm -f demo


.PHONY: all run stop clean clean_bin reset