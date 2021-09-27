#include "RetroAchievements.h"
#include "HttpReq.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/pointer.h>
#include "EsLocale.h"


bool RetroAchievements::testAccount(const std::string& username, const std::string& password, std::string& error)
{
	if (username.empty() || password.empty())
	{
		error = _("A valid account is required. Please register an account on https://retroachievements.org");
		return false;
	}

	std::map<std::string, std::string> ret;

	try
	{
		HttpReq request("https://retroachievements.org/dorequest.php?r=login&u=" + HttpReq::urlEncode(username) + "&p=" + HttpReq::urlEncode(password));
		if (!request.wait())
		{
			error = request.getErrorMsg();
			return false;
		}

		rapidjson::Document ogdoc;
		ogdoc.Parse(request.getContent().c_str());
		if (ogdoc.HasParseError() || !ogdoc.HasMember("Success"))
		{
			error = "Unable to parse response";
			return false;
		}

		if (ogdoc["Success"].IsTrue())
			return true;

		if (ogdoc.HasMember("Error"))
			error = ogdoc["Error"].GetString();
	}
	catch (...)
	{
		error = "Unknown error";
	}

	return false;
}
