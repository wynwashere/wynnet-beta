#pragma once

// Send a Discord webhook notification with an embed and optional attachment file.
// Reads webhook URL from environment variable: DISCORD_WEBHOOK_URL
// If attachment_path is NULL or empty, no file is attached.
void send_discord_notification(const char *title, const char *description, int color_rgb_int, const char *attachment_path);


