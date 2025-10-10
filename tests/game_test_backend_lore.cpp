int verify_resource_lore_rotation()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT(log_after_first.size() == lore_start + 1);
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Old Miner Joe") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RESOURCE_IRON_FOUNDATION));
    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT(journal_after_first.size() >= 1);
    FT_ASSERT(ft_strstr(journal_after_first[journal_after_first.size() - 1].c_str(),
        "Iron Lifeblood") != ft_nullptr);
    size_t journal_count = journal_after_first.size();

    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT(log_after_second.size() == lore_start + 2);
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Quartermaster Nia") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.get_journal_entries().size() == journal_count);

    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_third = game.get_lore_log();
    FT_ASSERT(log_after_third.size() == lore_start + 3);
    const ft_string &third_entry = log_after_third[lore_start + 2];
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Old Miner Joe") != ft_nullptr);
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.get_journal_entries().size() == journal_count);

    return 1;
}

int verify_nanomaterial_resource_lore()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    size_t journal_start = game.get_journal_entries().size();

    FT_ASSERT(game.append_resource_lore_snippet(ORE_NANOMATERIAL, PLANET_TERRA, PLANET_NOCTARIS_PRIME));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT_EQ(lore_start + 1, log_after_first.size());
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "nanomaterial") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Noctaris Prime") != ft_nullptr);

    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT_EQ(journal_start + 1, journal_after_first.size());
    const ft_string &journal_entry = journal_after_first[journal_after_first.size() - 1];
    FT_ASSERT(ft_strstr(journal_entry.c_str(), "Nanomaterial Renewal") != ft_nullptr);

    FT_ASSERT(game.append_resource_lore_snippet(ORE_NANOMATERIAL, PLANET_MARS, PLANET_LUNA));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT_EQ(lore_start + 2, log_after_second.size());
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "nanomaterial") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Mars -> Luna") != ft_nullptr);

    FT_ASSERT_EQ(journal_after_first.size(), game.get_journal_entries().size());

    return 1;
}

int verify_raider_lore_rotation()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    FT_ASSERT(game.append_raider_lore_snippet(PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT(log_after_first.size() == lore_start + 1);
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Old Scout Finn") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RAIDER_SIGNAL_WEB));
    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT(journal_after_first.size() >= 1);
    FT_ASSERT(ft_strstr(journal_after_first[journal_after_first.size() - 1].c_str(),
        "Raider Signal Web") != ft_nullptr);
    size_t journal_count = journal_after_first.size();

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_MARS, PLANET_ZALTHOR));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT(log_after_second.size() == lore_start + 2);
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Captain Blackthorne") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Mars -> Zalthor") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_ZALTHOR, PLANET_VULCAN));
    const ft_vector<ft_string> &log_after_third = game.get_lore_log();
    FT_ASSERT(log_after_third.size() == lore_start + 3);
    const ft_string &third_entry = log_after_third[lore_start + 2];
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Professor Lumen") != ft_nullptr);
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Zalthor -> Vulcan") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_VULCAN, PLANET_NOCTARIS_PRIME));
    const ft_vector<ft_string> &log_after_fourth = game.get_lore_log();
    FT_ASSERT(log_after_fourth.size() == lore_start + 4);
    const ft_string &fourth_entry = log_after_fourth[lore_start + 3];
    FT_ASSERT(ft_strstr(fourth_entry.c_str(), "Navigator Zara") != ft_nullptr);
    FT_ASSERT(ft_strstr(fourth_entry.c_str(), "Vulcan -> Noctaris Prime") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_NOCTARIS_PRIME, PLANET_TERRA));
    const ft_vector<ft_string> &log_after_fifth = game.get_lore_log();
    FT_ASSERT(log_after_fifth.size() == lore_start + 5);
    const ft_string &fifth_entry = log_after_fifth[lore_start + 4];
    FT_ASSERT(ft_strstr(fifth_entry.c_str(), "Old Scout Finn") != ft_nullptr);
    FT_ASSERT(ft_strstr(fifth_entry.c_str(), "Noctaris Prime -> Terra") != ft_nullptr);

    const ft_vector<ft_string> &journal_after = game.get_journal_entries();
    FT_ASSERT(journal_after.size() == journal_count);

    return 1;
}

