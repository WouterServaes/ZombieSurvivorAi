#include "stdafx.h"
#include "SteeringBehaviors.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalcSteering(const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};
	steering.LinearVelocity = m_TargetPos - agent.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;
	return steering;
}

//FLEE 
//******
SteeringPlugin_Output Flee::CalcSteering(const AgentInfo& agent)
{
	auto fleeAngle{ Elite::randomFloat(-180.f, 180.f) };
	auto fleeDirection{agent.LinearVelocity.GetNormalized()};
	auto fleeCircleCentre{agent.Position + fleeDirection * 5.f};
	float radius{2.f};
	m_TargetPos.x = fleeCircleCentre.x + std::cos(fleeAngle) * radius;
	m_TargetPos.y = fleeCircleCentre.y + std::sin(fleeAngle) * radius;
	auto fleeing{ Seek::CalcSteering(agent) };
	return fleeing;
}

//WANDER
//******
SteeringPlugin_Output Wander::CalcSteering(const AgentInfo& agent)
{
	float angleOffset{ Elite::randomFloat(m_AngleChange / 2, -m_AngleChange / 2) };
	m_WanderAngle += angleOffset;
	Elite::Vector2 circlePos{ agent.Position + agent.LinearVelocity.GetNormalized() * m_Offset };
	Elite::Vector2 randomPoint{};
	randomPoint.x = circlePos.x + cos(m_WanderAngle) * m_Radius;
	randomPoint.y = circlePos.y + sin(m_WanderAngle) * m_Radius;

	SteeringPlugin_Output wandering{};
	wandering.LinearVelocity = randomPoint - agent.Position;
	wandering.LinearVelocity.Normalize();
	wandering.LinearVelocity *= agent.MaxLinearSpeed;
	return wandering;
}

//FACE (base > SEEK)
//******
SteeringPlugin_Output Face::CalcSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output facing{};
	if (Elite::Cross(Elite::OrientationToVector(agent.Orientation), (m_TargetPos - agent.Position)) < 0)
		facing.AngularVelocity = -agent.MaxAngularSpeed;
	else
		facing.AngularVelocity = agent.MaxAngularSpeed;
	facing.LinearVelocity = {0,0};
	return facing;
}


