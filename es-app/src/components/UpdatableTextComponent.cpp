#include "components/UpdatableTextComponent.h"

#include "components/TextComponent.h"

#include "utils/StringUtil.h"
#include "Log.h"
#include "Settings.h"


UpdatableTextComponent::UpdatableTextComponent(Window* window) : TextComponent(window)
{
	mCheckTime = 0;
}


UpdatableTextComponent::UpdatableTextComponent(Window* window, const std::string& text, const std::shared_ptr<Font>& font, unsigned int color, Alignment align,
	Vector3f pos, Vector2f size, unsigned int bgcolor) : TextComponent(window, text, font, color, align, pos, size, bgcolor)
{
	mCheckTime = 0;
}

UpdatableTextComponent::~UpdatableTextComponent()
{
}

void UpdatableTextComponent::update(int deltaTime)
{
	TextComponent::update(deltaTime);

	if (isUpdatable())
		executeUpdateFunction(deltaTime);
}

void UpdatableTextComponent::setUpdatableFunction(const std::function<void()>& updateFunction, int updateElapsedTime)
{
	if ((updateFunction == nullptr) || (updateElapsedTime < 100))
	{
		LOG(LogError) << "UpdatableTextComponent::setUpdatableFunction() - not valid update data, function is null or elapsed time < 100 ms";
		clearUpdateData();
		return;
	}
	mUpdatable = true;
	mUpdateFunction = updateFunction;
	mUpdateElapsedTime = updateElapsedTime;
}

void UpdatableTextComponent::clearUpdateData()
{
	mUpdatable = false;
	mUpdateFunction = nullptr;
	mUpdateElapsedTime = 100;
	mCheckTime = 0;
}

bool UpdatableTextComponent::isUpdatable() const
{
	return mUpdatable;
}

void UpdatableTextComponent::executeUpdateFunction(int deltaTime)
{
	mCheckTime += deltaTime;
	if (mCheckTime < mUpdateElapsedTime)
		return;

	mCheckTime = 0;
	mUpdateFunction();
}
