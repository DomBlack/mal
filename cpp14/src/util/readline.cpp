#include <linenoise/linenoise.hpp>
#include <cassert>

#include "readline.h"

/**
 * Expands the `~` character into the Users home directory
 *
 * Taken from: http://stackoverflow.com/a/4891126/404679
 *
 * @param path The path to expand
 *
 * @return The expanded path
 */
std::string expandUser(std::string path) {
  if (!path.empty() && path[0] == '~') {
    assert(path.size() == 1 || path[1] == '/');

    char const *home = getenv("HOME");
    if (home || ((home = getenv("USERPROFILE")))) {
      path.replace(0, 1, home);
    } else {
      char const *hdrive = getenv("HOMEDRIVE"), *hpath = getenv("HOMEPATH");
      assert(hdrive);
      assert(hpath);
      path.replace(0, 1, std::string(hdrive) + hpath);
    }
  }

  return path;
}


mal::ReadLine::ReadLine(const char *prompt, const char *historyFile) :
    prompt(prompt),
    historyFile(expandUser(historyFile))
{
  linenoise::SetHistoryMaxLen(250);
  linenoise::LoadHistory(this->historyFile.c_str());
}


mal::ReadLine::~ReadLine() {
  linenoise::SaveHistory(this->historyFile.c_str());
}


bool mal::ReadLine::operator>>(std::string &line) {
  const auto keepGoing = !linenoise::Readline(prompt, line);

  if (keepGoing) {
    linenoise::AddHistory(line.c_str());
  }

  return keepGoing;
}
