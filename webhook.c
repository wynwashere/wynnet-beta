#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "headers/webhook.h"
#include "headers/env.h"

static void escape_json(const char *input, char *output, size_t output_size)
{
    size_t j = 0;
    for(size_t i = 0; input[i] && j + 2 < output_size; i++)
    {
        unsigned char c = (unsigned char)input[i];
        if(c == '"' || c == '\\') { if(j + 2 >= output_size) break; output[j++]='\\'; output[j++]=c; }
        else if(c == '\n') { if(j + 2 >= output_size) break; output[j++]='\\'; output[j++]='n'; }
        else if(c == '\r') { if(j + 2 >= output_size) break; output[j++]='\\'; output[j++]='r'; }
        else if(c < 0x20) { if(j + 6 >= output_size) break; j+=snprintf(output+j, output_size-j, "\\u%04x", c); }
        else output[j++] = c;
    }
    output[j] = '\0';
}

void send_discord_notification(const char *title, const char *description, int color_rgb_int, const char *attachment_path)
{
    const char *webhook_url = getenv("DISCORD_WEBHOOK_URL");
    if(webhook_url == NULL || webhook_url[0] == '\0')
    {
        dotenv_load_once(".env");
        webhook_url = dotenv_get("DISCORD_WEBHOOK_URL");
    }
    if(webhook_url == NULL || webhook_url[0] == '\0') return;

    char safe_title[512];
    char safe_desc[4096];
    escape_json(title ? title : "", safe_title, sizeof(safe_title));
    escape_json(description ? description : "", safe_desc, sizeof(safe_desc));

    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S.000Z", tm);

    char json_payload[8192];
    snprintf(
        json_payload, sizeof(json_payload),
        "{\n"
        "  \"embeds\": [\n"
        "    {\n"
        "      \"title\": \"%s\",\n"
        "      \"description\": \"%s\",\n"
        "      \"color\": %d,\n"
        "      \"timestamp\": \"%s\"\n"
        "    }\n"
        "  ]\n"
        "}",
        safe_title, safe_desc, color_rgb_int, timestamp
    );

    // Build command to call curl. Use -sS for silent plus errors.
    // If attachment_path provided, use multipart form; otherwise send JSON directly.
    char *cmd = NULL;
    if(attachment_path && attachment_path[0] != '\0')
    {
        // Discord requires payload_json for multipart with file
        // We will write payload to a temp file to avoid shell escaping issues
        char tmp_payload[FILENAME_MAX];
        snprintf(tmp_payload, sizeof(tmp_payload), "discord_payload_%ld.json", (long)now);
        FILE *fp = fopen(tmp_payload, "wb");
        if(fp){ fwrite(json_payload, 1, strlen(json_payload), fp); fclose(fp); }

        // Use name "file1" for attachment
        // Note: on Windows Git Bash, quotes are okay. Avoid newlines.
        int n = asprintf(&cmd,
            "curl -sS -X POST -F payload_json=@%s -F file1=@%s '%s'",
            tmp_payload, attachment_path, webhook_url);
        if(n < 0) return;
        system(cmd);
        // Clean temp
        remove(tmp_payload);
    }
    else
    {
        int n = asprintf(&cmd,
            "curl -sS -H Content-Type:application/json -X POST -d '%s' '%s'",
            json_payload, webhook_url);
        if(n < 0) return;
        system(cmd);
    }

    if(cmd) free(cmd);
}


