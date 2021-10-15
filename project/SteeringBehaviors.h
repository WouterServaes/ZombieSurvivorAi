/*=============================================================================*/
// Based on the SteeringBehaviors.h
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#pragma once


#include "Exam_HelperStructs.h"


//////////////////////////
//Bse behavior
//******
class SteeringBehaviors
{
public:
	SteeringBehaviors() = default;
	virtual ~SteeringBehaviors() = default;
	virtual SteeringPlugin_Output CalcSteering(const AgentInfo& agent) = 0;
	void SetTarget(const Elite::Vector2& targetPos) { m_TargetPos = targetPos; };
protected:
	Elite::Vector2 m_TargetPos;
};

//////////////////////////
//Seek
//******
class Seek :public SteeringBehaviors
{
public:
	Seek() = default;
	virtual ~Seek() = default;
	SteeringPlugin_Output CalcSteering(const AgentInfo& agent) override;
	virtual void SetTarget(const Elite::Vector2& targetPos) { m_TargetPos = targetPos; };
};

//////////////////////////
//Flee
//******

class Flee :public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;
	SteeringPlugin_Output CalcSteering(const AgentInfo& agent);
private:
	float m_FleeRadius = 10.f;
};

//////////////////////////
//Wander
//******
class Wander : public SteeringBehaviors
{
public:
	Wander() = default;
	virtual ~Wander() = default;
	SteeringPlugin_Output CalcSteering(const AgentInfo& agent) override;
	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }
protected:
	float m_Offset = 6.f;
	float m_Radius = 4.f;
	float m_AngleChange = Elite::ToRadians(45);
	float m_WanderAngle = Elite::randomFloat(10);
private:
	virtual void SetTarget(const Elite::Vector2& targetPos) {};
};

//////////////////////////
//FACE
//******
class Face :public Seek
{
public:
	Face() = default;
	virtual ~Face() = default;
	SteeringPlugin_Output CalcSteering(float deltaT, const AgentInfo& agent);

};



