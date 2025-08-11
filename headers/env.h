#pragma once

// Load a .env file once. Safe to call multiple times; it will only parse once.
// Pass path like ".env" in project root.
void dotenv_load_once(const char *path);

// Get a configuration value by key from the previously loaded .env.
// Returns NULL if not found.
const char *dotenv_get(const char *key);


