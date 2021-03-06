/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "NativePluralRules"

#include "IcuUtilities.h"
#include "JNIHelp.h"
#include "JniConstants.h"
#include "JniException.h"
#include "ScopedUtfChars.h"
#include "unicode/plurrule.h"

#include <string>

static icu::PluralRules* toPluralRules(jlong address) {
    return reinterpret_cast<icu::PluralRules*>(static_cast<uintptr_t>(address));
}

static void NativePluralRules_finalizeImpl(JNIEnv*, jclass, jlong address) {
    delete toPluralRules(address);
}

static jlong NativePluralRules_forLocaleImpl(JNIEnv* env, jclass, jstring javaLocaleName) {
    // The icu4c PluralRules returns a "other: n" default rule for the deprecated locales Java uses.
    // Work around this by translating back to the current language codes.
    std::string localeName(ScopedUtfChars(env, javaLocaleName).c_str());
    if (localeName[0] == 'i' && localeName[1] == 'w') {
        localeName[0] = 'h';
        localeName[1] = 'e';
    } else if (localeName[0] == 'i' && localeName[1] == 'n') {
        localeName[0] = 'i';
        localeName[1] = 'd';
    } else if (localeName[0] == 'j' && localeName[1] == 'i') {
        localeName[0] = 'y';
        localeName[1] = 'i';
    }

    icu::Locale locale = icu::Locale::createFromName(localeName.c_str());
    UErrorCode status = U_ZERO_ERROR;
    icu::PluralRules* result = icu::PluralRules::forLocale(locale, status);
    maybeThrowIcuException(env, "PluralRules::forLocale", status);
    return reinterpret_cast<uintptr_t>(result);
}

static jint NativePluralRules_quantityForIntImpl(JNIEnv*, jclass, jlong address, jint value) {
    icu::UnicodeString keyword = toPluralRules(address)->select(value);
    if (keyword == "zero") {
        return 0;
    } else if (keyword == "one") {
        return 1;
    } else if (keyword == "two") {
        return 2;
    } else if (keyword == "few") {
        return 3;
    } else if (keyword == "many") {
        return 4;
    } else {
        return 5;
    }
}

static JNINativeMethod gMethods[] = {
    NATIVE_METHOD(NativePluralRules, finalizeImpl, "(J)V"),
    NATIVE_METHOD(NativePluralRules, forLocaleImpl, "(Ljava/lang/String;)J"),
    NATIVE_METHOD(NativePluralRules, quantityForIntImpl, "(JI)I"),
};
void register_libcore_icu_NativePluralRules(JNIEnv* env) {
    jniRegisterNativeMethods(env, "libcore/icu/NativePluralRules", gMethods, NELEM(gMethods));
}
