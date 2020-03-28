#include <wordexp.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "errors.h"
#include "init2.h"
#include "utils.h"

int parse_config_line(const char *line, entry_t *entry) {
    wordexp_t result;
    int wres;
    if(line[0] == '#') {
        return -1;
    }
    wres = wordexp(line, &result, 0);
    if(wres != 0) {
        char *errmsg;
        switch(wres) {
            case WRDE_BADCHAR:
                errmsg = ERR_WRDE_BADCHAR;
                break;
            case WRDE_SYNTAX:
                errmsg = ERR_WRDE_SYNTAX;
                break;
            case WRDE_NOSPACE:
                errmsg = ERR_WRDE_NOSPACE;
                break;
        }
        err(errmsg, line, false);
        return -1;
    }
    if(result.we_wordv[0] == NULL) {
        return -1;
    }
    entry->argc = result.we_wordc-1;
    if(entry->argc < 1) {
        err(ERRLNFMT, line, true);
        return -1;
    }
    entry->fails = 0;
    entry->action = strdup(result.we_wordv[entry->argc]);
    result.we_wordv[entry->argc] = (char*)0;
    entry->argv = result.we_wordv;
    entry->exec_name = strdup(entry->argv[0]);
    entry->argv[0] = get_exec_from_abspath(entry->argv[0]);
    entry->full_cmd = join_str(entry->argv, " ", entry->argc);
    entry->finished = false;
    if(strcmp(entry->action, "wait") != 0 && 
       strcmp(entry->action, "respawn") != 0) {
        err(ERRACT, entry->action, false);
        return -1;
    }
    return 0;
}

void read_cfg(char *cfg_path, entries_t* parsed_entries) {
    char *cfg_raw, *tok_ptr;
    int cfg_lines_count = 0;
    long fsize;
    FILE *f;

    syslog(LOG_INFO, "Parsing config");
    f = fopen(cfg_path, "r");
    if(f == NULL) {
        err(NULL, cfg_path, true);
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    cfg_raw = malloc(fsize+1);
    if(cfg_raw == NULL) {
        err(NULL, NULL, true);
    }
    fread(cfg_raw, 1, fsize, f);
    fclose(f);
    cfg_raw[fsize] = '\0';

    for(int i = 0; cfg_raw[i]; i++) {
        cfg_lines_count += (cfg_raw[i] == *(char*)CFG_DELIM);
    }
    DBG("newlines found: %d\n", cfg_lines_count+1);

    parsed_entries->ev = calloc(cfg_lines_count, sizeof(entry_t));
    if(parsed_entries->ev == NULL) {
        err(NULL, NULL, true);
    }

    tok_ptr = strtok(cfg_raw, CFG_DELIM);
    if(tok_ptr == NULL) {
        err(ERRFMT, NULL, true);
    }
    while(tok_ptr != NULL) {
        DBG("Raw entry #%d: %s\n", parsed_entries->ec, tok_ptr);
        if(strlen(tok_ptr) > 0) {
            entry_t entry;
            if(!parse_config_line(tok_ptr, &entry)) {
                unsigned *e_cnt = &parsed_entries->ec;
                parsed_entries->ev[*e_cnt] = entry;
                syslog(LOG_INFO, "Parsed entry: %s", tok_ptr);
                *e_cnt += 1;
            }
        }
        tok_ptr = strtok(NULL, CFG_DELIM);
    }
    free(cfg_raw);
}
