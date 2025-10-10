    size_t max_recent = 5;

    const ft_vector<ft_string> &journal_entries = this->get_journal_entries();
    size_t journal_count = journal_entries.size();
    size_t start_index = 0;
    if (journal_count > max_recent)
        start_index = journal_count - max_recent;
    for (size_t i = start_index; i < journal_count; ++i)
        out.recent_journal_entries.push_back(journal_entries[i]);

    const ft_vector<ft_string> &lore_entries = this->get_lore_log();
    size_t lore_count = lore_entries.size();
    size_t lore_start = 0;
    if (lore_count > max_recent)
        lore_start = lore_count - max_recent;
    for (size_t i = lore_start; i < lore_count; ++i)
        out.recent_lore_entries.push_back(lore_entries[i]);
