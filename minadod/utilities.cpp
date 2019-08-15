#include <string>
#include <vector>

/**
 * Ascii Array
 */
static const char* const ascii[] = {
    "00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f",
    "10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f",
    "20","21","22","23","24","25","26","27","28","29","2a","2b","2c","2d","2e","2f",
    "30","31","32","33","34","35","36","37","38","39","3a","3b","3c","3d","3e","3f",
    "40","41","42","43","44","45","46","47","48","49","4a","4b","4c","4d","4e","4f",
    "50","51","52","53","54","55","56","57","58","59","5a","5b","5c","5d","5e","5f",
    "60","61","62","63","64","65","66","67","68","69","6a","6b","6c","6d","6e","6f",
    "70","71","72","73","74","75","76","77","78","79","7a","7b","7c","7d","7e","7f",
    "80","81","82","83","84","85","86","87","88","89","8a","8b","8c","8d","8e","8f",
    "90","91","92","93","94","95","96","97","98","99","9a","9b","9c","9d","9e","9f",
    "a0","a1","a2","a3","a4","a5","a6","a7","a8","a9","aa","ab","ac","ad","ae","af",
    "b0","b1","b2","b3","b4","b5","b6","b7","b8","b9","ba","bb","bc","bd","be","bf",
    "c0","c1","c2","c3","c4","c5","c6","c7","c8","c9","ca","cb","cc","cd","ce","cf",
    "d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","da","db","dc","dd","de","df",
    "e0","e1","e2","e3","e4","e5","e6","e7","e8","e9","ea","eb","ec","ed","ee","ef",
    "f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff"
};

/**
 * From Ascii
 */
static uint8_t fromAscii(uint8_t c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');

    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);

    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);

#if defined(__EXCEPTIONS) || defined(DEBUG)
    throw std::runtime_error("[fromAscii] Invalid character!");
#else
    return 0xff;
#endif
}

/**
 * Ascii (R)
 */
static uint8_t ascii_r(uint8_t a, uint8_t b)
{
    return fromAscii(a) * 16 + fromAscii(b);
}

/**
 * Hex To Bytes
 */
static void HexToBytes(std::string const& hex, uint8_t bytes[])
{
    for (std::string::size_type i = 0, j = 0; i < hex.length(); i += 2, ++j) {
        bytes[j] = ascii_r(hex[i], hex[i + 1]);
    }
}

/**
 * Current Date as Formatted String
 */
static std::string CurrentDateAsString(bool abbreviated = true)
{
    time_t rawtime;
    struct tm* timeInfo;
    char buffer[80];

    time(&rawtime);
    timeInfo = localtime(&rawtime);

    /* Format the date string (as required). */
    if (abbreviated) {
        strftime(buffer, 80, "%Y%m%d", timeInfo);
    } else {
        strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", timeInfo);
    }

    std::string dateString(buffer);

    return dateString;
}

/**
 * Time Ago
 */
static std::string TimeAgo(std::string _timeSince)
{
    auto nowTime = std::chrono::system_clock::now();

    std::time_t timestamp =
        std::chrono::system_clock::to_time_t(nowTime);

    /* Convert to integer. */
    int timeSince = std::stoi(_timeSince);

    /* Calculate the time difference. */
    int timeDiff = timestamp - timeSince;

    /* Format the label (based on duration). */
    if (timeDiff > 3600) {
        return std::to_string(timeDiff / 3600) + " hours ago";
    } else if (timeDiff > 60) {
        return std::to_string(timeDiff / 60) + " minutes ago";
    } else {
        return std::to_string(timeDiff) + " seconds ago";
    }
}
