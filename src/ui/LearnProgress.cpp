#include "LearnProgress.h"

namespace studoo::learn
{
namespace
{
    // Iconos de las medallas (se dibujan en LearnView.cpp)
    enum Icon { star, book, drum, note, mixer, globe, check, trophy, target, ear, flame };

    juce::String today()                  { return juce::Time::getCurrentTime().formatted ("%Y-%m-%d"); }
    juce::String dayOffset (int days)     { return (juce::Time::getCurrentTime() - juce::RelativeTime::days (days)).formatted ("%Y-%m-%d"); }
}

const std::vector<Achievement>& achievements()
{
    static const std::vector<Achievement> list = {
        { "primer_paso",   "Primer paso",          "Aprende tu primera lección",                   star   },
        { "aprendiz",      "Aprendiz",             "Aprende 10 lecciones",                         book   },
        { "estudiante",    "Estudiante aplicado",  "Aprende 25 lecciones",                         book   },
        { "nivel_ritmo",   "Manos de batería",     "Completa el nivel Ritmo y batería",            drum   },
        { "nivel_melodia", "Melódico",             "Completa el nivel Notas y melodía",            note   },
        { "nivel_mezcla",  "Ingeniero de mezcla",  "Completa el nivel Mezcla",                     mixer  },
        { "nivel_generos", "Trotamundos",          "Completa el nivel Géneros musicales",          globe  },
        { "maestro",       "Maestro de Studoo",    "Aprende todas las lecciones",                  trophy },
        { "test_perfecto", "¡Sin fallos!",         "Saca un test perfecto",                        check  },
        { "sabelotodo",    "Sabelotodo",           "Saca 10 tests perfectos",                      check  },
        { "primer_reto",   "Manos a la obra",      "Supera tu primer reto en Studoo",              target },
        { "retador",       "Imparable",            "Supera 10 retos",                              target },
        { "oido_fino",     "Oído fino",            "Acierta 5 seguidas en un juego de oído",       ear    },
        { "oido_absoluto", "Oído de oro",          "Acierta 15 seguidas en un juego de oído",      ear    },
        { "racha_3",       "En racha",             "Practica 3 días seguidos",                     flame  },
        { "racha_7",       "Una semana entera",    "Practica 7 días seguidos",                     flame  },
        { "racha_30",      "Constancia de campeón","Practica 30 días seguidos",                    flame  },
    };
    return list;
}

// ----------------------------------------------------------------------------
juce::StringArray Progress::getList (const juce::String& key) const
{
    if (settings == nullptr)
        return {};
    auto list = juce::StringArray::fromTokens (settings->getValue (key), "|", {});
    list.removeEmptyStrings();
    return list;
}

void Progress::setList (const juce::String& key, const juce::StringArray& list)
{
    if (settings == nullptr)
        return;
    settings->setValue (key, list.joinIntoString ("|"));
    settings->saveIfNeeded();
}

void Progress::changed()
{
    checkAchievements();
    if (onChanged)
        onChanged();
}

// --- Lecciones -------------------------------------------------------------------
bool Progress::isLessonDone (const juce::String& title) const { return getList ("learnDone").contains (title); }
int  Progress::numLessonsDone() const                        { return getList ("learnDone").size(); }

void Progress::setLessonDone (const juce::String& title, bool done)
{
    auto list = getList ("learnDone");
    list.removeString (title);
    if (done)
        list.add (title);
    setList ("learnDone", list);
    if (done)
        practiced();
    changed();
}

// --- Tests ------------------------------------------------------------------------
int Progress::bestQuizScore (const juce::String& lessonTitle) const
{
    for (auto& entry : getList ("learnQuiz"))
        if (entry.upToLastOccurrenceOf ("=", false, false) == lessonTitle)
            return entry.fromLastOccurrenceOf ("=", false, false).getIntValue();
    return -1;
}

void Progress::recordQuiz (const juce::String& lessonTitle, int score, int total)
{
    auto list = getList ("learnQuiz");
    const int best = juce::jmax (score, bestQuizScore (lessonTitle));
    for (int i = list.size(); --i >= 0;)
        if (list[i].upToLastOccurrenceOf ("=", false, false) == lessonTitle)
            list.remove (i);
    list.add (lessonTitle + "=" + juce::String (best));
    setList ("learnQuiz", list);

    if (score == total)
    {
        auto perfect = getList ("learnQuizPerfect");
        perfect.addIfNotAlreadyThere (lessonTitle);
        setList ("learnQuizPerfect", perfect);
    }
    practiced();
    changed();
}

int Progress::numPerfectQuizzes() const { return getList ("learnQuizPerfect").size(); }

// --- Retos ------------------------------------------------------------------------
bool Progress::isChallengeDone (const juce::String& id) const { return getList ("learnChallenges").contains (id); }
int  Progress::numChallengesDone() const                     { return getList ("learnChallenges").size(); }

void Progress::setChallengeDone (const juce::String& id)
{
    if (isChallengeDone (id))
        return;
    auto list = getList ("learnChallenges");
    list.add (id);
    setList ("learnChallenges", list);
    practiced();
    changed();
}

// --- Juegos -------------------------------------------------------------------------
int Progress::bestGameStreak (const juce::String& gameId) const
{
    return settings != nullptr ? settings->getIntValue ("learnGame_" + gameId, 0) : 0;
}

void Progress::recordGameStreak (const juce::String& gameId, int streak)
{
    if (settings == nullptr || streak <= bestGameStreak (gameId))
        return;
    settings->setValue ("learnGame_" + gameId, streak);
    settings->saveIfNeeded();
    changed();
}

int Progress::totalGameAnswers() const { return settings != nullptr ? settings->getIntValue ("learnGameAnswers", 0) : 0; }

void Progress::addGameAnswer (bool correct)
{
    if (settings == nullptr)
        return;
    settings->setValue ("learnGameAnswers", totalGameAnswers() + 1);
    if (correct)
        practiced();
    settings->saveIfNeeded();
}

// --- Racha ----------------------------------------------------------------------------
void Progress::practiced()
{
    if (settings == nullptr)
        return;
    const auto last = settings->getValue ("learnLastDay");
    if (last == today())
        return;
    const int streak = last == dayOffset (1) ? settings->getIntValue ("learnStreak", 0) + 1 : 1;
    settings->setValue ("learnStreak", streak);
    settings->setValue ("learnBestStreak", juce::jmax (streak, settings->getIntValue ("learnBestStreak", 0)));
    settings->setValue ("learnLastDay", today());
    settings->saveIfNeeded();
}

int Progress::currentStreak() const
{
    if (settings == nullptr)
        return 0;
    const auto last = settings->getValue ("learnLastDay");
    if (last != today() && last != dayOffset (1))
        return 0;
    return settings->getIntValue ("learnStreak", 0);
}

int  Progress::bestStreak() const     { return settings != nullptr ? settings->getIntValue ("learnBestStreak", 0) : 0; }
bool Progress::practicedToday() const { return settings != nullptr && settings->getValue ("learnLastDay") == today(); }

// --- Logros -----------------------------------------------------------------------------
bool Progress::isUnlocked (const juce::String& id) const { return getList ("learnAchievements").contains (id); }
int  Progress::numUnlocked() const                       { return getList ("learnAchievements").size(); }

void Progress::checkAchievements()
{
    if (settings == nullptr)
        return;

    auto levelDone = [this] (int level) { return isLevelComplete != nullptr && isLevelComplete (level); };
    int bestGame = 0;
    for (auto id : { "instrumento", "altura", "acordes", "genero" })
        bestGame = juce::jmax (bestGame, bestGameStreak (id));
    bool allLevels = numLevels > 0;
    for (int l = 0; l < numLevels; ++l)
        allLevels = allLevels && levelDone (l);
    const int lessons = numLessonsDone(), streak = bestStreak();

    auto earned = [&] (const juce::String& id)
    {
        if (id == "primer_paso")   return lessons >= 1;
        if (id == "aprendiz")      return lessons >= 10;
        if (id == "estudiante")    return lessons >= 25;
        if (id == "nivel_ritmo")   return levelDone (1);
        if (id == "nivel_melodia") return levelDone (2);
        if (id == "nivel_mezcla")  return levelDone (4);
        if (id == "nivel_generos") return levelDone (7);
        if (id == "maestro")       return allLevels;
        if (id == "test_perfecto") return numPerfectQuizzes() >= 1;
        if (id == "sabelotodo")    return numPerfectQuizzes() >= 10;
        if (id == "primer_reto")   return numChallengesDone() >= 1;
        if (id == "retador")       return numChallengesDone() >= 10;
        if (id == "oido_fino")     return bestGame >= 5;
        if (id == "oido_absoluto") return bestGame >= 15;
        if (id == "racha_3")       return streak >= 3;
        if (id == "racha_7")       return streak >= 7;
        if (id == "racha_30")      return streak >= 30;
        return false;
    };

    auto unlocked = getList ("learnAchievements");
    for (auto& a : achievements())
        if (! unlocked.contains (a.id) && earned (a.id))
        {
            unlocked.add (a.id);
            setList ("learnAchievements", unlocked);
            if (onUnlocked)
                onUnlocked (a);
        }
}
}
