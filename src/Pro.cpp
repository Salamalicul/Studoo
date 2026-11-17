// En Android necesitamos las ayudas de JUCE para llamar a Java (JNI).
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#include "Pro.h"
#include <juce_events/juce_events.h>

namespace studoo::pro
{
namespace
{
    juce::PropertiesFile* settings = nullptr;
    EventHandler handler;
    bool unlockedCache = false;

   #if STUDOO_PLAY && JUCE_ANDROID
    // --- Llamadas a com.studoo.app.StudooBilling (packaging/android/java-play) ----------
    jclass billingClass()
    {
        static jclass cls = nullptr;
        if (cls != nullptr)
            return cls;

        auto* env = juce::getEnv();
        auto context = juce::getAppContext();
        if (env == nullptr || context.get() == nullptr)
            return nullptr;

        jclass contextClass = env->GetObjectClass (context.get());
        jmethodID getClassLoader = env->GetMethodID (contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
        jobject loader = env->CallObjectMethod (context.get(), getClassLoader);
        env->DeleteLocalRef (contextClass);
        if (env->ExceptionCheck() || loader == nullptr) { env->ExceptionClear(); return nullptr; }

        jclass loaderClass = env->GetObjectClass (loader);
        jmethodID loadClass = env->GetMethodID (loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        jstring name = env->NewStringUTF ("com.studoo.app.StudooBilling");
        auto local = (jclass) env->CallObjectMethod (loader, loadClass, name);
        env->DeleteLocalRef (name);
        env->DeleteLocalRef (loaderClass);
        env->DeleteLocalRef (loader);
        if (env->ExceptionCheck() || local == nullptr) { env->ExceptionClear(); return nullptr; }

        cls = (jclass) env->NewGlobalRef (local);
        env->DeleteLocalRef (local);
        return cls;
    }

    juce::String toString (JNIEnv* env, jstring s)
    {
        if (s == nullptr)
            return {};
        const char* chars = env->GetStringUTFChars (s, nullptr);
        juce::String result = juce::String::fromUTF8 (chars);
        env->ReleaseStringUTFChars (s, chars);
        env->DeleteLocalRef (s);
        return result;
    }

    bool clearException (JNIEnv* env)
    {
        if (! env->ExceptionCheck())
            return false;
        env->ExceptionClear();
        return true;
    }
   #endif

    void setUnlocked (bool unlocked)
    {
        unlockedCache = unlocked;
        if (settings != nullptr)
        {
            // Se recuerda para poder usar Pro sin internet
            settings->setValue ("proUnlocked", unlocked);
            settings->saveIfNeeded();
        }
    }
}

bool hasStore()
{
   #if STUDOO_PLAY
    return true;
   #else
    static const bool simulated = juce::SystemStats::getEnvironmentVariable ("STUDOO_PRUEBA_TIENDA", {}).isNotEmpty();
    return simulated;
   #endif
}

// --- Qué es gratis ------------------------------------------------------------------------
bool isUnlocked()                 { return ! hasStore() || unlockedCache; }

bool isKitFree (const juce::String& kitId)
{
    if (isUnlocked())
        return true;
    for (auto id : { "trap", "reggaeton", "house", "boombap", "lofi" })
        if (kitId == id)
            return true;
    return false;
}

bool isSynthPresetFree (int index) { return isUnlocked() || index < 6; }
bool is808PresetFree (int index)   { return isUnlocked() || index < 2; }

bool isGenreFree (const juce::String& genreId)
{
    return isUnlocked() || genreId == "trap" || genreId == "reggaeton" || genreId == "house";
}

bool isLearnLevelFree (int level) { return isUnlocked() || level < 4; }
bool areGamesFree()               { return isUnlocked(); }
bool areAchievementsFree()        { return isUnlocked(); }

// --- Tienda ---------------------------------------------------------------------------------
void start (juce::PropertiesFile* s)
{
    settings = s;
    unlockedCache = settings != nullptr && settings->getBoolValue ("proUnlocked", false);

   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    if (auto cls = billingClass())
    {
        jmethodID m = env->GetStaticMethodID (cls, "start", "(Landroid/content/Context;Ljava/lang/String;)V");
        jstring key = env->NewStringUTF ("");
        if (m != nullptr)
            env->CallStaticVoidMethod (cls, m, juce::getAppContext().get(), key);
        env->DeleteLocalRef (key);
        clearException (env);
    }
   #endif
}

juce::String price (const juce::String& productId)
{
   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    if (auto cls = billingClass())
    {
        jmethodID m = env->GetStaticMethodID (cls, "price", "(Ljava/lang/String;)Ljava/lang/String;");
        jstring id = env->NewStringUTF (productId.toRawUTF8());
        auto result = m != nullptr ? (jstring) env->CallStaticObjectMethod (cls, m, id) : nullptr;
        env->DeleteLocalRef (id);
        if (clearException (env))
            return {};
        return toString (env, result);
    }
   #endif
    if (hasStore()) // modo prueba en el ordenador
    {
        if (productId == proProduct) return juce::String::fromUTF8 ("2,99 €");
        if (productId == donationProducts[0]) return juce::String::fromUTF8 ("0,99 €");
        if (productId == donationProducts[1]) return juce::String::fromUTF8 ("2,99 €");
        if (productId == donationProducts[2]) return juce::String::fromUTF8 ("4,99 €");
    }
    return {};
}

bool storeReady()
{
   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    if (auto cls = billingClass())
    {
        jmethodID m = env->GetStaticMethodID (cls, "isReady", "()Z");
        const bool ready = m != nullptr && env->CallStaticBooleanMethod (cls, m);
        return ! clearException (env) && ready;
    }
   #endif
    return false;
}

void buy (const juce::String& productId)
{
   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    if (auto cls = billingClass())
    {
        jmethodID m = env->GetStaticMethodID (cls, "buy", "(Landroid/app/Activity;Ljava/lang/String;)Z");
        jstring id = env->NewStringUTF (productId.toRawUTF8());
        auto activity = juce::getCurrentActivity();
        if (m != nullptr)
            env->CallStaticBooleanMethod (cls, m, activity.get(), id);
        env->DeleteLocalRef (id);
        clearException (env);
        return;
    }
   #endif
    // Modo prueba en el ordenador: se simula la compra (no se cobra nada)
    if (hasStore())
    {
        if (productId == proProduct)
            setUnlocked (true);
        if (handler)
            handler (productId == proProduct ? "pro_comprado" : "donacion_ok");
        return;
    }
    if (handler)
        handler ("error:" + juce::String::fromUTF8 ("La tienda no está disponible en esta versión de Studoo."));
}

void refresh()
{
   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    if (auto cls = billingClass())
    {
        jmethodID m = env->GetStaticMethodID (cls, "refresh", "()V");
        if (m != nullptr)
            env->CallStaticVoidMethod (cls, m);
        clearException (env);
    }
   #endif
}

void setEventHandler (EventHandler h) { handler = std::move (h); }

void poll()
{
   #if STUDOO_PLAY && JUCE_ANDROID
    auto* env = juce::getEnv();
    auto cls = billingClass();
    if (cls == nullptr)
        return;

    // ¿Qué dice Google Play de la compra de Pro? (-1 = aún no lo sabe)
    if (jmethodID m = env->GetStaticMethodID (cls, "proState", "()I"))
    {
        const int state = env->CallStaticIntMethod (cls, m);
        if (! clearException (env) && state >= 0 && (state == 1) != unlockedCache)
            setUnlocked (state == 1);
    }

    if (jmethodID m = env->GetStaticMethodID (cls, "nextEvent", "()Ljava/lang/String;"))
    {
        for (int i = 0; i < 8; ++i)
        {
            auto result = (jstring) env->CallStaticObjectMethod (cls, m);
            if (clearException (env))
                break;
            const auto event = toString (env, result);
            if (event.isEmpty())
                break;
            if (event == "pro_comprado")
                setUnlocked (true);
            if (handler)
                handler (event);
        }
    }
   #endif
}
}
