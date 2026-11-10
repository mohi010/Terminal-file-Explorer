// ============================================================================
// File: src/cli/commands.cpp
// Purpose: Implements CLI commands, REPL loop, and table formatting.
// OS Concept: Shell architecture, command dispatch, and formatted standard output.
// ============================================================================

#include "commands.hpp"
#include "terminal.hpp"
#include "../core/fs_ops.hpp"
#include "../core/search.hpp"
#include "../core/preview.hpp"
#include "../core/format.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

#if defined(_WIN32)
void printNativeDirectoryEntries(const std::wstring& directoryPath);
#else
void printNativeDirectoryEntries(const std::string& directoryPath);
#endif

namespace fx::cli {

std::vector<std::string> tokenize(const std::string& line) {
    std::string cleanLine = line;
    // Strip UTF-8 Byte Order Mark (BOM) if present at start of stream
    if (cleanLine.size() >= 3 &&
        static_cast<unsigned char>(cleanLine[0]) == 0xEF &&
        static_cast<unsigned char>(cleanLine[1]) == 0xBB &&
        static_cast<unsigned char>(cleanLine[2]) == 0xBF) {
        cleanLine = cleanLine.substr(3);
    }
    while (!cleanLine.empty() && (cleanLine.back() == '\r' || cleanLine.back() == '\n')) {
        cleanLine.pop_back();
    }

    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < cleanLine.length(); ++i) {
        char ch = cleanLine[i];

        if (inQuotes) {
            if (ch == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
            } else {
                current += ch;
            }
        } else {
            if (ch == '"' || ch == '\'') {
                inQuotes = true;
                quoteChar = ch;
            } else if (std::isspace(static_cast<unsigned char>(ch))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += ch;
            }
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

void handleHelp() {
    std::cout << cyan("====================================================================\n")
              << boldBlue("                  OS File Explorer - Command Guide                  \n")
              << cyan("====================================================================\n")
              << "  help                      : Show this command list\n"
              << "  pwd                       : Print current working directory\n"
              << "  ls [path] [-l] [-a]       : List entries (flags: -a show hidden, -l details,\n"
              << "    [--sort name|size|date|type] [--desc])\n"
              << "  cd <path>                 : Change directory (supports ~, .., relative/abs)\n"
              << "  up                        : Move to parent directory\n"
              << "  back                      : Navigate backward in history\n"
              << "  forward                   : Navigate forward in history\n"
              << "  tree [depth]              : ASCII directory tree (default depth 2, max 5)\n"
              << "  info <path>               : Detailed metadata (size, time, permissions, inode)\n"
              << "  view <file>               : Read-only text preview (binary files rejected)\n"
              << "  mkdir <name>              : Create a new directory\n"
              << "  touch <name>              : Create a new empty regular file\n"
              << "  rename <path> <newname>   : Rename file or directory\n"
              << "  cp <src> <destdir>        : Copy file or directory tree\n"
              << "  mv <src> <destdir>        : Move file or directory\n"
              << "  rm <path> [-r]            : Remove file or directory (-r for recursive)\n"
              << "  find <pattern> [--depth n]: Search files recursively by pattern\n"
              << "  du <path>                 : Calculate total directory size\n"
              << "  space                     : Display drive capacity and free storage\n"
              << "  osdemo                    : Run OS system call explainability demo\n"
              << "  clear                     : Clear the terminal screen\n"
              << "  exit / quit               : Exit the file explorer\n"
              << cyan("====================================================================\n");
}

void handlePwd(CommandContext& ctx) {
    std::cout << ctx.navigator().current().u8string() << std::endl;
}

void handleLs(const std::vector<std::string>& tokens, CommandContext& ctx) {
    std::filesystem::path target = ctx.navigator().current();
    ListOptions options;

    for (size_t i = 1; i < tokens.size(); ++i) {
        const auto& arg = tokens[i];
        if (arg == "-a" || arg == "--all") {
            options.showHidden = true;
        } else if (arg == "-l") {
            // Detailed mode is default in table view
        } else if (arg == "--desc") {
            options.ascending = false;
        } else if (arg == "--sort" && i + 1 < tokens.size()) {
            std::string sortVal = tokens[++i];
            if (sortVal == "size") options.sortBy = SortBy::Size;
            else if (sortVal == "date") options.sortBy = SortBy::Date;
            else if (sortVal == "type") options.sortBy = SortBy::Type;
            else options.sortBy = SortBy::Name;
        } else if (arg.front() != '-') {
            target = Navigator::resolvePath(arg, ctx.navigator().current());
        }
    }

    auto listRes = listDirectory(target, options);
    if (!listRes.ok) {
        std::cout << red(listRes.error) << std::endl;
        return;
    }

    // Print aligned table header
    std::cout << std::left << std::setw(8) << "Type"
              << std::setw(32) << "Name"
              << std::setw(12) << "Size"
              << std::setw(18) << "Modified"
              << std::setw(12) << "Permissions" << "\n";
    std::cout << std::string(82, '-') << "\n";

    for (const auto& item : listRes.value) {
        std::string typeStr;
        if (item.type == EntryType::Directory) typeStr = "<DIR>";
        else if (item.type == EntryType::Symlink) typeStr = "<LINK>";
        else if (item.type == EntryType::File) typeStr = "<FILE>";
        else typeStr = "<OTHER>";

        std::string printName = item.name;
        if (printName.length() > 30) {
            printName = printName.substr(0, 27) + "...";
        }
        if (item.type == EntryType::Directory) {
            printName = boldBlue(printName);
        }

        std::cout << std::left << std::setw(8) << typeStr
                  << std::setw(32) << printName
                  << std::setw(12) << formatSize(item.size)
                  << std::setw(18) << formatTime(item.modified)
                  << std::setw(12) << formatPermissions(item.permissions) << "\n";
    }
}

void handleCd(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        // cd with no args navigates to home directory
        auto res = ctx.navigator().cd("~");
        if (!res.ok) std::cout << red(res.error) << std::endl;
        return;
    }

    auto res = ctx.navigator().cd(tokens[1]);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    }
}

void handleUp(CommandContext& ctx) {
    auto res = ctx.navigator().up();
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    }
}

void handleBack(CommandContext& ctx) {
    auto res = ctx.navigator().back();
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    }
}

void handleForward(CommandContext& ctx) {
    auto res = ctx.navigator().forward();
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    }
}

static void printTreeRecursive(const std::filesystem::path& dir, int currentDepth, int maxDepth, const std::string& prefix) {
    if (currentDepth > maxDepth) return;

    ListOptions opt;
    opt.showHidden = false;
    auto listRes = listDirectory(dir, opt);
    if (!listRes.ok) return;

    for (size_t i = 0; i < listRes.value.size(); ++i) {
        bool isLast = (i == listRes.value.size() - 1);
        const auto& item = listRes.value[i];

        std::cout << prefix << (isLast ? "`-- " : "|-- ")
                  << (item.type == EntryType::Directory ? boldBlue(item.name) : item.name)
                  << "\n";

        if (item.type == EntryType::Directory) {
            printTreeRecursive(item.path, currentDepth + 1, maxDepth, prefix + (isLast ? "    " : "|   "));
        }
    }
}

void handleTree(const std::vector<std::string>& tokens, CommandContext& ctx) {
    int maxDepth = 2;
    if (tokens.size() >= 2) {
        try {
            maxDepth = std::clamp(std::stoi(tokens[1]), 1, 5);
        } catch (...) {
            maxDepth = 2;
        }
    }

    const auto& cur = ctx.navigator().current();
    std::cout << boldBlue(cur.u8string()) << "\n";
    printTreeRecursive(cur, 1, maxDepth, "");
}

void handleInfo(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: info <path>") << std::endl;
        return;
    }

    auto target = Navigator::resolvePath(tokens[1], ctx.navigator().current());
    auto infoRes = getInfo(target);
    if (!infoRes.ok) {
        std::cout << red(infoRes.error) << std::endl;
        return;
    }

    const auto& info = infoRes.value;
    std::cout << cyan("--------------------------------------------------\n")
              << boldBlue(" File Information & OS Metadata                   \n")
              << cyan("--------------------------------------------------\n")
              << "  Name         : " << info.name << "\n"
              << "  Absolute Path: " << info.path.u8string() << "\n"
              << "  Type         : " << (info.type == EntryType::Directory ? "Directory" :
                                       info.type == EntryType::File ? "Regular File" :
                                       info.type == EntryType::Symlink ? "Symbolic Link" : "Other") << "\n"
              << "  Size         : " << info.size << " bytes (" << formatSize(info.size) << ")\n"
              << "  Last Modified: " << formatTime(info.modified) << "\n"
              << "  Permissions  : " << formatPermissions(info.permissions) << "\n"
              << "  Hidden Flag  : " << (info.isHidden ? "Yes" : "No") << "\n"
              << cyan("--------------------------------------------------\n");
}

void handleView(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: view <file>") << std::endl;
        return;
    }

    auto target = Navigator::resolvePath(tokens[1], ctx.navigator().current());
    auto prevRes = readTextPreview(target);
    if (!prevRes.ok) {
        std::cout << red(prevRes.error) << std::endl;
        return;
    }

    std::cout << cyan("--- Preview: ") << target.filename().u8string() << cyan(" ---\n")
              << prevRes.value
              << (prevRes.value.empty() || prevRes.value.back() != '\n' ? "\n" : "")
              << cyan("--- End of Preview ---\n");
}

void handleMkdir(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: mkdir <name>") << std::endl;
        return;
    }

    auto target = ctx.navigator().current() / tokens[1];
    auto res = createDirectory(target);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << green("Directory created: " + tokens[1]) << std::endl;
    }
}

void handleTouch(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: touch <name>") << std::endl;
        return;
    }

    auto target = ctx.navigator().current() / tokens[1];
    auto res = createFile(target);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << green("File created: " + tokens[1]) << std::endl;
    }
}

void handleRename(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 3) {
        std::cout << red("Usage: rename <path> <newname>") << std::endl;
        return;
    }

    auto from = Navigator::resolvePath(tokens[1], ctx.navigator().current());
    auto res = renamePath(from, tokens[2]);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << green("Renamed successfully.") << std::endl;
    }
}

void handleCp(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 3) {
        std::cout << red("Usage: cp <src> <destdir>") << std::endl;
        return;
    }

    auto src = Navigator::resolvePath(tokens[1], ctx.navigator().current());
    auto dest = Navigator::resolvePath(tokens[2], ctx.navigator().current());
    auto res = copyPath(src, dest);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << green("Copied successfully.") << std::endl;
    }
}

void handleMv(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 3) {
        std::cout << red("Usage: mv <src> <destdir>") << std::endl;
        return;
    }

    auto src = Navigator::resolvePath(tokens[1], ctx.navigator().current());
    auto dest = Navigator::resolvePath(tokens[2], ctx.navigator().current());
    auto res = movePath(src, dest);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << green("Moved successfully.") << std::endl;
    }
}

void handleRm(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: rm <path> [-r]") << std::endl;
        return;
    }

    bool recursive = false;
    std::string pathArg;

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == "-r" || tokens[i] == "-R" || tokens[i] == "--recursive") {
            recursive = true;
        } else {
            pathArg = tokens[i];
        }
    }

    if (pathArg.empty()) {
        std::cout << red("Usage: rm <path> [-r]") << std::endl;
        return;
    }

    auto target = Navigator::resolvePath(pathArg, ctx.navigator().current());

    // Prompt user confirmation: Delete 'x'? (y/N)
    std::cout << yellow("Delete '" + target.filename().u8string() + "'? (y/N): ");
    std::string confirm;
    if (std::cin >> confirm) {
        if (confirm == "y" || confirm == "Y" || confirm == "yes" || confirm == "YES") {
            auto res = removePath(target, recursive);
            if (!res.ok) {
                std::cout << red(res.error) << std::endl;
            } else {
                std::cout << green("Deleted successfully.") << std::endl;
            }
        } else {
            std::cout << "Deletion canceled." << std::endl;
        }
    }
}

void handleFind(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.size() < 2) {
        std::cout << red("Usage: find <pattern> [--depth n]") << std::endl;
        return;
    }

    std::string pattern = tokens[1];
    SearchOptions options;

    for (size_t i = 2; i < tokens.size(); ++i) {
        if (tokens[i] == "--depth" && i + 1 < tokens.size()) {
            try {
                options.maxDepth = std::stoi(tokens[++i]);
            } catch (...) {
                options.maxDepth = 10;
            }
        }
    }

    auto res = searchByName(ctx.navigator().current(), pattern, options);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
        return;
    }

    std::cout << "Found " << res.value.size() << " match(es):\n";
    for (const auto& item : res.value) {
        std::string nameDisplay = (item.type == EntryType::Directory) ? boldBlue(item.path.u8string()) : item.path.u8string();
        std::cout << "  " << nameDisplay << " (" << formatSize(item.size) << ")\n";
    }
}

void handleDu(const std::vector<std::string>& tokens, CommandContext& ctx) {
    auto target = (tokens.size() >= 2) ?
                  Navigator::resolvePath(tokens[1], ctx.navigator().current()) :
                  ctx.navigator().current();

    auto res = directorySize(target);
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        std::cout << formatSize(res.value) << " (" << res.value << " bytes)  "
                  << target.u8string() << std::endl;
    }
}

void handleSpace(CommandContext& ctx) {
    auto res = getSpace(ctx.navigator().current());
    if (!res.ok) {
        std::cout << red(res.error) << std::endl;
    } else {
        const auto& sp = res.value;
        std::cout << cyan("--------------------------------------------------\n")
                  << boldBlue(" Drive Storage Information                        \n")
                  << cyan("--------------------------------------------------\n")
                  << "  Total Capacity   : " << formatSize(sp.capacity) << " (" << sp.capacity << " bytes)\n"
                  << "  Free Space       : " << formatSize(sp.free) << " (" << sp.free << " bytes)\n"
                  << "  Available Space  : " << formatSize(sp.available) << " (" << sp.available << " bytes)\n"
                  << cyan("--------------------------------------------------\n");
    }
}

void handleOsDemo(CommandContext& ctx) {
#if defined(_WIN32)
    printNativeDirectoryEntries(ctx.navigator().current().wstring());
#else
    printNativeDirectoryEntries(ctx.navigator().current().u8string());
#endif
}

void executeCommand(const std::vector<std::string>& tokens, CommandContext& ctx) {
    if (tokens.empty()) {
        return; // Empty input is ignored
    }

    const auto& cmd = tokens[0];

    if (cmd == "help") handleHelp();
    else if (cmd == "pwd") handlePwd(ctx);
    else if (cmd == "ls") handleLs(tokens, ctx);
    else if (cmd == "cd") handleCd(tokens, ctx);
    else if (cmd == "up") handleUp(ctx);
    else if (cmd == "back") handleBack(ctx);
    else if (cmd == "forward") handleForward(ctx);
    else if (cmd == "tree") handleTree(tokens, ctx);
    else if (cmd == "info") handleInfo(tokens, ctx);
    else if (cmd == "view") handleView(tokens, ctx);
    else if (cmd == "mkdir") handleMkdir(tokens, ctx);
    else if (cmd == "touch") handleTouch(tokens, ctx);
    else if (cmd == "rename") handleRename(tokens, ctx);
    else if (cmd == "cp") handleCp(tokens, ctx);
    else if (cmd == "mv") handleMv(tokens, ctx);
    else if (cmd == "rm") handleRm(tokens, ctx);
    else if (cmd == "find") handleFind(tokens, ctx);
    else if (cmd == "du") handleDu(tokens, ctx);
    else if (cmd == "space") handleSpace(ctx);
    else if (cmd == "osdemo") handleOsDemo(ctx);
    else if (cmd == "clear") {
#if defined(_WIN32)
        std::system("cls");
#else
        std::system("clear");
#endif
    }
    else if (cmd == "exit" || cmd == "quit") {
        ctx.requestQuit();
    } else {
        std::cout << red("Unknown command. Type help.") << std::endl;
    }
}

} // namespace fx::cli
