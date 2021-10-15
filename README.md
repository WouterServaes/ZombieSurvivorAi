# Zombie Survivor AI
Connect with me on [LinkedIn](https://www.linkedin.com/in/wouterservaes-dae/)! 

View my portfolio [here](https://wouterservaes.myportfolio.com/)!

##### This repo does not include all files to run the program. I do not provide included libraries. The program itself was run by a pre-build executable provided to us to avoid us looking at the zombie, loot and world code.

## Table of contents
- [Program overview](#overview)
- [AI Explanation](#explanation)
    - [Behavior tree](#bt)
    - [World](#world)
    - [Houses](#houses)
    - [Loot](#loot)
    - [Zombies](#zombies)
    - [Purge](#purge)
- [Conclusion](#conclusion)
- [Framework](#framework)
- [End](#end)
## Program overview <a name ="overview"></a>
Once the program gets executed everything starts. The goal is to have the survivor survive for as long as possible while killing zombies. The Survivor does not know anything about the world, only what is in it's vision or nearby. The survivor has an inventory for food, water, weapons and ammo. After surviving for some time, "purge zones" can spawn in the world. The survivor rapidly loses health when in a purge zone. ![Screenshot of application](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/Images/Zombie_Img03.png) ![Gif of application](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/Images/Zombie_Img02.gif) 
## Ai explanation <a name ="explanation"></a>
I use a behavior tree with blackboard for the AI. AI movement is done using simple steering behaviors. It remembers where it has been, houses it has found and items it didn't pick up yet (saving the item for later).
### Behavior Tree <a name ="bt"></a>
The behavior tree can be found [here](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/project/BehaviorTrees.cpp). The implementation of each condition and action is [here](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/project/Behaviors.h).

#### World <a name ="world"></a> [(code)](https://github.com/WouterServaes/ZombieSurvivorAi/blob/117cab0dd50c59b66bff94848abbdd42f5a35de9/project/BehaviorTrees.cpp#L105) 

The survivor explores the world by either going to an unexplored position near it or by wandering around. It remembers the path it has walked (including some extra distance around the path). When it can't find a valid position to go to (if the area around the survivor is already explored), it simply wanders around.

#### Houses <a name ="houses"></a> [(code)](https://github.com/WouterServaes/ZombieSurvivorAi/blob/117cab0dd50c59b66bff94848abbdd42f5a35de9/project/BehaviorTrees.cpp#L74)

When the survivor sees a house for the first time, it remembers it and goes inside it. It goes to the middle of the house and looks around for loot. 

#### Loot <a name ="loot"></a> [(code)](https://github.com/WouterServaes/ZombieSurvivorAi/blob/117cab0dd50c59b66bff94848abbdd42f5a35de9/project/BehaviorTrees.cpp#L47)

When it sees loot, it checks if it needs it and either takes it or leaves it for later. If the survivor needs an item that it doesn't have (eg. extra health when hp is low), it goes to this item if it has seen the item before.

#### Zombies <a name ="zombies"></a> [(code)](https://github.com/WouterServaes/ZombieSurvivorAi/blob/117cab0dd50c59b66bff94848abbdd42f5a35de9/project/BehaviorTrees.cpp#L22)

When the survivor sees and enemy, it checks whether it should attack or run away. If it has enough hp and a gun, it will attack, otherwise it runs away. If the survivor is bitten, it panics and sprints away. Sprinting uses stamina, so it can only sprint for so long.

#### Purge zone <a name ="purge"></a> [(code)](https://github.com/WouterServaes/ZombieSurvivorAi/blob/117cab0dd50c59b66bff94848abbdd42f5a35de9/project/BehaviorTrees.cpp#L42)
When the survivor sees a purge zone, it avoids it. If it is in a purge zone it goes out of it (or dies...).

## Conclusion <a name ="conclusion"></a>
The AI was almost a success, it successfully does everything I want it to do but dies faster than many of the other students' projects (oops). I learned a lot from it and am happy with the result.

![Gif of application](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/Images/Zombie_Img01.gif) 

## Framework <a name ="framework"></a>
For this project I used the Elite Engine framework, authors of this framework are Matthieu Delaere and Thomas Goussaert. Provided during the course.


## End <a name ="end"></a>
Connect with me on [LinkedIn](https://www.linkedin.com/in/wouterservaes-dae/)!  