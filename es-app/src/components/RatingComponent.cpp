#include "components/RatingComponent.h"

#include "resources/TextureResource.h"
#include "ThemeData.h"
#include "EsLocale.h"
#include "Settings.h"

RatingComponent::RatingComponent(Window* window) : GuiComponent(window), mColorShift(0xFFFFFFFF), mUnfilledColor(0xFFFFFFFF)
{
	mFilledTexture = TextureResource::get(":/star_filled.svg", true);
	mUnfilledTexture = TextureResource::get(":/star_unfilled.svg", true);
	mValue = 0.5f;
	mSize = Vector2f(64 * NUM_RATING_STARS, 64);
	updateVertices();
	updateColors();
}

void RatingComponent::setValue(const std::string& value)
{
	float newValue = Math::clamp(0.0f, 1.0f, Utils::String::toFloat(value));
	if (mValue == newValue)
		return;

	mValue = newValue;
	updateVertices();
}

std::string RatingComponent::getValue() const
{
	// do not use std::to_string here as it will use the current locale
	// and that sometimes encodes decimals as commas
	std::stringstream ss;
	ss << mValue;
	return ss.str();
}

void RatingComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
	mColorShift = (mColorShift >> 8 << 8) | mOpacity;
	updateColors();
}

void RatingComponent::setColorShift(unsigned int color)
{
	mColorShift = color;
	// Grab the opacity from the color shift because we may need to apply it if
	// fading textures in
	mOpacity = color & 0xff;
	updateColors();
}

void RatingComponent::onSizeChanged()
{
	if(mSize.y() == 0)
		mSize[1] = mSize.x() / NUM_RATING_STARS;
	else if(mSize.x() == 0)
		mSize[0] = mSize.y() * NUM_RATING_STARS;

	if(mSize.y() > 0)
	{
		size_t heightPx = (size_t)Math::round(mSize.y());
		if (mFilledTexture)
			mFilledTexture->rasterizeAt(heightPx, heightPx);
		if(mUnfilledTexture)
			mUnfilledTexture->rasterizeAt(heightPx, heightPx);
	}

	updateVertices();
}

void RatingComponent::updateVertices()
{
	const float        numStars = NUM_RATING_STARS;
	const float        h        = getSize().y(); // is the same as a single star's width
	const float        w        = getSize().y() * mValue * numStars;
	const float        fw       = getSize().y() * numStars;
	const unsigned int color    = Renderer::convertColor(mColorShift);

	mVertices[0] = { { 0.0f, 0.0f }, { 0.0f,              1.0f }, color };
	mVertices[1] = { { 0.0f, h    }, { 0.0f,              0.0f }, color };
	mVertices[2] = { { w,    0.0f }, { mValue * numStars, 1.0f }, color };
	mVertices[3] = { { w,    h    }, { mValue * numStars, 0.0f }, color };

	mVertices[4] = { { 0.0f, 0.0f }, { 0.0f,              1.0f }, color };
	mVertices[5] = { { 0.0f, h    }, { 0.0f,              0.0f }, color };
	mVertices[6] = { { fw,   0.0f }, { numStars,          1.0f }, color };
	mVertices[7] = { { fw,   h    }, { numStars,          0.0f }, color };

	// round vertices
	for(int i = 0; i < 8; ++i)
		mVertices[i].pos.round();
}

void RatingComponent::updateColors()
{
	const unsigned int color = Renderer::convertColor(mColorShift);

	for(int i = 0; i < 8; ++i)
		mVertices[i].col = color;
}

void RatingComponent::render(const Transform4x4f& parentTrans)
{
	if (!isVisible())
		return;

	if (mFilledTexture == nullptr)
		mFilledTexture = TextureResource::get(":/star_filled.svg", true, true);

	if (mUnfilledTexture == nullptr)
		mUnfilledTexture = TextureResource::get(":/star_unfilled.svg", true, true);

	if (mFilledTexture == nullptr || mUnfilledTexture == nullptr)
		return;

	Transform4x4f trans = parentTrans * getTransform();
	

	Vector2f clipPos(trans.translation().x(), trans.translation().y());
	if (!Renderer::isVisibleOnScreen(clipPos.x(), clipPos.y(), mSize.x(), mSize.y()))
		return;

	Renderer::setMatrix(trans);

	if (Settings::getInstance()->getBool("DebugImage"))
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0xFFFF0033, 0xFFFF0033);

	if (mUnfilledTexture->bind())
	{
		if (mUnfilledColor != mColorShift)
		{
			const unsigned int color = Renderer::convertColor(mUnfilledColor);
			for (int i = 0; i < 8; ++i)
				mVertices[i].col = color;
		}

		Renderer::drawTriangleStrips(&mVertices[4], 4);
		Renderer::bindTexture(0);

		if (mUnfilledColor != mColorShift)
			updateColors();
	}

	if (mFilledTexture->bind())
	{
		Renderer::drawTriangleStrips(&mVertices[0], 4);
		Renderer::bindTexture(0);
	}
	
	renderChildren(trans);
}

bool RatingComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		mValue += 1.f / NUM_RATING_STARS;
		if(mValue > 1.0f)
			mValue = 0.0f;

		updateVertices();
	}

	return GuiComponent::input(config, input);
}

void RatingComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "rating");
	if(!elem)
		return;

	bool imgChanged = false;
	if(properties & PATH && elem->has("filledPath"))
	{
		mFilledTexture = TextureResource::get(elem->get<std::string>("filledPath"), true);
		imgChanged = true;
	}
	if(properties & PATH && elem->has("unfilledPath"))
	{
		mUnfilledTexture = TextureResource::get(elem->get<std::string>("unfilledPath"), true);
		imgChanged = true;
	}


	if (properties & COLOR)
	{
		if (elem->has("color"))
			setColorShift(elem->get<unsigned int>("color"));

		if (elem->has("unfilledColor"))
			mUnfilledColor = elem->get<unsigned int>("unfilledColor");
		else
			mUnfilledColor = mColorShift;
	}

	if(imgChanged)
		onSizeChanged();
}

std::vector<HelpPrompt> RatingComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", _("ADD STAR")));
	return prompts;
}
