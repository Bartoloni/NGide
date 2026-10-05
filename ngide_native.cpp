// NGide native helpers - 1.23
// Primo passo della migrazione graduale BMX -> C++.
// La GUI e l'applicazione degli stili restano in BlitzMax.

#include <cstdint>
#include <cstring>

#ifdef _WIN32
#define NGIDE_EXPORT extern "C" __declspec(dllexport)
#else
#define NGIDE_EXPORT extern "C"
#endif

static inline bool ngide_bmx_operator(unsigned char ch) {
    switch (ch) {
        case 33: case 35: case 36: case 37: case 38:
        case 40: case 41: case 42: case 43: case 44:
        case 45: case 46: case 47: case 58: case 60:
        case 61: case 62: case 63: case 64: case 91:
        case 92: case 93: case 94: case 123: case 124:
        case 125:
            return true;
        default:
            return false;
    }
}

static inline unsigned char ngide_lower_ascii(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z') return (unsigned char)(ch + ('a' - 'A'));
    return ch;
}

static bool ngide_word_at_ci(const char *s, int len, int p, const char *word) {
    const int n = (int)std::strlen(word);
    if (p < 0 || p + n > len) return false;
    for (int i = 0; i < n; ++i) {
        if (ngide_lower_ascii((unsigned char)s[p + i]) != (unsigned char)word[i]) return false;
    }
    return true;
}

// Restituisce:
//  >= 0 : numero di posizioni operatore scritte in outPositions
//  -1   : sorgente non ASCII; il chiamante deve usare il fallback BlitzMax.
//
// Replica intenzionalmente la logica della 1.22:
// - ignora operatori dentro stringhe
// - ignora commenti a riga singola con apostrofo
// - ignora blocchi Rem/EndRem riconosciuti all'inizio logico della riga
// - '~' dentro una stringa salta il carattere successivo
NGIDE_EXPORT int ngide_scan_bmx_operators(const char *src, int len,
                                           int *outPositions, int maxPositions) {
    if (!src || len <= 0 || !outPositions || maxPositions <= 0) return 0;

    // Gli indici BlitzMax e quelli UTF-8 coincidono solo per ASCII.
    // In presenza di Unicode lasciamo lavorare il codice BMX originale.
    for (int i = 0; i < len; ++i) {
        if (((unsigned char)src[i]) >= 0x80) return -1;
    }

    bool inString = false;
    bool inLineComment = false;
    bool inRem = false;
    bool lineStart = true;
    int count = 0;

    int p = 0;
    while (p < len) {
        const unsigned char ch = (unsigned char)src[p];

        if (ch == 10 || ch == 13) {
            inLineComment = false;
            lineStart = true;
            ++p;
            continue;
        }

        if (lineStart) {
            if (ch == 32 || ch == 9) {
                ++p;
                continue;
            }
            if (inRem) {
                if (ngide_word_at_ci(src, len, p, "endrem")) inRem = false;
            } else {
                if (ngide_word_at_ci(src, len, p, "rem")) inRem = true;
            }
            lineStart = false;
        }

        if (inRem || inLineComment) {
            ++p;
            continue;
        }

        if (inString) {
            if (ch == 126) { // '~' escape BlitzMax
                p += 2;
                continue;
            }
            if (ch == 34) inString = false;
            ++p;
            continue;
        }

        if (ch == 34) {
            inString = true;
            ++p;
            continue;
        }
        if (ch == 39) {
            inLineComment = true;
            ++p;
            continue;
        }

        if (ngide_bmx_operator(ch)) {
            if (count < maxPositions) outPositions[count] = p;
            ++count;
        }
        ++p;
    }

    return count > maxPositions ? maxPositions : count;
}
