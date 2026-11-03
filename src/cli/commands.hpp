// ============================================================================
// File: src/cli/commands.hpp
// Purpose: CLI command parsing, tokenizer, and command handlers.
// OS Concept: Shell command interpreter, lexical analysis (tokenization), and command dispatch.
// ============================================================================

#pragma once

#include "../core/navigator.hpp"
#include <string>
#include <vector>

namespace fx::cli {

// Tokenizes an input string handling quoted arguments with spaces
std::vector<std::string> tokenize(const std::string& line);

// Context for executing CLI commands
class CommandContext {
public:
    explicit CommandContext(Navigator& nav) : m_navigator(nav) {}

    Navigator& navigator() { return m_navigator; }
    bool shouldQuit() const { return m_quit; }
    void requestQuit() { m_quit = true; }

private:
    Navigator& m_navigator;
    bool m_quit = false;
};

// Dispatches a command line to its corresponding handler
void executeCommand(const std::vector<std::string>& tokens, CommandContext& ctx);

// Command handlers
void handleHelp();
void handlePwd(CommandContext& ctx);
void handleLs(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleCd(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleUp(CommandContext& ctx);
void handleBack(CommandContext& ctx);
void handleForward(CommandContext& ctx);
void handleTree(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleInfo(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleView(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleMkdir(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleTouch(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleRename(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleCp(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleMv(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleRm(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleFind(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleDu(const std::vector<std::string>& tokens, CommandContext& ctx);
void handleSpace(CommandContext& ctx);
void handleOsDemo(CommandContext& ctx);

} // namespace fx::cli
