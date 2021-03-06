#ifndef RWE_UNIT_H
#define RWE_UNIT_H

#include "MovementClassId.h"
#include "UnitWeapon.h"
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <deque>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/DiscreteRect.h>
#include <rwe/PlayerId.h>
#include <rwe/SelectionMesh.h>
#include <rwe/UnitMesh.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/geometry/BoundingBox3f.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/pathfinding/UnitPath.h>

namespace rwe
{
    struct MoveOrder
    {
        Vector3f destination;
        explicit MoveOrder(const Vector3f& destination);
    };

    struct AttackOrder
    {
        UnitId target;
        explicit AttackOrder(UnitId target) : target(target) {}
    };

    struct AttackGroundOrder
    {
        Vector3f target;
        explicit AttackGroundOrder(const Vector3f& target) : target(target) {}
    };

    struct PathFollowingInfo
    {
        UnitPath path;
        GameTime pathCreationTime;
        std::vector<Vector3f>::const_iterator currentWaypoint;
        explicit PathFollowingInfo(UnitPath&& path, GameTime creationTime)
            : path(std::move(path)), pathCreationTime(creationTime), currentWaypoint(this->path.waypoints.begin()) {}
    };

    using MovingStateGoal = boost::variant<Vector3f, DiscreteRect>;

    struct MovingState
    {
        MovingStateGoal destination;
        boost::optional<PathFollowingInfo> path;
        bool pathRequested;
    };

    struct IdleState
    {
    };

    using UnitState = boost::variant<IdleState, MovingState>;

    using UnitOrder = boost::variant<MoveOrder, AttackOrder, AttackGroundOrder>;

    UnitOrder createMoveOrder(const Vector3f& destination);

    UnitOrder createAttackOrder(UnitId target);

    UnitOrder createAttackGroundOrder(const Vector3f& target);

    class Unit
    {
    public:
        UnitMesh mesh;
        Vector3f position;
        std::unique_ptr<CobEnvironment> cobEnvironment;
        SelectionMesh selectionMesh;
        boost::optional<AudioService::SoundHandle> selectionSound;
        boost::optional<AudioService::SoundHandle> okSound;
        boost::optional<AudioService::SoundHandle> arrivedSound;
        PlayerId owner;

        /**
         * Anticlockwise rotation of the unit around the Y axis in radians.
         * The other two axes of rotation are normally determined
         * by the normal of the terrain the unit is standing on.
         */
        float rotation{0.0f};


        /**
         * Rate at which the unit turns in rads/tick.
         */
        float turnRate;

        /**
         * Rate at which the unit is travelling forwards in game units/tick.
         */
        float currentSpeed{0.0f};

        /**
         * Maximum speed the unit can travel forwards in game units/tick.
         */
        float maxSpeed;

        /**
         * Speed at which the unit accelerates in game units/tick.
         */
        float acceleration;

        /**
         * Speed at which the unit brakes in game units/tick.
         */
        float brakeRate;

        /** The angle we are trying to steer towards. */
        float targetAngle{0.0f};

        /** The speed we are trying to accelerate/decelerate to */
        float targetSpeed{0.0f};

        boost::optional<MovementClassId> movementClass;

        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;

        std::deque<UnitOrder> orders;
        UnitState behaviourState;

        /**
         * True if the unit attempted to move last frame
         * and its movement was limited (or prevented entirely) by a collision.
         */
        bool inCollision{false};

        std::vector<UnitWeapon> weapons;

        bool canAttack;

        Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, SelectionMesh&& selectionMesh);

        void moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed);

        void moveObjectNow(const std::string& pieceName, Axis axis, float targetPosition);

        void turnObject(const std::string& pieceName, Axis axis, RadiansAngle targetAngle, float speed);

        void turnObjectNow(const std::string& pieceName, Axis axis, RadiansAngle targetAngle);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        bool isTurnInProgress(const std::string& pieceName, Axis axis) const;

        /**
         * Returns a value if the given ray intersects this unit
         * for the purposes of unit selection.
         * The value returned is the distance along the ray
         * where the intersection occurred.
         */
        boost::optional<float> selectionIntersect(const Ray3f& ray) const;

        bool isOwnedBy(PlayerId playerId) const;

        void clearOrders();

        void addOrder(const UnitOrder& order);

        void setWeaponTarget(unsigned int weaponIndex, UnitId target);
        void setWeaponTarget(unsigned int weaponIndex, const Vector3f& target);
        void clearWeaponTarget(unsigned int weaponIndex);
        void clearWeaponTargets();
    };
}

#endif
