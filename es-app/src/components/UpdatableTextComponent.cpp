#include "components/UpdatableTextComponent.h"

#include "components/TextComponent.h"

#include "utils/StringUtil.h"
#include "Log.h"
#include "Settings.h"
#include <SDL_timer.h>

namespace UTC
{
	std::map<std::string, unsigned> startTimes;
}

UpdatableTextComponent::UpdatableTextComponent(Window* window) : TextComponent(window)
{
}


UpdatableTextComponent::UpdatableTextComponent(Window* window, const std::string& text, const std::shared_ptr<Font>& font, unsigned int color, Alignment align,
	Vector3f pos, Vector2f size, unsigned int bgcolor) : TextComponent(window, text, font, color, align, pos, size, bgcolor)
{
}

UpdatableTextComponent::~UpdatableTextComponent()
{
}

void UpdatableTextComponent::initializeStartTime(const std::string& id, unsigned startTime)
{
	UTC::startTimes[id] = startTime;
}

void UpdatableTextComponent::update(int deltaTime)
{
	if (isUpdatable())
		executeUpdateFunction(deltaTime);

	TextComponent::update(deltaTime);
}

void UpdatableTextComponent::setUpdatableFunction(const std::string& id, const std::function<void()>& updateFunction, int updateElapsedTime)
{
	if ((updateFunction == nullptr) || (updateElapsedTime < 100))
	{
		LOG(LogError) << "UpdatableTextComponent::setUpdatableFunction() - not valid update data, function is null or elapsed time < 100 ms";
		clearUpdateData();
		return;
	}
	mId = id;
	mUpdatable = true;
	mUpdateFunction = updateFunction;
	mUpdateElapsedTime = updateElapsedTime;
	initializeStartTime(id, SDL_GetTicks());
}

void UpdatableTextComponent::clearUpdateData()
{
	mUpdatable = false;
	mUpdateFunction = nullptr;
	mUpdateElapsedTime = 100;
	UTC::startTimes.erase(mId);
}

bool UpdatableTextComponent::isUpdatable() const
{
	return mUpdatable;
}

unsigned UpdatableTextComponent::getStartTime(const std::string& id) const
{
	return UTC::startTimes[id];
}

void UpdatableTextComponent::executeUpdateFunction(int deltaTime)
{
	unsigned mEndTime = SDL_GetTicks();
	unsigned mStartTime = getStartTime(mId);

	if ((mUpdateFunction != nullptr) && (mUpdateElapsedTime < (int) (mEndTime - mStartTime)))
	{
		Log::flush();
		mUpdateFunction();
		mEndTime = 0;
		initializeStartTime(mId, SDL_GetTicks());
	}
}
