#include <Parser/Parsing/IParserLineStream.h>

#include <memory>
#include <string>
#include <utility>

namespace
{
  std::shared_ptr<std::string>
  make_parser_filename (const char* fallback)
  {
    return std::make_shared<std::string> (fallback);
  }
}

ParserLine::
ParserLine ()
  : m_filename (make_parser_filename ("<eof>")),
    m_line_number (0)
{
}

ParserLine::
ParserLine (std::shared_ptr<std::string> filename,
            const int lineNumber,
            std::string line)
  : m_filename (filename != nullptr
                  ? std::move (filename)
                  : make_parser_filename ("<unknown>")),
    m_line_number (lineNumber),
    m_line (std::move (line))
{
}

bool
ParserLine::
IsEof () const
{
  return m_line_number <= 0;
}
