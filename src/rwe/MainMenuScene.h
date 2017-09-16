#ifndef RWE_MAINMENUSCENE_H
#define RWE_MAINMENUSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/tdf/SimpleTdfAdapter.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/ui/UiFactory.h>

namespace rwe
{
    class MainMenuScene : public SceneManager::Scene
    {
    private:
        SceneManager* sceneManager;
        AbstractVirtualFileSystem* vfs;
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;
        CursorService* cursor;

        MainMenuModel model;
        UiFactory uiFactory;

        std::vector<std::unique_ptr<UiPanel>> panelStack;
        std::vector<std::unique_ptr<UiPanel>> dialogStack;
        UiCamera camera;

        AudioService::LoopToken bgm;

    public:
        MainMenuScene(
            SceneManager* sceneManager,
            AbstractVirtualFileSystem* vfs,
            TextureService* textureService,
            AudioService* audioService,
            TdfBlock* audioLookup,
            CursorService* cursor,
            float width,
            float height);

        // "this" is passed to the owned UiFactory instance,
        // and this would not be correctly updated
        // if we were copied or moved.
        MainMenuScene(const MainMenuScene&) = delete;
        MainMenuScene& operator=(const MainMenuScene&) = delete;
        MainMenuScene(MainMenuScene&&) = delete;
        MainMenuScene& operator=(MainMenuScene&&) = delete;

        void init() override;

        void render(GraphicsContext& context) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void onMouseWheel(MouseWheelEvent event) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void update() override;

        void goToPreviousMenu();

        void goToMenu(std::unique_ptr<UiPanel>&& panel);

        void openDialog(std::unique_ptr<UiPanel>&& panel);

        void goToMainMenu();

        void goToSingleMenu();

        void goToSkirmishMenu();

        void openMapSelectionDialog();

        void exit();

        void message(const std::string& topic, const std::string& message);

        void scrollMessage(const std::string& topic, unsigned int group, const std::string& name, const ScrollPositionMessage& message);

        void scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name);

        void scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name);

        void setCandidateSelectedMap(const std::string& mapName);

        void clearCandidateSelectedMap();

        void commitSelectedMap();

        void resetCandidateSelectedMap();

        void togglePlayer(int playerIndex);

        void incrementPlayerMetal(int playerIndex);

        void decrementPlayerMetal(int playerIndex);

        void incrementPlayerEnergy(int playerIndex);

        void decrementPlayerEnergy(int playerIndex);

        void togglePlayerSide(int playerIndex);

        void cyclePlayerColor(int playerIndex);

        void reverseCyclePlayerColor(int playerIndex);

        void cyclePlayerTeam(int playerIndex);

        void startGame();

    private:
        AudioService::LoopToken startBgm();

        UiPanel& topPanel();
    };
}

#endif
