#include "Networks.h"
#include "ReplicationManagerServer.h"


void ReplicationManagerServer::Create(uint32 networkId) 
{
	ReplicationCommand command;
	command.action = ReplicationAction::Create;
	command.networkId = networkId;
	bool is = false;
	for (int i = 0; i < commands.size(); i++) {

		if (commands[i].action == command.action && commands[i].networkId == command.networkId)
		{
			is = true;
		}
			
	}

	if (!is)
	{
		commands.push_back(command);
	}
}

void ReplicationManagerServer::Update(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Update;
	command.networkId = networkId;
	bool is = false;
	
	//for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i)
	for (int i = 0; i < commands.size(); i++)
	{
		if (commands[i].networkId == command.networkId && commands[i].action == command.action)
		{
			is = true;
		}
	}
	if (!is)
	{
		commands.push_back(command);
	}
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Destroy;
	command.networkId = networkId;
	bool is = false;
	//for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i) 
	for (int i = 0; i < commands.size(); i++)
	{
		if (commands[i].networkId == command.networkId && commands[i].action == command.action)
		{
			is = true;
		}
	}
	if (!is)
	{
		commands.push_back(command);
	}
}

void ReplicationManagerServer::write(OutputMemoryStream& packet) 
{
	//for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i)
	for (int i = 0; i < commands.size(); i++)
	{
		packet << commands[i].networkId;
		packet << commands[i].action;

		if (commands[i].action == ReplicationAction::Create || commands[i].action == ReplicationAction::Update)
		{
		
			GameObject* game_object = App->modLinkingContext->getNetworkGameObject(commands[i].networkId);
			if (game_object != nullptr)
			{
				packet << game_object->position.x;
				packet << game_object->position.y;
				packet << game_object->angle;
				packet << game_object->size.x;
				packet << game_object->size.y;
				
				if (game_object->sprite->texture == App->modResources->spacecraft1)
				{
					packet << (uint8)1;
				}
				if (game_object->sprite->texture == App->modResources->spacecraft2)
				{
					packet << (uint8)2;
				}
				if (game_object->sprite->texture == App->modResources->spacecraft3)
				{
					packet << (uint8)3;
				}
				else 
				{
					packet << (uint8)0;
				}
			}
		
		
		}

		commands.clear();
	}
}