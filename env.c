#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "headers/env.h"

typedef struct EnvItem {
    char *key;
    char *value;
} EnvItem;

static EnvItem *g_items = NULL;
static size_t g_count = 0;
static int g_loaded = 0;

static char *trim(char *s)
{
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

static void add_item(const char *key, const char *value)
{
    g_items = (EnvItem *)realloc(g_items, (g_count + 1) * sizeof(EnvItem));
    g_items[g_count].key = strdup(key);
    g_items[g_count].value = strdup(value);
    g_count++;
}

void dotenv_load_once(const char *path)
{
    if(g_loaded) return;
    g_loaded = 1;

    FILE *fp = fopen(path, "rb");
    if(!fp) return;

    char line[4096];
    while(fgets(line, sizeof(line), fp))
    {
        char *p = line;
        // skip comments
        while(isspace((unsigned char)*p)) p++;
        if(*p == '#' || *p == '\0' || *p == '\n' || *p == '\r') continue;

        char *eq = strchr(p, '=');
        if(!eq) continue;
        *eq = '\0';
        char *key = trim(p);
        char *value = trim(eq + 1);

        // strip optional quotes
        size_t len = strlen(value);
        if(len >= 2 && ((value[0] == '"' && value[len-1] == '"') || (value[0] == '\'' && value[len-1] == '\'')))
        {
            value[len-1] = '\0';
            value = value + 1;
        }

        add_item(key, value);
    }
    fclose(fp);
}

const char *dotenv_get(const char *key)
{
    for(size_t i = 0; i < g_count; i++)
    {
        if(strcmp(g_items[i].key, key) == 0)
            return g_items[i].value;
    }
    return NULL;
}


