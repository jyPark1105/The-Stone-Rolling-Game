#ifndef OURBULLETS_H
#define OURBULLETS_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include "scene.h"

extern btRigidBody* groundBody;
extern btRigidBody* groundWallBodies[5];
extern btRigidBody* slopeBody;
extern btRigidBody* slopeEdgeBodyLeft;
extern btRigidBody* slopeEdgeBodyRight;
extern btRigidBody* characterBody;
extern btRigidBody* ballBody;
extern std::vector<btRigidBody*> obstacleBodies;
extern std::vector<btRigidBody*> ramBodies;

extern float gCharacterRadius;
extern float gCapsuleHeight;
extern float gTotalColliderHeight;

// ----------------------------------------------- Add to Bullet Physics -----------------------------------------------
inline void addGroundToBullet(btDiscreteDynamicsWorld* dynamicsWorld, float gSize)
{
    btCollisionShape* groundShape = new btBoxShape(btVector3(gSize, 0.1f, gSize));
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.0f, 0.0f, 0.0f)));

    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
        0.0f, groundMotionState, groundShape, btVector3(0, 0, 0));

    groundBody = new btRigidBody(groundRigidBodyCI);
    groundBody->setCollisionFlags(groundBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    groundBody->setFriction(1.0f);

    dynamicsWorld->addRigidBody(groundBody);
}

inline void addGroundWallToBullet(btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 wallPos, glm::vec3 wallSize, int idx)
{
    btCollisionShape* groundShape = new btBoxShape(btVector3(wallSize.x, wallSize.y, wallSize.z));
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(wallPos.x, wallPos.y, wallPos.z)));

    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
        0.0f, groundMotionState, groundShape, btVector3(0, 0, 0));

    groundWallBodies[idx] = new btRigidBody(groundRigidBodyCI);
    groundWallBodies[idx]->setCollisionFlags(groundWallBodies[idx]->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    groundWallBodies[idx]->setFriction(1.0f);
    groundWallBodies[idx]->setRestitution(0.4f);

    dynamicsWorld->addRigidBody(groundWallBodies[idx]);
}

inline void addSlopeToBullet(btDiscreteDynamicsWorld* dynamicsWorld, float xsize, float zSize)
{
    btBoxShape* boxShape = new btBoxShape(btVector3(xsize, 0.01f, zSize));
    btCompoundShape* compoundShape = new btCompoundShape();

    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::sin(glm::radians(15.0f))));
    compoundShape->addChildShape(localTransform, boxShape);

    btTransform groundTransform;
    groundTransform.setIdentity();

    float slopeOffset = 0.05f;
    groundTransform.setOrigin(btVector3(0.0f, zSize * glm::sin(glm::radians(15.0f)) - 0.2f, -20.0f - zSize * glm::cos(glm::radians(15.0f)) + 0.1f));


    btDefaultMotionState* slopeMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo slopeRigidBodyCI(0.0f, slopeMotionState, compoundShape, btVector3(0, 0, 0));

    slopeBody = new btRigidBody(slopeRigidBodyCI);
    slopeBody->setCollisionFlags(slopeBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    slopeBody->setFriction(0.8f);

    dynamicsWorld->addRigidBody(slopeBody);
}

inline void addSlopeLeftEdgeToBullet(btDiscreteDynamicsWorld* dynamicsWorld, float xOffset, float zSize)
{
    btBoxShape* boxShape = new btBoxShape(btVector3(0.15f, 1.0f, zSize + 0.3f));
    btCompoundShape* compoundShape = new btCompoundShape();

    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::sin(glm::radians(15.0f))));
    compoundShape->addChildShape(localTransform, boxShape);

    btTransform groundTransform;
    groundTransform.setIdentity();

    float slopeOffset = 0.15f;
    float edgeOffset = 0.15f;   // Wall.x: 0.3f
    groundTransform.setOrigin(btVector3(xOffset + edgeOffset,
        (zSize + 0.3f) * glm::sin(glm::radians(15.0f)) + 0.5f + slopeOffset,
        -20.0f - zSize * glm::cos(glm::radians(15.0f))));   // Wall.z: 40.f

    btDefaultMotionState* slopeMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo slopeRigidBodyCI(0.0f, slopeMotionState, compoundShape, btVector3(0, 0, 0));

    slopeEdgeBodyLeft = new btRigidBody(slopeRigidBodyCI);
    slopeEdgeBodyLeft->setCollisionFlags(slopeEdgeBodyLeft->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    slopeEdgeBodyLeft->setFriction(0.8f);
    slopeEdgeBodyLeft->setRestitution(0.4f);

    dynamicsWorld->addRigidBody(slopeEdgeBodyLeft);
}

inline void addSlopeRightEdgeToBullet(btDiscreteDynamicsWorld* dynamicsWorld, float xOffset, float zSize)
{
    btBoxShape* boxShape = new btBoxShape(btVector3(0.15f, 1.0f, zSize + 0.3f));
    btCompoundShape* compoundShape = new btCompoundShape();

    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::sin(glm::radians(15.0f))));
    compoundShape->addChildShape(localTransform, boxShape);

    btTransform groundTransform;
    groundTransform.setIdentity();

    float slopeOffset = 0.15f;
    float edgeOffset = 0.15f;
    groundTransform.setOrigin(btVector3(xOffset - edgeOffset,
        (zSize + 0.3f) * glm::sin(glm::radians(15.0f)) + 0.5f + slopeOffset,
        -20.0f - zSize * glm::cos(glm::radians(15.0f))));

    btDefaultMotionState* slopeMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo slopeRigidBodyCI(0.0f, slopeMotionState, compoundShape, btVector3(0, 0, 0));

    slopeEdgeBodyRight = new btRigidBody(slopeRigidBodyCI);
    slopeEdgeBodyRight->setCollisionFlags(slopeEdgeBodyRight->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    slopeEdgeBodyRight->setFriction(0.8f);
    slopeEdgeBodyRight->setRestitution(0.4f);

    dynamicsWorld->addRigidBody(slopeEdgeBodyRight);
}

inline void addObstacleToBullet(btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 position, glm::vec3 halfSize, int index)
{
    btCollisionShape* boxShape = new btBoxShape(btVector3(halfSize.x * 1.5f, halfSize.y * 0.8f, halfSize.z * 1.1f));
    btCompoundShape* compoundShape = new btCompoundShape();

    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::sin(glm::radians(15.0f))));
    compoundShape->addChildShape(localTransform, boxShape);

    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(position.x + 0.2f, position.y + 0.65f, position.z + 0.5f));

    btDefaultMotionState* obsMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo obsRigidBodyCI(0.0f, obsMotionState, compoundShape, btVector3(0, 0, 0));
    btRigidBody* body = new btRigidBody(obsRigidBodyCI);
    body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

    dynamicsWorld->addRigidBody(body);

    if (obstacleBodies.size() <= index)
        obstacleBodies.resize(index + 1);

    obstacleBodies[index] = body;
}


inline void switchCharacterColliderXZExpanded_BoxVersion(btDiscreteDynamicsWorld* dynamicsWorld, bool useExtended)
{
    btCollisionShape* newShape = nullptr;

    if (useExtended)
    {
        float halfX = gCharacterRadius * 3.0f;
        float halfY = (gCapsuleHeight + 2.0f * gCharacterRadius) * 0.4f;
        float halfZ = gCharacterRadius * 3.0f;

        newShape = new btBoxShape(btVector3(halfX, halfY, halfZ));
    }
    else
    {
        newShape = new btCapsuleShape(gCharacterRadius, gCapsuleHeight);
    }

    btTransform transform;
    characterBody->getMotionState()->getWorldTransform(transform);
    btVector3 velocity = characterBody->getLinearVelocity();
    btVector3 angular = characterBody->getAngularVelocity();

    dynamicsWorld->removeRigidBody(characterBody);
    delete characterBody->getMotionState();
    delete characterBody->getCollisionShape();
    delete characterBody;

    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btScalar mass = 5.0f;
    btVector3 inertia(0, 0, 0);
    newShape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, newShape, inertia);
    characterBody = new btRigidBody(rbInfo);

    characterBody->setAngularFactor(btVector3(0, 1, 0));
    characterBody->setLinearFactor(btVector3(1, 0.8f, 1));
    characterBody->setRestitution(0.0f);
    characterBody->setFriction(1.0f);
    characterBody->setCcdMotionThreshold(0.01);
    characterBody->setCcdSweptSphereRadius(gCharacterRadius * 0.5f);
    characterBody->setContactProcessingThreshold(BT_LARGE_FLOAT);
    characterBody->setActivationState(DISABLE_DEACTIVATION);
    characterBody->setLinearVelocity(velocity);
    characterBody->setAngularVelocity(angular);

    dynamicsWorld->addRigidBody(characterBody);
}

inline void addCharacterToBullet(btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 characterWorldSize, glm::vec3 capsuleHalfExtents, Node* character)
{
    gCharacterRadius = std::min(capsuleHalfExtents.x, capsuleHalfExtents.z);
    gCapsuleHeight = characterWorldSize.y - 2.0f * gCharacterRadius;
    if (gCapsuleHeight < 0.01f) gCapsuleHeight = 0.01f;
    gTotalColliderHeight = gCapsuleHeight + 2.0f * gCharacterRadius;

    float colliderCenterY = gTotalColliderHeight * 0.5f;
    float modelShiftY = -characterWorldSize.y * 0.5f;
    character->position = glm::vec3(0.0f, colliderCenterY + modelShiftY, 0.0f);

    btCollisionShape* capsuleShape = new btCapsuleShape(gCharacterRadius, gCapsuleHeight);
    capsuleShape->setMargin(0.0f);

    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.0f, colliderCenterY, 0.0f)));

    btScalar mass = 5.0f;
    btVector3 inertia(0, 0, 0);
    capsuleShape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, capsuleShape, inertia);
    characterBody = new btRigidBody(rbInfo);

    characterBody->setAngularFactor(btVector3(0, 1, 0));
    characterBody->setLinearFactor(btVector3(1, 0.8f, 1));
    characterBody->setRestitution(0.0f);
    characterBody->setFriction(1.0f);
    characterBody->setCcdMotionThreshold(0.01);
    characterBody->setCcdSweptSphereRadius(gCharacterRadius * 0.5f);
    characterBody->setContactProcessingThreshold(BT_LARGE_FLOAT);
    characterBody->setActivationState(DISABLE_DEACTIVATION);

    dynamicsWorld->addRigidBody(characterBody);
}

inline void addRamToBullet(btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 position, glm::vec3 halfSize, int index)
{
    btCollisionShape* boxShape = new btBoxShape(btVector3(halfSize.x + 0.6f, halfSize.y + 0.5f, halfSize.z + 1.5f));
    btCompoundShape* compoundShape = new btCompoundShape();

    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::sin(glm::radians(15.0f))));

    compoundShape->addChildShape(localTransform, boxShape);

    btTransform worldTransform;
    worldTransform.setIdentity();

    float offsetX = 0.125f;
    std::vector<float> offsetY = { -5.5f, -5.75f, -6.0f, -6.0f, -6.5f, -6.5f };
    std::vector<float> offsetZ = { 4.375f, 5.125f, 5.5f, 5.5f, 7.25f, 7.25f };
    worldTransform.setOrigin(btVector3(position.x + offsetX,
        glm::tan(glm::radians(15.0f)) * glm::abs(position.z) + halfSize.y + offsetY[index],
        position.z + offsetZ[index]));

    btDefaultMotionState* motionState = new btDefaultMotionState(worldTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, compoundShape, btVector3(0, 0, 0));
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

    dynamicsWorld->addRigidBody(body);

    if (ramBodies.size() <= index)
        ramBodies.resize(index + 1);
    ramBodies[index] = body;
}

inline void addSphereToBullet(btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 position, glm::vec3 halfExtents)
{
    float radius = 2.0f * std::max({ halfExtents.x, halfExtents.y, halfExtents.z });

    btCollisionShape* sphereShape = new btSphereShape(radius); 
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));

    btScalar mass = 15.0f;
    btVector3 inertia(0, 0, 0);
    sphereShape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, sphereShape, inertia);
    ballBody = new btRigidBody(rbInfo);


    ballBody->setRestitution(0.3f);
    ballBody->setFriction(0.6f);
    ballBody->setRollingFriction(0.0f);
    ballBody->setSpinningFriction(0.0f);
    ballBody->setAngularFactor(btVector3(1, 1, 1));
    ballBody->setCcdMotionThreshold(0.01);
    ballBody->setCcdSweptSphereRadius(radius * 0.5f);
    ballBody->setActivationState(DISABLE_DEACTIVATION);

    dynamicsWorld->addRigidBody(ballBody);
}

inline void resetBallPositionToInitial(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& initialPosition, glm::vec3& halfExtents)
{
    dynamicsWorld->removeRigidBody(ballBody);
    delete ballBody->getMotionState();
    delete ballBody->getCollisionShape();
    delete ballBody;

    float radius = 2.0f * std::max({ halfExtents.x, halfExtents.y, halfExtents.z });

    btCollisionShape* sphereShape = new btSphereShape(radius);
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(initialPosition.x, initialPosition.y, initialPosition.z)));

    btScalar mass = 15.0f;
    btVector3 inertia(0, 0, 0);
    sphereShape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, sphereShape, inertia);
    ballBody = new btRigidBody(rbInfo);


    ballBody->setRestitution(0.3f);
    ballBody->setFriction(0.6f);
    ballBody->setRollingFriction(0.0f);
    ballBody->setSpinningFriction(0.0f);
    ballBody->setAngularFactor(btVector3(1, 1, 1));
    ballBody->setCcdMotionThreshold(0.01);
    ballBody->setCcdSweptSphereRadius(radius * 0.5f);
    ballBody->setActivationState(DISABLE_DEACTIVATION);

    dynamicsWorld->addRigidBody(ballBody);
}

// ------------------------------------------ Collider Root Function ------------------------------------------
inline void drawBox(btBoxShape* box, const btTransform& trans, btIDebugDraw* drawer)
{
    btVector3 h = box->getHalfExtentsWithMargin();
    btVector3 v[8] = {
       { -h.x(), -h.y(), -h.z() }, { h.x(), -h.y(), -h.z() },
       { h.x(),  h.y(), -h.z() }, { -h.x(),  h.y(), -h.z() },
       { -h.x(), -h.y(),  h.z() }, { h.x(), -h.y(),  h.z() },
       { h.x(),  h.y(),  h.z() }, { -h.x(),  h.y(),  h.z() }
    };

    for (int i = 0; i < 8; ++i) v[i] = trans * v[i];
    btVector3 c(0.5, 0, 0.5);
    int e[12][2] = { {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7} };
    for (int i = 0; i < 12; ++i) drawer->drawLine(v[e[i][0]], v[e[i][1]], c);
}

// ------------------------------------------ DEBUG Collider ------------------------------------------
inline void drawCapsuleCollider(btRigidBody* body, btIDebugDraw* drawer, const btVector3& color)
{
    if (!body || !drawer) return;

    btCollisionShape* shape = body->getCollisionShape();
    if (!shape || shape->getShapeType() != CAPSULE_SHAPE_PROXYTYPE) return;

    btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
    btTransform t; body->getMotionState()->getWorldTransform(t);
    btVector3 o = t.getOrigin();

    float r = capsule->getRadius();
    float h = capsule->getHalfHeight() * 2.0f;
    float total = h + 2.0f * r;

    btVector3 bottom = o - btVector3(0, total / 2.0f, 0);
    btVector3 top = o + btVector3(0, total / 2.0f, 0);

    drawer->drawLine(bottom, top, color);

    btVector3 min = o - btVector3(r, h / 2 + r, r);
    btVector3 max = o + btVector3(r, h / 2 + r, r);
    btVector3 v[8] = {
       {min.x(), min.y(), min.z()}, {max.x(), min.y(), min.z()},
       {max.x(), max.y(), min.z()}, {min.x(), max.y(), min.z()},
       {min.x(), min.y(), max.z()}, {max.x(), min.y(), max.z()},
       {max.x(), max.y(), max.z()}, {min.x(), max.y(), max.z()}
    };
    int e[12][2] = { {0,1},{1,5},{5,4},{4,0}, {3,2},{2,6},{6,7},{7,3}, {0,3},{1,2},{5,6},{4,7} };
    for (int i = 0; i < 12; ++i) drawer->drawLine(v[e[i][0]], v[e[i][1]], color);
}

inline void drawGroundCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;
    btCollisionShape* shape = body->getCollisionShape();
    if (!shape || shape->getShapeType() != BOX_SHAPE_PROXYTYPE) return;
    drawBox(static_cast<btBoxShape*>(shape), body->getWorldTransform(), drawer);
}

inline void drawGroundWallCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;
    btCollisionShape* shape = body->getCollisionShape();
    if (!shape || shape->getShapeType() != BOX_SHAPE_PROXYTYPE) return;
    drawBox(static_cast<btBoxShape*>(shape), body->getWorldTransform(), drawer);
}

inline void drawSlopeCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;
    btCollisionShape* shape = body->getCollisionShape();
    if (!shape) return;

    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
    {
        btCompoundShape* compound = static_cast<btCompoundShape*>(shape);
        for (int i = 0; i < compound->getNumChildShapes(); ++i)
        {
            btCollisionShape* child = compound->getChildShape(i);
            btTransform childTransform = worldTransform * compound->getChildTransform(i);
            if (child->getShapeType() == BOX_SHAPE_PROXYTYPE)
                drawBox(static_cast<btBoxShape*>(child), childTransform, drawer);
        }
    }
    else if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE)
    {
        drawBox(static_cast<btBoxShape*>(shape), worldTransform, drawer);
    }
}

inline void drawSlopeEdgeCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;
    btCollisionShape* shape = body->getCollisionShape();
    if (!shape) return;

    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
    {
        btCompoundShape* compound = static_cast<btCompoundShape*>(shape);
        for (int i = 0; i < compound->getNumChildShapes(); ++i)
        {
            btCollisionShape* child = compound->getChildShape(i);
            btTransform childTransform = worldTransform * compound->getChildTransform(i);
            if (child->getShapeType() == BOX_SHAPE_PROXYTYPE)
                drawBox(static_cast<btBoxShape*>(child), childTransform, drawer);
        }
    }
    else if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE)
    {
        drawBox(static_cast<btBoxShape*>(shape), worldTransform, drawer);
    }
}

inline void drawSphereCollider(btRigidBody* body, btIDebugDraw* drawer, const btVector3& color)
{
    if (!body || !drawer) return;

    btCollisionShape* shape = body->getCollisionShape();
    if (!shape || shape->getShapeType() != SPHERE_SHAPE_PROXYTYPE) return;

    btSphereShape* sphere = static_cast<btSphereShape*>(shape);
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);

    btVector3 center = transform.getOrigin();
    float radius = sphere->getRadius();

    // ������ wireframe sphere ����: 3���� �� (XY, YZ, ZX ���)
    const int segments = 16;
    for (int i = 0; i < segments; ++i)
    {
        float theta1 = (float)i / segments * btScalar(2.0 * SIMD_PI);
        float theta2 = (float)(i + 1) / segments * btScalar(2.0 * SIMD_PI);

        // XY
        btVector3 p1 = center + btVector3(radius * cos(theta1), radius * sin(theta1), 0);
        btVector3 p2 = center + btVector3(radius * cos(theta2), radius * sin(theta2), 0);
        drawer->drawLine(p1, p2, color);

        // YZ
        btVector3 p3 = center + btVector3(0, radius * cos(theta1), radius * sin(theta1));
        btVector3 p4 = center + btVector3(0, radius * cos(theta2), radius * sin(theta2));
        drawer->drawLine(p3, p4, color);

        // ZX
        btVector3 p5 = center + btVector3(radius * cos(theta1), 0, radius * sin(theta1));
        btVector3 p6 = center + btVector3(radius * cos(theta2), 0, radius * sin(theta2));
        drawer->drawLine(p5, p6, color);
    }
}

inline void drawObstacleCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;

    btCollisionShape* shape = body->getCollisionShape();
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    if (!shape) return;

    if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE)
    {
        drawBox(static_cast<btBoxShape*>(shape), worldTransform, drawer);
    }
    else if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
    {
        btCompoundShape* compound = static_cast<btCompoundShape*>(shape);
        for (int i = 0; i < compound->getNumChildShapes(); ++i)
        {
            btCollisionShape* child = compound->getChildShape(i);
            btTransform childTransform = worldTransform * compound->getChildTransform(i);

            if (child->getShapeType() == BOX_SHAPE_PROXYTYPE)
            {
                drawBox(static_cast<btBoxShape*>(child), childTransform, drawer);
            }
        }
    }
}

inline void drawRamCollider(btRigidBody* body, btIDebugDraw* drawer)
{
    if (!body || !drawer) return;

    btCollisionShape* shape = body->getCollisionShape();
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);

    if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE)
    {
        drawBox(static_cast<btBoxShape*>(shape), transform, drawer);
    }
    else if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
    {
        btCompoundShape* compound = static_cast<btCompoundShape*>(shape);
        for (int i = 0; i < compound->getNumChildShapes(); ++i)
        {
            btCollisionShape* child = compound->getChildShape(i);
            btTransform childTransform = transform * compound->getChildTransform(i);

            if (child->getShapeType() == BOX_SHAPE_PROXYTYPE)
            {
                drawBox(static_cast<btBoxShape*>(child), childTransform, drawer);
            }
        }
    }
}
#endif