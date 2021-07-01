#pragma once

#ifndef ES_CORE_ESLOCALE_H
#define ES_CORE_ESLOCALE_H

#include <string>
#include <map>
#include <functional>
#include "utils/StringUtil.h"
#include <fstream>

struct PluralRule
{
	std::string key;
	std::string rule;
	std::function<int(int n)> evaluate;
};

class EsLocale
{
public:
	static const std::string getText(const std::string msgid);
	static const std::string nGetText(const std::string msgid, const std::string msgid_plural, int n);

	static const std::string getLanguage() { return mCurrentLanguage; }

	static const void reset() { mCurrentLanguageLoaded = false; }

private:
	static void checkLocalisationLoaded();
	static const void readFileSplitedValues(std::ifstream &file, std::string &msg, std::string &line_next);
	static std::map<std::string, std::string> mItems;
	static std::string mCurrentLanguage;
	static bool mCurrentLanguageLoaded;

	static PluralRule mPluralRule;
};


#if defined(_WIN32)
	#define UNICODE_CHARTYPE wchar_t*
	#define _L(x) L ## x
	#define _U(x) Utils::String::convertFromWideString(L ## x)
	
	#define _(x) EsLocale::getText(x)
#else

	#define UNICODE_CHARTYPE char*
	#define _L(x) x
	#define _U(x) x

	#define _(x) EsLocale::getText(x)
#endif // _WIN32

#endif // ES_CORE_ESLOCALE_H
