/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>*/

#pragma once

#include "LuxBase.h"
#include <memory>

#include <graphics/ForgeHandles.h>
#include <graphics/Image.h>

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include <FixPreprocessor.h>

enum eLuxMainMenuWindow
{
	eLuxMainMenuWindow_Profiles,
	eLuxMainMenuWindow_Options,
	eLuxMainMenuWindow_KeyConfig,
	eLuxMainMenuWindow_LoadGame,
	eLuxMainMenuWindow_CustomStoryList,
	eLuxMainMenuWindow_CustomStory,

	//////////////
	// HARDMODE
	eLuxMainMenuWindow_StartGame,

	eLuxMainMenuWindow_LastEnum,
};

//----------------------------------------------

enum eLuxMainMenuExit
{
	eLuxMainMenuExit_QuitGame,
	eLuxMainMenuExit_ReturnToGame,
	eLuxMainMenuExit_StartGame,
	eLuxMainMenuExit_ContinueGame,
	eLuxMainMenuExit_QuitToMenu,
	eLuxMainMenuExit_QuitAndSave,
	eLuxMainMenuExit_LoadGame,

	eLuxMainMenuExit_LastEnum,
};

//----------------------------------------------

class iLuxMainMenuWindow
{
public:
	iLuxMainMenuWindow(cGuiSet *apGuiSet, cGuiSkin *apGuiSkin);
	virtual ~iLuxMainMenuWindow(){}

	virtual void CreateGui()=0;

	virtual void ExitPressed()=0;

	void SetActive(bool abX);

protected:
	virtual void OnSetActive(bool abX){}

	cGui *mpGui;

	cGuiSkin *mpGuiSkin;
	cGuiSet *mpGuiSet;

	cWidgetWindow *mpWindow;

	// cVector2f mvScreenSize;
};



//----------------------------------------------

class cLuxMainMenu : public iLuxUpdateable
{
public:

	cLuxMainMenu();
	~cLuxMainMenu();

	void LoadUserConfig();
	void SaveUserConfig();

	void LoadFonts();
	void OnClearFonts();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
    void OnQuit();

	void OnEnterContainer(const tString& asOldContainer);
	void OnLeaveContainer(const tString& asNewContainer);

	void OnDraw(float afFrameTime);
	void OnPostRender(float afFrameTime);

	cGuiSet* GetSet() { return mpGuiSet; }

	void SetWindowActive(eLuxMainMenuWindow aWindow);
	void SetTopMenuAlpha(float afX) { mfTopMenuAlpha = afX; }

	void ExitPressed();

	void RecreateGui(){ mbRecreateGui = true;}

	void AppLostInputFocus();
	void AppGotInputFocus();

	void ExitMenu(eLuxMainMenuExit aMessage);
	tWString msLoadGameFile;
#ifdef USE_GAMEPAD
	void AppDeviceWasPlugged();
	void AppDeviceWasRemoved();
#endif

private:
	///////////////////////
	// Helper methods

	void OnMenuExit();

	void UpdateBase(float afTimeStep);

	void UpdateTopMenu(float afTimeStep);
	void SetTopMenuVisible(bool abVisible);

	void CreateGui();

	void CreateTopMenuGui();
	void SetupTopMenuLabel(cWidgetLabel *apLabel);

	void CreateBackground();
    void UpdateBackground();

	void DestroyBackground();

	///////////////////////
	// Widget callbacks
    bool TopMenuTextMouseEnter(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(TopMenuTextMouseEnter);

	bool TopMenuTextMouseLeave(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(TopMenuTextMouseLeave);

	bool TopMenuTextPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(TopMenuTextPress);

	bool TopMenuTextDraw(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(TopMenuTextDraw);

	bool PressContinue(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressContinue);

	bool ClickedContinuePopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedContinuePopup);

	bool PressStartGame(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressStartGame);

	bool PressBackToGame(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressBackToGame);

	bool ClickedStartGamePopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedStartGamePopup);

	bool PressLoadGame(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressLoadGame);

	bool ClickedLoadGamePopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedLoadGamePopup);

	bool PressCustomStory(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressCustomStory);

	bool PressExit(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressExit);

	bool ClickedExitPopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedExitPopup);

	bool PressExitToMainMenu(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressExitToMainMenu);

	bool ClickedExitToMainMenuPopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedExitToMainMenuPopup);

	bool PressExitAndSave(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressExitAndSave);

	bool ClickedExitAndSavePopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedExitAndSavePopup);

	bool PressChangeProfile(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressChangeProfile);

	bool PressOptions(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressOptions);


	/////////////////////
	// HARDMODE
	// Callbacks
	bool PressSaveGame(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(PressSaveGame);

	bool ClickedSaveGamePopup(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ClickedSaveGamePopup);

	bool HardModeTextDraw(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(HardModeTextDraw);

	float mfDescriptionAlpha;
	bool mbFadeInDescription;

	cWidgetLabel * mpSaveCost;
	cWidgetLabel * mpNumTinderboxes;

	///////////////////////
	// Settings
	float mfMainFadeInTime;
	float mfMainFadeOutTimeFast;
	float mfMainFadeOutTimeSlow;

	float mfTopMenuFadeInTime;
	float mfTopMenuFadeOutTime;
	float mfTopMenuFontSizeMul;
	cVector2f mvTopMenuFontSize;
	cVector3f mvTopMenuStartPos;
	cVector3f mvTopMenuStartPosInGame;

	tString msMusic;
	tString msZoomSound;

	cVector2f mvLogoSize;
	cVector3f mvLogoPos;

	float mfBgCamera_FOV;
	float mfBgCamera_ZoomedFOV;

	///////////////////////
	// Variables
	cGui *mpGui;
	cScene *mpScene;
	cGraphics *mpGraphics;

	cGuiSkin *mpGuiSkin;
	cGuiSet *mpGuiSet;

	iFontData *mpFont;

	cViewport *mpViewport;

	std::shared_ptr<Image> m_screenImage;
	std::shared_ptr<Image> m_screenBlurImage;

    SharedRenderTarget m_screenBlurTarget;
    SharedRenderTarget m_screenTarget;

    cGuiGfxElement *mpScreenGfx;
	cGuiGfxElement *mpScreenBlurGfx;

    static constexpr uint32_t BlurSetSize = 64;
    Sampler* m_inputSampler;
    SharedShader m_blurShader;

    RootSignature* m_blurRootSignature;
    std::array<DescriptorSet*, ForgeRenderer::SwapChainLength> m_perFrameBlurDescriptorSet;
    uint32_t m_setIndex = 0;
    Pipeline* m_blurPipeline;

	cGuiGfxElement *mpLogoGfx;

	std::vector<iLuxMainMenuWindow*> mvWindows;
	eLuxMainMenuWindow mCurrentWindow;

	iWidget* mpLastFocusedItem;

	cWorld *mpBgWorld;
	cCamera *mpBgCamera;

	bool mbGuiCreated;
	cVector2f mvScreenSize;

	std::vector<cWidgetLabel*> mvTopMenuLabels;
	bool mbTopMenuVisible;
	float mfTopMenuAlpha;

	float mfMenuFadeAlpha;

	bool mbRecreateGui;
	bool mbExiting;
	eLuxMainMenuExit mExitMessage;

	cGuiGfxElement *mpBlackFade;
	cGuiGfxElement *mpTopBackground;

	float mfCamTimer;
};

//----------------------------------------------


