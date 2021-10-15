# Zombie Survivor AI
Connect with me on [LinkedIn](https://www.linkedin.com/in/wouterservaes-dae/)! 

View my portfolio [here](https://wouterservaes.myportfolio.com/)!

##### This repo does not include all files to run the program. I do not provide included libraries. The program itself was run by a pre-build executable provided to us to avoid us looking at the zombie, loot and world code.

## Table of contents

## Program overview
Once the program gets executed everything starts. The goal is to have the survivor survive for as long as possible while killing zombies. The Survivor does not know anything about the world, only what is in it's vision or nearby. The survivor has an inventory for food, water, weapons and ammo. After surviving for some time, "purge zones" can spawn in the world. The survivor rapidly loses health when in a purge zone. ![Screenshot of application](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/Images/Zombie_Img03.png) ![Gif of application](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/Images/Zombie_Img01.gif) 
## Ai explanation
I use a behavior tree with blackboard for the AI. AI movement is done using simple steering behaviors. It remembers where it has been, houses it has found and items it didn't pick up yet (saving the item for later).
### Behavior Tree
The behavior tree can be found [here](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/project/BehaviorTrees.cpp). The implementation of each condition and action is [here](https://github.com/WouterServaes/ZombieSurvivorAi/blob/master/project/Behaviors.h).

#### World
The survivor explores the world by either going to an unexplored position near it or by wandering around. It remembers the path it has walked (including some extra distance around the path). When it can't find a valid position to go to (if the area around the survivor is already explored), it simply wanders around.

#### Houses
When the survivor sees a house for the first time, it remembers it and goes inside it. It goes to the middle of the house and looks around for loot. 

#### Loot
When it sees loot, it checks if it needs it and either takes it or leaves it for later. If the survivor needs an item that it doesn't have (eg. extra health when hp is low), it goes to this item if it has seen the item before.

#### Zombies
When the survivor sees and enemy, it checks whether it should attack or run away. If it has enough hp and a gun, it will attack, otherwise it runs away. If the survivor is bitten, it panics and sprints away. Sprinting uses stamina, so it can only sprint for so long.

## Framework
For this project I used the Elite Engine framework, authors of this framework are Matthieu Delaere and Thomas Goussaert. Provided during the course.

## End
Connect with me on [LinkedIn](https://www.linkedin.com/in/wouterservaes-dae/)!  