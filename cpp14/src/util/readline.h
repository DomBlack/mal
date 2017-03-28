#pragma once

#include <string>

namespace mal {
  /**
   * Reads a line form the user input
   */
  class ReadLine {
    private:
      /// The prompt to display
      const char* prompt;

      /// The history file to write to
      const std::string historyFile;

    public:
      /**
       * ReadLine constructor
       *
       * @param prompt      The prompt to display each time
       * @param historyFile The history file to write to
       */
      ReadLine(const char* prompt, const char* historyFile);

      /**
       * ReadLine destructor
       */
      ~ReadLine();

      /**
       * Reads a line from the terminal
       *
       * @param prompt The prompt to show the user
       * @param line   The line read in
       *
       * @return false if EOF is reached
       */
      bool operator>>(std::string& line);
  };
}
