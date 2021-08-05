#pragma once
#ifndef ES_APP_COMPONENTS_UPDATABLE_TEXT_COMPONENT_H
#define ES_APP_COMPONENTS_UPDATABLE_TEXT_COMPONENT_H

#include "resources/Font.h"
#include "components/TextComponent.h"

class TextComponent;

class UpdatableTextComponent : public TextComponent
{
public:
	UpdatableTextComponent(Window* window);
	UpdatableTextComponent(Window* window, const std::string& text, const std::shared_ptr<Font>& font, unsigned int color = 0x000000FF, Alignment align = ALIGN_LEFT, Vector3f pos = Vector3f::Zero(), Vector2f size = Vector2f::Zero(), unsigned int bgcolor = 0x00000000);

	~UpdatableTextComponent();

	virtual void update(int deltaTime);

	void setUpdatableFunction(const std::string& id, const std::function<void()>& updateFunction = nullptr, int updateElapsedTime = 100);

	void clearUpdateData();

	bool isUpdatable() const;

private:
	void initializeStartTime(const std::string& id, unsigned startTime);
	void executeUpdateFunction(int deltaTime);
	unsigned getStartTime(const std::string& id) const;

	std::string mId;
	bool mUpdatable;
	std::function<void()> mUpdateFunction;
	int mUpdateElapsedTime;

};

#endif // ES_APP_COMPONENTS_UPDATABLE_TEXT_COMPONENT_H