#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_VIDEO_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_VIDEO_GAME_LIST_VIEW_H

#include "components/DateTimeComponent.h"
#include "components/RatingComponent.h"
#include "components/ScrollableContainer.h"
#include "views/gamelist/BasicGameListView.h"

class VideoComponent;

class VideoGameListView : public BasicGameListView
{
public:
	VideoGameListView(Window* window, FolderData* root);
	virtual ~VideoGameListView();

	virtual void onShow() override;

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual const char* getName() const override 
	{ 
		if (!mCustomThemeName.empty())
			return mCustomThemeName.c_str();

		return "video"; 
	}

	virtual void launch(FileData* game) override;

protected:
	virtual void update(int deltaTime) override;

private:
	void updateInfoPanel();
	void createImage();
	void createThumbnail();

	void initMDLabels();
	void initMDValues();

	ImageComponent *mImage,
									*mThumbnail,
									mMarquee;
	VideoComponent *mVideo;

	TextComponent mLblRating,
								mLblReleaseDate,
								mLblDeveloper,
								mLblPublisher,
								mLblGenre,
								mLblPlayers,
								mLblLastPlayed,
								mLblPlayCount,
								mLblGameTime,

								mDeveloper,
								mPublisher,
								mGenre,
								mPlayers,
								mPlayCount,
								mGameTime,
								mName;

	RatingComponent mRating;
	DateTimeComponent mReleaseDate,
										mLastPlayed;

	std::vector<TextComponent*> getMDLabels();
	std::vector<GuiComponent*> getMDValues();

	ScrollableContainer mDescContainer;
	TextComponent mDescription;

	bool		mVideoPlaying;

};

#endif // ES_APP_VIEWS_GAME_LIST_VIDEO_GAME_LIST_VIEW_H
