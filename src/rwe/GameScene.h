#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/Unit.h>
#include <rwe/camera/UiCamera.h>
#include "MeshService.h"
#include "UnitDatabase.h"
#include "AudioService.h"
#include <rwe/ViewportService.h>

namespace rwe
{
    struct GamePlayerInfo
    {
        unsigned int color;
    };

    class GameScene : public SceneManager::Scene
    {
    private:
        static const unsigned int UnitSelectChannel = 0;

        static const unsigned int reservedChannelsCount = 1;

        /**
         * Speed the camera pans via the arrow keys
         * in world units/second.
         */
        static constexpr float CameraPanSpeed = 1000.0f;

        TextureService* textureService;
        CursorService* cursor;
        SdlContext* sdl;
        AudioService* audioService;
        ViewportService* viewportService;

        MeshService meshService;

        CabinetCamera camera;
        MapTerrain terrain;

        std::vector<Unit> units;

        UiCamera uiCamera;

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

        SharedShaderProgramHandle unitTextureShader;
        SharedShaderProgramHandle unitColorShader;

        SharedShaderProgramHandle selectBoxShader;

        UnitDatabase unitDatabase;

        unsigned int gameTime{0};

        std::array<boost::optional<GamePlayerInfo>, 10> players;

        unsigned int localPlayerId;

        boost::optional<unsigned int> hoveredUnit;
        boost::optional<unsigned int> selectedUnit;

    public:
        GameScene(
            TextureService* textureService,
            CursorService* cursor,
            SdlContext* sdl,
            AudioService* audioService,
            ViewportService* viewportService,
            MeshService&& meshService,
            CabinetCamera&& camera,
            MapTerrain&& terrain,
            SharedShaderProgramHandle&& unitTextureShader,
            SharedShaderProgramHandle&& unitColorShader,
            SharedShaderProgramHandle&& selectBoxShader,
            UnitDatabase&& unitDatabase,
            std::array<boost::optional<GamePlayerInfo>, 10>&& players,
            unsigned int localPlayerId);

        void init() override;

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void update() override;

        void spawnUnit(const std::string& unitType, unsigned int owner, const Vector3f& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

        void showObject(unsigned int unitId, const std::string& name);

        void hideObject(unsigned int unitId, const std::string& name);

        void moveObject(unsigned int unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(unsigned int unitId, const std::string& name, Axis axis, float position);

        void turnObject(unsigned int unitId, const std::string& name, Axis axis, float angle, float speed);

        void turnObjectNow(unsigned int unitId, const std::string& name, Axis axis, float angle);

        bool isPieceMoving(unsigned int unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(unsigned int unitId, const std::string& name, Axis axis) const;

        unsigned int getGameTime() const;

    private:
        Unit createUnit(unsigned int unitId, const std::string& unitType, unsigned int owner, const Vector3f& position);

        boost::optional<unsigned int> getUnitUnderCursor() const;

        Vector2f screenToClipSpace(Point p) const;

        Point getMousePosition() const;

        boost::optional<unsigned int> getFirstCollidingUnit(const Ray3f& ray) const;

        boost::optional<Vector3f> getMouseTerrainCoordinate() const;

        void issueMoveOrder(unsigned int unitId, Vector3f position);

        void stopSelectedUnit();
    };
}

#endif
