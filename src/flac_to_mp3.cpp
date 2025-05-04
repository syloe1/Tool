
#include "flac_to_mp3.h"
#include <cstdlib>
#include <sstream>

bool convert_flac_to_mp3(const std::string& in, const std::string& out) {
    std::ostringstream cmd;
    cmd << "ffmpeg -y -i \"" << in << "\" "
        << "-codec:a libmp3lame -qscale:a 2 \"" << out << "\""
        << " >/dev/null 2>&1";
    return std::system(cmd.str().c_str()) == 0;
}
