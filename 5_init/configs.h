int parse_config_line(const char *line, entry_t *entry);

// read the whole config
void read_cfg(char *path, entries_t* parsed_entries);