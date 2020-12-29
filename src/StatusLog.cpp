#include "StatusLog.h"

// Runtime          |  Completed  |  Status
//------------------------------------------
// 00d 00h 00m 03s  |  1000/1960  |  51%
// 00d 00h 00m 13s  |  1960/1960  |  100%
//------------------------------------------

std::string StatusLog::status() {

  // If too little runs, logs are useless and formatting would not work as well
  if (m_totalIters < 1000) return "";

  size_t osec = (size_t)((std::chrono::duration<float>)(
    std::chrono::high_resolution_clock::now() - m_start)).count();

	// Format into (dd:hh:mm:ss)
	uint days = osec / 60 / 60 / 24;
	uint hour = (osec / 60 / 60) % 24;
	uint min = (osec / 60) % 60;
	uint sec = osec % 60;

	// Build the final string output
	std::stringstream ss;
	ss << " " << std::setw(2) << std::setfill('0') << days << "d "
    << std::setw(2) << std::setfill('0') << hour << "h "
    << std::setw(2) << std::setfill('0') << min << "m "
    << std::setw(2) << std::setfill('0') << sec << "s  |  "
	  << std::setw(m_dig) << m_curIters << "/" << m_totalIters << "  |  "
   << (uint)(100 * m_curIters / m_totalIters) << "%";

	return ss.str();
}


std::string StatusLog::header() {

  if (m_totalIters < 1000) return "";

  // Build the final string output
  std::stringstream ss;
  ss << " Runtime          |  Completed" << std::string(2 * (m_dig - 4), ' ')
     << "  |  Status\n" << std::string(34 + 2 * m_dig, '-');

  return ss.str();
}


std::string StatusLog::footer() {

  if (m_totalIters < 1000) return "";

  return status() + '\n' + std::string(34 + 2 * m_dig, '-');
}
