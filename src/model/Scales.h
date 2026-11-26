#pragma once
// ============================================================================
//  Scales: las escalas que usan el piano roll y el autotune.
// ============================================================================
#include <vector>

namespace studoo::scales
{
    constexpr int numScales = 9;   // 0 = sin escala (cromática)

    // Intervalos de cada escala (en semitonos desde la nota base)
    inline const std::vector<int>& intervals (int type)
    {
        static const std::vector<std::vector<int>> table = {
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },   // cromática (todas las notas)
            { 0, 2, 4, 5, 7, 9, 11 },     // Mayor
            { 0, 2, 3, 5, 7, 8, 10 },     // Menor
            { 0, 2, 3, 5, 7, 8, 11 },     // Menor armónica
            { 0, 3, 5, 7, 10 },           // Pentatónica menor
            { 0, 2, 4, 7, 9 },            // Pentatónica mayor
            { 0, 3, 5, 6, 7, 10 },        // Blues
            { 0, 2, 3, 5, 7, 9, 10 },     // Dórica
            { 0, 1, 3, 5, 7, 8, 10 },     // Frigia (drill, flamenco)
        };
        return table[(size_t) (type < 0 || type >= numScales ? 0 : type)];
    }

    inline bool contains (int type, int root, int pitch)
    {
        const int degree = ((pitch - root) % 12 + 12) % 12;
        for (int i : intervals (type))
            if (i == degree)
                return true;
        return false;
    }
}
