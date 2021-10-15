#include "stdafx.h"
#include "IExamInterface.h"
#include "Behaviors.h"

//=========Default BT
//Explore world until a certain nr of houses is found. Agent then goes from found house to found house.
//If the agent needs an item, it'll go to this item if the item is in the agent's memory. If it is not in the agent's memory,
//it explores the world until it finds this item (avoiding already found houses and adding newly found houses to it's memory)
//If the agent sees and enemy, run away from this enemy unless the agent's hp is high enough and the agent has a gun, attack if this is the case.
//When the agent sees a purge zone, run away from this purge zone
//world exploration: agent travels the world grid in a straight line, until a found has been found. If this is a new house, the agent will explore this house.
//if this house was already found the agent will change its explore direction, same when the agent hits the world edge.
//=================
Elite::BehaviorTree* DefaultBT(Elite::Blackboard* pBlackboard)
{
	using namespace Elite;

	return new BehaviorTree(
		pBlackboard,
		new BehaviorSelector
		({
			new BehaviorSelector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(WasBitten), //if bitten => run mode = on
					new BehaviorAction(Run)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(SeeEnemy), //see enemy? have enough hp and a gun? => attack
					new BehaviorConditional(CanTargetEnemy),
					new BehaviorAction(AttackEnemy)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(SeeEnemy), //see enemy? not enough hp or no gun? => run away
					new BehaviorConditional(ShouldRunFromEnemies),
					new BehaviorAction(RunFromEnemies)
				})
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(SeePurgeZone), //see purge zone? avoid it
				new BehaviorAction(RunFromPurgeZone)
			}),
			new BehaviorSelector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(NeedItem), //need any item in particular? It's in grab range? => grab it
					new BehaviorConditional(NeededItemInGrabRange),
					new BehaviorAction(GrabNeededItem)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(PickUpAnyItem), //enough inventory slots free to pick up an item and have a spot +for an emergency needed item? in grab range? => grab it
					new BehaviorConditional(AnyItemInGrabRange),
					new BehaviorAction(GrabNotNeededItem)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(ItemInFov), //any item in fov? => go to item to save in memory
					new BehaviorConditional(ShouldGoToItemInFov),
					new BehaviorAction(GoToItemInFov)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(NeedItem), //need any item in particular? it's in the agent's memory? go to the item
					new BehaviorConditional(NeededItemInMemory),
					new BehaviorAction(GoToNeededItem)
				})
			}),
			new BehaviorSelector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(AgentExploringHouse), //in a house, exploring? => go to center of the house and stay there while exploring
					new BehaviorAction(GoToHouse)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(CanLeaveHouse), //done with exploring? => leave the house
					new BehaviorAction(LeaveHouse)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(NewHouseInSight), //see a new house? go to it
					new BehaviorAction(GoToHouse)
					}),
				new BehaviorSequence
				({
					new BehaviorConditional(NoNeededItemInMemory), //no needed items in memory? min amount of houses have been found? => needed item is not in one of the houses => explore the world
					new BehaviorConditional(MinAmountHousesFound),
					new BehaviorAction(Explore)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(HaveItemsInMemory), //have items in memory and min amount of houses have been found? go to next house
					new BehaviorConditional(MinAmountHousesFound),
					new BehaviorAction(GoToHouse)
				})
				
			}),
			new BehaviorSequence
			({
				new BehaviorAction(Explore) //explore the world
			})
		})
	);
}
