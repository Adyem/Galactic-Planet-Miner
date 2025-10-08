#include "build_info.hpp"

#ifndef GALACTIC_BUILD_VERSION
# define GALACTIC_BUILD_VERSION "dev"
#endif

#ifndef GALACTIC_BUILD_BRANCH
# define GALACTIC_BUILD_BRANCH "local"
#endif

ft_string build_info_get_version() noexcept
{
    return ft_string(GALACTIC_BUILD_VERSION);
}

ft_string build_info_get_branch() noexcept
{
    return ft_string(GALACTIC_BUILD_BRANCH);
}

ft_string build_info_format_label() noexcept
{
    ft_string version = build_info_get_version();
    ft_string branch = build_info_get_branch();
    ft_string label("Version: ");

    if (!version.empty())
        label.append(version);
    else
        label.append("(unknown)");

    if (!branch.empty())
    {
        label.append(" (");
        label.append(branch);
        label.append(")");
    }

    return label;
}
