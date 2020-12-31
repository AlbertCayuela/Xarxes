#include "Networks.h"
#include "ReplicationManagerServer.h"


void ReplicationManagerServer::Create(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Create;
	command.networkId = networkId;
	bool is = false;
	for(std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); i++)
	{
		if ((*i).action == command.action && (*i).networkId == command.networkId) 
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

	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); i++)
	{
		if ((*i).action == command.action && (*i).networkId == command.networkId) 
		{
			is = true;
		}
	}
	if (!is) 
	{
		commands.push_back(command);
	}
}

void ReplicationManagerServer::Destroy(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Destroy;
	command.networkId = networkId;
	bool is = false;

	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); i++)
	{
		if ((*i).action == command.action && (*i).networkId == command.networkId) 
		{
			is = true;
		}
	}
	if (!is)
	{
		commands.push_back(command);
	}
}

void ReplicationManagerServer::Write(OutputMemoryStream& packet)
{

	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); i++)
	{

		ReplicationCommand command = (*i);
		packet << command.networkId;
		packet << command.action;

		if (command.action == ReplicationAction::Create || command.action == ReplicationAction::Update) {


			GameObject* game_object = App->modLinkingContext->getNetworkGameObject(command.networkId);

			if (game_object)
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
				else if (game_object->sprite->texture == App->modResources->spacecraft2)
				{
					packet << (uint8)2;
				}		
				else if (game_object->sprite->texture == App->modResources->spacecraft3)
				{
					packet << (uint8)3;
				}
				else
				{
					packet << (uint8)0;
				}
			}
		}
	}

	commands.clear();

}
