#include "Networks.h"
#include "ReplicationManagerServer.h"

void ReplicationManagerServer::Create(uint32 networkId) 
{
	ReplicationCommand command;
	command.networkId = networkId;
	command.action = ReplicationAction::Create;

	commands.push_back(command);
}

void ReplicationManagerServer::Update(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i)
	{
		if ((*i).networkId == networkId)
		{
			(*i).action = ReplicationAction::Update;
			break;
		}
	}
}

void ReplicationManagerServer::Destroy(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i) 
	{
		if (i->networkId == networkId) 
		{
			i->action = ReplicationAction::Destroy;
			break;
		}
	}
}

void ReplicationManagerServer::Write(OutputMemoryStream& packet) 
{
	for (std::vector<ReplicationCommand>::iterator i = commands.begin(); i != commands.end(); ++i)
	{
		packet << i->networkId;
		packet << i->action;

		GameObject* game_object = App->modLinkingContext->getNetworkGameObject(i->networkId);

		switch (i->action) 
		{
		case ReplicationAction::Create:
		{
			packet << game_object->position.x;
			packet << game_object->position.y;
			packet << game_object->angle;
			packet << game_object->size.x;
			packet << game_object->size.y;
			//packet << game_object->sprite->texture;
			packet << game_object->angle;
			//packet << game_object->collider;
			//packet << game_object->behaviour;
			break;
		}
		case ReplicationAction::Update:
		{
			packet << game_object->position.x;
			packet << game_object->position.y;
			packet << game_object->angle;
			break;
		}
		}

		i->action = ReplicationAction::None;
	}
}